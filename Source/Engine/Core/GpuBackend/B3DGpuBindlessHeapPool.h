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
	 * Native heap behind the bindless resource heap. A command buffer gets and tracks the current heap the first time it may read it,
	 * and keeps it bound until its work completes. See GpuBindlessHeapPool.
	 */
	class B3D_EXPORT GpuBindlessHeap : public IGpuResource
	{
	public:
		/**
		 * @param	owner		Manager responsible for freeing the heap.
		 * @param	capacity	Number of slots in the heap, including the invalid slot.
		 */
		GpuBindlessHeap(GpuResourceManager* owner, u32 capacity);

		/** Returns the number of slots in the heap, including the invalid slot. */
		u32 GetCapacity() const { return mCapacity; }

		/** Returns a value that changes whenever the heap is reused with different slot contents. Unique across all heaps of a pool. */
		u64 GetVersion() const { return mVersion; }

		/** Returns true if the heap is the current heap of its pool, the one work recorded from now on gets. Thread safe. */
		bool IsCurrent() const { return mIsCurrent.load(std::memory_order_relaxed); }

	private:
		friend class GpuBindlessHeapPool;

		u32 mCapacity;
		u64 mVersion = 0;
		std::atomic<bool> mIsCurrent = false;
		TBitfield<> mDirtySlots; /**< Slots changed while the heap was not current, which it must copy before it is current again. */
	};

	/** Native operations GpuBindlessHeapPool needs from a backend. */
	class B3D_EXPORT IGpuBindlessHeapPoolBackend
	{
	public:
		virtual ~IGpuBindlessHeapPoolBackend() = default;

		/** Creates a heap of @p capacity slots. Its slot contents are undefined. Returns null on failure. */
		virtual GpuBindlessHeap* CreateHeap(u32 capacity) = 0;

		/**
		 * Copies the latest descriptors of the slots in @p ranges into @p destination. The latest descriptors are those of @p current,
		 * which is up to date, or of a backend mirror of it.
		 */
		virtual void CopySlots(GpuBindlessHeap& destination, const GpuBindlessHeap& current, TArrayView<const TIndexRange<u32>> ranges) = 0;

		/** Writes the descriptors of @p updates into @p destination, and into the backend's mirror if it has one. */
		virtual void WriteUpdates(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) = 0;
	};

	/**
	 * Native heaps behind the bindless resource heap. Growth, and on backends that can only write descriptors on the CPU also changes
	 * of slots the GPU may be reading, switch to a heap no command buffer uses; work recorded before the switch keeps reading the
	 * heap it got. Command buffers finish recording in the frame they start in, so only the current heap needs slots allocated or
	 * written since the last switch.
	 *
	 * Heaps that stop being current retire. Once no command buffer binds them, they return to a pool if they have the current
	 * capacity and pooling is enabled, and are destroyed otherwise. Pooled heaps remember the slots changed since they were last
	 * current, and copy only those when they are reused.
	 *
	 * @note	GetAndTrack() is thread safe. Other methods must be serialized with each other, and with writes into the heaps.
	 */
	class B3D_EXPORT GpuBindlessHeapPool
	{
	public:
		/**
		 * @param	backend				Native operations on heaps.
		 * @param	maximumHeapCount	Maximum number of heaps kept alive for reuse, including the current heap. 1 disables reuse.
		 */
		GpuBindlessHeapPool(IGpuBindlessHeapPoolBackend& backend, u32 maximumHeapCount);

		/** Destroys every heap. No command buffer may bind them anymore. */
		~GpuBindlessHeapPool();

		/** Creates the first heap, of @p capacity slots. Returns false on failure. */
		bool Initialize(u32 capacity);

		/**
		 * Returns the current heap, and tracks it with @p resourceTracker so it stays alive and isn't reused while the command
		 * buffer that owns the tracker is recording or executing.
		 */
		GpuBindlessHeap* GetAndTrack(render::GpuResourceTracker& resourceTracker);

		/** Returns the current heap. Must not be used for binding, see GetAndTrack(). */
		GpuBindlessHeap* GetCurrent() const { return mCurrent; }

		/**
		 * Records that resource slot @p slotIndex was written into the current heap only, so other heaps copy it before they are current
		 * again. For slots no GPU work reads.
		 */
		void MarkSlotWritten(u32 slotIndex);

		/** Calls @p function for every heap still alive: the current one, retiring ones and pooled ones. */
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
		 * Switches to a heap that holds @p updates on top of the current contents. Returns false and leaves the current heap
		 * unchanged if no heap could be created.
		 *
		 * @param	updates		New descriptors of slots in the current capacity.
		 * @param	liveSlots	Slots whose descriptors are valid to copy.
		 */
		bool ApplyUpdates(TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots);

		/**
		 * Switches to a new heap of @p capacity slots, holding the current contents. Returns false and leaves the current heap
		 * unchanged if no heap could be created.
		 *
		 * @param	capacity	Number of slots of the new heap. Must be larger than the current capacity.
		 * @param	liveSlots	Slots whose descriptors are valid to copy.
		 */
		bool Grow(u32 capacity, const TBitfield<>& liveSlots);

		/** Pools or destroys retiring heaps that no command buffer binds anymore. */
		void ProcessRetiredHeaps();

	private:
		/** Makes @p heap current, and starts retiring the previous current heap. */
		void MakeCurrent(GpuBindlessHeap* heap);

		/** Copies the runs of @p slots that are also in @p liveSlots into @p destination, from the current heap. */
		void CopyLiveSlots(GpuBindlessHeap& destination, const TBitfield<>& slots, const TBitfield<>& liveSlots);

		IGpuBindlessHeapPoolBackend& mBackend;
		u32 mMaximumHeapCount;
		u64 mNextVersion = 1;

		GpuBindlessHeap* mCurrent = nullptr;
		TArray<GpuBindlessHeap*> mRetiringHeaps; /**< Former current heaps that command buffers may still bind. */
		TArray<GpuBindlessHeap*> mPooledHeaps; /**< Idle heaps of the current capacity, ready for reuse. */
		mutable Mutex mMutex; /**< Guards the heap lists against GetAndTrack(). */
	};

	/** @} */
} // namespace b3d
