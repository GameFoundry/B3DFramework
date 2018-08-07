//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Class containing common functionality for all DirectX 11 hardware buffers. */
	class D3D11HardwareBuffer : public HardwareBuffer
	{
	public:
		/**	Available types of DX11 buffers. */
		enum BufferType
		{
			/** Contains geometry vertices and their properties. */
			BT_VERTEX = 1 << 0, 
			/** Contains triangle to vertex mapping. */
			BT_INDEX = 1 << 1, 
			/** Contains GPU program parameters. */
			BT_CONSTANT = 1 << 2, 
			/** Generic buffer that contains primitive types. */
			BT_STANDARD = 1 << 3,
			/** Generic buffer that holds one or more user-defined structures laid out sequentially. */
			BT_STRUCTURED = 1 << 4, 
			/** Generic buffer that holds raw block of bytes with no defined structure. */
			BT_RAW = 1 << 5, 
			/** Generic buffer that is used for holding parameters used for indirect rendering. */
			BT_INDIRECTARGUMENT = 1 << 6, 
		};

		D3D11HardwareBuffer(BufferType btype, GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize, 
			D3D11Device& device, bool systemMemory = false, bool streamOut = false);
		~D3D11HardwareBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, 
			UINT32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/**	Returns the internal DX11 buffer object. */
		ID3D11Buffer* getD3DBuffer() const { return mD3DBuffer; }

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		BufferType mBufferType;
		UINT32 mElementCount;
		UINT32 mElementSize;
		GpuBufferUsage mUsage;

		ID3D11Buffer* mD3DBuffer = nullptr;

		bool mUseTempStagingBuffer = false;
		D3D11HardwareBuffer* mpTempStagingBuffer = nullptr;
		bool mStagingUploadNeeded;
		
		D3D11Device& mDevice;
		D3D11_BUFFER_DESC mDesc;
	};

	/** @} */
}}

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::D3D11HardwareBuffer, 32)
}