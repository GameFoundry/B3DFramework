//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuVirtualMemory.h"
#include "Threading/B3DThreading.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class GpuDevice;
	class GpuPageTable;
	class IGpuCompletionTracker;

	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Changes of the memory behind pages of virtual buffers and textures, which the submit thread executes with one native call. See
	 * IGpuSubmitThreadBackend::ExecutePageMappings().
	 */
	struct GpuPageMappingBatch
	{
		TArray<GpuPageMapping> Mappings; /**< In the order they were made. Each resource is pinned until the batch executes. */
		u32 RemovalCount = 0; /**< Number of mappings that unmap pages. */
		bool ChangesBindlessResources = false; /**< True if a mapping changes a resource with a bindless slot. */
	};

	/**
	 * Page mappings of one device's virtual buffers and textures that haven't executed yet. Adds, which map memory to unmapped pages, are
	 * queued immediately. Removals, which unmap pages, stay pending in their GpuPageTable until the frame
	 * they were made in completes, since earlier work may still use the old memory, and are queued then. Flush() hands the queued mappings to
	 * the submit thread as one batch, which executes before the next command, so every command buffer submitted after a mapping was queued
	 * sees it.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuPageMappingQueue
	{
	public:
		/** @p frameCompletionTracker decides when pending removals are queued. Its marker is the device's frame index. */
		GpuPageMappingQueue(GpuDevice& device, const IGpuCompletionTracker& frameCompletionTracker);
		~GpuPageMappingQueue();

		/**
		 * Maps memory to unmapped pages. If the device supports immediate mapping (see GpuDevice::TryExecutePageMappingsImmediately()) and
		 * every removal queued by QueueRemovals() has executed, the mappings execute before this method returns. Otherwise they are queued.
		 */
		void ExecuteOrQueueAdds(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags);

		/** Queues mappings that unmap pages. They execute after every mapping queued before them. */
		void QueueRemovals(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags);

		/** Hands the queued mappings to the submit thread, which executes them before any command posted after this call. */
		void Flush();

		/** Returns true if mappings are queued and not yet handed to the submit thread. */
		bool HasQueuedMappings() const;

		/**
		 * Registers @p table, which has pending removals, so that QueueCompletedRemovals() queues them once their frame completes. Does
		 * nothing if it is registered already.
		 */
		void RegisterPendingRemovals(GpuPageTable& table);

		/** Unregisters @p table, registered by RegisterPendingRemovals(). Does nothing if it is not registered. */
		void UnregisterTable(GpuPageTable& table);

		/**
		 * Queues the pending removals of every registered table whose frame has completed, see GpuPageTable::QueueCompletedRemovals(). Tables
		 * left without pending removals are unregistered.
		 */
		void QueueCompletedRemovals();

		/** Releases the pins of a batch the submit thread executed, see GpuPageMappingBatch. */
		void NotifyBatchExecuted(const GpuPageMappingBatch& batch);

		/** Drops the queued mappings and releases their pins. The GPU must be idle. */
		void Clear();

	private:
		/** Appends @p mappings to the queued batch, and pins their resources. Must be called with mMutex locked. */
		void AddToQueuedBatch(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags);

		GpuDevice& mDevice;
		const IGpuCompletionTracker& mFrameCompletionTracker;

		GpuPageMappingBatch mQueuedBatch;
		std::atomic<u32> mUnexecutedRemovalCount = 0; /**< Removals queued or handed to the submit thread, and not yet executed. */
		mutable Mutex mMutex;

		TArray<GpuPageTable*> mTablesWithPendingRemovals;
		Mutex mTableMutex; /**< Guards mTablesWithPendingRemovals. Locked before any table, which is locked before mMutex. */
	};

	/** @} */
} // namespace b3d
