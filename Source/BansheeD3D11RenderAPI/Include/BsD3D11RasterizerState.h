//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRasterizerState.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** DirectX 11 implementation of a rasterizer state. Wraps a DX11 rasterizer state object. */
	class D3D11RasterizerState : public RasterizerState
	{
	public:
		~D3D11RasterizerState();
		ID3D11RasterizerState* getInternal() const { return mRasterizerState; }

	protected:
		friend class D3D11RenderStateManager;

		D3D11RasterizerState(const RASTERIZER_STATE_DESC& desc, UINT32 id);

		/** @copydoc RasterizerState::createInternal */
		void createInternal() override;

		ID3D11RasterizerState* mRasterizerState;
	};

	/** @} */
}}