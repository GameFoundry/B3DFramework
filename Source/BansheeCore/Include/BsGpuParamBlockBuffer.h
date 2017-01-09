//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"

namespace bs
{
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
		GpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage);
		virtual ~GpuParamBlockBuffer();

		/**
		 * Write some data to the specified offset in the buffer. 
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void write(UINT32 offset, const void* data, UINT32 size);

		/**
		 * Read some data from the specified offset in the buffer.
		 *			
		 * @note	All values are in bytes. This reads from the cached CPU buffer and not from the GPU.
		 */
		void read(UINT32 offset, void* data, UINT32 size);

		/**
		 * Clear specified section of the buffer to zero.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void zeroOut(UINT32 offset, UINT32 size);

		/** Returns internal cached data of the buffer. */
		const UINT8* getCachedData() const { return mCachedData; }

		/**	Returns the size of the buffer in bytes. */
		UINT32 getSize() const { return mSize; }

		/**	Retrieves a core implementation of a GPU param block buffer usable only from the core thread. */
		SPtr<ct::GpuParamBlockBufferCore> getCore() const;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBuffer */
		static SPtr<GpuParamBlockBuffer> create(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC);

	protected:
		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		GpuParamBlockUsage mUsage;
		UINT32 mSize;
		UINT8* mCachedData;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a GpuParamBlockBuffer.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuParamBlockBufferCore : public CoreObject
	{
	public:
		GpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage, GpuDeviceFlags deviceMask);
		virtual ~GpuParamBlockBufferCore();

		/** 
		 * Writes all of the specified data to the buffer. Data size must be the same size as the buffer. 
		 *
		 * @param[in]	data		Data to write. Must match the size of the buffer.
		 * @param[in]	queueIdx	Device queue to perform the write operation on. See @ref queuesDoc.
		 */
		virtual void writeToGPU(const UINT8* data, UINT32 queueIdx = 0) = 0;

		/** 
		 * Flushes any cached data into the actual GPU buffer. 
		 *
		 * @param[in]	queueIdx	Device queue to perform the write operation on. See @ref queuesDoc.
		 */
		void flushToGPU(UINT32 queueIdx = 0);

		/**
		 * Write some data to the specified offset in the buffer. 
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering or until 
		 *			flushToGPU() is called.
		 */
		void write(UINT32 offset, const void* data, UINT32 size);

		/**
		 * Read some data from the specified offset in the buffer.
		 *			
		 * @note	All values are in bytes. This reads from the cached CPU buffer and not directly from the GPU.
		 */
		void read(UINT32 offset, void* data, UINT32 size);

		/**
		 * Clear specified section of the buffer to zero.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering or until 
		 *			flushToGPU() is called.
		 */
		void zeroOut(UINT32 offset, UINT32 size);

		/**	Returns the size of the buffer in bytes. */
		UINT32 getSize() const { return mSize; }

		/** @copydoc HardwareBufferManager::createGpuParamBlockBuffer */
		static SPtr<GpuParamBlockBufferCore> create(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data)  override;

		GpuParamBlockUsage mUsage;
		UINT32 mSize;

		UINT8* mCachedData;
		bool mGPUBufferDirty;
	};

	/** @} */
	}
}