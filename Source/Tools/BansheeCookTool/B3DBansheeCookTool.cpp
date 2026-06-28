//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DApplication.h"
#include "B3DShaderCooker.h"
#include "B3DBuiltinShaderCookerSource.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderRegistry.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Resources/B3DBuiltinResources.h"
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

	/** Runs the builtin shader cook. Returns the process exit code. */
	int RunShaderCook(const Path& inputFolder, const Path& outputPath, const String& language, bool force)
	{
		if(!IsLanguageSupported(language))
			return 2;

		// Load renderer so we can query its renderer materials, but don't activate the renderer.
		GetApplication().LoadPlugin("bsfRenderBeast");

		// If nothing registered, every renderer-material shader would be mis-keyed, so abort loudly rather than cook silently wrong.
		Vector<RendererMaterialManager::RendererMaterialShaderInfo> rendererMaterialShaders;
		RendererMaterialManager::GetRegisteredMaterialShaders(rendererMaterialShaders);
		if(rendererMaterialShaders.empty())
		{
			B3D_LOG(Error, LogGeneric, "No renderer materials are registered. The cook cannot classify renderer-material shaders; ensure bsfRenderBeast is available. Aborting.");
			return 1;
		}

		B3D_LOG(Info, LogGeneric, "{0} renderer-material shader registration(s) found.", (u32)rendererMaterialShaders.size());

		// Skip-up-to-date: if the output package is newer than every input file, there is nothing to do (unless forced).
		if(!force && FileSystem::Exists(outputPath))
		{
			const std::time_t outputTime = FileSystem::GetLastModifiedTime(outputPath);
			if(outputTime >= GetNewestModifiedTime(inputFolder))
			{
				B3D_LOG(Info, LogGeneric, "Prebuilt shader store \"{0}\" is up to date. Skipping (use -force to re-cook).", outputPath.ToString());
				return 0;
			}
		}

		BuiltinShaderCookerSource source(inputFolder);

		Vector<ShaderCookItem> items;
		source.GetItems(items);

		B3D_LOG(Info, LogGeneric, "Cooking {0} shader(s) from \"{1}\" for language \"{2}\".", (u32)items.size(), inputFolder.ToString(), language);

		ShaderCooker::CookOptions cookOptions;
		cookOptions.Language = language;
		cookOptions.OutputPath = outputPath;

		return ShaderCooker::Cook(items, cookOptions) ? 0 : 1;
	}
}

int main(int argc, char* argv[])
{
	CommandLine::Initialize(argc, argv);

	// CLI: -input/-output folders and the single low-level shading language to cook for (-language, for example
	// "-language vksl"). -force re-cooks even if up to date.
	const String inputParameter = CommandLine::GetParameterValue("input");
	const String outputParameter = CommandLine::GetParameterValue("output");

	String language = StringUtility::Trim(CommandLine::GetParameterValue("language"));
	if(language.empty())
		language = kGpuProgramLanguageVksl;

	const bool force = CommandLine::HasParameter("force");

	ApplicationCreateInformation createInformation = Application::BuildCreateInformation(VideoMode(64, 64), "Banshee Cook Tool", false);
	createInformation.GpuBackend = "bsfNullGpuBackend";
	createInformation.Renderer = "bsfNullRenderer";
	createInformation.PrimaryWindow.Headless = true;
	createInformation.CrashHandling.SuppressErrorPopup = true;

	Application::StartUp(createInformation);

	const Path inputFolder = inputParameter.empty() ? BuiltinResources::GetShaderFolder() : Path(inputParameter);
	const Path outputPath = outputParameter.empty() ? ShaderRegistry::GetPrebuiltStorePath() : Path(outputParameter);

	B3D_LOG(Info, LogGeneric, "Banshee Cook Tool started. Input \"{0}\", output \"{1}\", language \"{2}\", force {3}.",
		inputFolder.ToString(), outputPath.ToString(), language, force ? "yes" : "no");

	int exitCode = RunShaderCook(inputFolder, outputPath, language, force);

	Application::ShutDown();
	return exitCode;
}
