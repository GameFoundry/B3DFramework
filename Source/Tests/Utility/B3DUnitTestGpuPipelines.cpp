//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestGpuPipelines.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"

namespace b3d
{
	namespace
	{
		/** Compiles the BSL shader @p source with one compute pass into a pipeline, or returns null if the backend cannot compile it on the host. */
		TShared<GpuComputePipelineState> CreateComputePipeline(GpuDevice& device, const char* name, const String& source)
		{
#if B3D_PLATFORM_PS5
			(void)device;
			(void)name;
			(void)source;
			return nullptr;
#else
			const String backendName = GpuBackend::Instance().GetBackendName();
			if(backendName != "bsfD3D12GpuBackend" && backendName != "bsfVulkanGpuBackend" && backendName != "bsfMetalGpuBackend")
				return nullptr;

			const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
			if(compiler == nullptr)
				return nullptr;

			String language = "vksl";
			if(backendName == "bsfD3D12GpuBackend")
				language = "hlsl";
			else if(backendName == "bsfMetalGpuBackend")
				language = "msl";

			TShared<Shader> shader;
			const ShaderCompilerResult result = compiler->Compile(name, source, {}, { language }, true, shader);
			if(!result.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1 || shader->GetVariations().front()->GetPassCount() != 1)
				return nullptr;

			GpuComputePipelineStateCreateInformation pipelineInformation;
			pipelineInformation.Program = device.CreateGpuProgram(shader->GetVariations().front()->GetPass(0)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
			return device.CreateGpuComputePipelineState(pipelineInformation);
#endif
		}
	} // namespace

	TShared<GpuComputePipelineState> CreateTestDispatchPipeline(GpuDevice& device)
	{
		const String source = R"(
shader BarrierDispatch
{
	code
	{
		RWStructuredBuffer<uint> OutputBuffer;
		RWTexture2D<uint> OutputTexture;
		[numthreads(16, 16, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			const uint value = 0x5A000000 | (threadId.y * 16 + threadId.x);
			OutputBuffer[threadId.y * 16 + threadId.x] = value;
			OutputTexture[threadId.xy] = value;
		}
	};
};
)";
		return CreateComputePipeline(device, "BarrierDispatch", source);
	}

	TShared<GpuComputePipelineState> CreateTestSamplePipeline(GpuDevice& device)
	{
		const String source = R"(
shader BarrierSample
{
	code
	{
		Texture2D<uint> InputTexture;
		RWStructuredBuffer<uint> OutputBuffer;
		[numthreads(16, 16, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			OutputBuffer[threadId.y * 16 + threadId.x] = InputTexture.Load(int3(threadId.xy, 0));
		}
	};
};
)";
		return CreateComputePipeline(device, "BarrierSample", source);
	}

	TShared<GpuComputePipelineState> CreateTestSlowWritePipeline(GpuDevice& device)
	{
		const String source = R"(
shader SlowWriteDispatch
{
	code
	{
		RWStructuredBuffer<uint> OutputBuffer;
		[numthreads(64, 1, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			uint state = threadId.x;
			for(uint iteration = 0; iteration < 16384; iteration++)
				state = state * 1664525 + 1013904223;

			OutputBuffer[threadId.x * 2] = 0x5A000000 | threadId.x;
			OutputBuffer[threadId.x * 2 + 1] = state;
		}
	};
};
)";
		return CreateComputePipeline(device, "SlowWriteDispatch", source);
	}
} // namespace b3d
