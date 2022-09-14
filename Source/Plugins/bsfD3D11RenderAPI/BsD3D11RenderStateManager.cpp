//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderStateManager.h"
#include "BsD3D11SamplerState.h"
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11RasterizerState.h"
#include "BsD3D11BlendState.h"

namespace bs { namespace ct
{
	SPtr<SamplerState> D3D11RenderStateManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> ret = bs_shared_ptr<D3D11SamplerState>(new (bs_alloc<D3D11SamplerState>()) D3D11SamplerState(desc, deviceMask));
		ret->SetThisPtrInternal(ret);

		return ret;
	}

	SPtr<BlendState> D3D11RenderStateManager::createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<BlendState> ret = bs_shared_ptr<D3D11BlendState>(new (bs_alloc<D3D11BlendState>()) D3D11BlendState(desc, id));
		ret->SetThisPtrInternal(ret);

		return ret;
	}

	SPtr<RasterizerState> D3D11RenderStateManager::createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<RasterizerState> ret = bs_shared_ptr<D3D11RasterizerState>(new (bs_alloc<D3D11RasterizerState>()) D3D11RasterizerState(desc, id));
		ret->SetThisPtrInternal(ret);

		return ret;
	}

	SPtr<DepthStencilState> D3D11RenderStateManager::createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<DepthStencilState> ret = bs_shared_ptr<D3D11DepthStencilState>(new (bs_alloc<D3D11DepthStencilState>()) D3D11DepthStencilState(desc, id));
		ret->SetThisPtrInternal(ret);

		return ret;
	}
}}
