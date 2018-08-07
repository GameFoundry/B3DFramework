//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "RenderAPI/BsHardwareBuffer.h"

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initialization of a GpuBuffer. */
	struct GPU_BUFFER_DESC
	{
		/** Number of elements in the buffer. */
		UINT32 elementCount = 0;

		/** 
		 * Size of each individual element in the buffer, in bytes. Only needed if using non-standard buffer. If using
		 * standard buffers element size is calculated from format and this must be zero.
		 */
		UINT32 elementSize = 0;

		/** Type of the buffer. Determines how is buffer seen by the GPU program and in what ways can it be used. */
		GpuBufferType type = GBT_STANDARD;

		/** Format if the data in the buffer. Only relevant for standard buffers, must be BF_UNKNOWN otherwise. */
		GpuBufferFormat format = BF_32X4F;

		/** Usage that tells the hardware how will be buffer be used. */
		GpuBufferUsage usage = GBU_STATIC;
	};

	/** 
	 * Information about a GpuBuffer. Allows core and non-core versions of GpuBuffer to share the same structure for 
	 * properties. 
	 */
	class BS_CORE_EXPORT GpuBufferProperties
	{
	public:
		GpuBufferProperties(const GPU_BUFFER_DESC& desc);

		/**
		 * Returns the type of the GPU buffer. Type determines which kind of views (if any) can be created for the buffer, 
		 * and how is data read or modified in it.
		 */
		GpuBufferType getType() const { return mDesc.type; }

		/** Returns format used by the buffer. Only relevant for standard buffers. */
		GpuBufferFormat getFormat() const { return mDesc.format; }

		/** Returns buffer usage which determines how are planning on updating the buffer contents. */
		GpuBufferUsage getUsage() const { return mDesc.usage; }

		/**	Returns number of elements in the buffer. */
		UINT32 getElementCount() const { return mDesc.elementCount; }

		/**	Returns size of a single element in the buffer in bytes. */
		UINT32 getElementSize() const { return mDesc.elementSize; }

	protected:
		friend class GpuBuffer;

		GPU_BUFFER_DESC mDesc;
	};

	/**
	 * Handles a generic GPU buffer that you may use for storing any kind of sequential data you wish to be accessible to
	 * the GPU.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuBuffer : public CoreObject
	{
	public:
		virtual ~GpuBuffer() = default;

		/** Returns properties describing the buffer. */
		const GpuBufferProperties& getProperties() const { return mProperties; }

		/** Retrieves a core implementation of a GPU buffer usable only from the core thread. */
		SPtr<ct::GpuBuffer> getCore() const;

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static UINT32 getFormatSize(GpuBufferFormat format);

		/** @copydoc HardwareBufferManager::createGpuBuffer */
		static SPtr<GpuBuffer> create(const GPU_BUFFER_DESC& desc);

	protected:
		friend class HardwareBufferManager;

		GpuBuffer(const GPU_BUFFER_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		GpuBufferProperties mProperties;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a bs::GpuBuffer.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuBuffer : public CoreObject, public HardwareBuffer
	{
	public:
		virtual ~GpuBuffer() = default;

		/** Returns properties describing the buffer. */
		const GpuBufferProperties& getProperties() const { return mProperties; }

		/** @copydoc HardwareBufferManager::createGpuBuffer */
		static SPtr<GpuBuffer> create(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/** 
		 * Creates a view of an existing hardware buffer. No internal buffer will be allocated and the provided buffer
		 * will be used for all internal operations instead. Information provided in @p desc (such as element size and
		 * count) must match the provided @p underlyingBuffer.
		 */
		GpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

		GpuBufferProperties mProperties;
		SPtr<HardwareBuffer> mExternalBuffer;
	};

	/** @} */
	}
}
