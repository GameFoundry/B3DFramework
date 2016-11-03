//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuBuffer.h"
#include "BsGLBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a generic GPU buffer. */
	class BS_RSGL_EXPORT GLGpuBufferCore : public GpuBufferCore
	{
	public:
		~GLGpuBufferCore();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource,
				BufferWriteType writeFlags = BWT_NORMAL) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) override;

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
		friend class GLHardwareBufferCoreManager;

		GLGpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuBufferCore::initialize */
		void initialize() override;

		GLuint mTextureID;
		GLBuffer mBuffer;
		GLenum mFormat;
	};

	/** @} */
}