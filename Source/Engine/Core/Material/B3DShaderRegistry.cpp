//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderRegistry.h"

#include "B3DApplication.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderVariation.h"
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
	/**
	 * Resource wrapper that holds a single cooked shader (main or render-thread variant) as a generic reflectable, so it
	 * can be stored within a Package.
	 */
	class PrebuiltShader final : public Resource
	{
	public:
		/** Returns the wrapped shader object, or null if none is present. */
		TShared<IReflectable> GetObject() const { return mObjects.Empty() ? nullptr : mObjects[0]; }

		/** Creates a wrapper holding the provided shader object. */
		static TShared<PrebuiltShader> Create(const TShared<IReflectable>& object);

	private:
		explicit PrebuiltShader(const TShared<IReflectable>& object)
			: Resource(false), mObjects({ object })
		{ }

		TInlineArray<TShared<IReflectable>, 1> mObjects;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PrebuiltShaderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

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
	 * Returns true if the shader's stored source/include hashes still match the freshly computed source hash and the
	 * include files currently on disk - i.e. the compiled shader is up to date with its source. A shader with no compiler
	 * meta-data cannot be validated and is treated as up to date.
	 */
	bool IsShaderUpToDate(const TShared<ShaderCompilerMetaData>& compilerMetaData, const Array<u64, 2>& sourceHash)
	{
		if(compilerMetaData == nullptr)
			return true;

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

void ShaderRegistry::OnStartUp()
{
	const String packageFileName = String(kPrebuiltShaderPackageName) + Package::kPackageExtension;
	const Path prebuiltStorePath = Paths::GetDataPath() + kPrebuiltShaderFolderName + packageFileName;

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

template <bool IsRenderProxy>
TShared<CoreVariantType<Shader, IsRenderProxy>> ShaderRegistry::GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines)
{
	using ShaderType = CoreVariantType<Shader, IsRenderProxy>;

	const String shadingLanguageName = ShaderCompilers::Instance().DetectActiveShadingLanguage();
	const String& shaderName = shaderPath.GetFilename(false);
	const String shaderNameInCache = cachePrefix + shaderName + "/";
	const Path shaderPathInCache = Path(shaderNameInCache) + shadingLanguageName + "/MetaData";

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

#if B3D_BUILD_TYPE_DEVELOPMENT
	// In development builds, discard the precompiled data if it no longer matches the source on disk so the cache/
	// compilation paths below pick up local shader edits.
	if(precompiledData != nullptr && !IsShaderUpToDate(precompiledData->CompilerMetaData, shaderHash))
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
		if(precompiledData != nullptr && !IsShaderUpToDate(precompiledData->CompilerMetaData, shaderHash))
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
	using VariationType = CoreVariantType<Variation, IsRenderProxy>;

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

	const ShaderVariationParameters& variationParameters = variation->GetVariationParameters();
	const String& variationName = variationParameters.CreateVariationName();

	StringStream hashStringStream;
	hashStringStream << variationName << "\n";

	const TShared<ShaderCompilerMetaData>& shaderCompilerMetaData = shader->GetCompilerMetaData();
	if(shaderCompilerMetaData != nullptr)
	{
		hashStringStream << StringUtility::HexToLiteral(shaderCompilerMetaData->ShaderHash.data(), (u32)shaderCompilerMetaData->ShaderHash.size()) << "\n";

		for(const auto& includeHashPair : shaderCompilerMetaData->IncludeHashes)
			hashStringStream << includeHashPair.first << " = " << StringUtility::HexToLiteral(includeHashPair.second.data(), (u32)includeHashPair.second.size()) << "\n";
	}

	const String& hashString = hashStringStream.str();
	const Array<u64, 2> variationHash = Shader::ComputeHash(hashString);

	Path cacheName;
	PersistentCache& cache = GetApplication().GetApplicationCache();

	if(shaderCompilerMetaData != nullptr && !shaderCompilerMetaData->NameInCache.empty())
	{
		cacheName = Path(shaderCompilerMetaData->NameInCache)+ language + StringUtility::HexToLiteral(variationHash.data(), (u32)variationHash.size());

		const TShared<VariationType> cachedVariation = cache.TryGetEntry<VariationType>(cacheName);
		if(cachedVariation != nullptr)
		{
			TInlineArray<TShared<PassType>, 1> cachedPasses;
			const u32 passCount = cachedVariation->GetPassCount();
			for(u32 passIndex = 0; passIndex < passCount; passIndex++)
				cachedPasses.Add(cachedVariation->GetPass(passIndex));

			variation->SetCompiledPassData(cachedPasses);
			return true;
		}
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

	if(!cacheName.IsEmpty())
		cache.SetEntry(cacheName, variation);

	return true;
}

template B3D_EXPORT TShared<CoreVariantType<Shader, false>> ShaderRegistry::GetOrCompileShader<false>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_EXPORT TShared<CoreVariantType<Shader, true>> ShaderRegistry::GetOrCompileShader<true>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_EXPORT bool ShaderRegistry::GetOrCompileVariation<false>(const TShared<CoreVariantType<Shader, false>>& shader, const TShared<CoreVariantType<Variation, false>>& variation, const String& language);
template B3D_EXPORT bool ShaderRegistry::GetOrCompileVariation<true>(const TShared<CoreVariantType<Shader, true>>& shader, const TShared<CoreVariantType<Variation, true>>& variation, const String& language);
