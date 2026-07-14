//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DPool.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Thread-safe pool of linear-allocator pages, meant to be shared between multiple TGpuLinearAllocators
	 * (usually each thread has its own linear allocator).
	 *
	 * All pooled pages are exactly @ref Configuration::PageSize bytes and share the same
	 * @ref Configuration::HeapCreateInfo. Oversize allocations never enter the pool —
	 * they stay allocator-local one-shot heaps.
	 *
	 * **GPU-safety.** The pool assumes every released page is GPU-safe and immediately reusable. The
	 * allocator upholds this: a page reaches @ref ReleasePage only at *drain* time — either a
	 * fence-verified drain (the retiring context's completion marker has signaled) or a teardown drain,
	 * where the allocator contract requires the caller to have already waited for in-flight work.
	 * Pages never submitted to the GPU are trivially safe as well.
	 *
	 * **Bound.** At most @ref Configuration::MaxRetainedPages drained pages are retained; further
	 * releases are destroyed immediately. The bound must be sized for the peak number of concurrent
	 * contexts or they fall back to fresh CreateHeap calls.
	 *
	 * @tparam HeapBackend		Backend trait satisfying the GpuHeapBackend contract.
	 */
	template <typename HeapBackend>
	class TGpuLinearPagePool
	{
	public:
		B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(HeapBackend);

		using HeapHandle = typename HeapBackend::HeapHandle;
		using HeapCreateInformation = typename HeapBackend::HeapCreateInformation;

		/** Runtime configuration shared by every page the pool hands out. */
		struct Configuration
		{
			/** Size in bytes of every pooled page. */
			u64 PageSize = 8ull * 1024 * 1024;

			/** Maximum number of drained pages retained as warm spares; further releases are destroyed. */
			u32 MaxRetainedPages = 8;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap for each page. */
			HeapCreateInformation HeapCreateInfo{};
		};

		TGpuLinearPagePool(HeapBackend* backend, const Configuration& configuration)
			: mBackend(backend), mConfig(configuration)
		{
			B3D_ASSERT(backend != nullptr);
			B3D_ASSERT(mConfig.PageSize > 0);
		}

		~TGpuLinearPagePool()
		{
			for (HeapHandle& handle : mSparePages)
				mBackend->DestroyHeap(handle);

			mSparePages.clear();
		}

		// Non-copyable — owns backend heaps.
		TGpuLinearPagePool(const TGpuLinearPagePool&) = delete;
		TGpuLinearPagePool& operator=(const TGpuLinearPagePool&) = delete;

		/** Page size every acquired/released page is expected to be. */
		u64 GetPageSize() const { return mConfig.PageSize; }

		/**
		 * Returns a warm spare page if one is available, otherwise creates a fresh @ref GetPageSize
		 * byte heap. The returned heap is owned by the caller until handed back via @ref ReleasePage.
		 */
		HeapHandle AcquirePage()
		{
			Lock lock(mMutex);
			if (!mSparePages.empty())
			{
				const HeapHandle handle = mSparePages.back();
				mSparePages.pop_back();
				return handle;
			}

			lock.unlock();
			return mBackend->CreateHeap(mConfig.PageSize, mConfig.HeapCreateInfo);
		}

		/**
		 * Returns a drained, GPU-safe page to the pool. Retained as a warm spare while the pool is
		 * below @ref Configuration::MaxRetainedPages, otherwise destroyed immediately. The caller must
		 * guarantee the page is no longer in flight (see class docs).
		 */
		void ReleasePage(HeapHandle handle)
		{
			if (handle == nullptr)
				return;

			Lock lock(mMutex);
			if ((u32)mSparePages.size() < mConfig.MaxRetainedPages)
			{
				mSparePages.push_back(handle);
				return;
			}

			lock.unlock();
			mBackend->DestroyHeap(handle);
		}

		/** @name Diagnostics.
		 *  @{
		 */

		/** Number of drained pages currently held warm, ready for immediate reuse. */
		u32 GetSparePageCount() const
		{
			Lock lock(mMutex);
			return (u32)mSparePages.size();
		}

		/** Total bytes of backing memory currently held as warm spares. */
		u64 GetCommittedBytes() const
		{
			Lock lock(mMutex);
			return (u64)mSparePages.size() * mConfig.PageSize;
		}

		/** @} */

	private:
		HeapBackend* mBackend = nullptr;
		Configuration mConfig;
		Vector<HeapHandle> mSparePages;
		mutable Mutex mMutex;
	};

	/**
	 * Linear (bump) GPU memory allocator for transient allocations such as staging buffers and
	 * one-off scratch buffers. Allocations are produced by bumping a per-page offset; per-allocation
	 * Free is a no-op. Whole pages recycle once their retire fence completes — at which point the
	 * page returns to its page source (a shared @ref TGpuLinearPagePool when one is supplied, otherwise
	 * an internal spare list bounded by Configuration::MaxRetainedPages) or is destroyed.
	 *
	 * **Page source.** When constructed with a non-null @c pagePool the allocator draws normal pages from
	 * (and returns them to) that thread-safe pool, so many single-threaded allocators can recycle a
	 * shared, bounded set of heaps.
	 *
	 * **Page lifecycle.** Three kinds of pages coexist:
	 *  - Active page (at most one): the page currently being bumped from.
	 *  - Retired pages: pages rotated out earlier in the frame because they filled up, or because
	 *    @ref FreeAll was called. Their existing allocations are still being read by the GPU, so the
	 *    memory cannot be reused yet — the page sits in the base's deferred-free queue stamped
	 *    against the frame index that retired it. Slot indices in mPages stay live so the
	 *    page-index stored in Location::AllocatorData0 keeps resolving.
	 *  - Spare pages: pages whose retire fence has completed (drained by ReclaimUnused). Bump offset is
	 *    reset to zero, GPU-safe to overwrite, eligible for immediate reuse on the next overflow.
	 *
	 * Within a single frame the allocator can grow without bound — every overflow just retires the
	 * current page and acquires a fresh one from the spares list (or via HeapBackend::CreateHeap
	 * if no spare is available).
	 *
	 * **Oversize allocations.** A request larger than Configuration::PageSize allocates a
	 * dedicated one-shot heap sized exactly to the request, emits the location into it, then
	 * immediately retires that heap. Oversize heaps never re-enter the spare list; @ref FreeAndReclaimImpl
	 * destroys them outright once their fence completes.
	 *
	 * **Free model.** Both @ref FreeImpl (deferred Free) and the per-allocation FreeAndReclaim path
	 * are no-ops apart from the base-driven Location::Reset. A page is shared by every allocation
	 * that fit into it, so a single Location can't reclaim the page without invalidating its peers.
	 * Reclaim is meaningful at the page level only — pages are retired implicitly on overflow or
	 * explicitly via @ref FreeAll, and the actual recycling runs from the deferred-free drain after
	 * the retired page's fence completes.
	 *
	 * **Threading.** This allocator is always thread-unsafe: it performs no internal locking and the
	 * caller is responsible for external synchronization. A single allocator is intended to be owned and
	 * driven by one thread/operation at a time; cross-thread page recycling goes through the thread-safe
	 * @ref TGpuLinearPagePool, not through a shared allocator.
	 *
	 * @tparam HeapBackend		Backend trait satisfying the GpuHeapBackend contract.
	 *
	 * @see TGpuAllocator
	 */
	template <typename HeapBackend>
	class TGpuLinearAllocator : public TGpuAllocator<TGpuLinearAllocator<HeapBackend>, HeapBackend, ThreadUnsafe>
	{
	public:
		using Base = TGpuAllocator<TGpuLinearAllocator<HeapBackend>, HeapBackend, ThreadUnsafe>;
		using HeapHandle = typename HeapBackend::HeapHandle;
		using PagePool = TGpuLinearPagePool<HeapBackend>;

		/** Runtime configuration for the allocator. */
		struct Configuration
		{
			/**
			 * Default backing-heap size (one page == one heap). When a shared pool is supplied, the pool's
			 * page size governs normal pages and this is used only for the oversize threshold; they should
			 * match.
			 */
			u64 PageSize = 4ull * 1024 * 1024;

			/**
			 * Number of fence-completed normal pages to retain as warm spares before destroying further
			 * drained pages. Oversize pages never enter the spare list regardless of this setting. Ignored
			 * when a shared pool is supplied (the pool owns the retention bound).
			 */
			u32 MaxRetainedPages = 2;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap for each page. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		/**
		 * @param backend			Heap backend; must outlive this allocator.
		 * @param completionTracker	Determines when pages can be safely retired (i.e. GPU is done with them); must be non-null.
		 * @param configuration		Page size (oversize threshold), oversize heap create-info, internal bound.
		 * @param pagePool			Optional shared page source for normal pages. When null the allocator keeps its own internal spare list.
		 */
		TGpuLinearAllocator(HeapBackend* backend, IGpuCompletionTracker* completionTracker, const Configuration& configuration, PagePool* pagePool = nullptr);
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
		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuResourceLocation& out);

		/**
		 * No-op apart from the base-driven Location::Reset. Linear allocations don't track
		 * per-allocation lifetime; the page is the unit of recycling.
		 */
		void FreeImpl(GpuResourceLocation& allocation);

		/**
		 * Reached from two different paths, distinguished by @p reclaimKind:
		 *  - @p kReclaimAllocation (per-allocation FreeAndReclaim): no-op. A page is shared by every
		 *    allocation that fit into it, so a single Location can't release the page without
		 *    invalidating its peers.
		 *  - @p kReclaimPage (page-retirement drain): returns the page to the spare list when it's a
		 *    normal page and the spare list isn't full; otherwise destroys it via HeapBackend::DestroyHeap.
		 *    Oversize pages always destruct.
		 */
		void FreeAndReclaimImpl(u32 pageIndex, u32 reclaimKind);

		/** @} */

		/**
		 * Frees every live allocation by retiring the active page against the current frame index. The
		 * active page becomes invalid; the next TryAllocate acquires a fresh one. Use at end-of-frame to
		 * bound page lifetime. No-op if there is no active page. The retired pages only become reusable
		 * once their fence completes and ReclaimUnused drains them.
		 */
		void FreeAll() override;

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
		 * entry) so @p FreeAndReclaimImpl can tell whether it was reached via the per-allocation
		 * @p FreeAndReclaim / @p Free path or via the page-retirement drain. The two paths want completely
		 * different behavior — see @p FreeAndReclaimImpl.
		 */
		static constexpr u32 kReclaimAllocation = 0; /**< Per-allocation reclaim — no-op for linear. */
		static constexpr u32 kReclaimPage = 1;       /**< Page-retirement drain — recycle the whole page. */

		/** Effective normal-page size: the shared pool's when present, otherwise the configured size. */
		u64 GetPageSize() const { return mPagePool != nullptr ? mPagePool->GetPageSize() : mConfig.PageSize; }

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
		u32 AcquirePage();

		/** Allocates a fresh dedicated heap of @p size bytes; never enters the spare list. */
		u32 CreateOversizedPage(u64 size);

		/** Stamps @p pageIndex into the base's deferred-free queue against the current frame index. */
		void RetirePage(u32 pageIndex);

		/** Synchronously destroys the page at @p pageIndex and vacates its slot. */
		void DestroyPage(u32 pageIndex);

		/** Inserts @p page into mPages, reusing a vacated slot if available. */
		u32 InsertIntoPageTable(Page* page);

		Configuration mConfig;
		PagePool* mPagePool = nullptr;    /**< Shared page source; null means use the internal spare list. */
		TPool<Page> mPageStorage;         /**< Backing store for Page structs; recycles slots so acquire/drain doesn't churn heap allocations. */
		Vector<Page*> mPages;             /**< Indexed; FreeAndReclaimImpl uses these indices. nullptr for vacated slots. */
		Vector<u32> mSparePages;          /**< Drained pages held warm; popped on AcquireNormalPage. Only relevant when mPagePool is null. */
		u32 mCurrentPageIndex = kInvalidPageIndex;
	};

	template <typename HeapBackend>
	TGpuLinearAllocator<HeapBackend>::TGpuLinearAllocator(HeapBackend* backend, IGpuCompletionTracker* completionTracker, const Configuration& configuration, PagePool* pagePool)
		: Base(backend, completionTracker), mConfig(configuration), mPagePool(pagePool)
	{
		B3D_ASSERT(mConfig.PageSize > 0);
		B3D_ASSERT(completionTracker != nullptr); // Linear pages always retire against a completion marker.
	}

	template <typename HeapBackend>
	TGpuLinearAllocator<HeapBackend>::~TGpuLinearAllocator()
	{
		// Drain unconditionally — any submissions still in flight at destructor time are the caller's
		// responsibility to wait for via WaitUntilIdle, matching the convention from TGpuAllocator.
		Base::ReclaimUnused(true);

		for (u32 pageIndex = 0; pageIndex < (u32)mPages.size(); pageIndex++)
		{
			if (mPages[pageIndex] != nullptr)
			{
				Base::mBackend->DestroyHeap(mPages[pageIndex]->Handle);
				mPageStorage.Release(mPages[pageIndex]);
				mPages[pageIndex] = nullptr;
			}
		}
	}

	template <typename HeapBackend>
	bool TGpuLinearAllocator<HeapBackend>::TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuResourceLocation& out)
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
		if (size > GetPageSize())
		{
			const u32 oversizeIndex = CreateOversizedPage(size);
			if (oversizeIndex == kInvalidPageIndex)
				return false;

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
		{
			mCurrentPageIndex = AcquirePage();
			if (mCurrentPageIndex == kInvalidPageIndex)
				return false;
		}

		Page* active = mPages[mCurrentPageIndex];
		u64 alignedOffset = AlignUp(active->BumpOffset, alignment);

		// Overflow: retire the active page (it stays alive in the deferred-free queue, holding the
		// allocations the GPU is still reading) and acquire a fresh one. The fresh page is empty by
		// construction so the bump must succeed.
		if (alignedOffset + size > active->Size)
		{
			RetirePage(mCurrentPageIndex);
			mCurrentPageIndex = AcquirePage();
			if (mCurrentPageIndex == kInvalidPageIndex)
				return false;

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

	template <typename HeapBackend>
	void TGpuLinearAllocator<HeapBackend>::FreeImpl(GpuResourceLocation& allocation)
	{
		// Per-allocation Free is a no-op for the linear allocator. Pages recycle as a whole when they
		// fill up or when Reset is called; individual allocations never reclaim space. The base wraps
		// this call with allocation.Reset() so the caller's Location is invalidated as expected.
		(void)allocation;
	}

	template <typename HeapBackend>
	void TGpuLinearAllocator<HeapBackend>::FreeAndReclaimImpl(u32 pageIndex, u32 reclaimKind)
	{
		// Per-allocation FreeAndReclaim is a no-op for the linear allocator: a page is shared by every
		// allocation that fit into it, so a single Location can't release the page without invalidating
		// its peers. Pages recycle as a whole via the page-retirement drain (kReclaimPage), which is the
		// only path that should actually return memory.
		if (reclaimKind == kReclaimAllocation)
			return;

		B3D_ASSERT(reclaimKind == kReclaimPage);
		B3D_ASSERT(pageIndex < (u32)mPages.size());
		Page* page = mPages[pageIndex];
		B3D_ASSERT(page != nullptr);

		// Oversize pages are dedicated one-shot heaps; they never re-enter any spare list.
		if (page->Oversize)
		{
			DestroyPage(pageIndex);
			return;
		}

		if (mPagePool != nullptr)
		{
			// Hand the GPU-safe heap back to the shared pool and vacate our slot
			mPagePool->ReleasePage(page->Handle);
			mPages[pageIndex] = nullptr;
			mPageStorage.Release(page);
			return;
		}

		// Internal spare list: park the page for reuse while under the retention bound, else destroy it.
		if (mSparePages.size() >= mConfig.MaxRetainedPages)
		{
			DestroyPage(pageIndex);
			return;
		}

		page->BumpOffset = 0;
		mSparePages.push_back(pageIndex);
	}

	template <typename HeapBackend>
	void TGpuLinearAllocator<HeapBackend>::FreeAll()
	{
		if (mCurrentPageIndex == kInvalidPageIndex)
			return;

		RetirePage(mCurrentPageIndex);
		mCurrentPageIndex = kInvalidPageIndex;
	}

	template <typename HeapBackend>
	u64 TGpuLinearAllocator<HeapBackend>::GetCommittedBytes() const
	{
		u64 total = 0;
		for (Page* page : mPages)
		{
			if (page != nullptr)
				total += page->Size;
		}

		return total;
	}

	template <typename HeapBackend>
	u64 TGpuLinearAllocator<HeapBackend>::GetUsedBytes() const
	{
		if (mCurrentPageIndex == kInvalidPageIndex)
			return 0;

		return mPages[mCurrentPageIndex]->BumpOffset;
	}

	template <typename HeapBackend>
	u32 TGpuLinearAllocator<HeapBackend>::GetSparePageCount() const
	{
		return (u32)mSparePages.size();
	}

	template <typename HeapBackend>
	u32 TGpuLinearAllocator<HeapBackend>::GetLivePageCount() const
	{
		u32 count = 0;
		for (Page* page : mPages)
		{
			if (page != nullptr)
				count++;
		}

		return count;
	}

	template <typename HeapBackend>
	u32 TGpuLinearAllocator<HeapBackend>::AcquirePage()
	{
		if (mPagePool != nullptr)
		{
			// The pool owns the GPU heap; mPageStorage owns the Page struct (recycling its slot).
			Page* page = mPageStorage.Allocate();
			page->Handle = mPagePool->AcquirePage();
			if (page->Handle == nullptr)
			{
				mPageStorage.Release(page);
				return kInvalidPageIndex;
			}

			page->Size = mPagePool->GetPageSize();
			page->BumpOffset = 0;
			page->Oversize = false;

			return InsertIntoPageTable(page);
		}

		if (!mSparePages.empty())
		{
			const u32 spareIndex = mSparePages.back();
			mSparePages.pop_back();
			return spareIndex;
		}

		Page* page = mPageStorage.Allocate();
		page->Handle = Base::mBackend->CreateHeap(mConfig.PageSize, mConfig.HeapCreateInfo);
		if (page->Handle == nullptr)
		{
			mPageStorage.Release(page);
			return kInvalidPageIndex;
		}

		page->Size = mConfig.PageSize;
		page->BumpOffset = 0;
		page->Oversize = false;

		return InsertIntoPageTable(page);
	}

	template <typename HeapBackend>
	u32 TGpuLinearAllocator<HeapBackend>::CreateOversizedPage(u64 size)
	{
		Page* page = mPageStorage.Allocate();
		page->Handle = Base::mBackend->CreateHeap(size, mConfig.HeapCreateInfo);
		if (page->Handle == nullptr)
		{
			mPageStorage.Release(page);
			return kInvalidPageIndex;
		}

		page->Size = size;
		page->BumpOffset = 0;
		page->Oversize = true;

		return InsertIntoPageTable(page);
	}

	template <typename HeapBackend>
	void TGpuLinearAllocator<HeapBackend>::RetirePage(u32 pageIndex)
	{
		B3D_ASSERT(pageIndex < (u32)mPages.size());
		B3D_ASSERT(mPages[pageIndex] != nullptr);

		GpuResourceLocation snapshot;
		snapshot.Allocator = this;
		snapshot.AllocatorData0 = pageIndex;
		snapshot.AllocatorData1 = kReclaimPage;

		Base::RetireAllocation(snapshot);
	}

	template <typename HeapBackend>
	void TGpuLinearAllocator<HeapBackend>::DestroyPage(u32 pageIndex)
	{
		Page* page = mPages[pageIndex];
		B3D_ASSERT(page != nullptr);

		Base::mBackend->DestroyHeap(page->Handle);
		mPageStorage.Release(page);
		mPages[pageIndex] = nullptr;
	}

	template <typename HeapBackend>
	u32 TGpuLinearAllocator<HeapBackend>::InsertIntoPageTable(Page* page)
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
