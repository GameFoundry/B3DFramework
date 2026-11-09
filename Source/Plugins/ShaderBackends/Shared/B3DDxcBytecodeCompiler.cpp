//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DDxcBytecodeCompiler.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Debug/B3DLog.h"
#include "String/B3DUnicode.h"
#include "Utility/B3DConfigVariable.h"

using namespace b3d;
using namespace b3d::render;

namespace b3d
{
	TConfigVariable<bool> gGpuSkipShaderOptimization("gpu.SkipShaderOptimization",
		"Compile HLSL shaders with optimizations disabled and debug info attached, so they can be stepped through in a "
		"shader debugger. Slows down shader execution significantly.", false,
		ConfigVariableFlag::ReadOnly);
}

namespace
{
	/** Returns the interface held by @p pointer, as TDxcPointer has no accessor common to all platforms. */
	template <class T>
	T& GetInterface(const TDxcPointer<T>& pointer)
	{
#if B3D_PLATFORM_WIN32
		return *pointer.Get();
#else
		return *pointer.p;
#endif
	}
} // namespace

DxcBytecodeCompiler::DxcBytecodeCompiler(const char* compilerId, u32 compilerVersion)
	: mCompilerId(compilerId), mCompilerVersion(compilerVersion)
{
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&mUtilities));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler));
}

bool DxcBytecodeCompiler::IsUpToDate(const GpuProgramBytecode& bytecode) const
{
	return bytecode.CompilerId == mCompilerId && bytecode.CompilerVersion == mCompilerVersion;
}

TShared<GpuProgramBytecode> DxcBytecodeCompiler::CompileBytecode(const GpuProgramCreateInformation& createInformation)
{
	TShared<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = mCompilerId;
	bytecode->CompilerVersion = mCompilerVersion;

	if(mUtilities == nullptr || mCompiler == nullptr)
	{
		bytecode->Messages = "Failed to initialize the DirectX Shader Compiler.";
		B3D_LOG(Error, LogRenderBackend, "{0}", bytecode->Messages);

		return bytecode;
	}

	const wchar_t* target = nullptr;
	switch (createInformation.Type)
	{
	case GPT_VERTEX_PROGRAM:   target = L"vs_6_6"; break;
	case GPT_FRAGMENT_PROGRAM: target = L"ps_6_6"; break;
	case GPT_GEOMETRY_PROGRAM: target = L"gs_6_6"; break;
	case GPT_HULL_PROGRAM:     target = L"hs_6_6"; break;
	case GPT_DOMAIN_PROGRAM:   target = L"ds_6_6"; break;
	case GPT_COMPUTE_PROGRAM:  target = L"cs_6_6"; break;
	// Ray tracing programs compile as a library that exports the entry point as its only shader
	case GPT_RAY_GENERATION_PROGRAM:
	case GPT_MISS_PROGRAM:
	case GPT_CLOSEST_HIT_PROGRAM:
	case GPT_ANY_HIT_PROGRAM:
	case GPT_INTERSECTION_PROGRAM:
	case GPT_CALLABLE_PROGRAM:
		target = L"lib_6_6";
		break;
	default:
		bytecode->Messages = "Unsupported shader type";
		return bytecode;
	}

	Vector<WString> arguments;
	arguments.push_back(L"-Zpc"); // Pack matrices in column-major order.
	arguments.push_back(L"-Ges"); // Enable strict language checking.
	arguments.push_back(L"-HV"); // Select the HLSL language version.
	arguments.push_back(L"2016"); // Use HLSL 2016 language and scoping rules.
	arguments.push_back(L"-Wno-for-redefinition"); // Redeclaring a loop variable is valid under the requested legacy HLSL scoping rules.
	AddCompileArguments(arguments);

	// Unoptimized shaders are opt-in even in development builds because they cost significant GPU time.
	bool skipOptimization = false;
#if B3D_BUILD_TYPE_DEVELOPMENT
	skipOptimization = gGpuSkipShaderOptimization.Get();
#endif

	if(skipOptimization)
	{
		arguments.push_back(L"-Zi"); // Generate debug information.
		arguments.push_back(L"-Od"); // Disable optimizations.
		arguments.push_back(L"-Qembed_debug"); // Embed debug information in the shader container.
	}
	else
		arguments.push_back(L"-O3"); // Use optimization level 3.

	Vector<LPCWSTR> argumentPointers;
	argumentPointers.reserve(arguments.size());
	for(const WString& argument : arguments)
		argumentPointers.push_back(argument.c_str());

	const WString sourceName = UTF8::ToWide(!createInformation.Name.empty() ? createInformation.Name : "unnamed_shader");
	const WString entryPoint = UTF8::ToWide(createInformation.EntryPoint);

	TDxcPointer<IDxcCompilerArgs> compilerArguments;
	HRESULT result = mUtilities->BuildArguments(sourceName.c_str(), entryPoint.c_str(), target, argumentPointers.data(), (UINT32)argumentPointers.size(), nullptr, 0, &compilerArguments);
	if(FAILED(result))
	{
		bytecode->Messages = "Failed to build DirectX Shader Compiler arguments.";
		B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}': {1} (hr={2}).", createInformation.Name, bytecode->Messages, (u32)result);

		return bytecode;
	}

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = createInformation.Source.data();
	sourceBuffer.Size = createInformation.Source.size();
	sourceBuffer.Encoding = DXC_CP_UTF8;

	TDxcPointer<IDxcResult> compilationResult;
	result = mCompiler->Compile(&sourceBuffer, compilerArguments->GetArguments(), compilerArguments->GetCount(), nullptr, IID_PPV_ARGS(&compilationResult));
	if(FAILED(result) || compilationResult == nullptr)
	{
		bytecode->Messages = "Failed to invoke the DirectX Shader Compiler.";
		B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}': {1} (hr={2}).", createInformation.Name, bytecode->Messages, (u32)result);

		return bytecode;
	}

	TDxcPointer<IDxcBlobUtf8> diagnostics;
	compilationResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&diagnostics), nullptr);
	const String diagnosticMessages = diagnostics != nullptr && diagnostics->GetStringLength() > 0 ? String(diagnostics->GetStringPointer(), diagnostics->GetStringLength()) : String();

	HRESULT compilationStatus;
	result = compilationResult->GetStatus(&compilationStatus);
	if(FAILED(result) || FAILED(compilationStatus))
	{
		bytecode->Messages = diagnosticMessages.empty() ? "Shader compilation failed with unknown error." : String("Shader compilation failed:\n") + diagnosticMessages;

		// The generated source only exists in memory, so include it in the log with line numbers.
		StringStream numberedSource;
		u32 lineNumber = 1;
		size_t lineStart = 0;
		while (lineStart <= createInformation.Source.size())
		{
			size_t lineEnd = createInformation.Source.find('\n', lineStart);
			if (lineEnd == String::npos)
				lineEnd = createInformation.Source.size();

			numberedSource << lineNumber << ": " << createInformation.Source.substr(lineStart, lineEnd - lineStart) << "\n";
			lineStart = lineEnd + 1;
			lineNumber++;
		}

		B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}':\n{1}\nGenerated source:\n{2}", createInformation.Name, bytecode->Messages, numberedSource.str());
		return bytecode;
	}

	if (!diagnosticMessages.empty())
	{
		bytecode->Messages = String("Shader compiled with warnings:\n") + diagnosticMessages;
		B3D_LOG(Warning, LogRenderBackend, "Shader '{0}' compiled with warnings:\n{1}", createInformation.Name, diagnosticMessages);
	}

	TDxcPointer<IDxcBlob> program;
	result = compilationResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&program), nullptr);
	if(FAILED(result) || program == nullptr)
	{
		bytecode->Messages = "Failed to retrieve compiled shader bytecode.";
		B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}': {1} (hr={2}).", createInformation.Name, bytecode->Messages, (u32)result);

		return bytecode;
	}

	if(!ReflectBytecode(createInformation, GetInterface(compilationResult), GetInterface(program), *bytecode))
		return bytecode;

	bytecode->Instructions.Size = (u32)program->GetBufferSize();
	bytecode->Instructions.Data = (u8*)B3DAllocate(bytecode->Instructions.Size);
	memcpy(bytecode->Instructions.Data, program->GetBufferPointer(), bytecode->Instructions.Size);

	return bytecode;
}
