//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Utility/B3DBitwise.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Linear (bump) GPU memory allocator for transient allocations such as staging buffers and
	 * one-off scratch buffers. Allocations are produced by bumping a per-page offset; per-allocation
	 * Free is a no-op. Whole pages recycle once their retire fence completes — at which point the
	 * page returns to a spare list (up to Configuration::MaxRetainedPages) or is destroyed.
	 *
	 * **Page lifecycle.** Three kinds of pages coexist:
	 *  - Active page (at most one): the page currently being bumped from.
	 *  - Retired pages: pages rotated out earlier in the frame because they filled up, or because
	 *    @ref Reset was called. Their existing allocations are still being read by the GPU, so the
	 *    memory cannot be reused yet — the page sits in the base's deferred-free queue stamped
	 *    against the frame index that retired it. Slot indices in mPages stay live so the
	 *    page-index stored in Location::AllocatorData0 keeps resolving.
	 *  - Spare pages: pages whose retire fence has completed (drained by Flush). Bump offset is
	 *    reset to zero, GPU-safe to overwrite, eligible for immediate reuse on the next overflow.
	 *
	 * Within a single frame the allocator can grow without bound — every overflow just retires the
	 * current page and acquires a fresh one from the spares list (or via HeapBackend::CreateHeap
	 * if no spare is available).
	 *
	 * **Oversize allocations.** A request larger than Configuration::PageSize allocates a
	 * dedicated one-shot heap sized exactly to the request, emits the location into it, then
	 * immediately retires that heap. Oversize heaps never re-enter the spare list; @ref FreeImmediateImpl
	 * destroys them outright once their fence completes.
	 *
	 * **Free model.** Both @ref FreeImpl (deferred Free) and the per-allocation FreeImmediate path
	 * are no-ops apart from the base-driven Location::Reset. A page is shared by every allocation
	 * that fit into it, so a single Location can't reclaim the page without invalidating its peers.
	 * Reclaim is meaningful at the page level only — pages are retired implicitly on overflow or
	 * explicitly via @ref Reset, and the actual recycling runs from the deferred-free drain after
	 * the retired page's fence completes.
	 *
	 * **Threading.** When @p ThreadPolicy is ThreadSafe (the default), every public entry point
	 * acquires the allocator-wide mutex inherited from TGpuAllocator. When @p ThreadPolicy is
	 * ThreadUnsafe, locking compiles out and the caller is responsible for external synchronization.
	 *
	 * @tparam HeapBackend		Backend trait satisfying the GpuHeapBackend contract.
	 * @tparam ThreadPolicy		Compile-time thread-safety policy.
	 *
	 * @see TGpuAllocator
	 */
	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy = ThreadSafe>
	class TGpuLinearAllocator : public TGpuAllocator<TGpuLinearAllocator<HeapBackend, ThreadPolicy>, HeapBackend, ThreadPolicy>
	{
	public:
		using Base = TGpuAllocator<TGpuLinearAllocator<HeapBackend, ThreadPolicy>, HeapBackend, ThreadPolicy>;
		using Location = typename Base::Location;
		using HeapHandle = typename HeapBackend::HeapHandle;

		/** Runtime configuration for the allocator. */
		struct Configuration
		{
			/** Default backing-heap size (one page == one heap). */
			u64 PageSize = 4ull * 1024 * 1024;

			/**
			 * Number of fence-completed normal pages to retain as warm spares before destroying further
			 * drained pages. Oversize pages never enter the spare list regardless of this setting.
			 */
			u32 MaxRetainedPages = 2;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap for each page. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		TGpuLinearAllocator(HeapBackend* backend, IGpuFrameTracker* frameTracker, const Configuration& configuration);
		~TGpuLinearAllocator();

		// Non-copyable — page state is not safe to duplicate.
		TGpuLinearAllocator(const TGpuLinearAllocator&) = delete;
		TGpuLinearAllocator& operator=(const TGpuLinearAllocator&) = delete;

		/** @name TGpuAllocator CRTP surface.
		 *  @{
		 */

		/**
		 * Bumps the active page by @p size bytes (with @p alignment) and writes the resulting slot
		 * to @p out. Rotates pages on overflow and creates a dedicated one-shot heap for oversize
		 * requests. @p kind must be GpuResourceKind::Linear and @p owner must be null — linear
		 * allocations don't participate in defragmentation.
		 */
		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out);

		/**
		 * No-op apart from the base-driven Location::Reset. Linear allocations don't track
		 * per-allocation lifetime; the page is the unit of recycling.
		 */
		void FreeImpl(Location& allocation);

		/**
		 * Reached from two different paths, distinguished by @p reclaimKind:
		 *  - @p kReclaimAllocation (per-allocation FreeImmediate): no-op. A page is shared by every
		 *    allocation that fit into it, so a single Location can't release the page without
		 *    invalidating its peers.
		 *  - @p kReclaimPage (page-retirement drain): returns the page to the spare list when it's a
		 *    normal page and the spare list isn't full; otherwise destroys it via HeapBackend::DestroyHeap.
		 *    Oversize pages always destruct.
		 */
		void FreeImmediateImpl(u32 pageIndex, u32 reclaimKind);

		/** @} */

		/**
		 * Retires the active page against the current frame index. The active page becomes invalid;
		 * the next TryAllocate acquires a fresh one. Use at end-of-frame to bound page lifetime.
		 * No-op if there is no active page.
		 */
		void Reset();

		/** @name Diagnostics.
		 *  @{
		 */

		/** Sum of all live page sizes (active + retired-pending-drain + spares + oversize-pending-drain). */
		u64 GetCommittedBytes() const;

		/** Active-page bump offset. Retired pages are full-by-definition and not double-counted here. */
		u64 GetUsedBytes() const;

		/** Number of drained pages currently held on the spare list, ready for immediate reuse. */
		u32 GetSparePageCount() const;

		/** Number of populated slots in the page table (active + retired-pending-drain + spares + oversize-pending-drain). */
		u32 GetLivePageCount() const;

		/** @} */

	private:
		/** Sentinel index for the "no active page" state. */
		static constexpr u32 kInvalidPageIndex = 0xFFFFFFFFu;

		/**
		 * Discriminator stored in @p Location::AllocatorData1 (and in the matching deferred-free queue
		 * entry) so @p FreeImmediateImpl can tell whether it was reached via the per-allocation
		 * @p FreeImmediate / @p Free path or via the page-retirement drain. The two paths want completely
		 * different behavior — see @p FreeImmediateImpl.
		 */
		static constexpr u32 kReclaimAllocation = 0; /**< Per-allocation reclaim — no-op for linear. */
		static constexpr u32 kReclaimPage = 1;       /**< Page-retirement drain — recycle the whole page. */

		/** Round @p value up to the next multiple of @p alignment, which must be a power of two. */
		static u64 AlignUp(u64 value, u32 alignment)
		{
			const u64 mask = (u64)alignment - 1;
			return (value + mask) & ~mask;
		}

		/** One backend heap. Lives in mPages until its fence drains and the slot is vacated. */
		struct Page
		{
			HeapHandle Handle{};
			u64 Size = 0;
			u64 BumpOffset = 0;
			bool Oversize = false; /**< Dedicated one-shot heap; never goes to spares. */
		};

		/** Acquires a normal page (from spares if available, otherwise a fresh CreateHeap). */
		u32 AcquireNormalPage();

		/** Allocates a fresh dedicated heap of @p size bytes; never enters the spare list. */
		u32 CreateOversizePage(u64 size);

		/** Stamps @p pageIndex into the base's deferred-free queue against the current frame index. */
		void RetirePage(u32 pageIndex);

		/** Synchronously destroys the page at @p pageIndex and vacates its slot. */
		void DestroyPage(u32 pageIndex);

		/** Inserts @p page into mPages, reusing a vacated slot if available. */
		u32 InsertIntoPageTable(Page* page);

		Configuration mConfig;
		Vector<Page*> mPages;             /**< Indexed; FreeImmediateImpl uses these indices. nullptr for vacated slots. */
		Vector<u32> mSparePages;          /**< Drained pages held warm; popped on AcquireNormalPage. */
		u32 mCurrentPageIndex = kInvalidPageIndex;
	};

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TGpuLinearAllocator<HeapBackend, ThreadPolicy>::TGpuLinearAllocator(HeapBackend* backend, IGpuFrameTracker* frameTracker, const Configuration& configuration)
		: Base(backend, frameTracker), mConfig(configuration)
	{
		B3D_ASSERT(mConfig.PageSize > 0);
		B3D_ASSERT(frameTracker != nullptr); // Linear pages always retire against a frame fence.
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TGpuLinearAllocator<HeapBackend, ThreadPolicy>::~TGpuLinearAllocator()
	{
		// Drain unconditionally — any submissions still in flight at destructor time are the caller's
		// responsibility to wait for via WaitUntilIdle, matching the convention from TGpuAllocator.
		Base::Flush(true);

		for (u32 pageIndex = 0; pageIndex < (u32)mPages.size(); pageIndex++)
		{
			if (mPages[pageIndex] != nullptr)
			{
				Base::mBackend->DestroyHeap(mPages[pageIndex]->Handle);
				B3DDelete(mPages[pageIndex]);
				mPages[pageIndex] = nullptr;
			}
		}
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TGpuLinearAllocator<HeapBackend, ThreadPolicy>::TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out)
	{
		B3D_ASSERT(out.Allocator == nullptr);
		B3D_ASSERT(alignment > 0);
		B3D_ASSERT(Bitwise::IsPow2(alignment));
		B3D_ASSERT(kind == GpuResourceKind::Linear); // Linear pages are buffer-only by convention.
		B3D_ASSERT(owner == nullptr); // Linear allocations don't participate in defrag.
		(void)kind;
		(void)owner;

		// Oversize: dedicated one-shot heap, retired immediately. Active page is left untouched so the
		// next regular request keeps bumping from where it was.
		if (size > mConfig.PageSize)
		{
			const u32 oversizeIndex = CreateOversizePage(size);
			Page* oversize = mPages[oversizeIndex];
			oversize->BumpOffset = size;

			out.Heap = oversize->Handle;
			out.Offset = 0;
			out.Size = size;
			out.Allocator = this;
			out.AllocatorData0 = oversizeIndex;
			out.AllocatorData1 = kReclaimAllocation;

			RetirePage(oversizeIndex);

			return true;
		}

		// Acquire an active page on first allocate.
		if (mCurrentPageIndex == kInvalidPageIndex)
			mCurrentPageIndex = AcquireNormalPage();

		Page* active = mPages[mCurrentPageIndex];
		u64 alignedOffset = AlignUp(active->BumpOffset, alignment);

		// Overflow: retire the active page (it stays alive in the deferred-free queue, holding the
		// allocations the GPU is still reading) and acquire a fresh one. The fresh page is empty by
		// construction so the bump must succeed.
		if (alignedOffset + size > active->Size)
		{
			RetirePage(mCurrentPageIndex);
			mCurrentPageIndex = AcquireNormalPage();
			active = mPages[mCurrentPageIndex];
			alignedOffset = AlignUp(active->BumpOffset, alignment);
			B3D_ASSERT(alignedOffset + size <= active->Size);
		}

		active->BumpOffset = alignedOffset + size;

		out.Heap = active->Handle;
		out.Offset = alignedOffset;
		out.Size = size;
		out.Allocator = this;
		out.AllocatorData0 = mCurrentPageIndex;
		out.AllocatorData1 = kReclaimAllocation;

		return true;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuLinearAllocator<HeapBackend, ThreadPolicy>::FreeImpl(Location& allocation)
	{
		// Per-allocation Free is a no-op for the linear allocator. Pages recycle as a whole when they
		// fill up or when Reset is called; individual allocations never reclaim space. The base wraps
		// this call with allocation.Reset() so the caller's Location is invalidated as expected.
		(void)allocation;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuLinearAllocator<HeapBackend, ThreadPolicy>::FreeImmediateImpl(u32 pageIndex, u32 reclaimKind)
	{
		// Per-allocation FreeImmediate is a no-op for the linear allocator: a page is shared by every
		// allocation that fit into it, so a single Location can't release the page without invalidating
		// its peers. Pages recycle as a whole via the page-retirement drain (kReclaimPage), which is the
		// only path that should actually return memory.
		if (reclaimKind == kReclaimAllocation)
			return;

		B3D_ASSERT(reclaimKind == kReclaimPage);
		B3D_ASSERT(pageIndex < (u32)mPages.size());
		Page* page = mPages[pageIndex];
		B3D_ASSERT(page != nullptr);

		if (page->Oversize || mSparePages.size() >= mConfig.MaxRetainedPages)
		{
			DestroyPage(pageIndex);
			return;
		}

		// Reset for reuse and park on the spare list.
		page->BumpOffset = 0;
		mSparePages.push_back(pageIndex);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuLinearAllocator<HeapBackend, ThreadPolicy>::Reset()
	{
		typename Base::ScopedLock lock(this->GetMutex());
		if (mCurrentPageIndex == kInvalidPageIndex)
			return;

		RetirePage(mCurrentPageIndex);
		mCurrentPageIndex = kInvalidPageIndex;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u64 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::GetCommittedBytes() const
	{
		typename Base::ScopedLock lock(this->GetMutex());
		u64 total = 0;
		for (Page* page : mPages)
		{
			if (page != nullptr)
				total += page->Size;
		}

		return total;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u64 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::GetUsedBytes() const
	{
		typename Base::ScopedLock lock(this->GetMutex());
		if (mCurrentPageIndex == kInvalidPageIndex)
			return 0;

		return mPages[mCurrentPageIndex]->BumpOffset;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::GetSparePageCount() const
	{
		typename Base::ScopedLock lock(this->GetMutex());
		return (u32)mSparePages.size();
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::GetLivePageCount() const
	{
		typename Base::ScopedLock lock(this->GetMutex());
		u32 count = 0;
		for (Page* page : mPages)
		{
			if (page != nullptr)
				count++;
		}

		return count;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::AcquireNormalPage()
	{
		if (!mSparePages.empty())
		{
			const u32 spareIndex = mSparePages.back();
			mSparePages.pop_back();
			return spareIndex;
		}

		const HeapHandle handle = Base::mBackend->CreateHeap(mConfig.PageSize, mConfig.HeapCreateInfo);
		Page* page = B3DNew<Page>();
		page->Handle = handle;
		page->Size = mConfig.PageSize;
		page->Oversize = false;

		return InsertIntoPageTable(page);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::CreateOversizePage(u64 size)
	{
		const HeapHandle handle = Base::mBackend->CreateHeap(size, mConfig.HeapCreateInfo);
		Page* page = B3DNew<Page>();
		page->Handle = handle;
		page->Size = size;
		page->Oversize = true;

		return InsertIntoPageTable(page);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuLinearAllocator<HeapBackend, ThreadPolicy>::RetirePage(u32 pageIndex)
	{
		B3D_ASSERT(pageIndex < (u32)mPages.size());
		B3D_ASSERT(mPages[pageIndex] != nullptr);

		Location snapshot;
		snapshot.Allocator = this;
		snapshot.AllocatorData0 = pageIndex;
		snapshot.AllocatorData1 = kReclaimPage;

		Base::RetireAllocation(snapshot);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuLinearAllocator<HeapBackend, ThreadPolicy>::DestroyPage(u32 pageIndex)
	{
		Page* page = mPages[pageIndex];
		B3D_ASSERT(page != nullptr);

		Base::mBackend->DestroyHeap(page->Handle);
		B3DDelete(page);
		mPages[pageIndex] = nullptr;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TGpuLinearAllocator<HeapBackend, ThreadPolicy>::InsertIntoPageTable(Page* page)
	{
		// Reuse a vacated slot if one exists; otherwise grow the vector. Slot reuse keeps page indices
		// from drifting upward forever and matches the pattern used by TGpuTlsfAllocator::CreateNewHeap.
		for (u32 pageIndex = 0; pageIndex < (u32)mPages.size(); pageIndex++)
		{
			if (mPages[pageIndex] == nullptr)
			{
				mPages[pageIndex] = page;
				return pageIndex;
			}
		}

		const u32 newIndex = (u32)mPages.size();
		mPages.push_back(page);
		return newIndex;
	}

	/** @} */
} // namespace b3d
