//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"

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

	/**
	 * Creates a compute pipeline that traces one ray per thread group against the top level acceleration structure Scene. Group N reads its
	 * ray from element N of the structured buffer Rays (origin and minimum distance, direction and maximum distance, then ray flags, instance
	 * mask and mode as four words), and writes what the ray hit into element N of the structured buffer Hits: 1 or 0 for a hit or a miss,
	 * the distance and the two barycentrics, then the instance index, instance id, geometry index and primitive index, then 1 for a front
	 * face hit and the number of any-hit callbacks, padded to four words. Mode 0 traces the closest hit, mode 1 any hit, and mode 2 the
	 * closest hit with any-hit callbacks that reject every candidate. Returns null if the shader cannot be resolved for the active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestInlineRayTracingPipeline(GpuDevice& device);

	/**
	 * Creates a compute pipeline whose single-thread dispatch traces a ray from (0.25, 10.25, 0) along +Z against the top level acceleration
	 * structure at the resource heap index in the push constant SceneIndex, and writes 1 or 0 for a hit or a miss, the distance, the instance
	 * id and the instance index, as floats, into element 0 of the storage buffer Hit. Returns null if the shader cannot be resolved for the
	 * active backend.
	 */
	TShared<GpuComputePipelineState> CreateTestInlineRayTracingBindlessPipeline(GpuDevice& device);

	/**
	 * Creates the programs of the ray tracing test shader @p name from Data/Shaders/Tests, by entry point name. RayTracingPipeline has:
	 *  - TraceTestRays (ray generation): thread N reads its ray from element N of the structured buffer Rays (origin, direction, then the hit
	 *    group contribution, geometry multiplier and miss index as four words), traces it against the top level acceleration structure Scene
	 *    with a payload of 1, and writes the payload into element N of the storage buffer Results;
	 *  - WriteIndices (ray generation): thread N writes 1000 + N into element N of Results;
	 *  - MissA and MissB (miss): set the payload to 100 and 101;
	 *  - ClosestHitA, ClosestHitB and ClosestHitC (closest hit): set the payload to 10, 11 and 12.
	 *
	 * RayTracingOtherParameters declares other parameters, and has WriteZero (ray generation).
	 *
	 * Returns no programs if the shader cannot be resolved for the active backend.
	 */
	UnorderedMap<String, TShared<GpuProgram>> CreateTestRayTracingPrograms(GpuDevice& device, const String& name);

	/**
	 * Returns the information CreateTestRayTracingPrograms() creates the programs of the ray tracing test shader @p name from, by entry point
	 * name. Empty if the shader cannot be resolved for the active backend.
	 */
	UnorderedMap<String, GpuProgramCreateInformation> GetTestRayTracingProgramInformation(const String& name);
} // namespace b3d
