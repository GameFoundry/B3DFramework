//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuBufferView.h"
#include "BsCoreObject.h"

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** 
	 * Information about a GpuBuffer. Allows core and non-core versions of GpuBuffer to share the same structure for 
	 * properties. 
	 */
	class BS_CORE_EXPORT GpuBufferProperties
	{
	public:
		GpuBufferProperties(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format,
			GpuBufferUsage usage, bool randomGpuWrite, bool useCounter);

		/**
		 * Returns the type of the GPU buffer. Type determines which kind of views (if any) can be created for the buffer, 
		 * and how is data read or modified in it.
		 */
		GpuBufferType getType() const { return mType; }

		/** Returns format used by the buffer. Only relevant for standard buffers. */
		GpuBufferFormat getFormat() const { return mFormat; }

		/** Returns buffer usage which determines how are planning on updating the buffer contents. */
		GpuBufferUsage getUsage() const { return mUsage; }

		/** Return whether the buffer supports random reads and writes within the GPU programs. */
		bool getRandomGpuWrite() const { return mRandomGpuWrite; }

		/**	Returns whether the buffer supports counter use within GPU programs. */
		bool getUseCounter() const { return mUseCounter; }

		/**	Returns number of elements in the buffer. */
		UINT32 getElementCount() const { return mElementCount; }

		/**	Returns size of a single element in the buffer in bytes. */
		UINT32 getElementSize() const { return mElementSize; }

	protected:
		GpuBufferType mType;
		GpuBufferUsage mUsage;
		GpuBufferFormat mFormat;
		bool mRandomGpuWrite;
		bool mUseCounter;
		UINT32 mElementCount;
		UINT32 mElementSize;
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
		SPtr<GpuBufferCore> getCore() const;

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static UINT32 getFormatSize(GpuBufferFormat format);

	protected:
		friend class HardwareBufferManager;

		GpuBuffer(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format, GpuBufferUsage usage,
			bool randomGpuWrite = false, bool useCounter = false);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		GpuBufferProperties mProperties;
    };

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a GpuBuffer.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuBufferCore : public CoreObjectCore
	{
	public:
		virtual ~GpuBufferCore();

		/**
		 * Locks the buffer returning a pointer to the internal buffer data that you may then read or write to. 
		 * Caller must ensure it will only perform actions promised in the provided GPU lock options parameter.
		 *
		 * @param[in]	offset	Number of bytes at which to lock the buffer. Returned pointer points to this location.
		 * @param[in]	length	Number of bytes to lock.
		 * @param[in]	options How to lock the buffer. Certain options offer better performance than others.
		 */
		virtual void* lock(UINT32 offset, UINT32 length, GpuLockOptions options) = 0;

		/**
		 * Unlocks a previously locked buffer. Any pointers to internal buffers returned when it was locked will become 
		 * invalid.
		 */
		virtual void unlock() = 0;

		/**
		 * Reads buffer data into the previously allocated buffer.
		 *
		 * @param[in]	offset	Number of bytes at which to start reading the buffer.
		 * @param[in]	length	Number of bytes to read.
		 * @param[in]	pDest	Previously allocated buffer of @p length bytes size that the data will be written to.
		 */
        virtual void readData(UINT32 offset, UINT32 length, void* pDest) = 0;

		/**
		 * Writes data into the buffer.
		 *
		 * @param[in]	offset		Number of bytes at which to start writing to the buffer.
		 * @param[in]	length		Number of bytes to write.
		 * @param[in]	pSource		Buffer containg the data to write.
		 * @param[in]	writeFlags  Flags that may be used to improve performance for specific use cases.
		 */
        virtual void writeData(UINT32 offset, UINT32 length, const void* pSource, 
			BufferWriteType writeFlags = BufferWriteType::Normal) = 0;

		/**
		 * Copies data from another buffer into this buffer.
		 *
		 * @param[in]	srcBuffer			Buffer to copy the data from.
		 * @param[in]	srcOffset			Offset in bytes into the source buffer - this is where reading starts from.
		 * @param[in]	dstOffset			Offset in bytes into the destination buffer - this is where writing starts from.
		 * @param[in]	length				Number of bytes to copy from source to destination.
		 * @param[in]	discardWholeBuffer	If true, the contents of the current buffer will be entirely discarded. This can
		 *									improve performance if you know you wont be needing that data any more.
		 */
		virtual void copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) = 0;

		/** Returns properties describing the buffer. */
		const GpuBufferProperties& getProperties() const { return mProperties; }

		/**
		 * Creates a buffer view that may be used for binding a buffer to a slot in the pipeline. Views allow you to specify
		 * how is data in the buffer organized to make it easier for the pipeline to interpret.
		 *
		 * @param[in]	buffer			Buffer to create the view for.
		 * @param[in]	firstElement	Position of the first element visible by the view.
		 * @param[in]	numElements		Number of elements to bind to the view.
		 * @param[in]	usage			Determines type of the view we are creating, and which slots in the pipeline will
		 *								the view be bindable to.
		 *
		 * @note If a view with this exact parameters already exists, it will be returned and new one will not be created.
		 * @note Only Default and RandomWrite views are supported for this type of buffer. 
		 */
		// TODO Low Priority: Perhaps reflect usage flag limitation by having an enum with only the supported two options?
		static GpuBufferView* requestView(const SPtr<GpuBufferCore>& buffer, UINT32 firstElement, 
			UINT32 numElements, GpuViewUsage usage);

		/**
		 * Releases a view created with requestView. 
		 *
		 * @note	View will only truly get released once all references to it are released.
		 */
		static void releaseView(GpuBufferView* view);

	protected:
		GpuBufferCore(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format,
			GpuBufferUsage usage, bool randomGpuWrite = false, bool useCounter = false);

		/** Creates an empty view for the current buffer. */
		virtual GpuBufferView* createView() = 0;

		/**	Destroys a view previously created for this buffer. */
		virtual void destroyView(GpuBufferView* view) = 0;

		/**	Destroys all buffer views regardless if their reference count is zero or not. */
		void clearBufferViews();

		/**	Helper class to help with reference counting for GPU buffer views. */
		struct GpuBufferReference
		{
			GpuBufferReference(GpuBufferView* _view)
				:view(_view), refCount(0)
			{ }

			GpuBufferView* view;
			UINT32 refCount;
		};

		UnorderedMap<GPU_BUFFER_DESC, GpuBufferReference*, GpuBufferView::HashFunction, GpuBufferView::EqualFunction> mBufferViews;
		GpuBufferProperties mProperties;
	};

	/** @} */
}
