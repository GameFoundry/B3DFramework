//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12GpuDevice.h"
#include "Material/B3DShaderCompiler.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuProgram::D3D12GpuProgram(const GpuProgramCreateInformation& createInformation, GpuDevice& device)
	: GpuProgram(createInformation), mGpuDevice(device)
{
}

D3D12GpuProgram::~D3D12GpuProgram()
{
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

	// Recompile when a bytecode compiler is registered and the bytecode is missing or stale.
	const char* language = D3D12GpuDevice::kGpuProgramLanguageName;
	const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(language);
	if (bytecodeCompiler && (!mBytecode || !bytecodeCompiler->IsUpToDate(*mBytecode)))
	{
		GpuProgramCreateInformation createInformation;
		createInformation.Name = mName;
		createInformation.Type = mType;
		createInformation.EntryPoint = mEntryPoint;
		createInformation.Language = language;
		createInformation.Source = mSource;

		mBytecode = mGpuDevice.CompileGpuProgramBytecode(createInformation);
	}

	if (mBytecode)
		mCompileMessages = mBytecode->Messages;

	mIsCompiled = mBytecode && mBytecode->Instructions.Data && mBytecode->Instructions.Size > 0;

	if (mIsCompiled)
	{
		mShaderBytecode.pShaderBytecode = mBytecode->Instructions.Data;
		mShaderBytecode.BytecodeLength = mBytecode->Instructions.Size;

		mParametersDescription = mBytecode->ParameterDescription;

		if (mType == GPT_VERTEX_PROGRAM)
			mVertexInputDescription = B3DMakeShared<VertexDescription>(mBytecode->VertexInput, false);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuProgram);

	GpuProgram::Initialize();
}
