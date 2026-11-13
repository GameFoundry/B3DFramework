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
		 * Resolves the test shader @p name, with one pass, from Data/Shaders/Tests. The shader comes from the prebuilt shader store, or is
		 * compiled where the platform can. Returns its pass, or null if it cannot be resolved for the active backend.
		 */
		TShared<Pass> ResolveTestPass(const String& name)
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

			return variation->GetPass(0);
		}

		/** Resolves the test shader @p name, with one compute pass, and creates its pipeline. Returns null if it cannot be resolved for the active backend. */
		TShared<GpuComputePipelineState> CreateComputePipeline(GpuDevice& device, const String& name)
		{
			const TShared<Pass> pass = ResolveTestPass(name);
			if(pass == nullptr)
				return nullptr;

			GpuComputePipelineStateCreateInformation pipelineInformation;
			pipelineInformation.Program = device.CreateGpuProgram(pass->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
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

	TShared<GpuComputePipelineState> CreateTestByteAddressBufferAccessPipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "ByteAddressBufferAccess");
	}

	TShared<GpuComputePipelineState> CreateTestInlineRayTracingPipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "InlineRayTracing");
	}

	TShared<GpuComputePipelineState> CreateTestInlineRayTracingBindlessPipeline(GpuDevice& device)
	{
		return CreateComputePipeline(device, "InlineRayTracingBindless");
	}

	UnorderedMap<String, TShared<GpuProgram>> CreateTestRayTracingPrograms(GpuDevice& device, const String& name)
	{
		UnorderedMap<String, TShared<GpuProgram>> programs;
		for(const auto& [entryPoint, programInformation] : GetTestRayTracingProgramInformation(name))
			programs[entryPoint] = device.CreateGpuProgram(programInformation);

		return programs;
	}

	UnorderedMap<String, GpuProgramCreateInformation> GetTestRayTracingProgramInformation(const String& name)
	{
		UnorderedMap<String, GpuProgramCreateInformation> programInformation;

		const TShared<Pass> pass = ResolveTestPass(name);
		if(pass == nullptr)
			return programInformation;

		for(const GpuProgramCreateInformation& information : pass->GetInformation().Programs)
			programInformation[information.EntryPoint] = information;

		return programInformation;
	}
} // namespace b3d
