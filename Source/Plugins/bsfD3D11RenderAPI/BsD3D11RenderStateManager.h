//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles creation of DirectX 11 pipeline states. */
		class D3D11RenderStateManager : public RenderStateManager
		{
		protected:
			/** @copydoc RenderStateManager::createSamplerStateInternal */
			SPtr<SamplerState> CreateSamplerStateInternalInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const;

			/** @copydoc RenderStateManager::createBlendStateInternal */
			SPtr<BlendState> CreateBlendStateInternalInternal(const BLEND_STATE_DESC& desc, u32 id) const;

			/** @copydoc RenderStateManager::createRasterizerStateInternal */
			SPtr<RasterizerState> CreateRasterizerStateInternalInternal(const RASTERIZER_STATE_DESC& desc, u32 id) const;

			/** @copydoc RenderStateManager::createDepthStencilStateInternal */
			SPtr<DepthStencilState> CreateDepthStencilStateInternalInternal(const DEPTH_STENCIL_STATE_DESC& desc, u32 id) const;
		};

		/** @} */
	} // namespace ct
} // namespace bs
