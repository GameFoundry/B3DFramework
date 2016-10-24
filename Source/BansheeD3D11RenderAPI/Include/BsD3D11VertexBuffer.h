//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsVertexBuffer.h"
#include "BsD3D11HardwareBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a vertex buffer. */
	class BS_D3D11_EXPORT D3D11VertexBufferCore : public VertexBufferCore
	{
	public:
		D3D11VertexBufferCore(D3D11Device& device, const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		~D3D11VertexBufferCore();

		/** @copydoc VertexBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* dest) override;

		/** @copydoc VertexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/** @copydoc VertexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false) override;

		/**	Get the D3D-specific index buffer */
		ID3D11Buffer* getD3DVertexBuffer() const { return mBuffer->getD3DBuffer(); }		

	protected: 
		/** @copydoc VertexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc VertexBufferCore::unmap */
		void unmap(void) override;

		/** @copydoc VertexBufferCore::initialize */
		void initialize() override;

		D3D11HardwareBuffer* mBuffer;
		D3D11Device& mDevice;
		bool mStreamOut;
	};

	/** @} */
}