//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/**
	 * Creates a compute pipeline whose thread (x, y) of a 16 x 16 dispatch writes 0x5A000000 | (y * 16 + x) into element y * 16 + x of
	 * the storage buffer OutputBuffer, and into texel (x, y) of the R32U storage texture OutputTexture. Returns null if the shader cannot
	 * be resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestDispatchPipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose single-thread dispatch writes Bias + OutputIndex into element OutputIndex of the storage buffer
	 * OutputBuffer. OutputIndex is a push constant, and Bias comes from the uniform buffer Parameters. Returns null if the shader cannot be
	 * resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestIndexedWritePipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose thread (x, y) of a 16 x 16 dispatch reads texel (x, y) of the sampled R32U texture InputTexture,
	 * and writes it into element y * 16 + x of the storage buffer OutputBuffer. Returns null if the shader cannot be resolved for the
	 * active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestSamplePipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose thread (x, y) of a 16 x 16 dispatch reads the depth of texel (x, y) of the sampled depth texture
	 * InputTexture, and writes its bits into element y * 16 + x of the storage buffer OutputBuffer. Returns null if the shader cannot be
	 * resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestDepthSamplePipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose thread x runs a long serial loop, then writes 0x5A000000 | x into element 2 * x of the storage
	 * buffer OutputBuffer, and the loop's result into element 2 * x + 1, so the loop cannot be optimized away. Dispatched in groups
	 * of 64 threads. Returns null if the shader cannot be resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestSlowWritePipeline(GpuDevice& device);
} // namespace b3d
