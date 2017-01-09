//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"
#include "BsHardwareBuffer.h"

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initialization of a GpuBuffer. */
	struct GPU_BUFFER_DESC
	{
		/** Number of elements in the buffer. */
		UINT32 elementCount; 

		/** 
		 * Size of each individual element in the buffer, in bytes. Only needed if using non-standard buffer. If using
		 * standard buffers element size is calculated from format and this must be zero.
		 */
		UINT32 elementSize;

		/** Type of the buffer. Determines how is buffer seen by the GPU program and in what ways can it be used. */
		GpuBufferType type;

		/** Format if the data in the buffer. Only relevant for standard buffers, must be BF_UNKNOWN otherwise. */
		GpuBufferFormat format;

		/** Usage that tells the hardware how will be buffer be used. */
		GpuBufferUsage usage = GBU_STATIC;

		/** When true allows the GPU to write to the resource. Must be enabled if buffer type is GBT_APPENDCONSUME. */
		bool randomGpuWrite = false;

		/**
		 * When true binds a counter that can be used from a GPU program on the buffer. Can only be used in combination
		 * with GBT_STRUCTURED and randomGpuWrite must be enabled.
		 */
		bool useCounter = false;
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

		/** Return whether the buffer supports random reads and writes within the GPU programs. */
		bool getRandomGpuWrite() const { return mDesc.randomGpuWrite; }

		/**	Returns whether the buffer supports counter use within GPU programs. */
		bool getUseCounter() const { return mDesc.useCounter; }

		/**	Returns number of elements in the buffer. */
		UINT32 getElementCount() const { return mDesc.elementCount; }

		/**	Returns size of a single element in the buffer in bytes. */
		UINT32 getElementSize() const { return mDesc.elementSize; }

	protected:
		friend class GpuBuffer;

		GPU_BUFFER_DESC mDesc;
	};

	/**
	 * Handles a generic GPU buffer that you may use for storing any kind of data you wish to be accessible to the GPU.
	 * These buffers may be bounds to GPU program binding slots and accessed from a GPU program, or may be used by fixed 
	 * pipeline in some way.
	 *
	 * Buffer types:
	 *  - Raw buffers containing a block of bytes that are up to the GPU program to interpret.
	 *	- Structured buffer containing an array of structures compliant to a certain layout. Similar to raw buffer but 
	 *    easier to interpret the data.
	 *	- Random read/write buffers that allow you to write to random parts of the buffer from within the GPU program, and 
	 *    then read it later. These can only be bound to pixel and compute stages.
	 *	- Append/Consume buffers also allow you to write to them, but in a stack-like fashion, usually where one set of 
	 *    programs produces data while other set consumes it from the same buffer. Append/Consume buffers are structured
	 *	  by default.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuBuffer : public CoreObject
    {
    public:
		virtual ~GpuBuffer() { }

		/** Returns properties describing the buffer. */
		const GpuBufferProperties& getProperties() const { return mProperties; }

		/** Retrieves a core implementation of a GPU buffer usable only from the core thread. */
		SPtr<ct::GpuBufferCore> getCore() const;

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static UINT32 getFormatSize(GpuBufferFormat format);

		/** @copydoc HardwareBufferManager::createGpuBuffer */
		static SPtr<GpuBuffer> create(const GPU_BUFFER_DESC& desc);

	protected:
		friend class HardwareBufferManager;

		GpuBuffer(const GPU_BUFFER_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObjectCore> createCore() const override;

		GpuBufferProperties mProperties;
    };

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a GpuBuffer.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuBufferCore : public CoreObjectCore, public HardwareBuffer
	{
	public:
		virtual ~GpuBufferCore();

		/** Returns properties describing the buffer. */
		const GpuBufferProperties& getProperties() const { return mProperties; }

		/** @copydoc HardwareBufferManager::createGpuBuffer */
		static SPtr<GpuBufferCore> create(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		GpuBufferCore(const GPU_BUFFER_DESC& desc, UINT32 deviceMask);

		GpuBufferProperties mProperties;
	};

	/** @} */
	}
}
