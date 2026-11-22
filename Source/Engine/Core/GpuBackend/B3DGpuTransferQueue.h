//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Image/B3DTexture.h"
#include "Threading/B3DAsyncOp.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class GpuDevice;
	class GpuTransferQueue;
	class IGpuAllocator;
	class IGpuResource;
	struct GpuBufferCreateInformation;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Staging buffer created by GpuTransferQueue::CreateStagingWriteBuffer() or GpuTransferQueue::CreateStagingReadBuffer(). Move-only;
	 * releases its staging memory once destroyed, unless it was passed to the queue.
	 */
	class B3D_EXPORT GpuStagingBuffer
	{
	public:
		GpuStagingBuffer() = default;
		GpuStagingBuffer(GpuStagingBuffer&& other) noexcept;
		GpuStagingBuffer& operator=(GpuStagingBuffer&& other) noexcept;
		~GpuStagingBuffer();

		GpuStagingBuffer(const GpuStagingBuffer&) = delete;
		GpuStagingBuffer& operator=(const GpuStagingBuffer&) = delete;

		/** Returns the buffer, or null if the handle is empty. */
		const TShared<render::GpuBuffer>& GetBuffer() const { return mBuffer; }

	private:
		friend class GpuTransferQueue;

		GpuStagingBuffer(GpuTransferQueue& queue, TShared<render::GpuBuffer> buffer, u64 epoch);

		/** Releases the staging memory if the handle still owns it, and empties the handle. */
		void Release();

		GpuTransferQueue* mQueue = nullptr;
		TShared<render::GpuBuffer> mBuffer;
		u64 mEpoch = 0; /**< Staging epoch the buffer was created in. */
	};

	/**
	 * Transfers data between the CPU and buffers and textures through staging buffers, and copies the resources the memory defragmenter moves.
	 *
	 * Transfers to one resource run in the order they were queued. A queued transfer runs before the first submitted command buffer that
	 * starts using the resource after the transfer is queued, and at the latest at the end of the frame (GpuDevice::EndFrame()). Command
	 * buffers don't track bindless resources, so transfers to buffers created with GpuBufferFlag::Bindless and textures created with
	 * TextureUsageFlag::Bindless run before the next submitted command buffer instead. An upload that fully covers an earlier queued upload
	 * to the same resource replaces it, unless a read or a relocation was queued in between. Reads are submitted as soon as they are queued.
	 *
	 * A transfer writes and reads the native resource its resource had when the transfer was queued, even if the resource gets a new one
	 * later. Submitting a command buffer takes the transfers it needs into a numbered batch at once, on the submitting thread, so a transfer
	 * queued after the submission never joins them. Batches are recorded and submitted in the order they were taken, on the submit thread,
	 * on the first graphics queue. A batch waits for the earlier work on other queues that accessed what it transfers, and for all earlier
	 * work on every queue if it transfers to a bindless resource.
	 */
	class B3D_EXPORT GpuTransferQueue
	{
	public:
		/** Command buffer the queue recorded transfer batches into, for the submit thread to submit. */
		struct RecordedTransfers
		{
			/** Null if nothing was recorded. Ended and queued for submission on the first graphics queue. The caller submits it there. */
			TShared<render::GpuCommandBuffer> CommandBuffer;

			/** Batch fence and the number of the last batch recorded, for the submission to signal. */
			GpuTimelineFenceAndValue SignalFence;
		};

		explicit GpuTransferQueue(GpuDevice& device);
		~GpuTransferQueue();

		GpuTransferQueue(const GpuTransferQueue&) = delete;
		GpuTransferQueue& operator=(const GpuTransferQueue&) = delete;

		/**
		 * Creates a single-use, CPU-writable staging buffer of @p size bytes for a copy to the GPU. Pass it to QueueUpload() or RecordUpload().
		 * Its memory is reused once the copy from it has executed, or once the handle is destroyed if it was never passed on. Returns an empty
		 * handle if the buffer can't be created. Thread safe.
		 */
		GpuStagingBuffer CreateStagingWriteBuffer(u32 size);

		/**
		 * Creates a single-use, CPU-readable staging buffer of @p size bytes for a copy from the GPU. Its memory is reused once the handle is
		 * destroyed, so destroy it once the data has been read. Returns an empty handle if the buffer can't be created. Thread safe.
		 */
		GpuStagingBuffer CreateStagingReadBuffer(u32 size);

		/** Queues a copy of the first @p length bytes of @p source into @p destination at @p destinationOffset. Thread safe. */
		void QueueUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length);

		/**
		 * Queues a copy of @p source, laid out as returned by render::Texture::GetStagingBufferPitchForSubresource(), into one subresource.
		 * The copy discards the previous contents of the subresource. Thread safe.
		 */
		void QueueUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer);

		/**
		 * Records the copy QueueUpload() would queue, together with its barriers, into @p commandBuffer instead. The destination must be at rest.
		 * Must be called on the thread that records @p commandBuffer.
		 */
		void RecordUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length, render::GpuCommandBuffer& commandBuffer);

		/** @copydoc RecordUpload(GpuStagingBuffer&&, const TShared<render::GpuBuffer>&, u32, u32, render::GpuCommandBuffer&) */
		void RecordUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer, render::GpuCommandBuffer& commandBuffer);

		/**
		 * Reads @p length bytes of @p source at @p offset, after the transfers queued to it. The read is submitted at once. The operation
		 * completes on the submit thread once the GPU has executed the copy, with null if the read couldn't be performed. Thread safe.
		 */
		TAsyncOp<TShared<MemoryDataStream>> QueueRead(const TShared<render::GpuBuffer>& source, u32 offset, u32 length);

		/**
		 * Reads one subresource of @p source, after the transfers queued to it. The returned pixel data is in the texture's supported format,
		 * laid out as returned by render::Texture::GetStagingBufferPitchForSubresource(). Otherwise the same as
		 * QueueRead(const TShared<render::GpuBuffer>&, u32, u32). Thread safe.
		 */
		TAsyncOp<TShared<PixelData>> QueueRead(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer);

		/**
		 * Moves @p resource to new memory. @p oldResourceHolder is a buffer of the same description created at the new memory. The two
		 * exchange native resources, so @p resource gets the new memory and @p oldResourceHolder the old one. The queue keeps the old native
		 * resource until the queued copy of its contents into the new memory has been recorded. The copy runs after the transfers queued to
		 * the old native resource. No recorded command buffer may use the old native resource. Render thread only.
		 */
		void QueueRelocation(const TShared<render::GpuBuffer>& resource, const TShared<render::GpuBuffer>& oldResourceHolder);

		/** Texture equivalent of QueueRelocation(const TShared<render::GpuBuffer>&, const TShared<render::GpuBuffer>&). Copies every subresource. */
		void QueueRelocation(const TShared<render::Texture>& resource, const TShared<render::Texture>& oldResourceHolder);

		/**
		 * Gives @p resource a new native buffer and drops the uploads queued to the old one. Refuses, and returns false, if a read or a
		 * relocation is queued to the old one, or if uploads are queued to it and a recorded command buffer that isn't submitted yet uses it.
		 * Returns true if the buffer was recreated. Thread safe.
		 */
		bool TryRecreateNativeResource(render::GpuBuffer& resource);

		/** Texture equivalent of TryRecreateNativeResource(render::GpuBuffer&). Thread safe. */
		bool TryRecreateNativeResource(render::Texture& resource);

		/** Submits the transfers queued to the current native resource of @p destination, without waiting for them. Thread safe. */
		void SubmitQueuedTransfers(const render::GpuBuffer& destination);

		/** @copydoc SubmitQueuedTransfers(const render::GpuBuffer&) */
		void SubmitQueuedTransfers(const render::Texture& destination);

		/** Submits every queued transfer, without waiting for them, and starts a new staging epoch. Called by GpuDevice::EndFrame(). */
		void SubmitAllQueuedTransfers();

		/** @name Internal
		 *  @{
		 */

		/**
		 * Copies @p length bytes of @p source at @p offset into @p destination, a buffer created by CreateStagingReadBuffer(), after the
		 * transfers queued to @p source, and blocks until the copy has executed. Thread safe.
		 */
		void CopyToStagingAndWait(const TShared<render::GpuBuffer>& source, u32 offset, u32 length, const GpuStagingBuffer& destination);

		/** Texture equivalent of CopyToStagingAndWait(), for one subresource. Thread safe. */
		void CopyToStagingAndWait(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer, const GpuStagingBuffer& destination);

		/**
		 * Makes the writes of the GPU to @p source, and of the transfers queued to it, visible to the CPU, and blocks until they are. Used before
		 * reading a resource the CPU maps. Thread safe.
		 */
		void WaitUntilHostReadable(const TShared<render::GpuBuffer>& source);

		/** Texture equivalent of WaitUntilHostReadable(), for one subresource. Thread safe. */
		void WaitUntilHostReadable(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer);

		/**
		 * Takes the transfers queued to @p resources, and those to bindless resources, into a new batch, and returns its number. If there are
		 * none, returns the number of the last batch that isn't recorded yet, since it may hold transfers to @p resources, or 0 if every batch
		 * is recorded. Thread safe.
		 */
		u64 TakeQueuedTransfers(TArrayView<IGpuResource* const> resources);

		/** Same as TakeQueuedTransfers(), but takes every queued transfer. Thread safe. */
		u64 TakeAllQueuedTransfers();

		/**
		 * Records every batch up to and including @p lastBatch that isn't recorded yet, in order, into a command buffer of the submit thread's
		 * graphics pool, which is ended and queued for submission. 
		 * 
		 * Submit thread only.
		 */
		RecordedTransfers RecordTakenTransfers(u64 lastBatch);

		/** Fence each transfer submission signals with the number of the last batch it recorded. */
		const TShared<GpuTimelineFence>& GetBatchFence() const { return mBatchFence; }

		/** Returns true while transfers to bindless resources are queued, or taken into a batch that isn't recorded yet. Thread safe. */
		bool HasBindlessTransfers() const { return mBindlessTransferListCount.load(std::memory_order_relaxed) > 0; }

		/** @} */

	private:
		friend class GpuStagingBuffer;

		/** Completion tracker of the staging allocators. Its markers are staging epochs. */
		class EpochTracker final : public IGpuCompletionTracker
		{
		public:
			explicit EpochTracker(const GpuTransferQueue& queue) : mQueue(queue) {}

			u64 GetCurrentMarker() const override { return mQueue.mCurrentEpoch.load(std::memory_order_acquire); }
			bool IsMarkerComplete(u64 marker) const override { return marker < mQueue.mFirstIncompleteEpoch.load(std::memory_order_acquire); }

		private:
			const GpuTransferQueue& mQueue;
		};

		/** Kind of a queued transfer. */
		enum class QueuedTransferType
		{
			Upload, /**< Copies a staging buffer into the resource. */
			CopyToStaging, /**< Copies the resource into a staging buffer. */
			MakeHostReadable, /**< Makes the GPU's writes to the resource visible to the CPU, which reads it through a mapping. */
			Relocation /**< Copies the old native resource of a moved resource into its new one. */
		};

		/** Kind of native resource a transfer list is for. */
		enum class TransferResourceType
		{
			Buffer,
			Image
		};

		/** Data the transfers to one native resource need, captured from its proxy. */
		struct TransferResourceInformation
		{
			IGpuResource* Resource = nullptr; /**< Pinned while its transfer list exists. */
			TransferResourceType Type = TransferResourceType::Buffer;
			render::GpuAccessState RestingState; /**< Resting state on the queue type the transfers are recorded for. */
			bool IsBindless = false; /**< True if command buffers don't track the resource, see HasBindlessTransfers(). */

			// Images only
			GpuTextureAspectFlags Aspects; /**< Aspects of the format, for barriers. */
			GpuImageCopyInformation ImageCopyInformation; /**< Description of mip level 0 and array layer 0 (extent, format, type, samples), for relocations. */
			u32 MipCount = 1; /**< Number of mip levels, for relocations. */
		};

		/** Result state of a read queued by QueueRead(). */
		struct AsyncRead
		{
			GpuStagingBuffer Staging; /**< Staging buffer the read copies into. */
			TAsyncOp<TShared<MemoryDataStream>> BufferOperation; /**< Operation of a buffer read. */
			TAsyncOp<TShared<PixelData>> TextureOperation; /**< Operation of a texture read. */
			TShared<PixelData> TexturePixelData; /**< Layout of a texture read's staging data, and its result. Null for a buffer read. */
			u32 Length = 0; /**< Bytes a buffer read copies out. */

			/**
			 * Completes the operation with the contents of the staging buffer, or with null if @p isExecuted is false, then releases the staging
			 * buffer.
			 */
			void Complete(bool isExecuted);
		};

		/**
		 * Transfer queued to a native resource. Its resources are pinned while it is queued. The resource the transfer list is for is the
		 * destination of an upload or a relocation, and the source of the other types. Transfers queued before a relocation keep the old
		 * native resource.
		 */
		struct QueuedTransfer
		{
			QueuedTransferType Type = QueuedTransferType::Upload;
			IGpuResource* Source = nullptr; /**< Resource the transfer reads. A staging buffer for an upload. */
			IGpuResource* Destination = nullptr; /**< Resource the transfer writes. A staging buffer for CopyToStaging, null for MakeHostReadable. */

			u64 Epoch = 0; /**< Staging epoch of an upload's source. */
			u32 Offset = 0; /**< Byte offset in the buffer the transfer list is for. */
			u32 Length = 0; /**< Bytes copied to or from that buffer. */

			/** Subresource of an image transfer, and for uploads and copies to staging, the layout of its data in the staging buffer. */
			GpuImageCopyInformation ImageCopy;
			render::GpuAccessState StagingRestingState; /**< Resting state of the staging buffer of CopyToStaging. */

			// Qualified, because a member named like the type would change the meaning of an unqualified AsyncRead in this struct
			TShared<GpuTransferQueue::AsyncRead> AsyncRead; /**< Read queued by QueueRead(). Null for other transfers. */
		};

		/** Transfers queued to one native resource, in queue order. */
		struct TransferList
		{
			TransferResourceInformation Information;
			TInlineArray<QueuedTransfer, 1> Transfers;
		};

		/** Transfer lists taken by one TakeQueuedTransfers() or TakeAllQueuedTransfers() call. */
		struct TransferBatch
		{
			u64 Number = 0;
			TArray<TransferList> TransferLists;
		};

		/**
		 * Staging buffers created since the previous epoch ended. An epoch is complete once it ended and the copies of all its staging buffers
		 * executed, or the buffers were released, and all earlier epochs are complete.
		 */
		struct StagingEpoch
		{
			u32 UnreleasedBufferCount = 0; /**< Staging buffers that are neither released nor known to be executed. */
			u64 StagingByteCount = 0; /**< Bytes of staging buffers created in the epoch. */

			/** Pinned native upload staging buffers whose copies are recorded. Each is unpinned once no command buffer references it any more. */
			TArray<IGpuResource*> RecordedUploadBuffers;
		};

		/** Transfers dropped before they were recorded. Completes their reads with null once destroyed, outside of the mutex. */
		struct DroppedTransfers
		{
			TArray<TShared<AsyncRead>> Reads;

			DroppedTransfers() = default;
			DroppedTransfers(const DroppedTransfers&) = delete;
			DroppedTransfers& operator=(const DroppedTransfers&) = delete;
			~DroppedTransfers();
		};

		/** Creates a staging buffer described by @p createInformation in the current epoch. Thread safe. */
		GpuStagingBuffer CreateStagingBuffer(const GpuBufferCreateInformation& createInformation);

		/**
		 * Appends @p upload, a copy from @p source into the resource @p information describes, to the resource's transfer list. Drops the
		 * queued uploads it fully covers that come after the last transfer that isn't an upload. @p source no longer owns its staging memory
		 * afterwards, but keeps its proxy for the caller to release outside of the mutex. Caller holds the mutex.
		 */
		void AppendUpload(const TransferResourceInformation& information, GpuStagingBuffer& source, QueuedTransfer&& upload, DroppedTransfers& droppedTransfers);

		/**
		 * Appends @p read, a CopyToStaging or MakeHostReadable transfer, to the transfer list of the current native resource of @p resource,
		 * and takes the list into a new batch. Returns the batch number, or 0 if @p resource has no native resource. Thread safe.
		 */
		template<class ProxyType>
		u64 AppendReadAndTake(const ProxyType& resource, QueuedTransfer&& read);

		/**
		 * Appends a copy of @p oldResource into the resource @p information describes, its new native resource after a relocation, and moves
		 * the transfer list of @p oldResource to the new one. @p length is the size of a buffer. Caller holds the mutex.
		 */
		void AppendRelocation(IGpuResource& oldResource, const TransferResourceInformation& information, u32 length);

		/** Shared part of both public RecordUpload() overloads. Consumes @p source. */
		void RecordUpload(GpuStagingBuffer& source, const TransferResourceInformation& information, QueuedTransfer&& upload, render::GpuCommandBuffer& commandBuffer);

		/** Queues the submission of the batches up to and including @p batch, and blocks until they have executed. Does nothing if @p batch is 0. */
		void SubmitAndWait(u64 batch);

		/** Shared part of both public SubmitQueuedTransfers() overloads. */
		void SubmitQueuedTransfers(IGpuResource* resource);

		/** Returns the transfer list of the resource @p information describes, adding one if there is none. Caller holds the mutex. */
		TransferList& FindOrAddTransferList(const TransferResourceInformation& information);

		/**
		 * Takes the transfer lists of @p resources, and of bindless resources, into a new batch. Returns the batch number, or the one
		 * TakeQueuedTransfers() returns if there was nothing to take. Caller holds the mutex.
		 */
		u64 TakeTransferLists(TArrayView<IGpuResource* const> resources);

		/** Releases the resources of @p transfer, which will never be recorded. Caller holds the mutex. */
		void DropTransfer(QueuedTransfer& transfer, DroppedTransfers& droppedTransfers);

		/**
		 * Drops every transfer of @p transferList, dequeues them from its resource and releases its pin. The caller removes the list.
		 * Caller holds the mutex.
		 */
		void DropTransferList(TransferList& transferList, DroppedTransfers& droppedTransfers);

		/** Shared part of both TryRecreateNativeResource() overloads. Caller holds the mutex. */
		bool TryDropTransferListForRecreate(IGpuResource* resource, DroppedTransfers& droppedTransfers);

		/** Keeps @p source, a pinned upload staging buffer, in the recorded staging buffers of its epoch. Caller holds the mutex. */
		void RetainRecordedStagingBuffer(IGpuResource& source, u64 epoch);

		/** Records that a staging buffer of @p epoch was released, so the epoch no longer waits for it. Thread safe. */
		void NotifyStagingBufferReleased(u64 epoch);

		/** Ends the current staging epoch and reuses the staging memory of complete epochs. Thread safe. */
		void EndEpoch();

		/** Returns the epoch @p epoch, which must not be complete. Caller holds the mutex. */
		StagingEpoch& GetEpoch(u64 epoch);

		/**
		 * Records @p transfer, queued to the resource @p information describes, into @p commandBuffer, together with barriers that take its
		 * resources from their resting states and return them there.
		 */
		static void RecordTransfer(render::GpuCommandBuffer& commandBuffer, const TransferResourceInformation& information, const QueuedTransfer& transfer);

		/** Captures the data the transfers to @p buffer need, for transfers recorded on a queue of @p queueType. */
		static TransferResourceInformation GetTransferResourceInformation(const render::GpuBuffer& buffer, GpuQueueType queueType);

		/** Captures the data the transfers to @p texture need, for transfers recorded on a queue of @p queueType. */
		static TransferResourceInformation GetTransferResourceInformation(const render::Texture& texture, GpuQueueType queueType);

		GpuDevice& mDevice;
		EpochTracker mEpochTracker;
		mutable Mutex mMutex;

		UnorderedMap<IGpuResource*, TransferList> mTransferListsByResource;

		/** Number of transfer lists of bindless resources, in mTransferListsByResource and in batches that aren't recorded yet. */
		std::atomic<u32> mBindlessTransferListCount = 0;

		/** Batches taken but not recorded yet, in number order. */
		Deque<TransferBatch> mTakenBatches;
		u64 mLastTakenBatch = 0; /**< Number of the last batch taken. Numbers start at 1. */
		u64 mLastRecordedBatch = 0; /**< Number of the last batch recorded. */
		TShared<GpuTimelineFence> mBatchFence; /**< See GetBatchFence(). */

		/** Thread-safe staging allocators per memory type. Null for memory types the backend has no scratch allocator for. */
		UnorderedMap<u32, TUnique<IGpuAllocator>> mStagingAllocators;

		/** Epochs from mFirstIncompleteEpoch to mCurrentEpoch, the last one being the current epoch. */
		Deque<StagingEpoch> mEpochs;
		std::atomic<u64> mFirstIncompleteEpoch = 0;
		std::atomic<u64> mCurrentEpoch = 0;
	};

	/** @} */
} // namespace b3d
