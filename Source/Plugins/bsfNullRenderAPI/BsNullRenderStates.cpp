//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderStates.h"

namespace bs { namespace ct
{
	SPtr<SamplerState> NullRenderStateManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, 
		GpuDeviceFlags deviceMask) const
	{
		SPtr<NullSamplerState> ret = bs_shared_ptr_new<NullSamplerState>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<BlendState> NullRenderStateManager::createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<NullBlendState> ret = bs_shared_ptr_new<NullBlendState>(desc, id);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<RasterizerState> NullRenderStateManager::createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<NullRasterizerState> ret = bs_shared_ptr_new<NullRasterizerState>(desc, id);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<DepthStencilState> NullRenderStateManager::createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<NullDepthStencilState> ret = bs_shared_ptr_new<NullDepthStencilState>(desc, id);
		ret->_setThisPtr(ret);

		return ret;
	}
}}