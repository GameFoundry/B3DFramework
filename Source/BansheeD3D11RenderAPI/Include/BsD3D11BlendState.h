//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsBlendState.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** DirectX 11 implementation of a blend state. Corresponds directly with a DX11 blend state object. */
	class D3D11BlendState : public BlendState
	{
	public:
		~D3D11BlendState();

		/**	Returns the internal DX11 blend state object. */
		ID3D11BlendState* getInternal() const { return mBlendState; }

	protected:
		friend class D3D11RenderStateManager;

		D3D11BlendState(const BLEND_STATE_DESC& desc, UINT32 id);

		/** @copydoc BlendState::createInternal */
		void createInternal() override;

		ID3D11BlendState* mBlendState;
	};

	/** @} */
}}