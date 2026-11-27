//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuVirtualMemory.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	class GpuDevice;
	class IGpuCompletionTracker;
	class IGpuPagePool;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Maps the pages of a virtual buffer or texture to memory. TryCommit() gives pages memory, either from the device's page pool or from an
	 * user provided GpuPageAllocation, and Decommit() removes it.
	 *
	 * New memory is visible to command buffers submitted after the call, and to none submitted before. Its mappings execute on the GPU ahead
	 * of the next submission, which waits for them only if it uses the resource, or binds a bindless heap while the resource has a bindless
	 * slot.
	 *
	 * Decommitted memory stays mapped until the GPU finishes the current frame, since work submitted before may still use it. Until then the
	 * pages have a pending removal, and only a commit from the page pool that gives them their old memory back is accepted.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuPageTable
	{
	public:
		/** Creates the table of @p resource, a virtual buffer with @p pageCount pages. TryCommit() takes memory from @p pagePool. */
		GpuPageTable(GpuDevice& device, IGpuResource& resource, u32 pageCount, IGpuPagePool& pagePool, GpuPageTableFlags flags);

		/** Creates the table of @p resource, a virtual texture with pages laid out as @p textureLayout. TryCommit() takes memory from @p pagePool. */
		GpuPageTable(GpuDevice& device, IGpuResource& resource, const GpuVirtualTextureLayout& textureLayout, IGpuPagePool& pagePool,
			GpuPageTableFlags flags);

		/** Returns committed memory to the page pool once the GPU finishes the work submitted so far. */
		~GpuPageTable();

		GpuPageTable(const GpuPageTable&) = delete;
		GpuPageTable& operator=(const GpuPageTable&) = delete;

		/**
		 * Gives memory from the device's page pool to the pages of @p range that have none. Its contents are undefined, except on pages with a
		 * pending removal of pool memory, which get their old memory back, contents included. Returns false and changes nothing if the pool
		 * is out of memory. Pages with a pending removal of memory from a GpuPageAllocation can't be committed until the removal completes.
		 *
		 * Commit the pages a frame needs early in the frame, before recording the work that uses them, then call
		 * GpuDevice::FlushPageMappings() once. The mappings then execute while the GPU is still busy with the previous frame, instead of
		 * making the first submission that uses them wait. Group the commits under one flush, since every flush has a fixed cost regardless of
		 * how many pages it maps.
		 */
		bool TryCommit(const GpuPageRange& range);

		/**
		 * Gives @p memory to the pages starting at @p firstPageIndex, one page of the table per page of @p memory. The table doesn't own
		 * @p memory, which must stay alive until the GPU finishes the frame in which Decommit() removes it. Use it to share memory between
		 * resources. Returns false and changes nothing if a page already has memory or a pending removal. To give pages different memory,
		 * Decommit() them first; this succeeds once the GPU finishes the frame of the decommit.
		 */
		bool TryCommit(u32 firstPageIndex, const GpuPageAllocation& memory);

		/**
		 * Removes the memory of the pages of @p range. Work submitted after the call must not access them. Memory from the page pool goes
		 * back to it once the GPU finishes the current frame.
		 */
		void Decommit(const GpuPageRange& range);

		/** Returns true if every page of @p range has memory. */
		bool IsCommitted(const GpuPageRange& range) const;

		/** Returns the number of pages of the resource. */
		u32 GetPageCount() const { return mPageCount; }

		/** Returns the number of pages that have memory. */
		u32 GetCommittedPageCount() const;

		/** Returns how the pages of a virtual texture map to its mip levels, array layers and tiles. Only valid for a texture's table. */
		const GpuVirtualTextureLayout& GetTextureLayout() const { return mTextureLayout; }

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Queues the pending removals whose frame @p frameCompletionTracker reports complete, and returns their committed memory to the page
		 * pool. Returns true if removals are still pending.
		 */
		bool QueueCompletedRemovals(const IGpuCompletionTracker& frameCompletionTracker);

		/** @} */

	private:
		/** Consecutive pages of a heap, or no memory. */
		struct RunMemory
		{
			RunMemory() = default;
			RunMemory(IGpuHeap* heap, u32 firstHeapPageIndex, bool isOwned)
				: Heap(heap), FirstHeapPageIndex(firstHeapPageIndex), IsOwned(isOwned)
			{ }

			/** Returns the memory starting @p pageCount pages later. */
			RunMemory Offset(u32 pageCount) const { return Heap != nullptr ? RunMemory(Heap, FirstHeapPageIndex + pageCount, IsOwned) : RunMemory(); }

			IGpuHeap* Heap = nullptr; /**< Null for no memory. */
			u32 FirstHeapPageIndex = 0;
			bool IsOwned = false; /**< True if the table committed the memory from the page pool. */
		};

		/** Consecutive pages that have memory, or a pending removal. */
		struct Run
		{
			GpuPageRange Pages;
			RunMemory Memory; /**< Memory the GPU maps to the pages. None while HasPendingRemoval. */

			/**
			 * True if the pages were unmapped while the GPU still maps OldMemory to them. The removal is queued once frame RemovalFrame
			 * completes, and owned OldMemory then goes back to the page pool.
			 */
			bool HasPendingRemoval = false;
			RunMemory OldMemory;
			u64 RemovalFrame = 0;
		};

		/** Returns the index of the first run that ends after @p pageIndex, or the run count if none does. Must be called with mMutex locked. */
		u32 FindRunIndex(u32 pageIndex) const;

		/** Splits the run that contains @p pageIndex, unless it starts there. Must be called with mMutex locked. */
		void SplitRunAt(u32 pageIndex);

		/**
		 * Replaces runs @p firstRunIndex to @p endRunIndex (exclusive) with @p runs, which are sorted and cover no other pages, and merges
		 * them with their neighbours. Must be called with mMutex locked.
		 */
		void ReplaceRuns(u32 firstRunIndex, u32 endRunIndex, TArrayView<const Run> runs);

		/**
		 * Removes the runs from @p firstRunIndex to @p endRunIndex (exclusive) that have neither memory nor a pending removal, and merges
		 * adjacent runs with contiguous memory, including the runs on both sides of the range. Must be called with mMutex locked.
		 */
		void MergeRuns(u32 firstRunIndex, u32 endRunIndex);

		/** Returns true if a page of @p range has memory or a pending removal. Must be called with mMutex locked. */
		bool HasRunsIn(const GpuPageRange& range) const;

		/** Returns true if a run has a pending removal. Must be called with mMutex locked. */
		bool HasPendingRemovals() const { return mPendingRemovalPageCount > 0; }

		GpuDevice& mDevice;
		IGpuResource& mResource;
		IGpuPagePool& mPagePool;
		u32 mPageCount = 0;
		GpuPageTableFlags mFlags;
		GpuVirtualTextureLayout mTextureLayout; /**< See GetTextureLayout(). Only relevant for tables of a texture. */

		TArray<Run> mRuns; /**< Sorted by first page and non-overlapping. Adjacent runs with contiguous memory are merged. */
		u32 mCommittedPageCount = 0;
		u32 mPendingRemovalPageCount = 0; /**< Number of pages of runs with a pending removal. */
		mutable Mutex mMutex;
	};

	/** @} */
} // namespace b3d
