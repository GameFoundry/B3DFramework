//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuParamBlockBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a parameter block buffer (constant buffer in DX11 lingo). */
	class BS_D3D11_EXPORT D3D11GpuParamBlockBufferCore : public GpuParamBlockBufferCore
	{
	public:
		D3D11GpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage, GpuDeviceFlags deviceMask);
		~D3D11GpuParamBlockBufferCore();

		/** @copydoc GpuParamBlockBufferCore::writeToGPU */
		void writeToGPU(const UINT8* data) override;

		/** @copydoc GpuParamBlockBufferCore::readFromGPU */
		void readFromGPU(UINT8* data) const override;

		/**	Returns internal DX11 buffer object. */
		ID3D11Buffer* getD3D11Buffer() const;
	protected:
		/** @copydoc GpuParamBlockBufferCore::initialize */
		void initialize() override;

	private:
		D3D11HardwareBuffer* mBuffer;
	};

	/** @} */
}