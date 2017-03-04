//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuParamBlockBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a parameter block buffer (constant buffer in DX11 lingo). */
	class D3D11GpuParamBlockBuffer : public GpuParamBlockBuffer
	{
	public:
		D3D11GpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage, GpuDeviceFlags deviceMask);
		~D3D11GpuParamBlockBuffer();

		/** @copydoc GpuParamBlockBuffer::writeToGPU */
		void writeToGPU(const UINT8* data, UINT32 queueIdx = 0) override;

		/**	Returns internal DX11 buffer object. */
		ID3D11Buffer* getD3D11Buffer() const;
	protected:
		/** @copydoc GpuParamBlockBuffer::initialize */
		void initialize() override;

	private:
		D3D11HardwareBuffer* mBuffer;
	};

	/** @} */
}}