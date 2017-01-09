//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsIndexBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{ 
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an index buffer. */
    class BS_RSGL_EXPORT GLIndexBuffer : public IndexBufferCore
    {
    public:
		GLIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~GLIndexBuffer();

		/** @copydoc IndexBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/**	Returns internal OpenGL index buffer handle. */
        GLuint getGLBufferId() const { return mBuffer.getGLBufferId(); }

	protected:
		/** @copydoc IndexBufferCore::initialize */
		void initialize() override;	

		/** @copydoc IndexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc IndexBufferCore::unmap */
		void unmap() override;

	private:
		GLBuffer mBuffer;
		GpuBufferUsage mUsage;
    };

	/** @} */
}}