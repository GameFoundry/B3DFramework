//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderStateManager.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Handles creation of DirectX 11 pipeline states. */
	class D3D11RenderStateManager : public RenderStateManager
	{
	protected:
		/** @copydoc RenderStateManager::createSamplerStateInternal */
		SPtr<SamplerState> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const override;

		/** @copydoc RenderStateManager::createBlendStateInternal */
		SPtr<BlendState> createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const override;

		/** @copydoc RenderStateManager::createRasterizerStateInternal */
		SPtr<RasterizerState> createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const override;

		/** @copydoc RenderStateManager::createDepthStencilStateInternal */
		SPtr<DepthStencilState> createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const override;
	};

	/** @} */
}}