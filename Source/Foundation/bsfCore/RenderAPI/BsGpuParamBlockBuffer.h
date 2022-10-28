//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	class HardwareBuffer;

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
	class BS_CORE_EXPORT GpuParamBlockBuffer : public CoreObject
	{
	public:
		GpuParamBlockBuffer(u32 size, GpuBufferUsage usage);
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
		static SPtr<GpuParamBlockBuffer> Create(u32 size, GpuBufferUsage usage = GBU_DYNAMIC);

	protected:
		SPtr<ct::CoreObject> CreateCore() const override;
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		GpuBufferUsage mUsage;
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
		class BS_CORE_EXPORT GpuParamBlockBuffer : public CoreObject
		{
		public:
			GpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);
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

			/** @copydoc HardwareBufferManager::CreateGpuParamBlockBuffer */
			static SPtr<GpuParamBlockBuffer> Create(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class HardwareBufferManager;

			void SyncToCore(const CoreSyncData& data) override;
			void Initialize() override;

			HardwareBuffer* mBuffer;

			GpuBufferUsage mUsage;
			u32 mSize;

			u8* mCachedData;
			bool mGPUBufferDirty;
		};

		/** @} */
	} // namespace ct
} // namespace bs
