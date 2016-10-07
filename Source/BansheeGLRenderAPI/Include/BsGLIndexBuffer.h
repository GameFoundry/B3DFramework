//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsIndexBuffer.h"
#include "BsGLBuffer.h"

namespace BansheeEngine 
{ 
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an index buffer. */
    class BS_RSGL_EXPORT GLIndexBufferCore : public IndexBufferCore
    {
    public:
		GLIndexBufferCore(IndexType idxType, UINT32 numIndices, GpuBufferUsage usage);
		~GLIndexBufferCore();

		/** @copydoc IndexBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* dest) override;

		/** @copydoc IndexBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/**	Returns internal OpenGL index buffer handle. */
        GLuint getGLBufferId() const { return mBuffer.getGLBufferId(); }

	protected:
		/** @copydoc IndexBufferCore::initialize */
		void initialize() override;	

		/** @copydoc IndexBufferCore::lockImpl */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc IndexBufferCore::unlockImpl */
		void unmap() override;

	private:
		GLBuffer mBuffer;
    };

	/** @} */
}