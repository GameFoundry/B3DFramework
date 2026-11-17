//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderCompiler.h"
#include "B3DApplication.h"
#include "Material/B3DShaderVariation.h"
#include "RTTI/B3DShaderCompilerRTTI.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Material/B3DShader.h"

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

Vector<TShared<GpuProgramBytecode>> IGpuBytecodeCompiler::CompileBytecodeGroup(TArrayView<const GpuProgramCreateInformation> programs)
{
	// The group is identified by its programs' code, so identical groups (the same code in two variations) share the hash
	StringStream groupCode;
	for(const GpuProgramCreateInformation& program : programs)
	{
		if(!B3D_ENSURE_LOG(IsRayTracingProgramType(program.Type), "Program '{0}' is a {1} program. Only ray tracing pipeline programs compile in groups.",
			program.Name, GetProgramTypeName(program.Type)))
		{
			return {};
		}

		groupCode << (u32)program.Type << ':' << program.EntryPoint.size() << ':' << program.EntryPoint << ':' << program.Source.size() << ':' << program.Source;
	}

	const Array<u64, 2> groupHash = Shader::ComputeHash(groupCode.str());

	Vector<TShared<GpuProgramBytecode>> bytecodes = CompileBytecodeGroupInternal(programs);
	B3D_ASSERT(bytecodes.size() == programs.size());

	for(const TShared<GpuProgramBytecode>& bytecode : bytecodes)
	{
		if(bytecode != nullptr)
			bytecode->ProgramGroupHash = groupHash;
	}

	return bytecodes;
}

Vector<TShared<GpuProgramBytecode>> IGpuBytecodeCompiler::CompileBytecodeGroupInternal(TArrayView<const GpuProgramCreateInformation> programs)
{
	Vector<TShared<GpuProgramBytecode>> bytecodes;
	bytecodes.reserve(programs.size());

	for(const GpuProgramCreateInformation& program : programs)
		bytecodes.push_back(CompileBytecode(program));

	return bytecodes;
}

ShaderCompilers::ShaderCompilers()
	: mShadingLanguages{ kGpuProgramLanguageHlsl, kGpuProgramLanguageVksl, kGpuProgramLanguageMsl, kGpuProgramLanguageNullsl }
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
