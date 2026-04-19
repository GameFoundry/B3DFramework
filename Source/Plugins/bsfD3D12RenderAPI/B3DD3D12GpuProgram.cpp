//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12ShaderCompiler.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Profiling/B3DRenderStats.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"

#include <d3dcompiler.h>

using namespace b3d;
using namespace b3d::render;

D3D12GpuProgram::D3D12GpuProgram(const GpuProgramCreateInformation& createInformation, GpuDevice& device)
	: GpuProgram(createInformation)
{
}

D3D12GpuProgram::~D3D12GpuProgram()
{
	mShaderBlob.Reset();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuProgram);
}

void D3D12GpuProgram::Initialize()
{
	if (!IsSupported())
	{
		mIsCompiled = false;
		mCompileMessages = "Specified program is not supported by the current render system.";

		GpuProgram::Initialize();
		return;
	}

	// Check if we have valid cached bytecode
	bool needsCompilation = !mBytecode ||
		mBytecode->CompilerId != "D3D12_DXC" ||
		mBytecode->CompilerVersion != 1;

	if (needsCompilation)
	{
		// Need to compile the shader
		GpuProgramCreateInformation createInformation;
		createInformation.Name = mName;
		createInformation.Type = mType;
		createInformation.EntryPoint = mEntryPoint;
		createInformation.Language = "hlsl";
		createInformation.Source = mSource;

		// Compile using utility class
		mIsCompiled = D3D12ShaderCompiler::CompileShader(createInformation, mBytecode);
	}

	// Load bytecode into blob if compilation succeeded or we have cached data
	if (mBytecode && mBytecode->Instructions.Data && mBytecode->Instructions.Size > 0)
	{
		// Create blob from bytecode data
		HRESULT hr = D3DCreateBlob(mBytecode->Instructions.Size, &mShaderBlob);
		if (SUCCEEDED(hr))
		{
			memcpy(mShaderBlob->GetBufferPointer(), mBytecode->Instructions.Data, mBytecode->Instructions.Size);
			mIsCompiled = true;
		}
		else
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create shader blob for '{0}'", mName);
			mIsCompiled = false;
		}
	}

	if (mBytecode)
		mCompileMessages = mBytecode->Messages;

	mIsCompiled = mShaderBlob != nullptr;

	if (mIsCompiled)
	{
		// Set up shader bytecode descriptor
		mShaderBytecode.pShaderBytecode = mShaderBlob->GetBufferPointer();
		mShaderBytecode.BytecodeLength = mShaderBlob->GetBufferSize();

		// Extract parameter description if available
		if (mBytecode)
			mParametersDescription = mBytecode->ParameterDescription;

		// Extract vertex input description for vertex shaders
		if (mType == GPT_VERTEX_PROGRAM && mBytecode)
		{
			mVertexInputDescription = B3DMakeShared<VertexDescription>(mBytecode->VertexInput, false);
		}
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuProgram);

	GpuProgram::Initialize();
}

