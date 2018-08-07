//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a generic GPU buffer. */
	class GLGpuBuffer : public GpuBuffer
	{
	public:
		~GLGpuBuffer();

		/** @copydoc GpuBuffer::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::unlock */
		void unlock() override;

		/** @copydoc GpuBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source,
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/**	
		 * Returns internal OpenGL buffer ID. If binding the buffer to the pipeline, bind the texture using
		 * getGLTextureId() instead. 
		 */
		GLuint getGLBufferId() const { return mBuffer->getGLBufferId(); }

		/**	Returns internal OpenGL texture ID. */
		GLuint getGLTextureId() const { return mTextureID; }

		/** Returns the internal OpenGL format used by the elements of the buffer. */
		GLuint getGLFormat() const { return mFormat; }

	protected:
		friend class GLHardwareBufferManager;

		GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		GLGpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<GLBuffer>& underlyingBuffer);

		/** @copydoc GpuBuffer::initialize */
		void initialize() override;

		GLuint mTextureID = 0;
		GLBuffer* mBuffer = nullptr;
		GLenum mFormat = 0;
	};

	/** @} */
}}