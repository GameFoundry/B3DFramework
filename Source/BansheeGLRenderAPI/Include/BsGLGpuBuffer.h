//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a generic GPU buffer. */
	class BS_RSGL_EXPORT GLGpuBuffer : public GpuBufferCore
	{
	public:
		~GLGpuBuffer();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* source,
				BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false, UINT32 queueIdx = 0) override;

		/**	
		 * Returns internal OpenGL buffer ID. If binding the buffer to the pipeline, bind the texture using
		 * getGLTextureId() instead. 
		 */
		GLuint getGLBufferId() const { return mBuffer.getGLBufferId(); }

		/**	Returns internal OpenGL texture ID. */
		GLuint getGLTextureId() const { return mTextureID; }

		/** Returns the internal OpenGL format used by the elements of the buffer. */
		GLuint getGLFormat() const { return mFormat; }

	protected:
		friend class GLHardwareBufferManager;

		GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuBufferCore::initialize */
		void initialize() override;

		GLuint mTextureID;
		GLBuffer mBuffer;
		GLenum mFormat;
	};

	/** @} */
}}