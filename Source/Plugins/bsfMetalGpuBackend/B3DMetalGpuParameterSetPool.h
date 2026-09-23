//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of @c GpuParameterSetPool.
		 *
		 * Transient pools back their sets' argument buffers with a ring of pooled @c MTLBuffer blocks. Every set
		 * sub-allocates a slice from the first block with room (bump allocator), and a new block is appended when none
		 * has. @c Reset rewinds every block's cursor without releasing the backing memory, so subsequent frames reuse the
		 * same blocks. This keeps a transient set down to a pointer bump rather than a heap placement plus an
		 * @c MTLBuffer object per set. The engine guarantees no in-flight command buffer still references the sets
		 * before @c Reset.
		 *
		 * Persistent pools hand out sets that place their own argument buffers through the device's heap allocator,
		 * since individual sets go out of scope independently. The pool holds no memory for them.
		 *
		 * Requests larger than @c kLargeSliceThreshold bypass the ring even in transient pools, so a single large
		 * argument buffer does not pin an entire block.
		 *
		 * @note	Not thread safe, per the @c GpuParameterSetPool contract.
		 */
		class MetalGpuParameterSetPool final : public GpuParameterSetPool
		{
		public:
			/** Default size of each pooled argument-buffer block (2 MiB). */
			static constexpr u64 kDefaultBlockSize = 2ull * 1024ull * 1024ull;

			/**
			 * Requests above this size get a dedicated buffer rather than a slice of the ring. 256 KiB is well above a
			 * typical argument buffer (a few hundred bytes).
			 */
			static constexpr u64 kLargeSliceThreshold = 256ull * 1024ull;

			MetalGpuParameterSetPool(MetalGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation);
			~MetalGpuParameterSetPool() override;

			TShared<GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize = false) override;
			void Reset() override;

#ifdef __OBJC__
			/**
			 * Sub-allocates @p size bytes (aligned to @p alignment) from a transient pool. Returns the host
			 * @c MTLBuffer plus the offset into it. The returned @c MTLBuffer is owned by the pool; callers must
			 * @b not release it.
			 */
			id<MTLBuffer> AcquireArgumentBufferSlice(u64 size, u32 alignment, u64& outOffset);
#endif

		private:
			struct Block
			{
				MetalBufferNativeHandle Buffer = nullptr;
				u64 Size = 0;
				u64 Cursor = 0;
			};

			/**
			 * Grows the ring by one shared-storage block of @p minimumSize bytes. Returns a pointer into @c mBlocks to
			 * the new block, or nullptr if the allocation failed.
			 */
			Block* GrowByBlock(u64 minimumSize);

			MetalGpuDevice& mDevice;
			u32 mAllocatedSetCount = 0;
			Vector<Block> mBlocks;

			// Dedicated buffers for requests above kLargeSliceThreshold. Released on Reset, like the ring's slices.
			Vector<MetalBufferNativeHandle> mDirectBuffers;
		};

		/** @} */
	} // namespace render
} // namespace b3d
