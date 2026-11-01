//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d
{
	namespace render
	{
		class GpuResourceTracker;
	}

	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Native heap behind the bindless resource heap. A command buffer gets and tracks the current native heap the first time it may read
	 * it, and keeps it bound until its work completes. See GpuBindlessHeapPool.
	 */
	class B3D_EXPORT GpuBindlessHeap : public IGpuResource
	{
	public:
		/**
		 * @param	owner		Manager responsible for freeing the native heap.
		 * @param	capacity	Number of slots in the native heap, including the invalid slot.
		 */
		GpuBindlessHeap(GpuResourceManager* owner, u32 capacity);

		/** Returns the number of slots in the native heap, including the invalid slot. */
		u32 GetCapacity() const { return mCapacity; }

		/** Returns a value that changes whenever the native heap is reused with different slot contents. Unique across the native heaps of a pool. */
		u64 GetVersion() const { return mVersion; }

		/** Returns true if this is the current native heap of its pool, the one work recorded from now on gets. Thread safe. */
		bool IsCurrent() const { return mIsCurrent.load(std::memory_order_relaxed); }

	private:
		friend class GpuBindlessHeapPool;

		u32 mCapacity;
		u64 mVersion = 0;
		std::atomic<bool> mIsCurrent = false;
		TBitfield<> mDirtySlots; /**< Slots changed while the native heap was not current, which it must copy before it is current again. */
	};

	/**
	 * Tracks the native heaps behind the bindless resource heap. Switching the current native heap lets slots the GPU may be reading
	 * change, and lets the resource heap grow; work recorded before a switch keeps reading the native heap it got. Command buffers finish
	 * recording in the frame they start in, so only the current native heap needs slots allocated or written since the last switch.
	 *
	 * Native heaps that stop being current retire. Once no command buffer binds them, they return to the pool if they have the current
	 * capacity and the pool has room, and are destroyed otherwise. Pooled native heaps remember the slots changed since they were last
	 * current, so they only need those copied when they are reused.
	 *
	 * Bookkeeping only: creating native heaps and writing descriptors into them is up to the owner, see GpuBindlessHeaps.
	 *
	 * @note	GetAndTrack() is thread safe. Other methods must be serialized with each other, and with writes into the native heaps.
	 */
	class B3D_EXPORT GpuBindlessHeapPool
	{
	public:
		/** @param	maximumHeapCount	Maximum number of native heaps kept alive for reuse, including the current one. 1 disables reuse. */
		explicit GpuBindlessHeapPool(u32 maximumHeapCount);

		/** Destroys every native heap. No command buffer may bind them anymore. */
		~GpuBindlessHeapPool();

		/** Makes @p heap the first current native heap. Takes ownership of it. */
		void Initialize(GpuBindlessHeap* heap);

		/**
		 * Returns the current native heap, and tracks it with @p resourceTracker so it stays alive and isn't reused while the command
		 * buffer that owns the tracker is recording or executing.
		 */
		GpuBindlessHeap* GetAndTrack(render::GpuResourceTracker& resourceTracker);

		/** Returns the current native heap. Must not be used for binding, see GetAndTrack(). */
		GpuBindlessHeap* GetCurrent() const { return mCurrent; }

		/** Calls @p function for every native heap still alive: the current one, retiring ones and pooled ones. */
		template<class Function>
		void ForEachHeap(Function function) const
		{
			Lock lock(mMutex);

			function(*mCurrent);

			for(GpuBindlessHeap* heap : mRetiringHeaps)
				function(*heap);

			for(GpuBindlessHeap* heap : mPooledHeaps)
				function(*heap);
		}

		/**
		 * Removes an idle native heap of the current capacity from the pool and returns it, or returns null if there is none. The caller
		 * copies its dirty slots (see GetDirtySlots()) and makes it current with MakeCurrent().
		 */
		GpuBindlessHeap* TakePooledHeap();

		/** Returns the slots changed since @p heap was last current. */
		const TBitfield<>& GetDirtySlots(const GpuBindlessHeap& heap) const { return heap.mDirtySlots; }

		/**
		 * Makes @p heap current, and starts retiring the previous current native heap. Every other native heap now lacks @p updates, so
		 * they copy them before they are current again. Pooled native heaps smaller than @p heap are destroyed.
		 *
		 * @param	heap		Native heap holding the current contents of the resource heap, and @p updates. Takes ownership of it.
		 * @param	updates		Slots whose descriptors changed with the switch.
		 */
		void MakeCurrent(GpuBindlessHeap* heap, TArrayView<const GpuBindlessSlotUpdate> updates);

		/**
		 * Records that resource slot @p slotIndex was written into the current native heap only, so other native heaps copy it before
		 * they are current again. For slots no GPU work reads.
		 */
		void MarkSlotWritten(u32 slotIndex);

		/** Pools or destroys retiring native heaps that no command buffer binds anymore. */
		void ProcessRetiredHeaps();

	private:
		u32 mMaximumHeapCount;
		u64 mNextVersion = 1;

		GpuBindlessHeap* mCurrent = nullptr;
		TArray<GpuBindlessHeap*> mRetiringHeaps; /**< Former current native heaps that command buffers may still bind. */
		TArray<GpuBindlessHeap*> mPooledHeaps; /**< Idle native heaps of the current capacity, ready for reuse. */
		mutable Mutex mMutex; /**< Guards the native heap lists against GetAndTrack() and ForEachHeap(). */
	};

	/** @} */
} // namespace b3d
