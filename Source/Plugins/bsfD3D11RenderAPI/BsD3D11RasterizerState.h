//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsRasterizerState.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/** DirectX 11 implementation of a rasterizer state. Wraps a DX11 rasterizer state object. */
		class D3D11RasterizerState : public RasterizerState
		{
		public:
			~D3D11RasterizerState();

			ID3D11RasterizerState* GetInternal() const { return mRasterizerState; }

		protected:
			friend class D3D11RenderStateManager;

			D3D11RasterizerState(const RASTERIZER_STATE_DESC& desc, u32 id);

			void CreateInternal() override;

			ID3D11RasterizerState* mRasterizerState = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
