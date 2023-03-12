//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuBuffer.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	class GpuBuffer;

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Represents a GPU parameter block buffer. Parameter block buffers are bound to GPU programs which then fetch
	 * parameters from those buffers.
	 *
	 * Writing or reading from this buffer will translate directly to API calls that update the GPU.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT GpuParamBlockBuffer : public CoreObject
	{
	public:
		GpuParamBlockBuffer(u32 size, GpuBufferFlags flags);
		virtual ~GpuParamBlockBuffer();

		/**
		 * Write some data to the specified offset in the buffer.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void Write(u32 offset, const void* data, u32 size);

		/**
		 * Read some data from the specified offset in the buffer.
		 *
		 * @note	All values are in bytes. This reads from the cached CPU buffer and not from the GPU.
		 */
		void Read(u32 offset, void* data, u32 size);

		/**
		 * Clear specified section of the buffer to zero.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void ZeroOut(u32 offset, u32 size);

		/** Returns internal cached data of the buffer. */
		const u8* GetCachedData() const { return mCachedData; }

		/**	Returns the size of the buffer in bytes. */
		u32 GetSize() const { return mSize; }

		/**	Retrieves a core implementation of a GPU param block buffer usable only from the core thread. */
		SPtr<ct::GpuParamBlockBuffer> GetCore() const;

		/** @copydoc HardwareBufferManager::CreateGpuParamBlockBuffer */
		static SPtr<GpuParamBlockBuffer> Create(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU);

	protected:
		SPtr<ct::CoreObject> CreateCore() const override;
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		GpuBufferFlags mFlags;
		u32 mSize;
		u8* mCachedData;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Core thread version of a bs::GpuParamBlockBuffer.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT GpuParamBlockBuffer : public CoreObject
		{
		public:
			GpuParamBlockBuffer(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask);
			GpuParamBlockBuffer(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size);
			virtual ~GpuParamBlockBuffer();

			/**
			 * Writes all of the specified data to the buffer. Data size must be the same size as the buffer.
			 *
			 * @param[in]	data		Data to write. Must match the size of the buffer.
			 * @param[in]	queueIdx	Device queue to perform the write operation on. See @ref queuesDoc.
			 */
			void WriteToGpu(const u8* data, u32 queueIdx = 0);

			/**
			 * Flushes any cached data into the actual GPU buffer.
			 *
			 * @param[in]	queueIdx	Device queue to perform the write operation on. See @ref queuesDoc.
			 */
			void FlushToGpu(u32 queueIdx = 0);

			/**
			 * Write some data to the specified offset in the buffer.
			 *
			 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering or until
			 *			flushToGPU() is called.
			 */
			void Write(u32 offset, const void* data, u32 size);

			/**
			 * Read some data from the specified offset in the buffer.
			 *
			 * @note	All values are in bytes. This reads from the cached CPU buffer and not directly from the GPU.
			 */
			void Read(u32 offset, void* data, u32 size);

			/**
			 * Clear specified section of the buffer to zero.
			 *
			 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering or until
			 *			flushToGPU() is called.
			 */
			void ZeroOut(u32 offset, u32 size);

			/**	Returns the size of the buffer in bytes. */
			u32 GetSize() const { return mSize; }

			/** If the parameter block buffer was created using external backing memory, this allows you to change at which offset (in bytes) does the GPU parameter block start at. */
			void SetOffset(u32 offset) { mOffset = offset; }

			/** @copydoc SetOffset */
			u32 GetOffset() const { return mOffset; }

			/**
			 * Creates an GPU parameter block that you can use for setting parameters for GPU programs. 
			 *
			 * @param	size		Size of the parameter buffer in bytes.
			 * @param	flags		Flags that control the behavior of the buffer.
			 * @param	deviceMask	Usage mask that determines on which devices will the buffer be created on.
			 * @return				Newly created buffer.
			 */
			static SPtr<GpuParamBlockBuffer> Create(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates an GPU parameter block that you can use for setting parameters for GPU programs. Uses a pre-allocated buffer for its backing memory.
			 *
			 * @param	backingMemory		Buffer that contains the memory for the GPU parameter block buffer. Must be at least of @p size bytes.
			 * @param	offset				Offset into backing memory at which the parameter block buffer starts.
			 * @param	size				Size of the parameter buffer in bytes.
			 * @return						Newly created buffer.
			 */
			static SPtr<GpuParamBlockBuffer> Create(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size);
		protected:
			friend class HardwareBufferManager;

			void SyncToCore(const CoreSyncData& data) override;
			void Initialize() override;

			SPtr<GpuBuffer> mBuffer;

			GpuBufferFlags mBufferFlags;
			GpuDeviceFlags mDeviceMask = GDF_DEFAULT;
			u32 mSize;
			u32 mOffset = 0;

			u8* mCachedData;
			bool mGPUBufferDirty;
		};

		/** @} */
	} // namespace ct
} // namespace bs
