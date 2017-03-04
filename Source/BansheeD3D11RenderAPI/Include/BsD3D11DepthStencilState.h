//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsDepthStencilState.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** DirectX 11 implementation of a depth stencil state. Corresponds directly with a DX11 depth stencil state object. */
	class D3D11DepthStencilState : public DepthStencilState
	{
	public:
		~D3D11DepthStencilState();

		/**	Returns the internal DX11 depth stencil state object. */
		ID3D11DepthStencilState* getInternal() const { return mDepthStencilState; }

	protected:
		friend class D3D11RenderStateManager;

		D3D11DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id);

		/** @copydoc DepthStencilState::createInternal() */
		void createInternal() override;

		ID3D11DepthStencilState* mDepthStencilState;
	};

	/** @} */
}}