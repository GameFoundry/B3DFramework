//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsBlendState.h"

namespace bs
{
	namespace ct
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
			ID3D11BlendState* GetInternal() const { return mBlendState; }

		protected:
			friend class D3D11RenderStateManager;

			D3D11BlendState(const BLEND_STATE_DESC& desc, u32 id);

			void CreateInternal() override;

			ID3D11BlendState* mBlendState = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
