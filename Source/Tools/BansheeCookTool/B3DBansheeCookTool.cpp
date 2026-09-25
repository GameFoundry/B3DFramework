//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DApplication.h"
#include "B3DShaderCooker.h"
#include "B3DShaderCookerSource.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderRegistry.h"
#include "Plugin/B3DPluginLoader.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Resources/B3DBuiltinResources.h"
#include "Resources/B3DPackage.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "FileSystem/B3DFileSystem.h"
#include "Utility/B3DCommandLine.h"
#include "String/B3DString.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

namespace
{
	/** Returns true if the requested cook language id has a registered GPU bytecode compiler. */
	bool IsLanguageSupported(const String& language)
	{
		// The null language produces no bytecode by design (the null GPU device accepts empty programs), so it needs no shader backend.
		if(language == kGpuProgramLanguageNullsl)
			return true;

		if(ShaderCompilers::Instance().GetBytecodeCompiler(language) != nullptr)
			return true;

		B3D_LOG(Error, LogGeneric, "Cannot cook shading language id \"{0}\": no shader backend is registered for it on this host.", language);
		return false;
	}

	/**
	 * Returns the latest last-modified time of any file in @p folder (recursively). Used for the skip-up-to-date check so
	 * a change to any shader or shader include forces a re-cook.
	 */
	std::time_t GetNewestModifiedTime(const Path& folder)
	{
		std::time_t newest = 0;
		FileSystem::Iterate(folder, [&newest](const Path& file)
		{
			const std::time_t modifiedTime = FileSystem::GetLastModifiedTime(file);
			if(modifiedTime > newest)
				newest = modifiedTime;

			return true;
		}, nullptr, true);

		return newest;
	}

	/** Returns a comma-separated listing of @p folders, for log output. */
	String MakeFolderListing(const Vector<Path>& folders)
	{
		StringStream stream;
		for(u32 folderIndex = 0; folderIndex < (u32)folders.size(); ++folderIndex)
		{
			if(folderIndex > 0)
				stream << ", ";

			stream << "\"" << folders[folderIndex].ToString() << "\"";
		}

		return stream.str();
	}

	/**
	 * Every registered renderer material must have its shader source in one of the cooked folders - a missing one would
	 * silently miss the prebuilt store at runtime, so surface it loudly.
	 */
	void WarnAboutUncookedRendererMaterials(const Vector<ShaderCookItem>& items, const Vector<Path>& inputFolders)
	{
		Set<String> cookedRendererMaterialNames;
		for(const ShaderCookItem& item : items)
		{
			if(item.CachePrefix == render::RendererMaterialBase::kRendererMaterialShaderCachePrefix)
				cookedRendererMaterialNames.insert(item.Name);
		}

		Vector<RendererMaterialManager::RendererMaterialShaderInfo> rendererMaterialShaders;
		RendererMaterialManager::GetRegisteredMaterialShaders(rendererMaterialShaders);

		Set<String> reportedNames;
		for(const RendererMaterialManager::RendererMaterialShaderInfo& info : rendererMaterialShaders)
		{
			const String name = info.ShaderPath.GetFilename(false);
			if(cookedRendererMaterialNames.find(name) != cookedRendererMaterialNames.end())
				continue;

			if(reportedNames.insert(name).second)
				B3D_LOG(Warning, LogResources, "Renderer-material shader \"{0}\" is registered but no matching \"{0}.bsl\" was found in any input folder ({1}); it will not be cooked.", name, MakeFolderListing(inputFolders));
		}
	}

	/**
	 * Runs the shader cook: all *.bsl shaders from @p inputFolders into a single store package. @p modules name the
	 * libraries whose renderer materials must be registered before the shaders are classified. Returns the process exit
	 * code.
	 */
	int RunShaderCook(const Vector<Path>& inputFolders, const Vector<String>& modules, const Path& outputPath, const String& language, bool force)
	{
		if(!IsLanguageSupported(language))
			return 2;

		if(modules.empty())
			B3D_LOG(Warning, LogGeneric, "No shader cook modules were passed through -modules. Only renderer materials declared in bsf are registered, so shaders of renderer materials declared elsewhere are cooked without their defines.");

		// Loading a module runs its renderer-material registrations. None of its entry points are called, so nothing is activated.
		for(const String& module : modules)
		{
			if(!PluginLoader::LoadWithoutEntryPoint(module))
			{
				B3D_LOG(Error, LogGeneric, "Cannot load shader cook module \"{0}\". If the cook runs through B3D_HOST_TOOLS_DIR, build the module in the host tree.", module);
				return 1;
			}
		}

		// If nothing registered, every renderer-material shader would be mis-keyed, so abort loudly rather than cook silently wrong.
		Vector<RendererMaterialManager::RendererMaterialShaderInfo> rendererMaterialShaders;
		RendererMaterialManager::GetRegisteredMaterialShaders(rendererMaterialShaders);
		if(rendererMaterialShaders.empty())
		{
			B3D_LOG(Error, LogGeneric, "No renderer materials are registered. The cook cannot classify renderer-material shaders; pass the libraries that declare them through -modules (see B3DRegisterShaderCookModule). Aborting.");
			return 1;
		}

		B3D_LOG(Info, LogGeneric, "{0} renderer-material shader registration(s) found.", (u32)rendererMaterialShaders.size());

		// Skip only when the store uses the current cache format and is newer than every input file (unless forced).
		if(!force && FileSystem::Exists(outputPath))
		{
			const std::time_t outputTime = FileSystem::GetLastModifiedTime(outputPath);

			std::time_t newestInputTime = 0;
			for(const Path& inputFolder : inputFolders)
				newestInputTime = std::max(newestInputTime, GetNewestModifiedTime(inputFolder));

			if(outputTime >= newestInputTime)
			{
				// Inspect the package index without deserializing potentially incompatible shader data.
				const TShared<Package> package = Package::Load(outputPath);
				if(package != nullptr && ShaderRegistry::DoesPrebuiltStoreVersionMatch(*package))
				{
					B3D_LOG(Info, LogGeneric, "Prebuilt shader store \"{0}\" is up to date. Skipping (use -force to re-cook).", outputPath.ToString());
					return 0;
				}

				B3D_LOG(Info, LogGeneric, "Prebuilt shader store \"{0}\" has missing or incompatible version metadata. Rebuilding for version {1}.", outputPath.ToString(), ShaderRegistry::kCacheVersion);
			}
		}

		Vector<ShaderCookItem> items;
		for(const Path& inputFolder : inputFolders)
		{
			ShaderCookerSource source(inputFolder);
			source.GetItems(items);
		}

		WarnAboutUncookedRendererMaterials(items, inputFolders);

		B3D_LOG(Info, LogGeneric, "Cooking {0} shader(s) from {1} for language \"{2}\".", (u32)items.size(), MakeFolderListing(inputFolders), language);

		ShaderCooker::CookOptions cookOptions;
		cookOptions.Language = language;
		cookOptions.OutputPath = outputPath;

		return ShaderCooker::Cook(items, cookOptions) ? 0 : 1;
	}
}

int main(int argc, char* argv[])
{
	CommandLine::Initialize(argc, argv);

	// CLI: -input takes one or more shader source folders, separated by ';' (defaults to the engine's builtin shader
	// folder). -modules takes the names of libraries that declare renderer materials, separated by ';' (for example
	// "-modules bsfRenderBeast;FrameworkTests"). -output names the store package to write and -language the single
	// low-level shading language to cook for (for example "-language vksl"). -force re-cooks even if up to date.
	const String inputParameter = CommandLine::GetParameterValue("input");
	const String modulesParameter = StringUtility::Trim(CommandLine::GetParameterValue("modules"));
	const String outputParameter = CommandLine::GetParameterValue("output");

	String language = StringUtility::Trim(CommandLine::GetParameterValue("language"));
	if(language.empty())
		language = kGpuProgramLanguageVksl;

	const bool force = CommandLine::HasParameter("force");

	ApplicationCreateInformation createInformation = Application::BuildCreateInformation(VideoMode(64, 64), "Banshee Cook Tool", false);
#if !B3D_BUILD_IMPORTERS
	// The cook itself always needs the BSL compiler; BuildCreateInformation only lists it in importer-enabled builds.
	createInformation.Importers.push_back("bsfSL");
#endif
	createInformation.GpuBackend = "bsfNullGpuBackend";
	createInformation.Renderer = "bsfNullRenderer";
	createInformation.PrimaryWindow.Headless = true;
	createInformation.CrashHandling.SuppressErrorPopup = true;

	Application::StartUp(createInformation);

	Vector<Path> inputFolders;
	for(const String& folder : StringUtility::Split(inputParameter, ";"))
	{
		const String trimmed = StringUtility::Trim(folder);
		if(!trimmed.empty())
			inputFolders.push_back(Path(trimmed));
	}

	if(inputFolders.empty())
		inputFolders.push_back(BuiltinResources::GetShaderFolder());

	Vector<String> modules;
	for(const String& module : StringUtility::Split(modulesParameter, ";"))
	{
		const String trimmed = StringUtility::Trim(module);
		if(!trimmed.empty())
			modules.push_back(trimmed);
	}

	const Path outputPath = outputParameter.empty() ? ShaderRegistry::GetPrebuiltStorePath() : Path(outputParameter);

	B3D_LOG(Info, LogGeneric, "Banshee Cook Tool started. Input {0}, modules \"{1}\", output \"{2}\", language \"{3}\", force {4}.",
		MakeFolderListing(inputFolders), modulesParameter, outputPath.ToString(), language, force ? "yes" : "no");

	int exitCode = RunShaderCook(inputFolders, modules, outputPath, language, force);

	Application::ShutDown();
	return exitCode;
}
