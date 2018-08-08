//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Allocators/BsPoolAlloc.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/** Wrapper around a generic OpenGL buffer. */
	class GLBuffer : public HardwareBuffer
	{
	public:
		/** Creates and initializes the buffer object. */
		GLBuffer(GLenum target, UINT32 size, GpuBufferUsage usage);
		~GLBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, 
			UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length,
			bool discardWholeBuffer = false, const SPtr<ct::CommandBuffer>& commandBuffer = nullptr) override;

		/**	Returns internal OpenGL buffer ID. */
		GLuint getGLBufferId() const { return mBufferId; }

	private:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		GLenum mTarget;
		GLuint mBufferId = 0;

		bool mZeroLocked = false;
	};

	/** @} */
}}

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::GLBuffer, 32)
}