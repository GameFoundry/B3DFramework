//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Handles creation of DirectX 11 pipeline states. */
	class BS_D3D11_EXPORT D3D11RenderStateCoreManager : public RenderStateCoreManager
	{
	protected:
		/** @copydoc RenderStateCoreManager::createSamplerStateInternal */
		SPtr<SamplerStateCore> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const override;

		/** @copydoc RenderStateCoreManager::createBlendStateInternal */
		SPtr<BlendStateCore> createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const override;

		/** @copydoc RenderStateCoreManager::createRasterizerStateInternal */
		SPtr<RasterizerStateCore> createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const override;

		/** @copydoc RenderStateCoreManager::createDepthStencilStateInternal */
		SPtr<DepthStencilStateCore> createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const override;
	};

	/** @} */
}