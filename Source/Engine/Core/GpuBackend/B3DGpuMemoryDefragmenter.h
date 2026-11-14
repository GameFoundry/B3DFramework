//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	class GpuDevice;
	class GpuWorkContext;

	/** Enables GpuMemoryDefragmenter. */
	extern B3D_EXPORT TConfigVariable<bool> gGpuDefragmentation;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Counters of the moves made by GpuMemoryDefragmenter. */
	struct GpuDefragmentationStatistics
	{
		u64 MoveCount = 0; /**< Number of textures and buffers moved since the device was created. */
		u64 MovedByteCount = 0; /**< Number of bytes moved since the device was created. */
		u32 LastPassMoveCount = 0; /**< Number of textures and buffers moved by the last pass. */
		u64 LastPassMovedByteCount = 0; /**< Number of bytes moved by the last pass. */
	};

	/**
	 * Moves textures and buffers in persistent GPU memory, so that free memory coalesces and heaps empty out. Each pass plans moves
	 * with the device's persistent allocators (see IGpuAllocator::PlanDefragmentation()), copies the contents of each moved resource
	 * into a texture or buffer created at the reserved memory, and exchanges their native resources (see render::Texture::Relocate()
	 * and render::GpuBuffer::Relocate()). Moved resources keep their proxies and bindless indices. Enabled by gpu.Defragmentation.
	 *
	 * Only moves resources at rest that are not render targets, depth-stencil targets or frame graph resources, and that no recorded but
	 * unsubmitted command buffer references. Other textures move, except those the CPU accesses (TextureUsageFlag::StoreOnCPUWithGPUAccess).
	 * Buffers move only in GpuBufferMemory::Device memory, including those the device maps. The GPU copies a moved buffer's contents after
	 * the move, so CPU writes through render::GpuBuffer::Map() made before the copy executes are lost. GpuBufferUtility::Write() stages its
	 * writes behind the copy.
	 */
	class B3D_EXPORT GpuMemoryDefragmenter
	{
	public:
		explicit GpuMemoryDefragmenter(GpuDevice& device);

		/**
		 * Plans and records moves within the per-frame budget, if gpu.Defragmentation is enabled. Records the copies into
		 * @p gpuContext's transfer command buffer, which must be submitted next, waiting for all earlier work on every queue, and
		 * before pending bindless changes are applied. Called by the renderer at the end of each frame. Render thread only.
		 */
		void Run(GpuWorkContext& gpuContext);

		/** Returns the counters of the moves made so far. */
		const GpuDefragmentationStatistics& GetStatistics() const { return mStatistics; }

	private:
		GpuDevice& mDevice;
		u32 mFirstAllocatorIndex = 0; /**< Index of the allocator the next pass plans with first, so no allocator takes every budget. */
		GpuDefragmentationStatistics mStatistics;
	};

	/** @} */
} // namespace b3d
