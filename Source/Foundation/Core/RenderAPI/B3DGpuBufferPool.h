#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Utility/B3DTArray.h"

namespace b3d::render
{
	/**
	 * Pool allocator for GPU buffer suballocations. Allocates suballocations transiently - each allocation is valid for one frame
	 * and automatically recycled after all in-flight frames complete (typically 3).
	 */
	class B3D_EXPORT TransientGpuBufferPool
	{
	public:
		TransientGpuBufferPool() = default;
		~TransientGpuBufferPool() = default;

		/**
		 * Initializes the pool. Must be called before use.
		 *
		 * @param device                    GPU device for querying capabilities.
		 * @param createInfo                Buffer creation info (size is per-suballocation).
		 * @param initialBufferCount        Initial number of buffers, each with @p suballocationsPerBuffer suballocations.
		 * @param suballocationsPerBuffer   Number of suballocations per GpuBuffer.
		 */
		void Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 initialBufferCount, u32 suballocationsPerBuffer);

		/**
		 * Allocates a suballocation for the current frame.
		 *
		 * The suballocation is valid until AdvanceFrame() is called N times,
		 * where N is the number of frames in-flight (typically 3).
		 *
		 * If no free suballocations are available, the pool automatically
		 * grows by allocating a new GpuBuffer.
		 *
		 * @return  Suballocation handle (always valid)
		 */
		GpuBufferSuballocation Allocate();

		/**
		 * Advances to the next frame and recycles old suballocations.
		 *
		 * Call once per frame after submitting all command buffers.
		 *
		 * This marks all current allocations as not in-use for the next frame,
		 * and rebuilds the free-list to include suballocations from frame N-3
		 * that are now safe to reuse.
		 */
		void AdvanceFrame();

		/**
		 * Gets the size per suballocation (aligned).
		 *
		 * May be larger than the requested size due to GPU alignment requirements
		 * (typically 256 bytes for uniform buffers).
		 */
		u32 GetSuballocationSize() const { return mSuballocationSize; }

		/**
		 * Gets the number of currently allocated buffers.
		 */
		u32 GetBufferCount() const { return (u32)mBuffers.size(); }

		/**
		 * Gets the total number of suballocations (used + free).
		 */
		u32 GetTotalSuballocationCount() const { return (u32)mSuballocations.size(); }

	private:
		/** Entry in the suballocation pool with intrusive free-list link. */
		struct SuballocationEntry
		{
			SPtr<GpuBuffer> Buffer;
			u32 SuballocationIndex;
			u32 SuballocationOffset;
			u64 LastUsedFrameNumber;
			u32 NextFreeIndex;
		};

		/** Grows the pool by allocating a new GpuBuffer. */
		void AddNewBufferToPool();

	private:
		GpuDevice* mDevice = nullptr;
		GpuBufferCreateInformation mBufferCreateInformation;

		u32 mSuballocationSize = 0;
		u32 mSuballocationsPerBuffer = 0;
		u64 mCurrentFrameNumber = 0;

		// Free-list head (index of first free entry, ~0u = empty list)
		u32 mFreeListHead = ~0u;

		TInlineArray<SuballocationEntry, 4> mSuballocations;
		TInlineArray<SPtr<GpuBuffer>, 1> mBuffers;
	};
}
