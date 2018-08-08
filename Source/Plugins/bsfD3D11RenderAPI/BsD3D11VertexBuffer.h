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

		/**	Get the D3D-specific index buffer */
		ID3D11Buffer* getD3DVertexBuffer() const { return static_cast<D3D11HardwareBuffer*>(mBuffer)->getD3DBuffer(); }		

	protected: 
		/** @copydoc VertexBuffer::initialize */
		void initialize() override;

		D3D11Device& mDevice;
		bool mStreamOut;
	};

	/** @} */
}}