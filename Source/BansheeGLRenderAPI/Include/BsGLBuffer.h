//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	/** Wrapper around a generic OpenGL buffer. */
    class BS_RSGL_EXPORT GLBuffer
    {
    public:
		/** Creates an uninitialized buffer object. You must call initialize() before using it. */
		GLBuffer();

		/** Creates and initializes the buffer object. */
		GLBuffer(GLenum target, UINT32 size, GpuBufferUsage usage);
		~GLBuffer();

		/** 
		 * Initializes the internal buffer object. Done automatically if the buffer was constructed using non-zero
		 * parameter constructor.
		 */
		void initialize(GLenum target, UINT32 size, GpuBufferUsage usage);

		/**
		 * Locks a portion of the buffer and returns pointer to the locked area. You must call unlock() when done.
		 *
		 * @param[in]	offset	Offset in bytes from which to lock the buffer.
		 * @param[in]	length	Length of the area you want to lock, in bytes.
		 * @param[in]	options	Signifies what you want to do with the returned pointer. Caller must ensure not to do 
		 *						anything he hasn't requested (for example don't try to read from the buffer unless you
		 *						requested it here).
		 */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options);

		/**	Releases the lock on this buffer. */
		void unlock();

		/**
		 * Reads data from a portion of the buffer and copies it to the destination buffer. Caller must ensure destination 
		 * buffer is large enough.
		 *
		 * @param[in]	offset	Offset in bytes from which to copy the data.
		 * @param[in]	length	Length of the area you want to copy, in bytes.
		 * @param[in]	dest	Destination buffer large enough to store the read data.
		 */
        void readData(UINT32 offset, UINT32 length, void* dest);

		/**
		 * Writes data into a portion of the buffer from the source memory. 
		 *
		 * @param[in]	offset		Offset in bytes from which to copy the data.
		 * @param[in]	length		Length of the area you want to copy, in bytes.
		 * @param[in]	source		Source buffer containing the data to write.
		 * @param[in]	writeFlags	Optional write flags that may affect performance.
		 */
        void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal);

		/**	Returns internal OpenGL buffer ID. */
        GLuint getGLBufferId() const { return mBufferId; }

	private:
		GLenum mTarget;
		GLuint mBufferId;

		bool mZeroLocked;
    };

	/** @} */
}