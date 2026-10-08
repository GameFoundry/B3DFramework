//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Threading/B3DThreading.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Size of a new transient heap in megabytes, unless the resource that requires the heap is larger. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuTransientHeapMinimumSize;

	/** Number of frames a transient heap without references, or a cached transient resource, may go unused before it is released. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuTransientIdleFrames;

	/** Heap of a transient heap pool, together with its size. */
	struct GpuTransientHeap
	{
		GpuTransientHeap() = default;
		GpuTransientHeap(IGpuHeap* heap, u64 size)
			: Heap(heap), Size(size)
		{ }

		IGpuHeap* Heap = nullptr; /**< Null if no heap was acquired. */
		u64 Size = 0; /**< Size of the heap, in bytes. */
	};

	/** Memory use of one transient heap pool. */
	struct GpuTransientHeapPoolStatistics
	{
		u32 HeapCount = 0; /**< Number of heaps the pool currently owns. */
		u64 Capacity = 0; /**< Total size of the heaps, in bytes. */
	};

	/**
	 * Heaps that transient resources of one or more memory types are placed in.
	 * Heaps live in stable slots. A released heap leaves its slot empty, and a later heap may reuse the slot.
	 *
	 * @note	Thread safe.
	 */
	class IGpuTransientHeapPool
	{
	public:
		virtual ~IGpuTransientHeapPool() = default;

		/** Returns the number of heap slots, empty slots included. */
		virtual u32 GetSlotCount() const = 0;

		/**
		 * Returns the heap in @p slot with a reference added, if the heap has at least @p minimumSize bytes. Otherwise returns
		 * no heap, with GpuTransientHeap::Size set to the size of the heap in the slot, or to zero if the slot is empty or does
		 * not exist.
		 */
		virtual GpuTransientHeap AcquireHeap(u32 slot, u64 minimumSize) = 0;

		/**
		 * Creates a heap of at least @p size bytes in @p slot, unless the slot already holds a heap, which another scope
		 * created first. Returns the heap in the slot with a reference added, or no heap if creation failed. A heap created
		 * by another scope may be smaller than @p size.
		 */
		virtual GpuTransientHeap AcquireNewHeap(u32 slot, u64 size) = 0;

		/** Removes a reference added by AcquireHeap(), AcquireNewHeap() or AddHeapReference(). */
		virtual void ReleaseHeap(IGpuHeap* heap) = 0;

		/** Adds a reference to a heap the caller already references. */
		virtual void AddHeapReference(IGpuHeap* heap) = 0;

		/** Returns the value offsets and sizes of placements in the pool are rounded up to a multiple of. A power of two. */
		virtual u64 GetGranularity() const = 0;

		/**
		 * Releases heaps without references that were not used for gGpuTransientIdleFrames frames, and destroys released heaps
		 * the GPU has finished using. Frames are counted by the pool's completion tracker, so the call may be repeated within
		 * a frame.
		 */
		virtual void ReclaimUnused() = 0;

		/** Returns the memory use of the pool. */
		virtual GpuTransientHeapPoolStatistics GetStatistics() const = 0;
	};

	/**
	 * Transient heap pool over heaps created with @p HeapBackend.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 */
	template <typename HeapBackend>
	class TGpuTransientHeapPool final : public IGpuTransientHeapPool
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
			 * Offsets and sizes of placements are rounded up to a multiple of this value, so that no two placements share a
			 * block of this size. Must be a power of two. Covers Vulkan's VkPhysicalDeviceLimits::bufferImageGranularity, and
			 * D3D12's placement of buffers without tight alignment.
			 */
			u64 Granularity = 1;

			/**
			 * Determines when the GPU has finished using a released heap, so it can be destroyed. Its marker is the index of the
			 * frame, and a released heap is destroyed once the frame it was last used in is complete. Must outlive the pool.
			 */
			IGpuCompletionTracker* CompletionTracker = nullptr;
		};

		/** Constructs the pool. @p backend must outlive the pool. */
		TGpuTransientHeapPool(HeapBackend* backend, const Configuration& configuration);

		/** Destroys all heaps. The GPU must have finished using them, and no heap may have a reference. */
		~TGpuTransientHeapPool() override;

		TGpuTransientHeapPool(const TGpuTransientHeapPool&) = delete;
		TGpuTransientHeapPool& operator=(const TGpuTransientHeapPool&) = delete;

		/** @name IGpuTransientHeapPool
		 *  @{
		 */

		u32 GetSlotCount() const override;
		GpuTransientHeap AcquireHeap(u32 slot, u64 minimumSize) override;
		GpuTransientHeap AcquireNewHeap(u32 slot, u64 size) override;
		void ReleaseHeap(IGpuHeap* heap) override;
		void AddHeapReference(IGpuHeap* heap) override;
		u64 GetGranularity() const override { return mConfiguration.Granularity; }
		void ReclaimUnused() override;
		GpuTransientHeapPoolStatistics GetStatistics() const override;

		/** @} */

	private:
		/** Heap slot, and the references to its heap. */
		struct Slot
		{
			HeapHandle Heap = nullptr; /**< Null if the slot is empty. */
			u64 Size = 0;
			u32 ReferenceCount = 0;
			u64 LastUsedFrame = 0; /**< Frame in which the heap last had a reference. */
		};

		/** Heap no longer in a slot, waiting for the GPU to finish using it. */
		struct ReleasedHeap
		{
			ReleasedHeap(HeapHandle heap, u64 marker)
				: Heap(heap), Marker(marker)
			{ }

			HeapHandle Heap;
			u64 Marker; /**< Frame the heap was last used in. The heap can be destroyed once its marker is complete. */
		};

		/** Returns the slot holding @p heap. Must be called with mMutex locked. */
		Slot& FindSlot(IGpuHeap* heap);

		HeapBackend* mBackend = nullptr;
		Configuration mConfiguration;

		TArray<Slot> mSlots;
		TArray<ReleasedHeap> mReleasedHeaps;
		mutable Mutex mMutex;
	};

	/** @} */
} // namespace b3d

#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.inl"
