//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsDepthStencilState.h"

namespace bs
{
	namespace ct
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
			ID3D11DepthStencilState* GetInternal() const { return mDepthStencilState; }

		protected:
			friend class D3D11RenderStateManager;

			D3D11DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, u32 id);

			/** @copydoc DepthStencilState::createInternal() */
			void CreateInternal();

			ID3D11DepthStencilState* mDepthStencilState = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
