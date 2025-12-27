#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Utility/B3DTArray.h"

namespace b3d::render
{
	// Forward declarations
	class GpuBufferPool;

	/**
	 * Tracked GPU buffer suballocation that automatically releases itself when destroyed.
	 * Inherits from GpuBufferSuballocation to support all the same operations.
	 */
	class B3D_EXPORT TrackedGpuBufferSuballocation : public GpuBufferSuballocation
	{
	public:
		/**
		 * Constructor - for internal use by GpuBufferPool only.
		 * Do not call directly - use GpuBufferPool::AllocateTracked() instead.
		 */
		TrackedGpuBufferSuballocation(GpuBufferPool* owner, const GpuBufferSuballocation& base);

		/** Destructor - automatically releases the allocation back to the owner pool. */
		~TrackedGpuBufferSuballocation();

		/** Move constructor - transfers ownership. */
		TrackedGpuBufferSuballocation(TrackedGpuBufferSuballocation&& other) noexcept;

		/** Move assignment - transfers ownership. */
		TrackedGpuBufferSuballocation& operator=(TrackedGpuBufferSuballocation&& other) noexcept;

		/** Deleted copy constructor - prevent double-release. */
		TrackedGpuBufferSuballocation(const TrackedGpuBufferSuballocation&) = delete;

		/** Deleted copy assignment - prevent double-release. */
		TrackedGpuBufferSuballocation& operator=(const TrackedGpuBufferSuballocation&) = delete;

	private:
		GpuBufferPool* mOwner;
	};

	/**
	 * Pool allocator for GPU buffer suballocations. Allocates suballocations transiently - each allocation is valid for one frame
	 * and automatically recycled after all in-flight frames complete (typically 3).
	 */
	class B3D_EXPORT TransientGpuBufferPool
	{
	public:
		TransientGpuBufferPool() = default;

		/** Constructs and immediately initializes the pool. See Initialize(). */
		TransientGpuBufferPool(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount = 1);

		~TransientGpuBufferPool() = default;

		/**
		 * Initializes the pool. Must be called before use.
		 *
		 * @param device                    GPU device for querying capabilities.
		 * @param createInfo                Buffer creation info (size is per-suballocation).
		 * @param suballocationsPerBuffer   Number of suballocations per GpuBuffer.
		 * @param initialBufferCount        Initial number of buffers, each with @p suballocationsPerBuffer suballocations.
		 */
		void Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount = 1);

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

	/**
	 * GPU buffer pool with manual or automatic lifetime management. Returned allocations are sub-allocated from GpuBuffers,
	 * which makes them lightweight and efficient.
	 *
	 * Usage - Non-tracked (manual release required):
	 *   GpuBufferSuballocation allocation = pool.Allocate();
	 *   // ... use allocation ...
	 *   pool.Release(allocation);
	 *
	 * Usage - Tracked (automatic release on destruction):
	 *   UPtr<TrackedGpuBufferSuballocation> allocation = pool.AllocateTracked();
	 *   // ... use allocation ...
	 *   // Automatically released when allocation goes out of scope
	 */
	class B3D_EXPORT GpuBufferPool
	{
	public:
		GpuBufferPool() = default;
		~GpuBufferPool() = default;

		/**
		 * Initializes the pool. Must be called before use.
		 *
		 * @param device					GPU device for querying capabilities.
		 * @param createInfo				Buffer creation info (size is per-suballocation).
		 * @param suballocationsPerBuffer	Number of suballocations per GpuBuffer.
		 * @param initialBufferCount		Initial number of buffers, each with @p suballocationsPerBuffer suballocations.
		 */
		void Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount = 1);

		/**
		 * Allocates a suballocation (non-tracked).
		 *
		 * The allocation persists until manually released via Release().
		 * If no free suballocations are available, the pool automatically grows.
		 *
		 * @return	Suballocation handle (always valid)
		 */
		GpuBufferSuballocation Allocate();

		/**
		 * Allocates a tracked suballocation.
		 *
		 * The allocation is automatically released when the returned object is destroyed.
		 * If no free suballocations are available, the pool automatically grows.
		 *
		 * @return	Tracked suballocation handle (always valid)
		 */
		UPtr<TrackedGpuBufferSuballocation> AllocateTracked();

		/**
		 * Manually releases a suballocation.
		 *
		 * Only needed for non-tracked allocations from Allocate().
		 * Do NOT call this for tracked allocations - they release automatically.
		 *
		 * @param allocation	Allocation to release (must be from this pool and currently allocated)
		 */
		void Release(const GpuBufferSuballocation& allocation);

		/**
		 * Gets the size per suballocation (aligned).
		 *
		 * May be larger than the requested size due to GPU alignment requirements
		 * (typically 256 bytes for uniform buffers).
		 */
		u32 GetSuballocationSize() const { return mSuballocationSize; }

		/** Gets the number of currently allocated buffers. */
		u32 GetBufferCount() const { return (u32)mBuffers.size(); }

		/** Gets the total number of suballocations (used + free). */
		u32 GetTotalSuballocationCount() const { return (u32)mSuballocations.size(); }

	private:
		/** Entry in the suballocation pool with intrusive free-list link, tracking free suballocations. */
		struct SuballocationEntry
		{
			u32 NextFreeIndex;
		};

		/** Entry for a GPU buffer. */
		struct BufferEntry
		{
			SPtr<GpuBuffer> Buffer;
			u32 AllocatedCount; /**< Number of currently allocated suballocations. */
			u32 FreeListHead; /**< Head of this buffer's free-list (~0u = empty). */
		};

		/** Grows the pool by allocating a new GpuBuffer. */
		void AddNewBufferToPool();

	private:
		GpuDevice* mDevice = nullptr;
		GpuBufferCreateInformation mBufferCreateInformation;

		u32 mSuballocationSize = 0;
		u32 mSuballocationsPerBuffer = 0;

		TInlineArray<SuballocationEntry, 64> mSuballocations;
		TInlineArray<BufferEntry, 1> mBuffers;
	};
}
