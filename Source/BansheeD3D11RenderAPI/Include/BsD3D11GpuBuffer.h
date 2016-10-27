//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuBuffer.h"

namespace BansheeEngine 
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a generic GPU buffer. */
	class BS_D3D11_EXPORT D3D11GpuBufferCore : public GpuBufferCore
    {
    public:
		~D3D11GpuBufferCore();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource,
			BufferWriteType writeFlags = BWT_NORMAL) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) override;

		/**	Returns the internal DX11 GPU buffer object. */
		ID3D11Buffer* getDX11Buffer() const;

		/** Returns the DX11 shader resource view object for the buffer. */
		ID3D11ShaderResourceView* getSRV() const;

		/** Returns the DX11 unordered access view object for the buffer. */
		ID3D11UnorderedAccessView* getUAV() const;

	protected:
		friend class D3D11HardwareBufferCoreManager;

		D3D11GpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuBufferCore::createView */
		GpuBufferView* createView() override;

		/** @copydoc GpuBufferCore::destroyView */
		void destroyView(GpuBufferView* view) override;

		/** @copydoc GpuBufferCore::initialize */
		void initialize() override;

	private:
		D3D11HardwareBuffer* mBuffer;
		D3D11GpuBufferView* mBufferView;
    };

	/** @} */
}
