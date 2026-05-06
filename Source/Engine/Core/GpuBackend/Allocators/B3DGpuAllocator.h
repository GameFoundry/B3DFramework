//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Threading/B3DThreading.h"

#include <type_traits>

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Documentation-only specification for backend heap traits. Concrete backends satisfy this
	 * contract and allocator templates validate it with B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID.
	 *
	 * Required typedefs
	 * - HeapHandle: Backend heap handle type.
	 * - HeapCreateInformation: Backend-specific create-info struct passed to CreateHeap.
	 *
	 * Required methods
	 * - HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
	 * - void DestroyHeap(HeapHandle handle)
	 */
	class GpuHeapBackend final
	{
		GpuHeapBackend() = delete;
		~GpuHeapBackend() = delete;
	};

	namespace detail
	{
		/** Detection-idiom probe for a heap-backend requirement. */
		template <class, class = void>	struct HeapBackendHasHeapHandle : std::false_type {};
		template <class T>				struct HeapBackendHasHeapHandle<T, std::void_t<typename T::HeapHandle>> : std::true_type {};

		template <class, class = void>	struct HeapBackendHasHeapCreateInformation : std::false_type {};
		template <class T>				struct HeapBackendHasHeapCreateInformation<T, std::void_t<typename T::HeapCreateInformation>> : std::true_type {};

		template <class, class = void>	struct HeapBackendHasCreateHeap : std::false_type {};
		template <class T>				struct HeapBackendHasCreateHeap<T, std::void_t<decltype(std::declval<T&>().CreateHeap(std::declval<u64>(), std::declval<const typename T::HeapCreateInformation&>()))>> : std::true_type {};

		template <class, class = void>	struct HeapBackendHasDestroyHeap : std::false_type {};
		template <class T>				struct HeapBackendHasDestroyHeap<T, std::void_t<decltype(std::declval<T&>().DestroyHeap(std::declval<typename T::HeapHandle>()))>> : std::true_type {};

		/** Compile-time validator for heap-backend requirements. */
		template <typename T>
		struct CheckHeapBackend
		{
			static_assert(HeapBackendHasHeapHandle<T>::value, "Heap backend is missing the required typedef 'HeapHandle'.");
			static_assert(HeapBackendHasHeapCreateInformation<T>::value, "Heap backend is missing the required typedef 'HeapCreateInformation'.");
			static_assert(HeapBackendHasCreateHeap<T>::value, "Heap backend is missing 'HeapHandle CreateHeap(u64, const HeapCreateInformation&)'.");
			static_assert(HeapBackendHasDestroyHeap<T>::value, "Heap backend is missing 'void DestroyHeap(HeapHandle)'.");

			static constexpr bool kValid = true;
		};

		/**
		 * Internal mutex holder for GPU allocators. The thread-safe specialization wraps a
		 * RecursiveMutex; the thread-unsafe specialization is empty. RecursiveMutex is required
		 * because some derived strategies (e.g. defragmentation) re-enter the allocator through
		 * the IGpuResource::MoveAllocation callback, which calls SetAllocationOwner under the
		 * same lock.
		 */
		template <bool ThreadSafe>
		struct GpuAllocatorMutex { };

		template <>
		struct GpuAllocatorMutex<true>
		{
			mutable ::b3d::RecursiveMutex Mutex;
		};

		/**
		 * Internal RAII scoped lock that pairs with GpuAllocatorMutex. The thread-unsafe
		 * specialization compiles down to nothing; the thread-safe specialization holds a
		 * unique_lock on the recursive mutex for the scope of the guard.
		 */
		template <bool ThreadSafe>
		struct GpuAllocatorScopedLock
		{
			GpuAllocatorScopedLock(const GpuAllocatorMutex<ThreadSafe>&) {}
		};

		template <>
		struct GpuAllocatorScopedLock<true>
		{
			GpuAllocatorScopedLock(const GpuAllocatorMutex<true>& holder) : mLock(holder.Mutex) {}

		private:
			::b3d::RecursiveLock mLock;
		};
	} // namespace detail

	/** Compile-time assertion that @p T satisfies the GpuHeapBackend trait. */
	#define B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(T) static_assert(::b3d::detail::CheckHeapBackend<T>::kValid, "Heap backend does not satisfy the GpuHeapBackend trait.")

	/** Controls how a GPU allocator handles allocation release on Free and on defragmentation moves. */
	enum class GpuAllocatorFreeDeferralMode : u8
	{
		/**
		 * Default for backends without proper IGpuResource lifecycle wiring, and for the 
		 * linear / transient allocators. Free queues the allocation against the current frame index; 
		 * the queue drains when IFrameTracker::IsFrameComplete(frameIndex) is true. Defrag
		 * retires the source slot the same way. Requires a non-null IGpuFrameTracker.
		 */
		FrameTracker,

		/**
		 * Used by backends that fully implement IGpuResource::Notify* events. Free routes straight to 
		 * FreeImmediateImpl - as its assumed the caller will free memory only after the lifecycle
		 * reports the resource is no longer being used on the GPU. Similarly, defragment operation
		 * will not free memory internally, it relies on the tracking to release the resource from
		 * the old memory location.
		 */
		ResourceLifecycle
	};

	/**
	 * CRTP base for GPU allocation strategies. Provides deferred-free and owner-relocation logic.
	 *
	 * **Threading.** Public entry points (TryAllocate, Free, FreeImmediate, Flush) acquire an
	 * allocator-wide RecursiveMutex when @p ThreadPolicy is ThreadSafe (the default).
	 * When @p ThreadPolicy is ThreadUnsafe, locking compiles out entirely and
	 * the caller is responsible for external synchronization.
	 *
	 * @tparam Derived		Allocator strategy implementing TryAllocateImpl, FreeImpl and FreeImmediateImpl.
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 * @tparam ThreadPolicy	Compile-time thread-safety policy. ThreadSafe (default) wraps state with a
	 * 						RecursiveMutex; ThreadUnsafe compiles out all locking.
	 */
	template <typename Derived, typename HeapBackend, ThreadSafetyPolicy ThreadPolicy = ThreadSafe>
	class TGpuAllocator
	{
	public:
		B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(HeapBackend);

		using Location = TGpuResourceLocation<HeapBackend>;

		/** Attempts to allocate @p size bytes with @p alignment. Resource kind defaults to Linear. The allocation is untracked (won't participate in defragmentation). */
		bool TryAllocate(u64 size, u32 alignment, Location& out)
		{
			return TryAllocate(size, alignment, GpuResourceKind::Linear, nullptr, out);
		}

		/**
		 * Attempts to allocate @p size bytes with @p alignment, tagged with @p kind so the strategy
		 * can honor buffer-image granularity (if needed by the backend). The allocation is untracked (won't participate in defragmentation).
		 */
		bool TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, Location& out)
		{
			return TryAllocate(size, alignment, kind, nullptr, out);
		}

		/**
		 * Attempts to allocate @p size bytes with @p alignment, tagged with @p kind, and registers
		 * @p owner as the resource the allocator will call back during defragmentation. Pass
		 * nullptr if the allocation is untracked (won't participate in defragmentation).
		 */
		bool TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out)
		{
			ScopedLock lock(mMutex);
			return static_cast<Derived*>(this)->TryAllocateImpl(size, alignment, kind, owner, out);
		}

		/**
		 * Retires a previously allocated location and resets it to the empty state. Frees are
		 * deferred until the associated resource is no longer used on the GPU.
		 */
		void Free(Location& allocation)
		{
			ScopedLock lock(mMutex);
			static_cast<Derived*>(this)->FreeImpl(allocation);
			allocation.Reset();
		}

		/**
		 * Releases @p allocation immediately, bypassing the deferred-free queue. The slot is
		 * returned to the pool synchronously so a subsequent TryAllocate can reuse it.
		 *
		 * The caller must guarantee the GPU is no longer using the underlying memory range — for
		 * example, when the caller already gates resource destruction on a separate use-count or
		 * frame fence. Use Free when no such guarantee exists.
		 */
		void FreeImmediate(Location& allocation)
		{
			ScopedLock lock(mMutex);
			static_cast<Derived*>(this)->FreeImmediateImpl(allocation.AllocatorData0, allocation.AllocatorData1);
			allocation.Reset();
		}

		/**
		 * Releases retired allocations whose recorded frame index is no longer in flight on the GPU.
		 *
		 * @param frameLag       Strategy-specific retention policy: drain only entries that have been
		 *                       queued for at least @p frameLag frames. Honored by individual strategies
		 *                       that need it (e.g. transient page pools that hold a slot one extra frame
		 *                       to avoid thrash).
		 * @param forceComplete  When true, frame-completion checks are skipped and every retired entry
		 *                       is freed unconditionally. Use only at shutdown after ensuring all
		 *                       GPU work has completed.
		 */
		void Flush(u32 frameLag = 3, bool forceComplete = false)
		{
			(void)frameLag;
			ScopedLock lock(mMutex);
			DrainRetired(forceComplete);
		}

	protected:
		/** True when the allocator was instantiated with the thread-safe policy. */
		static constexpr bool kThreadSafe = (ThreadPolicy == ThreadSafe);

		/** Internal mutex holder; empty struct when @p ThreadPolicy is ThreadUnsafe. */
		using MutexHolder = detail::GpuAllocatorMutex<kThreadSafe>;

		/** RAII scoped lock; no-op when @p ThreadPolicy is ThreadUnsafe. */
		using ScopedLock = detail::GpuAllocatorScopedLock<kThreadSafe>;

		/**
		 * Returns the allocator-wide mutex holder. Derived strategies acquire a ScopedLock on it
		 * for paths the base doesn't already wrap (e.g. Defrag, SetAllocationOwner, stats).
		 */
		const MutexHolder& GetMutex() const { return mMutex; }

		/**
		 * Constructs the allocator base. @p backend must outlive this object. @p frameTracker may be
		 * null if the allocator is using ResourceLifecycle free deferral mode.
		 */
		TGpuAllocator(HeapBackend* backend, IGpuFrameTracker* frameTracker)
			: mBackend(backend), mFrameTracker(frameTracker)
		{
			B3D_ASSERT(backend != nullptr);
		}

		~TGpuAllocator() = default;

		// Allocators own bookkeeping that is not safe to duplicate.
		TGpuAllocator(const TGpuAllocator&) = delete;
		TGpuAllocator& operator=(const TGpuAllocator&) = delete;

		/**
		 * Schedule deferred-free of @p allocation against the allocator's current frame index.
		 * Caller must hold the allocator mutex when @p ThreadPolicy is ThreadSafe.
		 */
		void RetireAllocation(const Location& allocation)
		{
			B3D_ASSERT(mFrameTracker != nullptr);

			RetiredEntry entry;
			entry.AllocatorData0 = allocation.AllocatorData0;
			entry.AllocatorData1 = allocation.AllocatorData1;
			entry.FrameIndex = mFrameTracker->GetCurrentFrameIndex();

			mRetiredQueue.Add(entry);
		}

		/** Entry in the deferred-free FIFO queue. */
		struct RetiredEntry
		{
			u32 AllocatorData0;
			u32 AllocatorData1;
			u64 FrameIndex;
		};

		HeapBackend* mBackend = nullptr;
		IGpuFrameTracker* mFrameTracker = nullptr;
		TInlineArray<RetiredEntry, 64> mRetiredQueue;

	private:
		/**
		 * Drains completed retired entries from the front of the FIFO queue. Caller must hold
		 * the allocator mutex when @p ThreadPolicy is ThreadSafe.
		 */
		void DrainRetired(bool forceComplete)
		{
			u32 drainCount = 0;
			const u32 size = (u32)mRetiredQueue.size();
			for (u32 entryIndex = 0; entryIndex < size; entryIndex++)
			{
				const RetiredEntry& entry = mRetiredQueue[entryIndex];
				if (!forceComplete && !mFrameTracker->IsFrameComplete(entry.FrameIndex))
					break;

				static_cast<Derived*>(this)->FreeImmediateImpl(entry.AllocatorData0, entry.AllocatorData1);
				drainCount++;
			}

			if (drainCount > 0)
				mRetiredQueue.Erase(mRetiredQueue.Begin(), mRetiredQueue.Begin() + drainCount);
		}

		mutable MutexHolder mMutex;
	};

	/** @} */
} // namespace b3d
