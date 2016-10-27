//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsIndexBuffer.h"
#include "BsD3D11HardwareBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of an index buffer. */
	class BS_D3D11_EXPORT D3D11IndexBufferCore : public IndexBufferCore
	{
	public:
		D3D11IndexBufferCore(D3D11Device& device, const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		~D3D11IndexBufferCore();

		/** @copydoc IndexBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 syncMask = 0x00000001) override;

		/** @copydoc IndexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 syncMask = 0x00000001) override;

		/** @copydoc IndexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, UINT32 syncMask = 0x00000001) override;

		/**	Gets the internal DX11 index buffer object. */
		ID3D11Buffer* getD3DIndexBuffer() const { return mBuffer->getD3DBuffer(); }		

	protected:
		/** @copydoc IndexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask) override;

		/** @copydoc IndexBufferCore::unmap */
		void unmap() override;

		/** @copydoc IndexBufferCore::initialize */
		void initialize() override;

		D3D11HardwareBuffer* mBuffer;
		D3D11Device& mDevice;
	};

	/** @} */
}