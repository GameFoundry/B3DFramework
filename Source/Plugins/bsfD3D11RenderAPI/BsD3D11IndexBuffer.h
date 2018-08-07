//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "BsD3D11HardwareBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of an index buffer. */
	class D3D11IndexBuffer : public IndexBuffer
	{
	public:
		D3D11IndexBuffer(D3D11Device& device, const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		~D3D11IndexBuffer();

		/** @copydoc IndexBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/**	Gets the internal DX11 index buffer object. */
		ID3D11Buffer* getD3DIndexBuffer() const { return mBuffer->getD3DBuffer(); }		

	protected:
		/** @copydoc IndexBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc IndexBuffer::unmap */
		void unmap() override;

		/** @copydoc IndexBuffer::initialize */
		void initialize() override;

		D3D11HardwareBuffer* mBuffer = nullptr;
		D3D11Device& mDevice;
		GpuBufferUsage mUsage;

	};

	/** @} */
}}