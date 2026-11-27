//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuVirtualMemory.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Threading/B3DThreading.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Size of each heap of a page pool (see IGpuPagePool), in megabytes, rounded up to a whole number of pages. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuVirtualMemoryPageHeapSize;

	/**
	 * Number of frames a heap of a page pool without allocated pages may go unused before it is destroyed. A heap is never destroyed before
	 * the GPU finishes the last frame that used it. Heaps the pool keeps as spares (TGpuPagePool::Configuration::MaxEmptyHeapCount) are
	 * never destroyed. 0 by default.
	 */
	extern B3D_EXPORT TConfigVariable<u32> gGpuVirtualMemoryPageHeapIdleFrames;

	/** Memory use of one page pool. */
	struct GpuPagePoolStatistics
	{
		u32 HeapCount = 0; /**< Number of heaps the pool currently owns. */
		u32 PageCount = 0; /**< Number of pages of the heaps. */
		u32 AllocatedPageCount = 0; /**< Number of pages allocated, or freed but not yet available again. */
	};

	/**
	 * Memory for virtual buffers and textures. Owns heaps split into pages of kGpuPageSize bytes, which GpuPageTable commits to the
	 * pages of virtual resources. Heaps are created as pages are needed and destroyed once unused.
	 *
	 * @note	Thread safe.
	 */
	class IGpuPagePool
	{
	public:
		virtual ~IGpuPagePool() = default;

		/**
		 * Allocates @p pageCount pages in as few consecutive runs as it can, and appends them to @p outAllocations. Returns false with
		 * nothing allocated if the memory is not available.
		 */
		virtual bool TryAllocate(u32 pageCount, TArray<GpuPageAllocation>& outAllocations) = 0;

		/**
		 * Returns the pages of @p allocation to the pool once the GPU has finished the work submitted so far. May be any part of the pages an
		 * earlier TryAllocate() returned.
		 */
		virtual void Free(const GpuPageAllocation& allocation) = 0;

		/**
		 * Returns the pages of @p allocation to the pool at once, for pages no submitted work uses. Page mappings that still map virtual
		 * resources to the pages, or take them away, must be queued before the pages are returned, so they execute before the mappings of
		 * whoever allocates the pages next, and must execute before the current frame of the pool's completion tracker completes, since the
		 * pool may destroy the pages' heap after that. See Free().
		 */
		virtual void FreeAndReclaim(const GpuPageAllocation& allocation) = 0;

		/**
		 * Makes pages returned by Free() available once the GPU has finished the work submitted before, and destroys heaps without allocated
		 * pages that went unused for gGpuVirtualMemoryPageHeapIdleFrames frames, once the GPU has finished the last frame that used them. A
		 * pool may keep some such heaps as spares. Frames are counted by the pool's completion tracker.
		 */
		virtual void ReclaimUnused() = 0;

		/** Returns the memory use of the pool. */
		virtual GpuPagePoolStatistics GetStatistics() const = 0;
	};

	/**
	 * Page pool over heaps created with @p HeapBackend. Requests are met by the smallest free run that holds all pages if there is one,
	 * otherwise by the longest free runs in turn, adding heaps for what the free runs lack.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 */
	template <typename HeapBackend>
	class TGpuPagePool final : public IGpuPagePool
	{
	public:
		B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(HeapBackend);

		using HeapHandle = typename HeapBackend::HeapHandle;

		/** Runtime configuration for the pool. */
		struct Configuration
		{
			/** Passed to HeapBackend::CreateHeap() for every heap. */
			typename HeapBackend::HeapCreateInformation HeapCreateInformation;

			/**
			 * Determines when the GPU has finished using freed pages and unused heaps. Its marker is the index of the frame. Must outlive
			 * the pool.
			 */
			IGpuCompletionTracker* CompletionTracker = nullptr;

			/**
			 * Number of heaps without allocated pages the pool keeps as spares instead of destroying them, so that use that hovers around a
			 * heap boundary doesn't create and destroy heaps repeatedly.
			 */
			u32 MaxEmptyHeapCount = 1;
		};

		/** Constructs the pool. @p backend must outlive the pool. */
		TGpuPagePool(HeapBackend* backend, const Configuration& configuration);

		/** Destroys all heaps. The GPU must have finished using them. */
		~TGpuPagePool() override;

		TGpuPagePool(const TGpuPagePool&) = delete;
		TGpuPagePool& operator=(const TGpuPagePool&) = delete;

		/** @name IGpuPagePool
		 *  @{
		 */

		bool TryAllocate(u32 pageCount, TArray<GpuPageAllocation>& outAllocations) override;
		void Free(const GpuPageAllocation& allocation) override;
		void FreeAndReclaim(const GpuPageAllocation& allocation) override;
		void ReclaimUnused() override;
		GpuPagePoolStatistics GetStatistics() const override;

		/** @} */

	private:
		/** Heap of the pool and its free pages. */
		struct Heap
		{
			HeapHandle Handle = nullptr;
			u32 PageCount = 0;
			u32 FreePageCount = 0; /**< Pages in FreeRuns. */
			u32 PendingFreePageCount = 0; /**< Pages returned by Free() that are not available yet. */
			u64 LastUsedFrame = 0; /**< Last frame in which the GPU may use the heap: it had pages that were not free, or pages were reclaimed. */
			TArray<GpuPageRange> FreeRuns; /**< Sorted by first page, never adjacent to each other. */
		};

		/** Pages returned by Free(), waiting for the GPU to finish the work submitted before. */
		struct PendingFree
		{
			PendingFree(const GpuPageAllocation& allocation, u64 marker)
				: Allocation(allocation), Marker(marker)
			{ }

			GpuPageAllocation Allocation;
			u64 Marker;
		};

		/** Returns the heap of the pool with @p handle. Must be called with mMutex locked. */
		Heap& FindHeap(IGpuHeap* handle);

		/** Removes @p pages, which start at the first page of a free run, from the free runs of @p heap. Must be called with mMutex locked. */
		void TakeFreePages(Heap& heap, const GpuPageRange& pages);

		/** Adds @p pages to the free runs of @p heap, merging adjacent runs. Must be called with mMutex locked. */
		void AddFreePages(Heap& heap, const GpuPageRange& pages);

		HeapBackend* mBackend = nullptr;
		Configuration mConfiguration;

		TArray<Heap> mHeaps;
		TArray<PendingFree> mPendingFrees;
		mutable Mutex mMutex;
	};

	/** @} */
} // namespace b3d

#include "GpuBackend/Allocators/B3DGpuPagePool.inl"
