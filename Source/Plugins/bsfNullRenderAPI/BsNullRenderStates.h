//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsRenderStateManager.h"

namespace bs { namespace ct
{
	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/**	Handles creation of null pipeline states. */
	class NullRenderStateManager : public RenderStateManager
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

	/** @copydoc BlendState */
	class NullBlendState : public BlendState
	{
	public:
		NullBlendState(const BLEND_STATE_DESC& desc, UINT32 id)
			:BlendState(desc, id)
		{ }
	};

	/** @copydoc DepthStencilState */
	class NullDepthStencilState : public DepthStencilState 
	{
	public:
		NullDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id)
			:DepthStencilState(desc, id)
		{ }
	};

	/** @copydoc RasterizerState */
	class NullRasterizerState : public RasterizerState 
	{
	public:
		NullRasterizerState(const RASTERIZER_STATE_DESC& desc, UINT32 id)
			:RasterizerState(desc, id)
		{ }
	};

	/** @copydoc SamplerState */
	class NullSamplerState : public SamplerState 
	{
	public:
		NullSamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
			:SamplerState(desc, deviceMask)
		{ }
	};

	/** @} */
}}