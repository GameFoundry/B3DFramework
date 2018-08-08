//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a parameter block buffer (constant buffer in DX11 lingo). */
	class D3D11GpuParamBlockBuffer : public GpuParamBlockBuffer
	{
	public:
		D3D11GpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);
		~D3D11GpuParamBlockBuffer();

		/**	Returns internal DX11 buffer object. */
		ID3D11Buffer* getD3D11Buffer() const;
	protected:
		/** @copydoc GpuParamBlockBuffer::initialize */
		void initialize() override;
	};

	/** @} */
}}