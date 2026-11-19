//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class GpuDevice;
	class GpuTransferQueue;
	class IGpuAllocator;
	class IGpuResource;

	namespace render
	{
		class GpuBuffer;
		class GpuCommandBuffer;
		class Texture;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Staging buffer created by GpuTransferQueue::CreateStagingBuffer(). Move-only; releases its staging memory if destroyed unused. */
	class B3D_EXPORT GpuStagingBuffer
	{
	public:
		GpuStagingBuffer() = default;
		GpuStagingBuffer(GpuStagingBuffer&& other) noexcept;
		GpuStagingBuffer& operator=(GpuStagingBuffer&& other) noexcept;
		~GpuStagingBuffer();

		GpuStagingBuffer(const GpuStagingBuffer&) = delete;
		GpuStagingBuffer& operator=(const GpuStagingBuffer&) = delete;

		/** Returns the CPU-writable buffer, or null if the handle is empty. */
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
	 * Uploads CPU data to buffers and textures through staging buffers. An upload runs before the first submitted command buffer that starts
	 * using its destination after the upload is queued, and at the latest at the end of the frame (GpuDevice::EndFrame()). Command buffers
	 * don't track bindless resources, so uploads to buffers created with GpuBufferFlag::Bindless and textures created with
	 * TextureUsageFlag::Bindless run before the next submitted command buffer instead. An upload that fully covers an earlier queued upload
	 * to the same destination replaces it.
	 *
	 * Uploads are recorded and submitted on the submit thread, on the first graphics queue, and wait for all earlier work on every queue.
	 */
	class B3D_EXPORT GpuTransferQueue
	{
	public:
		explicit GpuTransferQueue(GpuDevice& device);
		~GpuTransferQueue();

		GpuTransferQueue(const GpuTransferQueue&) = delete;
		GpuTransferQueue& operator=(const GpuTransferQueue&) = delete;

		/**
		 * Creates a single-use, CPU-writable staging buffer of @p size bytes. Pass it to QueueUpload() or RecordUpload(). Its memory is reused
		 * once the copy from it has executed, or once the handle is destroyed if it was never passed on. Returns an empty handle if the buffer
		 * can't be created. Thread safe.
		 */
		GpuStagingBuffer CreateStagingBuffer(u32 size);

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
		 * Gives @p destination a new native buffer and drops the uploads queued to the old one, unless uploads to the old one are queued and a
		 * recorded command buffer that isn't submitted yet uses it. Returns true if the buffer was recreated. Thread safe.
		 */
		bool TryRecreateInternalBuffer(render::GpuBuffer& destination);

		/** Texture equivalent of TryRecreateInternalBuffer(). Thread safe. */
		bool TryRecreateInternalTexture(render::Texture& destination);

		/** Submits the uploads queued to the current native resource of @p destination, without waiting for them. Thread safe. */
		void SubmitQueuedUploads(const render::GpuBuffer& destination);

		/** @copydoc SubmitQueuedUploads(const render::GpuBuffer&) */
		void SubmitQueuedUploads(const render::Texture& destination);

		/** Submits every queued upload, without waiting for them, and starts a new staging epoch. Called by GpuDevice::EndFrame(). */
		void SubmitQueuedUploads();

		/** @name Internal
		 *  @{
		 */

		/**
		 * Records the uploads queued to @p destinations, and those to bindless destinations, into a command buffer of the submit thread's graphics
		 * pool, and forgets them. Returns the command buffer, which the caller ends and submits, or null if none were queued. Submit thread only.
		 */
		TShared<render::GpuCommandBuffer> RecordQueuedUploads(TArrayView<IGpuResource* const> destinations);

		/** Same as RecordQueuedUploads(), but records every queued upload. Submit thread only. */
		TShared<render::GpuCommandBuffer> RecordAllQueuedUploads();

		/** Returns true while uploads to bindless destinations are queued. Thread safe. */
		bool HasUntrackedUploads() const { return mUntrackedDestinationCount.load(std::memory_order_relaxed) > 0; }

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

		/** Copy from a staging buffer queued to a destination. */
		struct QueuedUpload
		{
			TShared<render::GpuBuffer> Source;
			u64 Epoch = 0; /**< Staging epoch of Source. */
			u32 DestinationOffset = 0; /**< Byte offset in a destination buffer. */
			u32 Length = 0; /**< Number of bytes copied to a destination buffer. */
			u32 MipLevel = 0; /**< Mip level of a destination texture. */
			u32 ArrayLayer = 0; /**< Array layer of a destination texture. */
		};

		/** Uploads queued to one native resource, in queue order. */
		struct QueuedResourceUploads
		{
			TShared<render::GpuBuffer> Buffer; /**< Destination, if it is a buffer. */
			TShared<render::Texture> Texture; /**< Destination, if it is a texture. */
			bool IsUntracked = false; /**< True if command buffers don't track the destination, see HasUntrackedUploads(). */
			TInlineArray<QueuedUpload, 1> Uploads;
		};

		/**
		 * Staging buffers created since the previous epoch ended. An epoch is complete once it ended and the copies of all its staging buffers
		 * executed, or the buffers were released unused, and all earlier epochs are complete.
		 */
		struct StagingEpoch
		{
			u32 OutstandingCount = 0; /**< Staging buffers that are neither released nor known to be executed. */
			u64 StagingByteCount = 0; /**< Bytes of staging buffers created in the epoch. */

			/** Staging buffers whose copies are recorded. Each is released once no command buffer references it any more. */
			TArray<TShared<render::GpuBuffer>> RecordedBuffers;
		};

		/** Proxies to release outside of the mutex. */
		struct ReleasedProxies
		{
			TArray<TShared<render::GpuBuffer>> Buffers;
			TArray<TShared<render::Texture>> Textures;
		};

		/** Returns the entry of uploads queued to @p buffer, whose native resource is @p destination, adding one if there is none. Caller holds the mutex. */
		QueuedResourceUploads& GetOrAddEntry(IGpuResource& destination, const TShared<render::GpuBuffer>& buffer);

		/** Returns the entry of uploads queued to @p texture, whose native resource is @p destination, adding one if there is none. Caller holds the mutex. */
		QueuedResourceUploads& GetOrAddEntry(IGpuResource& destination, const TShared<render::Texture>& texture);

		/** Adds an entry of uploads queued to @p destination, marking the resource. Caller holds the mutex. */
		QueuedResourceUploads& AddEntry(IGpuResource& destination);

		/** Releases the staging buffer of @p upload, whose copy will never be recorded. Caller holds the mutex. */
		void DropUpload(QueuedUpload& upload, ReleasedProxies& releasedProxies);

		/**
		 * Drops every upload of the entry at @p iterEntry, and the entry. Doesn't access the destination resource, which may already be destroyed
		 * if its proxy changed native resources. Caller holds the mutex.
		 */
		void DropEntry(UnorderedMap<IGpuResource*, QueuedResourceUploads>::iterator iterEntry, ReleasedProxies& releasedProxies);

		/** Records the uploads of the entry of @p destination into @p commandBuffer, creating it if null, and forgets them. Caller holds the mutex. */
		void RecordEntry(IGpuResource& destination, TShared<render::GpuCommandBuffer>& commandBuffer);

		/** Moves @p source into the recorded staging buffers of its epoch. Caller holds the mutex. */
		void AddRecordedBuffer(TShared<render::GpuBuffer> source, u64 epoch);

		/** Counts a staging buffer of @p epoch as released. Thread safe. */
		void ReleaseStagingBuffer(u64 epoch);

		/** Ends the current staging epoch and reuses the staging memory of complete epochs. Thread safe. */
		void EndEpoch();

		/** Releases recorded staging buffers no command buffer references, and forgets complete epochs. Caller holds the mutex. */
		void UpdateEpochs(ReleasedProxies& releasedProxies);

		/** Returns the epoch @p epoch, which must not be complete. Caller holds the mutex. */
		StagingEpoch& GetEpoch(u64 epoch);

		GpuDevice& mDevice;
		EpochTracker mEpochTracker;
		mutable Mutex mMutex;

		UnorderedMap<IGpuResource*, QueuedResourceUploads> mQueuedUploadsByDestination;
		std::atomic<u32> mUntrackedDestinationCount = 0; /**< Number of entries with QueuedResourceUploads::IsUntracked. */

		/** Thread-safe staging allocators per memory type. Null for memory types the backend has no scratch allocator for. */
		UnorderedMap<u32, TUnique<IGpuAllocator>> mStagingAllocators;

		/** Epochs from mFirstIncompleteEpoch to mCurrentEpoch, the last one being the current epoch. */
		Deque<StagingEpoch> mEpochs;
		std::atomic<u64> mFirstIncompleteEpoch = 0;
		std::atomic<u64> mCurrentEpoch = 0;

		/** Destinations of uploads the submit thread recorded, released on the next SubmitQueuedUploads() rather than on the submit thread. */
		ReleasedProxies mRecordedDestinations;
	};

	/** @} */
} // namespace b3d
