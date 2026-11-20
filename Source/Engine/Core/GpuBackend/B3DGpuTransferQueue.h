//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "Threading/B3DAsyncOp.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class GpuDevice;
	class GpuTransferQueue;
	class IGpuAllocator;
	class IGpuResource;
	struct GpuBufferCreateInformation;

	namespace render
	{
		class GpuBuffer;
		class GpuCommandBuffer;
		class Texture;
	}

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
	 * Transfers are recorded and submitted on the submit thread, on the first graphics queue, and wait for all earlier work on every queue.
	 */
	class B3D_EXPORT GpuTransferQueue
	{
	public:
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
		 * exchange native resources, so @p resource gets the new memory and @p oldResourceHolder holds the old one until the queued copy of
		 * the old contents into the new memory has executed. The copy runs after the transfers queued to the old native resource. No recorded
		 * command buffer may use the old native resource. Render thread only.
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

		/** Command buffer the queue recorded transfers into, for the submit thread to submit. */
		struct RecordedTransfers
		{
			/** Null if nothing was recorded. Ended and queued for submission on the first graphics queue. The caller submits it there. */
			TShared<render::GpuCommandBuffer> CommandBuffer;
			bool HasReads = false; /**< True if a read was recorded. */
		};

		/**
		 * Records the transfers queued to @p resources, and those to bindless resources, into a command buffer of the submit thread's graphics
		 * pool, and forgets them. Submit thread only.
		 */
		RecordedTransfers RecordQueuedTransfers(TArrayView<IGpuResource* const> resources);

		/** Same as RecordQueuedTransfers(), but for the transfers queued to the current native resource of @p resource. Submit thread only. */
		RecordedTransfers RecordQueuedTransfers(const render::GpuBuffer& resource);

		/** @copydoc RecordQueuedTransfers(const render::GpuBuffer&) */
		RecordedTransfers RecordQueuedTransfers(const render::Texture& resource);

		/** Same as RecordQueuedTransfers(), but records every queued transfer. Submit thread only. */
		RecordedTransfers RecordAllQueuedTransfers();

		/** Returns true while transfers to bindless resources are queued. Thread safe. */
		bool HasBindlessTransfers() const { return mBindlessTransferListCount.load(std::memory_order_relaxed) > 0; }

		/** @} */

	private:
		friend class GpuStagingBuffer;

		/** Completion tracker of the staging allocators. Its markers are staging epochs. */
		class EpochTracker final : public IGpuCompletionTracker
		{
		public:
			explicit EpochTracker(const GpuTransferQueue& queue) : mQueue(queue) {}

			/** @copydoc IGpuCompletionTracker::GetCurrentMarker */
			u64 GetCurrentMarker() const override { return mQueue.mCurrentEpoch.load(std::memory_order_acquire); }

			/** @copydoc IGpuCompletionTracker::IsMarkerComplete */
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

		/** Result state of a read queued by QueueRead(). */
		struct AsyncRead
		{
			GpuStagingBuffer Staging; /**< Staging buffer the read copies into. */
			TAsyncOp<TShared<MemoryDataStream>> BufferOperation; /**< Operation of a buffer read. */
			TAsyncOp<TShared<PixelData>> TextureOperation; /**< Operation of a texture read. */
			TShared<PixelData> TexturePixelData; /**< Layout of a texture read's staging data, and its result. Null for a buffer read. */
			u32 Length = 0; /**< Bytes a buffer read copies out. */

			/** Completes the operation with the contents of the staging buffer, or with null if @p isExecuted is false, and releases the staging buffer. */
			void Complete(bool isExecuted);
		};

		/** Transfer queued to a native resource. */
		struct QueuedTransfer
		{
			QueuedTransferType Type = QueuedTransferType::Upload;

			/** Upload: copy source. CopyToStaging: copy destination. Unused by the other types. */
			TShared<render::GpuBuffer> Staging;
			u64 Epoch = 0; /**< Staging epoch of an upload's staging buffer. */

			/**
			 * Resource the transfer accesses, if it is a buffer: the destination of an upload or a relocation, or the resource a read reads.
			 * Holds the native resource the transfer was queued to.
			 */
			TShared<render::GpuBuffer> Buffer;
			TShared<render::Texture> Texture; /**< Texture equivalent of Buffer. */
			TShared<render::GpuBuffer> RelocationSourceBuffer; /**< Holder of the old native buffer a relocation copies from. */
			TShared<render::Texture> RelocationSourceTexture; /**< Holder of the old native texture a relocation copies from. */

			u32 Offset = 0; /**< Byte offset in a buffer. */
			u32 Length = 0; /**< Number of bytes transferred to or from a buffer. */
			u32 MipLevel = 0; /**< Mip level of a texture. */
			u32 ArrayLayer = 0; /**< Array layer of a texture. */

			// Qualified, because a member named like the type would change the meaning of an unqualified AsyncRead in this struct
			TShared<GpuTransferQueue::AsyncRead> AsyncRead; /**< Read queued by QueueRead(). Null for other transfers. */
		};

		/** Transfers queued to one native resource, in queue order. */
		struct TransferList
		{
			/**
			 * Resource whose current native resource the list is for, if it is a buffer. Transfers queued before the resource was relocated
			 * hold the old native resource's holder instead.
			 */
			TShared<render::GpuBuffer> Buffer;
			TShared<render::Texture> Texture; /**< Texture equivalent of Buffer. */
			bool IsBindless = false; /**< True if command buffers don't track the resource, see HasBindlessTransfers(). */
			TInlineArray<QueuedTransfer, 1> Transfers;
		};

		/**
		 * Staging buffers created since the previous epoch ended. An epoch is complete once it ended and the copies of all its staging buffers
		 * executed, or the buffers were released, and all earlier epochs are complete.
		 */
		struct StagingEpoch
		{
			u32 UnreleasedBufferCount = 0; /**< Staging buffers that are neither released nor known to be executed. */
			u64 StagingByteCount = 0; /**< Bytes of staging buffers created in the epoch. */

			/** Upload staging buffers whose copies are recorded. Each is released once no command buffer references it any more. */
			TArray<TShared<render::GpuBuffer>> RecordedUploadBuffers;
		};

		/** Objects to release outside of the mutex. */
		struct ProxiesToRelease
		{
			TArray<TShared<render::GpuBuffer>> Buffers;
			TArray<TShared<render::Texture>> Textures;
			TArray<TShared<AsyncRead>> Reads; /**< Queued reads dropped before they were recorded, completed with null on release. */

			ProxiesToRelease() = default;
			ProxiesToRelease(ProxiesToRelease&& other) noexcept = default;
			ProxiesToRelease& operator=(ProxiesToRelease&& other) noexcept;
			~ProxiesToRelease();
		};

		/** Creates a staging buffer described by @p createInformation in the current epoch. Thread safe. */
		GpuStagingBuffer CreateStagingBuffer(const GpuBufferCreateInformation& createInformation);

		/** Returns the transfer list of @p buffer, whose native resource is @p resource, adding one if there is none. Caller holds the mutex. */
		TransferList& FindOrAddTransferList(IGpuResource& resource, const TShared<render::GpuBuffer>& buffer);

		/** Returns the transfer list of @p texture, whose native resource is @p resource, adding one if there is none. Caller holds the mutex. */
		TransferList& FindOrAddTransferList(IGpuResource& resource, const TShared<render::Texture>& texture);

		/** Adds an empty transfer list for @p resource, marking the resource. Caller holds the mutex. */
		TransferList& AddTransferList(IGpuResource& resource);

		/**
		 * Appends @p upload to @p transferList, dropping the queued uploads it fully covers that come after the last transfer that isn't an
		 * upload. @p fnCovers(const QueuedTransfer&) tells if the new upload covers a queued one. Caller holds the mutex.
		 */
		template<class CoversFunction>
		void AppendUpload(TransferList& transferList, QueuedTransfer&& upload, CoversFunction fnCovers, ProxiesToRelease& proxiesToRelease);

		/** Appends @p transfer, a read of @p buffer, to the transfer list of its current native resource and returns that resource. Thread safe. */
		IGpuResource* AppendRead(const TShared<render::GpuBuffer>& buffer, QueuedTransfer&& transfer);

		/** Texture equivalent of AppendRead(const TShared<render::GpuBuffer>&, QueuedTransfer&&). Thread safe. */
		IGpuResource* AppendRead(const TShared<render::Texture>& texture, QueuedTransfer&& transfer);

		/**
		 * Moves the transfer list of @p oldResource, if any, to @p newResource, the new native resource of @p resource after a relocation, and
		 * points the transfers that meant the old native resource at @p oldResourceHolder. Returns the transfer list of @p newResource, adding
		 * one if needed. Caller holds the mutex.
		 */
		template<class ProxyType>
		TransferList& MoveTransferListForRelocation(IGpuResource* oldResource, IGpuResource& newResource, const TShared<ProxyType>& resource,
			const TShared<ProxyType>& oldResourceHolder);

		/** Releases the resources of @p transfer, which will never be recorded. Caller holds the mutex. */
		void DropTransfer(QueuedTransfer& transfer, ProxiesToRelease& proxiesToRelease);

		/**
		 * Drops every transfer of the transfer list at @p iterTransferList, and the list. Doesn't access the native resource, which may already
		 * be destroyed if its proxy changed native resources. Caller holds the mutex.
		 */
		void DropTransferList(UnorderedMap<IGpuResource*, TransferList>::iterator iterTransferList, ProxiesToRelease& proxiesToRelease);

		/** Shared part of both TryRecreateNativeResource() overloads. Caller holds the mutex. */
		bool TryDropTransferListForRecreate(IGpuResource* resource, ProxiesToRelease& proxiesToRelease);

		/**
		 * Records the transfer list of @p resource into @p recorded, creating its command buffer if null, and forgets it. Adds the reads queued
		 * by QueueRead() to @p asyncReads. Caller holds the mutex.
		 */
		void RecordTransferList(IGpuResource& resource, RecordedTransfers& recorded, TArray<TShared<AsyncRead>>& asyncReads);

		/** Records the transfer lists of bindless resources, as RecordTransferList() does. Caller holds the mutex. */
		void RecordBindlessTransferLists(RecordedTransfers& recorded, TArray<TShared<AsyncRead>>& asyncReads);

		/**
		 * Ends the command buffer of @p recorded, if any, and queues it for submission. Then clears the queued transfers flag of the resources
		 * whose transfer lists it recorded. Caller holds the mutex.
		 */
		void EndRecording(RecordedTransfers& recorded);

		/** Makes the command buffer of @p recorded complete @p asyncReads, if there are any. */
		static void CompleteAsyncReadsAfterExecution(const RecordedTransfers& recorded, TArray<TShared<AsyncRead>>&& asyncReads);

		/** Moves @p source into the recorded staging buffers of its epoch. Caller holds the mutex. */
		void RetainRecordedStagingBuffer(TShared<render::GpuBuffer> source, u64 epoch);

		/** Records that a staging buffer of @p epoch was released, so the epoch no longer waits for it. Thread safe. */
		void NotifyStagingBufferReleased(u64 epoch);

		/** Ends the current staging epoch and reuses the staging memory of complete epochs. Thread safe. */
		void EndEpoch();

		/** Releases recorded staging buffers no command buffer references, and forgets complete epochs. Caller holds the mutex. */
		void ReleaseCompleteEpochs(ProxiesToRelease& proxiesToRelease);

		/** Returns the epoch @p epoch, which must not be complete. Caller holds the mutex. */
		StagingEpoch& GetEpoch(u64 epoch);

		GpuDevice& mDevice;
		EpochTracker mEpochTracker;
		mutable Mutex mMutex;

		UnorderedMap<IGpuResource*, TransferList> mTransferListsByResource;
		std::atomic<u32> mBindlessTransferListCount = 0; /**< Number of transfer lists with TransferList::IsBindless. */

		/** Thread-safe staging allocators per memory type. Null for memory types the backend has no scratch allocator for. */
		UnorderedMap<u32, TUnique<IGpuAllocator>> mStagingAllocators;

		/** Epochs from mFirstIncompleteEpoch to mCurrentEpoch, the last one being the current epoch. */
		Deque<StagingEpoch> mEpochs;
		std::atomic<u64> mFirstIncompleteEpoch = 0;
		std::atomic<u64> mCurrentEpoch = 0;

		/** Resources of transfers the submit thread recorded, released on the next SubmitAllQueuedTransfers() rather than on the submit thread. */
		ProxiesToRelease mSubmitThreadProxiesToRelease;

		/** Native resources whose transfer lists the submit thread is recording, see EndRecording(). */
		TArray<IGpuResource*> mResourcesInRecording;
	};

	/** @} */
} // namespace b3d
