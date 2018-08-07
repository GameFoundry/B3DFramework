//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{ 
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an index buffer. */
	class GLIndexBuffer : public IndexBuffer
	{
	public:
		GLIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~GLIndexBuffer();

		/** @copydoc IndexBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source,
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<ct::CommandBuffer>& commandBuffer = nullptr) override;

		/**	Returns internal OpenGL index buffer handle. */
		GLuint getGLBufferId() const { return mBuffer->getGLBufferId(); }

	protected:
		/** @copydoc IndexBuffer::initialize */
		void initialize() override;	

		/** @copydoc IndexBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc IndexBuffer::unmap */
		void unmap() override;

	private:
		GLBuffer* mBuffer;
		GpuBufferUsage mUsage;
	};

	/** @} */
}}