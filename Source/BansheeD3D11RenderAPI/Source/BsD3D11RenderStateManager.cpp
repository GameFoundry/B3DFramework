//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderStateManager.h"
#include "BsD3D11SamplerState.h"
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11RasterizerState.h"
#include "BsD3D11BlendState.h"

namespace bs
{
	SPtr<SamplerStateCore> D3D11RenderStateCoreManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerStateCore> ret = bs_shared_ptr<D3D11SamplerStateCore>(new (bs_alloc<D3D11SamplerStateCore>()) D3D11SamplerStateCore(desc, deviceMask));
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<BlendStateCore> D3D11RenderStateCoreManager::createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<BlendStateCore> ret = bs_shared_ptr<D3D11BlendStateCore>(new (bs_alloc<D3D11BlendStateCore>()) D3D11BlendStateCore(desc, id));
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<RasterizerStateCore> D3D11RenderStateCoreManager::createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<RasterizerStateCore> ret = bs_shared_ptr<D3D11RasterizerStateCore>(new (bs_alloc<D3D11RasterizerStateCore>()) D3D11RasterizerStateCore(desc, id));
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<DepthStencilStateCore> D3D11RenderStateCoreManager::createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<DepthStencilStateCore> ret = bs_shared_ptr<D3D11DepthStencilStateCore>(new (bs_alloc<D3D11DepthStencilStateCore>()) D3D11DepthStencilStateCore(desc, id));
		ret->_setThisPtr(ret);

		return ret;
	}
}