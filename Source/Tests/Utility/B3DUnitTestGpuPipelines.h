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

	/**
	 * Creates a compute pipeline whose single-thread dispatch reads through the bindless heaps and writes seven values into the storage buffer
	 * Output: element 1 of the structured buffer and element 2 of the typed buffer whose resource heap indices are the push constants
	 * StructuredIndex and TypedIndex, the red channel of texel (1, 0) of the texture at the resource heap index in the low 16 bits of
	 * TextureAndSamplerIndices, as a byte, the same texture's green channel sampled at (0.75, 0.5) with the sampler at the sampler heap
	 * index in its high 16 bits, and blue channel sampled at (0.25, 0.5) with the point clamp global sampler, and words 2 and 3 of the raw
	 * buffer at resource heap index RawIndex. Returns null if the shader cannot be resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestBindlessHeapAccessPipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose dispatch of four threads accesses the byte address buffers Input and Output. Each thread writes input word
	 * N plus one into output word N and adds one to output word 4, and output word 8 receives the maximum of N + 10 across threads. Thread 0
	 * also copies input words 1 and 2 into output words 5 and 6, and writes the size of the input in bytes into output word 7. Returns null if
	 * the shader cannot be resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestByteAddressBufferAccessPipeline(GpuDevice& device);
} // namespace b3d
