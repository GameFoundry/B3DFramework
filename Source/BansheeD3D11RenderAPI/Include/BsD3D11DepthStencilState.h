//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsDepthStencilState.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** DirectX 11 implementation of a depth stencil state. Corresponds directly with a DX11 depth stencil state object. */
	class BS_D3D11_EXPORT D3D11DepthStencilStateCore : public DepthStencilStateCore
	{
	public:
		~D3D11DepthStencilStateCore();

		/**	Returns the internal DX11 depth stencil state object. */
		ID3D11DepthStencilState* getInternal() const { return mDepthStencilState; }

	protected:
		friend class D3D11RenderStateCoreManager;

		D3D11DepthStencilStateCore(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id);

		/** @copydoc DepthStencilStateCore::createInternal() */
		void createInternal() override;

		ID3D11DepthStencilState* mDepthStencilState;
	};

	/** @} */
}