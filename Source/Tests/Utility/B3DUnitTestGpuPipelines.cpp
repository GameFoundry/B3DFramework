//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestGpuPipelines.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "Material/B3DShaderRegistry.h"
#include "Material/B3DShader.h"
#include "Resources/B3DBuiltinResources.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"

namespace b3d
{
	namespace
	{
		/**
		 * Resolves the test shader @p name, with one compute pass, from Data/Shaders/Tests and creates its pipeline. The shader comes from the
		 * prebuilt shader store, or is compiled where the platform can. Returns null if it cannot be resolved for the active backend.
		 */
		TShared<GpuComputePipelineState> CreateComputePipeline(GpuDevice& device, const String& name)
		{
			const TShared<Shader> shader = ShaderRegistry::Instance().GetOrCompileShader<false>(Path("Tests/" + name + ".bsl"),
				BuiltinResources::kBuiltinShaderCachePrefix, ShaderDefines());
			if(shader == nullptr)
				return nullptr;

			const Vector<TShared<Variation>> variations = shader->GetCompatibleVariations();
			if(variations.size() != 1)
				return nullptr;

			const TShared<Variation>& variation = variations.front();
			if(!variation->IsCompiled())
				variation->Compile().BlockUntilComplete();

			if(!variation->IsCompiled() || variation->GetPassCount() != 1)
				return nullptr;

			GpuComputePipelineStateCreateInformation pipelineInformation;
			pipelineInformation.Program = device.CreateGpuProgram(variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
			return device.CreateGpuComputePipelineState(pipelineInformation);
		}
	} // namespace

	TShared<GpuComputePipelineState> CreateTestDispatchPipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "BarrierDispatch");
	}

	TShared<GpuComputePipelineState> CreateTestIndexedWritePipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "IndexedWriteDispatch");
	}

	TShared<GpuComputePipelineState> CreateTestSamplePipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "BarrierSample");
	}

	TShared<GpuComputePipelineState> CreateTestDepthSamplePipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "BarrierDepthSample");
	}

	TShared<GpuComputePipelineState> CreateTestSlowWritePipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "SlowWriteDispatch");
	}

	TShared<GpuComputePipelineState> CreateTestBindlessHeapAccessPipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "BindlessHeapAccess");
	}
} // namespace b3d
