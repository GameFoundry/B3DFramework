//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderCompiler.h"
#include "B3DApplication.h"
#include "Material/B3DShaderVariation.h"
#include "RTTI/B3DShaderCompilerRTTI.h"
#include "GpuBackend/B3DGpuDevice.h"

using namespace b3d;

RTTIType* ShaderCompilerMetaData::GetRttiStatic()
{
	return ShaderCompilerMetaDataRTTI::Instance();
}

RTTIType* ShaderCompilerMetaData::GetRtti() const
{
	return ShaderCompilerMetaData::GetRttiStatic();
}

TShared<IShaderCompiler> ShaderCompilers::GetCompiler(const String& language)
{
	Lock lock(mCompilerMutex);
	auto found = mCompilers.find(language);
	if(found != mCompilers.end())
		return found->second;

	return nullptr;
}

TShared<IGpuBytecodeCompiler> ShaderCompilers::GetBytecodeCompiler(const String& language)
{
	Lock lock(mCompilerMutex);
	auto found = mBytecodeCompilers.find(language);
	if(found != mBytecodeCompilers.end())
		return found->second;

	return nullptr;
}

ShaderCompilers::ShaderCompilers()
	: mShadingLanguages{ kGpuProgramLanguageHlsl, kGpuProgramLanguageVksl, kGpuProgramLanguageMvksl, kGpuProgramLanguageMsl, kGpuProgramLanguageNullsl }
{
}

void ShaderCompilers::RegisterShadingLanguage(const String& language)
{
	Lock lock(mShadingLanguageMutex);
	for(const auto& existing : mShadingLanguages)
	{
		if(existing == language)
			return;
	}

	mShadingLanguages.push_back(language);
}

template <bool IsRenderProxy>
TShared<CoreVariantType<Shader, IsRenderProxy>> ShaderCompilers::CompileShader(const String& name, const String& source, const ShaderDefines& defines, const Vector<String>& languages)
{
	using ShaderType = CoreVariantType<Shader, IsRenderProxy>;

	TShared<ShaderType> shader;
	const TShared<IShaderCompiler> bslCompiler = GetCompiler("bsl");
	if(bslCompiler == nullptr)
	{
		B3D_LOG(Fatal, LogResources, "Shader compilation failed for shader \"{0}\". The BSL shader compiler is not available.", name);
		return shader;
	}

	ShaderCompilerResult compileResult = bslCompiler->Compile(name, source, defines.GetAll(), languages, false, shader);
	if(!compileResult.ErrorMessage.empty())
	{
		B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Compilation error:\n{1}. Location: {2} ({3})", name, compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
		return shader;
	}

	if(shader == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader compilation failed for shader \"{0}\". Unknown compilation failure.", name);
		return shader;
	}

	return shader;
}

String ShaderCompilers::DetectActiveShadingLanguage() const
{
	const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr)
		return StringUtility::kBlank;

	Lock lock(mShadingLanguageMutex);
	for(const auto& language : mShadingLanguages)
	{
		if(gpuDevice->IsGpuProgramLanguageSupported(language))
			return language;
	}

	return StringUtility::kBlank;
}

template B3D_EXPORT TShared<CoreVariantType<Shader, false>> ShaderCompilers::CompileShader<false>(const String& name, const String& source, const ShaderDefines& defines, const Vector<String>& languages);
template B3D_EXPORT TShared<CoreVariantType<Shader, true>> ShaderCompilers::CompileShader<true>(const String& name, const String& source, const ShaderDefines& defines, const Vector<String>& languages);
