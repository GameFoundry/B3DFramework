//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsHardwareBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Class containing common functionality for all DirectX 11 hardware buffers. */
	class BS_D3D11_EXPORT D3D11HardwareBuffer : public HardwareBuffer
	{
	public:
		/**	Available types of DX11 buffers. */
		enum BufferType
		{
			/** Contains geometry vertices and their properties. */
			BT_VERTEX = 0x1, 
			/** Contains triangle to vertex mapping. */
			BT_INDEX = 0x2, 
			/** Contains GPU program parameters. */
			BT_CONSTANT = 0x4, 
			/** Special value signifying a buffer is of generic type. Not an actual buffer. */
			BT_GROUP_GENERIC = 0x8, 
			/** Generic buffer that holds one or more user-defined structures laid out sequentially. */
			BT_STRUCTURED = BT_GROUP_GENERIC | 0x10, 
			/** Generic buffer that holds raw block of bytes with no defined structure. */
			BT_RAW = BT_GROUP_GENERIC | 0x20, 
			/** Generic buffer that is used for holding parameters used for indirect rendering. */
			BT_INDIRECTARGUMENT = BT_GROUP_GENERIC | 0x40, 
			/** Generic buffer that allows the GPU program to use append/consume functionality. */
			BT_APPENDCONSUME = BT_GROUP_GENERIC | 0x80 
		};

		D3D11HardwareBuffer(BufferType btype, GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize, 
			D3D11Device& device, bool systemMemory = false, bool streamOut = false, bool randomGpuWrite = false, 
			bool useCounter = false);
		~D3D11HardwareBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, 
			UINT32 length, bool discardWholeBuffer = false, UINT32 queueIdx = 1) override;

		/**	Returns the internal DX11 buffer object. */
		ID3D11Buffer* getD3DBuffer() const { return mD3DBuffer; }

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		BufferType mBufferType;
		bool mRandomGpuWrite;
		bool mUseCounter;
		UINT32 mElementCount;
		UINT32 mElementSize;
		GpuBufferUsage mUsage;

		ID3D11Buffer* mD3DBuffer;

		bool mUseTempStagingBuffer;
		D3D11HardwareBuffer* mpTempStagingBuffer;
		bool mStagingUploadNeeded;
		
		D3D11Device& mDevice;
		D3D11_BUFFER_DESC mDesc;
	};

	/** @} */
}