//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShaderCompiler.h"

#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"
#include "Managers/BsGpuProgramManager.h"
#include "Resources/BsBuiltinResources.h"

using namespace bs;

SPtr<IShaderCompiler> ShaderCompilers::GetCompiler(const String& language)
{
	auto found = _compilers.find(language);
	if(found != _compilers.end())
		return found->second;

	return nullptr;
}

template <bool Core>
SPtr<CoreVariantType<Shader, Core>> ShaderCompilers::GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines)
{
	using ShaderType = CoreVariantType<Shader, Core>;

	const SPtr<DataStream> shaderFileStream = FileSystem::OpenFile(shaderPath);

	if(shaderFileStream == nullptr)
	{
		B3D_LOG(Error, Resources, "Shader compilation failed for shader \"{0}\". Shader source cannot be found.", shaderPath);
		return nullptr;
	}

	const String& shaderSource = shaderFileStream->GetAsString();
	const String& shaderName = shaderPath.ToString();
	const ShadingLanguageFlag activeShadingLanguage = DetectActiveShadingLanguage();

	SPtr<CoreVariantType<Shader, Core>> shader;
	// TODO - Add caching

	if(shader == nullptr)
	{
		const SPtr<IShaderCompiler> bslCompiler = GetCompiler("bsl");
		if(bslCompiler == nullptr)
		{
			B3D_LOG(Error, Resources, "Shader compilation failed for shader \"{0}\". Shader compiler for BSL is not available.", shaderPath);
			return nullptr;
		}

		ShaderCompilerResult compileResult = bslCompiler->Compile(shaderName, shaderSource, defines.GetAll(), activeShadingLanguage, false, shader);
		if(!compileResult.ErrorMessage.empty())
		{
			B3D_LOG(Error, Resources, "Shader compilation failed for shader \"{0}\". Compilation error:\n{1}. Location: {2} ({3})", shaderPath, compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
			return nullptr;
		}

		if(shader == nullptr)
		{
			B3D_LOG(Error, Resources, "Shader compilation failed for shader \"{0}\". Unknown compilation failure.", shaderPath);
			return nullptr;
		}
	}

	return shader;
}

ShadingLanguageFlag ShaderCompilers::DetectActiveShadingLanguage()
{
	if(!ct::GpuProgramManager::IsStarted())
		return ShadingLanguageFlag::Unknown;

	for(u32 shadingLanguageIndex = 0; shadingLanguageIndex < (u32)ShadingLanguageFlag::Count; shadingLanguageIndex++)
	{
		const ShadingLanguageFlag shadingLanguageFlag = (ShadingLanguageFlag)(1 << shadingLanguageIndex);
		const String shadingLanguageName = GetShadingLanguageName(shadingLanguageFlag);

		if(ct::GpuProgramManager::Instance().IsLanguageSupported(shadingLanguageName))
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

template B3D_CORE_EXPORT SPtr<CoreVariantType<Shader, false>> ShaderCompilers::GetOrCompileShader<false>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
template B3D_CORE_EXPORT SPtr<CoreVariantType<Shader, true>> ShaderCompilers::GetOrCompileShader<true>(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);
