//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderCompiler.h"
#include "RTTI/B3DShaderCompilerRTTI.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Resources/B3DBuiltinResources.h"
#include "Utility/B3DPersistentCache.h"

using namespace b3d;

RTTIType* ShaderCompilerMetaData::GetRttiStatic()
{
	return ShaderCompilerMetaDataRTTI::Instance();
}

RTTIType* ShaderCompilerMetaData::GetRtti() const
{
	return ShaderCompilerMetaData::GetRttiStatic();
}

SPtr<IShaderCompiler> ShaderCompilers::GetCompiler(const String& language)
{
	auto found = mCompilers.find(language);
	if(found != mCompilers.end())
		return found->second;

	return nullptr;
}

void ShaderCompilers::RegisterSearchPath(const Path& folder)
{
	Lock lock(mSearchPathMutex);
	mSearchPaths.push_back(folder);
}

template <bool IsRenderProxy>
SPtr<CoreVariantType<Shader, IsRenderProxy>> ShaderCompilers::GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines)
{
	using ShaderType = CoreVariantType<Shader, IsRenderProxy>;

	SPtr<DataStream> shaderFileStream = nullptr;

	if(shaderPath.IsAbsolute())
		shaderFileStream = FileSystem::OpenFile(shaderPath);
	else
	{
		Lock lock(mSearchPathMutex);
		for(const auto& searchPath : mSearchPaths)
		{
			const Path absolutePath = shaderPath.GetAbsolute(searchPath);
			if (!FileSystem::Exists(absolutePath))
				continue;

			shaderFileStream = FileSystem::OpenFile(absolutePath);

			if (shaderFileStream != nullptr)
				break;
		}
	}

	if(shaderFileStream == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Shader source cannot be found.", shaderPath);
		return nullptr;
	}

	const String& shaderSource = shaderFileStream->GetAsString();
	const ShadingLanguageFlag activeShadingLanguage = DetectActiveShadingLanguage();
	const String shadingLanguageName = String(GetShadingLanguageName(activeShadingLanguage)) + "/";
	const String& shaderName = shaderPath.GetFilename(false);
	const Array<u64, 2> shaderHash = Shader::ComputeHash(shaderSource);
	const String shaderNameInCache = cachePrefix + shaderName + "/";

	const Path shaderPathInCache = Path(shaderNameInCache) + shadingLanguageName + "MetaData";
	PersistentCache& cache = GetApplication().GetApplicationCache();

	SPtr<ShaderType> shader = cache.TryGetEntry<ShaderType>(shaderPathInCache);
	if(shader != nullptr)
	{
		const SPtr<ShaderCompilerMetaData> compilerMetaData = shader->GetCompilerMetaData();
		if(compilerMetaData != nullptr)
		{
			if(shaderHash != compilerMetaData->ShaderHash)
			{
				shader = nullptr;
			}
			else
			{
				for(const auto& includeHash : compilerMetaData->IncludeHashes)
				{
					const Array<u64, 2> newIncludeHash = Shader::ComputeIncludeHash(includeHash.first);
					if(newIncludeHash != includeHash.second)
					{
						shader = nullptr;
						break;
					}
				}
			}
		}
	}

	if(shader == nullptr)
	{
		const SPtr<IShaderCompiler> bslCompiler = GetCompiler("bsl");
		if(bslCompiler == nullptr)
		{
			B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Shader compiler for BSL is not available.", shaderPath);
			return nullptr;
		}

		ShaderCompilerResult compileResult = bslCompiler->Compile(shaderName, shaderSource, defines.GetAll(), activeShadingLanguage, false, shader);
		if(!compileResult.ErrorMessage.empty())
		{
			B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Compilation error:\n{1}. Location: {2} ({3})", shaderPath, compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
			return nullptr;
		}

		if(shader == nullptr)
		{
			B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Unknown compilation failure.", shaderPath);
			return nullptr;
		}

		const SPtr<ShaderCompilerMetaData> compilerMetaData = shader->GetCompilerMetaData();
		if(B3D_ENSURE(compilerMetaData != nullptr))
		{
			compilerMetaData->NameInCache = shaderNameInCache;
			compilerMetaData->ShaderHash = shaderHash;

			cache.SetEntry(shaderPathInCache, shader);
		}
	}

	return shader;
}

ShadingLanguageFlag ShaderCompilers::DetectActiveShadingLanguage()
{
	const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr)
		return ShadingLanguageFlag::Unknown;

	for(u32 shadingLanguageIndex = 0; shadingLanguageIndex < (u32)ShadingLanguageFlag::Count; shadingLanguageIndex++)
	{
		const ShadingLanguageFlag shadingLanguageFlag = (ShadingLanguageFlag)(1 << shadingLanguageIndex);
		const String shadingLanguageName = GetShadingLanguageName(shadingLanguageFlag);

		if(gpuDevice->IsGpuProgramLanguageSupported(shadingLanguageName))
			return shadingLanguageFlag;
	}

	return ShadingLanguageFlag::Unknown;
}

ShadingLanguageFlag ShaderCompilers::ParseShadingLanguage(const String& name)
{
	if(name == "hlsl")
		return ShadingLanguageFlag::HLSL;

	if(name == "glsl")
		return ShadingLanguageFlag::GLSL;

	if(name == "vksl")
		return ShadingLanguageFlag::VKSL;

	if(name == "mvksl")
		return ShadingLanguageFlag::MSL;

	return ShadingLanguageFlag::Unknown;
}

const char* ShaderCompilers::GetShadingLanguageName(ShadingLanguageFlag language)
{
	switch(language)
	{
	case ShadingLanguageFlag::HLSL: return "hlsl";
	case ShadingLanguageFlag::GLSL: return "glsl";
	case ShadingLanguageFlag::VKSL: return "vksl";
	case ShadingLanguageFlag::MSL: return "mvksl";
	default: return "";
	}
}

template B3D_EXPORT SPtr<CoreVariantType<Shader, false>> ShaderCompilers::GetOrCompileShader<false>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_EXPORT SPtr<CoreVariantType<Shader, true>> ShaderCompilers::GetOrCompileShader<true>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
