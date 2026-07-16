//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "Utility/Threading/B3DThreading.h"

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
		 * Backs transient parameter-set argument buffers with a ring of pooled @c MTLBuffer blocks so
		 * that @c Reset() is a genuine recycle instead of a zero-counter no-op. Every @c Create call
		 * sub-allocates an argument-buffer slice from the current block (bump-allocator). When a
		 * request does not fit, a new block is appended. @c Reset rewinds every block's cursor without
		 * releasing the backing memory, so subsequent frames re-use the same allocations.
		 *
		 * Transient vs persistent semantics:
		 *  - @c Transient pools tolerate the ring-reset model: the engine releases every set back to
		 *    the pool at once via @c Reset, and the engine guarantees no in-flight command buffer still
		 *    references the sets before the reset lands.
		 *  - @c Persistent pools cannot use the ring — individual sets go out of scope independently
		 *    and must keep their argument-buffer slice alive until then. For @c Persistent pools this
		 *    class falls through to a direct @c newBufferWithLength: path, matching the previous
		 *    (pre-B9) behaviour.
		 *  - Requests larger than @c kLargeSliceThreshold bypass the ring entirely even in transient
		 *    pools; tiny frames should not force a multi-MB block to be reserved for them.
		 */
		class MetalGpuParameterSetPool final : public GpuParameterSetPool
		{
		public:
			/** Default size of each pooled argument-buffer block (2 MiB). */
			static constexpr u64 kDefaultBlockSize = 2ull * 1024ull * 1024ull;

			/**
			 * Requests above this size fall through to a direct device allocation rather than taking a
			 * giant slice out of the ring. Mirrors @c MetalHeapAllocator's large-resource threshold
			 * reasoning. 256 KiB is already well above the upper bound for a typical argument buffer
			 * (a few hundred bytes).
			 */
			static constexpr u64 kLargeSliceThreshold = 256ull * 1024ull;

			MetalGpuParameterSetPool(MetalGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation);
			~MetalGpuParameterSetPool() override;

			TShared<GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize = false) override;
			void Reset() override;

#ifdef __OBJC__
			/**
			 * Sub-allocates @p size bytes (aligned to @p alignment) from the pool. Returns the host
			 * @c MTLBuffer plus the offset into it. When the request does not fit the current block a
			 * new one is grown into place. When the request exceeds @c kLargeSliceThreshold or the
			 * pool is @c Persistent the function falls through to a direct device allocation and the
			 * returned offset is zero.
			 *
			 * The returned @c MTLBuffer is owned by the pool; callers must @b not release it. Its
			 * lifetime is gated on @c Reset (for pooled slices) or on the caller's ARC/MRC reference
			 * (for direct allocations). Invoked by @c MetalGpuParameters::Initialize.
			 */
			id<MTLBuffer> AcquireArgumentBufferSlice(u64 size, u32 alignment, u64& outOffset);
#endif

		private:
#ifdef __OBJC__
			struct Block
			{
				// +1 retained MTLBuffer under MRC, strong reference under ARC. Drained in the
				// destructor. Never released by Reset — Reset only rewinds @c Cursor.
				id<MTLBuffer> Buffer = nil;
				u64 Size = 0;
				u64 Cursor = 0;
			};

			/**
			 * Grows the ring by one block large enough to cover @p minimumSize bytes with the shared
			 * storage mode used by all argument buffers. Returns a pointer into @c mBlocks to the new
			 * block, or nullptr if @c newBufferWithLength: failed.
			 */
			Block* GrowByBlock(u64 minimumSize);
#endif

			MetalGpuDevice& mDevice;
			u32 mAllocatedSetCount = 0;

#ifdef __OBJC__
			// Worker fibers may create parameter sets concurrently; serialize ring mutations. Reset is
			// guaranteed by the engine to fire on a quiescent frame boundary, but still takes the lock
			// for defense-in-depth.
			Mutex mPoolMutex;
			Vector<Block> mBlocks;

			// Persistent pools bypass the ring path — keep the direct-allocated buffers alive here so
			// the pool controls their lifetime consistently with the transient path.
			Vector<id<MTLBuffer>> mDirectBuffers;
#endif
		};

		/** @} */
	} // namespace render
} // namespace b3d
