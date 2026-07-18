//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderRegistry.h"

#include "B3DApplication.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderVariation.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "Resources/B3DPackage.h"
#include "Resources/B3DResource.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DResourceRTTI.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Utility/B3DPaths.h"
#include "Utility/B3DPersistentCache.h"
#include "Utility/B3DUUID.h"

using namespace b3d;

namespace b3d
{
	/** @cond RTTI */
	class PrebuiltShaderRTTI final : public TRTTIType<PrebuiltShader, Resource, PrebuiltShaderRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mObjects, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PrebuiltShader";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PrebuiltShader;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return PrebuiltShader::Create(nullptr);
		}
	};
	/** @endcond */

	TShared<PrebuiltShader> PrebuiltShader::Create(const TShared<IReflectable>& object)
	{
		TShared<PrebuiltShader> prebuiltShader = B3DMakeSharedFromExisting<PrebuiltShader>(new (B3DAllocate<PrebuiltShader>()) PrebuiltShader(object));
		prebuiltShader->SetId(UUIDGenerator::GenerateRandom());
		prebuiltShader->SetShared(prebuiltShader);
		prebuiltShader->Initialize();

		return prebuiltShader;
	}

	RTTIType* PrebuiltShader::GetRttiStatic()
	{
		return PrebuiltShaderRTTI::Instance();
	}

	RTTIType* PrebuiltShader::GetRtti() const
	{
		return PrebuiltShader::GetRttiStatic();
	}
} // namespace b3d

namespace
{
	/** Folder, relative to the data folder, in which the offline shader cook tool writes prebuilt shaders. */
	constexpr const char* kPrebuiltShaderFolderName = "CompiledShaders/";

	/** Name of the package (without extension) holding the prebuilt shaders. */
	constexpr const char* kPrebuiltShaderPackageName = "Shaders";

	/**
	 * Returns true if the shader's stored compiler version and source/include hashes still match the current compiler
	 * and the source/include files currently on disk - i.e. the compiled shader is up to date. A shader with no compiler
	 * meta-data cannot be validated and is treated as up to date, as is one when no compiler is available (in that case
	 * recompilation is impossible and the cached data is the best we have).
	 */
	bool IsShaderUpToDate(const TShared<ShaderCompilerMetaData>& compilerMetaData, const Array<u64, 2>& sourceHash, const IShaderCompiler* currentCompiler)
	{
		if(compilerMetaData == nullptr)
			return true;

		if(currentCompiler != nullptr && compilerMetaData->CompilerVersion != currentCompiler->GetVersion())
			return false;

		if(compilerMetaData->ShaderHash != sourceHash)
			return false;

		for(const auto& includeHash : compilerMetaData->IncludeHashes)
		{
			if(Shader::ComputeIncludeHash(includeHash.first) != includeHash.second)
				return false;
		}

		return true;
	}
}

ShaderRegistry::ShaderRegistry() = default;
ShaderRegistry::~ShaderRegistry() = default;

Path ShaderRegistry::GetPrebuiltStorePath()
{
	const String packageFileName = String(kPrebuiltShaderPackageName) + Package::kPackageExtension;
	return Paths::GetDataPath() + kPrebuiltShaderFolderName + packageFileName;
}

void ShaderRegistry::OnStartUp()
{
	const Path prebuiltStorePath = GetPrebuiltStorePath();

	if(FileSystem::Exists(prebuiltStorePath))
	{
		mPrebuiltStore = Package::Load(prebuiltStorePath);
		if(mPrebuiltStore == nullptr)
			B3D_LOG(Warning, LogResources, "Found a prebuilt shader store at \"{0}\" but failed to load it. Shaders will be compiled on demand instead.", prebuiltStorePath);
	}
}

void ShaderRegistry::OnShutDown()
{
	mPrebuiltStore = nullptr;
}

void ShaderRegistry::RegisterSearchPath(const Path& folder)
{
	Lock lock(mSearchPathMutex);
	mSearchPaths.push_back(folder);
}

String ShaderRegistry::GetShaderCacheName(const String& cachePrefix, const String& shaderName)
{
	return cachePrefix + shaderName + "/";
}

Path ShaderRegistry::GetShaderMetaDataPath(const String& shaderCacheName, const String& language)
{
	return Path(shaderCacheName) + language + "/MetaData";
}

Path ShaderRegistry::GetVariationPath(const ShaderCompilerMetaData& metadata, const String& language, const String& variationName)
{
	StringStream hashStringStream;
	hashStringStream << metadata.CompilerVersion << "\n";
	hashStringStream << variationName << "\n";
	hashStringStream << StringUtility::HexToLiteral(metadata.ShaderHash.data(), (u32)metadata.ShaderHash.size()) << "\n";

	for(const auto& includeHashPair : metadata.IncludeHashes)
		hashStringStream << includeHashPair.first << " = " << StringUtility::HexToLiteral(includeHashPair.second.data(), (u32)includeHashPair.second.size()) << "\n";

	const Array<u64, 2> variationHash = Shader::ComputeHash(hashStringStream.str());
	return Path(metadata.NameInCache) + language + StringUtility::HexToLiteral(variationHash.data(), (u32)variationHash.size());
}

template <bool IsRenderProxy>
TShared<CoreVariantType<Shader, IsRenderProxy>> ShaderRegistry::GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines)
{
	using ShaderType = CoreVariantType<Shader, IsRenderProxy>;

	const String shadingLanguageName = ShaderCompilers::Instance().DetectActiveShadingLanguage();
	const String& shaderName = shaderPath.GetFilename(false);
	const String shaderNameInCache = GetShaderCacheName(cachePrefix, shaderName);
	const Path shaderPathInCache = GetShaderMetaDataPath(shaderNameInCache, shadingLanguageName);

	Vector<String> activeLanguages;
	if(!shadingLanguageName.empty())
		activeLanguages.push_back(shadingLanguageName);

	TShared<PrecompiledShaderData> precompiledData;

	// 1) Prebuilt shader store. Keyed by a source-independent virtual path so it can be resolved without the shader's
	//    high-level source being present. Each cooked shader is wrapped in a PrebuiltShader.
	if(mPrebuiltStore != nullptr && mPrebuiltStore->Contains(shaderPathInCache))
	{
		const TShared<PrebuiltShader> prebuilt = B3DRTTICast<PrebuiltShader>(mPrebuiltStore->LoadResource(shaderPathInCache));
		const TShared<IReflectable> prebuiltObject = prebuilt != nullptr ? prebuilt->GetObject() : nullptr;
		if(prebuiltObject != nullptr)
		{
			precompiledData = B3DRTTICast<PrecompiledShaderData>(prebuiltObject);
			if(precompiledData == nullptr)
				B3D_LOG(Warning, LogResources, "Prebuilt shader store contains an entry for \"{0}\" but it is not of the expected type. Falling back to compilation.", shaderPath);
		}
	}

#if !B3D_BUILD_TYPE_DEVELOPMENT
	// Outside development builds we trust the prebuilt store and never touch the source.
	if(precompiledData != nullptr)
		return ShaderType::Create(*precompiledData, activeLanguages);
#endif

	// Locate the shader source. In development builds it is used to verify the prebuilt shader is up to date; on a
	// prebuilt miss it is required for cache validation and compilation.
	TShared<DataStream> shaderFileStream;
	if(shaderPath.IsAbsolute())
		shaderFileStream = FileSystem::OpenFile(shaderPath);
	else
	{
		Lock lock(mSearchPathMutex);
		for(const auto& searchPath : mSearchPaths)
		{
			const Path absolutePath = shaderPath.GetAbsolute(searchPath);
			if(!FileSystem::Exists(absolutePath))
				continue;

			shaderFileStream = FileSystem::OpenFile(absolutePath);
			if(shaderFileStream != nullptr)
				break;
		}
	}

	if(shaderFileStream == nullptr)
	{
		// No source available: keep the (unverified) precompiled data if we have it, otherwise this is a hard failure.
		if(precompiledData == nullptr)
		{
			B3D_LOG(Error, LogResources, "Shader resolution failed for shader \"{0}\". No prebuilt shader was found and the shader source cannot be located.", shaderPath);
			return nullptr;
		}

		return ShaderType::Create(*precompiledData, activeLanguages);
	}

	const String shaderSource = shaderFileStream->GetAsString();
	const Array<u64, 2> shaderHash = Shader::ComputeHash(shaderSource);
	const TShared<IShaderCompiler>& bslCompiler = ShaderCompilers::Instance().GetCompiler("bsl");

#if B3D_BUILD_TYPE_DEVELOPMENT
	// In development builds, discard the precompiled data if it no longer matches the source on disk or the current
	// compiler version so the cache/compilation paths below pick up local shader and compiler changes.
	if(precompiledData != nullptr && !IsShaderUpToDate(precompiledData->CompilerMetaData, shaderHash, bslCompiler.get()))
	{
		B3D_LOG(Info, LogResources, "Prebuilt shader \"{0}\" is out of date with its source. Recompiling from source.", shaderPath);
		precompiledData = nullptr;
	}
#endif

	// 2) Writable application cache.
	PersistentCache& cache = GetApplication().GetApplicationCache();
	if(precompiledData == nullptr)
	{
		precompiledData = cache.TryGetEntry<PrecompiledShaderData>(shaderPathInCache);
		if(precompiledData != nullptr && !IsShaderUpToDate(precompiledData->CompilerMetaData, shaderHash, bslCompiler.get()))
			precompiledData = nullptr;
	}

	// Resolved valid cached data from the prebuilt store or the application cache: reconstruct the requested variant.
	if(precompiledData != nullptr)
		return ShaderType::Create(*precompiledData, activeLanguages);

	// 3) On-demand compilation from source, delegated to ShaderCompilers.
	TShared<ShaderType> shader = ShaderCompilers::Instance().CompileShader<IsRenderProxy>(shaderName, shaderSource, defines, activeLanguages);
	if(shader != nullptr)
	{
		const TShared<ShaderCompilerMetaData> compilerMetaData = shader->GetCompilerMetaData();
		if(B3D_ENSURE(compilerMetaData != nullptr))
		{
			compilerMetaData->NameInCache = shaderNameInCache;
			compilerMetaData->ShaderHash = shaderHash;

			cache.SetEntry(shaderPathInCache, shader->GetPrecompiledData());
		}
	}

	return shader;
}

template <bool IsRenderProxy>
bool ShaderRegistry::GetOrCompileVariation(const TShared<CoreVariantType<Shader, IsRenderProxy>>& shader, const TShared<CoreVariantType<Variation, IsRenderProxy>>& variation, const String& language)
{
	using PassType = CoreVariantType<Pass, IsRenderProxy>;

	if(!B3D_ENSURE(shader != nullptr))
	{
		B3D_LOG(Error, LogMaterial, "Cannot compile shader variation. Parent shader is not assigned.");
		return false;
	}

	if(!B3D_ENSURE(variation != nullptr))
	{
		B3D_LOG(Error, LogMaterial, "Cannot compile shader variation. Variation is not assigned.");
		return false;
	}

	const TShared<ShaderCompilerMetaData>& shaderCompilerMetaData = shader->GetCompilerMetaData();
	if(!B3D_ENSURE(shaderCompilerMetaData != nullptr))
		return false;

	const ShaderVariationParameters& variationParameters = variation->GetVariationParameters();
	const String& variationName = variationParameters.CreateVariationName();
	const Path variationPath = GetVariationPath(*shaderCompilerMetaData, language, variationName);

	// 1) Prebuilt shader store. Unlike the shader-level lookup, the variation key is source-sensitive (it folds in the
	//    shader and include hashes), so a store entry is found only when it was cooked against the current source.
	if(mPrebuiltStore != nullptr && mPrebuiltStore->Contains(variationPath))
	{
		const TShared<PrebuiltShader> prebuilt = B3DRTTICast<PrebuiltShader>(mPrebuiltStore->LoadResource(variationPath));
		const TShared<IReflectable> prebuiltObject = prebuilt != nullptr ? prebuilt->GetObject() : nullptr;
		if(prebuiltObject != nullptr)
		{
			const TShared<PrecompiledVariationData> prebuiltData = B3DRTTICast<PrecompiledVariationData>(prebuiltObject);
			if(prebuiltData != nullptr)
			{
				TInlineArray<TShared<PassType>, 1> prebuiltPasses;
				for(const auto& passInformation : prebuiltData->Passes)
					prebuiltPasses.Add(PassType::Create(PassCreateInformation(passInformation)));

				variation->SetCompiledPassData(std::move(prebuiltPasses));
				return true;
			}

			B3D_LOG(Warning, LogMaterial, "Prebuilt shader store contains a variation entry for \"{0}\" but it is not of the expected type. Falling back to compilation.", shader->GetShaderName());
		}
	}

	// 2) Writable application cache, keyed by the same source-sensitive path so local shader edits invalidate stale entries.
	PersistentCache& cache = GetApplication().GetApplicationCache();

	const TShared<PrecompiledVariationData> cachedData = cache.TryGetEntry<PrecompiledVariationData>(variationPath);
	if(cachedData != nullptr)
	{
		TInlineArray<TShared<PassType>, 1> cachedPasses;
		for(const auto& passInformation : cachedData->Passes)
			cachedPasses.Add(PassType::Create(PassCreateInformation(passInformation)));

		variation->SetCompiledPassData(std::move(cachedPasses));
		return true;
	}

	TShared<IShaderCompiler> shaderCompiler = ShaderCompilers::Instance().GetCompiler("bsl");
	if(shaderCompiler == nullptr)
	{
		B3D_LOG(Error, LogMaterial, "Cannot compile variation. BSL shader compiler is not available.");
		return false;
	}

	const ShaderCompilerResult compileResult = shaderCompiler->CompileVariation(*shader, variationParameters, language, *variation);

	if(!compileResult.ErrorMessage.empty())
	{
		B3D_LOG(Error, LogRenderer, "Compilation error when compiling a variation for shader \"{0}\":\n{1}. Location: {2} ({3})", shader->GetShaderName(), compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
		return false;
	}

	cache.SetEntry(variationPath, variation->GetPrecompiledData());
	return true;
}

template B3D_EXPORT TShared<CoreVariantType<Shader, false>> ShaderRegistry::GetOrCompileShader<false>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_EXPORT TShared<CoreVariantType<Shader, true>> ShaderRegistry::GetOrCompileShader<true>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_EXPORT bool ShaderRegistry::GetOrCompileVariation<false>(const TShared<CoreVariantType<Shader, false>>& shader, const TShared<CoreVariantType<Variation, false>>& variation, const String& language);
template B3D_EXPORT bool ShaderRegistry::GetOrCompileVariation<true>(const TShared<CoreVariantType<Shader, true>>& shader, const TShared<CoreVariantType<Variation, true>>& variation, const String& language);
