//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Abstract class defining common features of hardware buffers. Hardware buffers usually represent areas of memory the 
	 * GPU or the driver can access directly.
	 *
	 * @note	Core thread only.
	 * @note	Be aware that reading from non-system memory hardware buffers is usually slow and should be avoided.
	 */
	class BS_CORE_EXPORT HardwareBuffer
    {
    public:
        virtual ~HardwareBuffer() {}

		/**
		 * Locks a portion of the buffer and returns pointer to the locked area. You must call unlock() when done.
		 *
		 * @param[in]	offset		Offset in bytes from which to lock the buffer.
		 * @param[in]	length		Length of the area you want to lock, in bytes.
		 * @param[in]	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *							anything he hasn't requested (for example don't try to read from the buffer unless you
		 *							requested it here).
		 * @param[in]	deviceIdx	Index of the device whose memory to map. If the buffer doesn't exist on this device,
		 *							the method returns null.							
		 * @param[in]	queueIdx	Device queue to perform any read/write operations on. Using a non-default queue index
		 *							allows the GPU to perform write or read operations while executing rendering or compute
		 *							operations on the same time.
		 * 
		 *							Note that when writing to a buffer that is being used on a command buffer with a
		 *							different queue you must ensure to provide the command buffer with a valid sync mask
		 *							so it knows to wait before the write operation completes.
		 *							
		 *							This value is a global queue index which encodes both the queue type and queue index.
		 *							Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
		virtual void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx = 0, UINT32 queueIdx = 1)
        {
            assert(!isLocked() && "Cannot lock this buffer, it is already locked!");
            void* ret = map(offset, length, options, deviceIdx, queueIdx);
            mIsLocked = true;

			mLockStart = offset;
			mLockSize = length;
            return ret;
        }

		/**
		 * Locks the entire buffer and returns pointer to the locked area. You must call unlock() when done.
		 *
		 * @param[in]	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do 
		 *							anything he hasn't requested (for example don't try to read from the buffer unless you
		 *							requested it here).
		 * @param[in]	deviceIdx	Index of the device whose memory to map. If the buffer doesn't exist on this device,
		 *							the method returns null.
		 * @param[in]	queueIdx	Device queue to perform any read/write operations on. Using a non-default queue index
		 *							allows the GPU to perform write or read operations while executing rendering or compute
		 *							operations on the same time.
		 *
		 *							Note that when writing to a buffer that is being used on a command buffer with a
		 *							different queue you must ensure to provide the command buffer with a valid sync mask
		 *							so it knows to wait before the write operation completes.
		 *
		 *							This value is a global queue index which encodes both the queue type and queue index.
		 *							Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
        void* lock(GpuLockOptions options, UINT32 deviceIdx = 0, UINT32 queueIdx = 1)
        {
            return this->lock(0, mSize, options, deviceIdx, queueIdx);
        }

		/**	Releases the lock on this buffer. */
		virtual void unlock()
        {
            assert(isLocked() && "Cannot unlock this buffer, it is not locked!");

            unmap();
            mIsLocked = false;
        }

		/**
		 * Reads data from a portion of the buffer and copies it to the destination buffer. Caller must ensure destination 
		 * buffer is large enough.
		 *
		 * @param[in]	offset		Offset in bytes from which to copy the data.
		 * @param[in]	length		Length of the area you want to copy, in bytes.
		 * @param[in]	dest		Destination buffer large enough to store the read data.
		 * @param[in]	queueIdx	Device queue to perform any read/write operations on. Using a non-default queue index
		 *							allows the GPU to perform write or read operations while executing rendering or compute
		 *							operations on the same time.
		 *
		 *							Note that when writing to a buffer that is being used on a command buffer with a
		 *							different queue you must ensure to provide the command buffer with a valid sync mask
		 *							so it knows to wait before the write operation completes.
		 *
		 *							This value is a global queue index which encodes both the queue type and queue index.
		 *							Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
        virtual void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) = 0;

		/**
		 * Writes data into a portion of the buffer from the source memory. 
		 *
		 * @param[in]	offset		Offset in bytes from which to copy the data.
		 * @param[in]	length		Length of the area you want to copy, in bytes.
		 * @param[in]	source		Source buffer containing the data to write.
		 * @param[in]	writeFlags	Optional write flags that may affect performance.
		 * @param[in]	queueIdx	Device queue to perform any read/write operations on. Using a non-default queue index
		 *							allows the GPU to perform write or read operations while executing rendering or compute
		 *							operations on the same time.
		 *
		 *							Note that when writing to a buffer that is being used on a command buffer with a
		 *							different queue you must ensure to provide the command buffer with a valid sync mask
		 *							so it knows to wait before the write operation completes.
		 *
		 *							This value is a global queue index which encodes both the queue type and queue index.
		 *							Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
        virtual void writeData(UINT32 offset, UINT32 length, const void* source,
				BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) = 0;

		/**
		 * Copies data from a specific portion of the source buffer into a specific portion of this buffer.
		 *
		 * @param[in]	srcBuffer			Buffer to copy from.
		 * @param[in]	srcOffset			Offset into the source buffer to start copying from, in bytes.
		 * @param[in]	dstOffset			Offset into this buffer to start copying to, in bytes.
		 * @param[in]	length				Size of the data to copy, in bytes.
		 * @param[in]	discardWholeBuffer	Specify true if the data in the current buffer can be entirely discarded. This
		 *									may improve performance.
		 * @param[in]	queueIdx			Device queue to perform any read/write operations on. Using a non-default queue
		 *									index allows the GPU to perform write or read operations while executing 
		 *									rendering or compute operations on the same time.
		 *									
		 *									Note that when writing to a buffer that is being used on a command buffer with a
		 *									different queue you must ensure to provide the command buffer with a valid sync
		 *									mask so it knows to wait before the write operation completes.
		 *
		 *									This value is a global queue index which encodes both the queue type and queue
		 *									index. Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
		virtual void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false, UINT32 queueIdx = 1)
		{
			const void *srcData = srcBuffer.lock(
				srcOffset, length, GBL_READ_ONLY, queueIdx);
			this->writeData(dstOffset, length, srcData, discardWholeBuffer ? BWT_DISCARD : BWT_NORMAL, queueIdx);
			srcBuffer.unlock();
		}

		/**
		 * Copy data from the provided buffer into this buffer. If buffers are not the same size, smaller size will be used.
		 * 
		 * @param[in]	queueIdx	Device queue to perform any read/write operations on. Using a non-default queue index
		 *							allows the GPU to perform write or read operations while executing rendering or compute
		 *							operations on the same time.
		 *
		 *							Note that when writing to a buffer that is being used on a command buffer with a
		 *							different queue you must ensure to provide the command buffer with a valid sync mask
		 *							so it knows to wait before the write operation completes.
		 *
		 *							This value is a global queue index which encodes both the queue type and queue index.
		 *							Retrieve it from CommandSyncMask::getGlobalQueueIdx().
		 */
		virtual void copyData(HardwareBuffer& srcBuffer, UINT32 queueIdx = 1)
		{
			UINT32 sz = std::min(getSize(), srcBuffer.getSize());
			copyData(srcBuffer, 0, 0, sz, true, queueIdx);
		}
			
		/** Returns the size of this buffer in bytes. */
        UINT32 getSize() const { return mSize; }

		/**	Returns whether or not this buffer is currently locked. */
        bool isLocked() const { return mIsLocked; }	

	protected:
		friend class HardwareBufferManager;

		/**
		 * Constructs a new buffer.
		 *
		 * @param[in]	size			Size of the buffer, in bytes.
		 */
		HardwareBuffer(UINT32 size)
			: mSize(size), mIsLocked(false)
		{  }

		/** @copydoc lock */
		virtual void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, 
			UINT32 queueIdx) { return nullptr; }

		/** @copydoc unlock */
		virtual void unmap() { }

	protected:
		UINT32 mSize;

		bool mIsLocked;
		UINT32 mLockStart;
		UINT32 mLockSize;
    };

	/** @} */
}