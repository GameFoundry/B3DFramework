//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "BsD3D11HardwareBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a vertex buffer. */
	class D3D11VertexBuffer : public VertexBuffer
	{
	public:
		D3D11VertexBuffer(D3D11Device& device, const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~D3D11VertexBuffer();

		/** @copydoc VertexBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc VertexBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc VertexBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/**	Get the D3D-specific index buffer */
		ID3D11Buffer* getD3DVertexBuffer() const { return mBuffer->getD3DBuffer(); }		

	protected: 
		/** @copydoc VertexBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc VertexBuffer::unmap */
		void unmap() override;

		/** @copydoc VertexBuffer::initialize */
		void initialize() override;

		D3D11HardwareBuffer* mBuffer = nullptr;
		D3D11Device& mDevice;
		bool mStreamOut;
		GpuBufferUsage mUsage;
	};

	/** @} */
}}