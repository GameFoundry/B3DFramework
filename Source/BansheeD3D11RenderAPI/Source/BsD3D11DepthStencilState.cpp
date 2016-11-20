//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Mappings.h"
#include "BsRenderStats.h"

namespace bs
{
	D3D11DepthStencilStateCore::D3D11DepthStencilStateCore(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id)
		:DepthStencilStateCore(desc, id), mDepthStencilState(nullptr)
	{ }

	D3D11DepthStencilStateCore::~D3D11DepthStencilStateCore()
	{
		SAFE_RELEASE(mDepthStencilState);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_DepthStencilState);
	}

	void D3D11DepthStencilStateCore::createInternal()
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilState;
		ZeroMemory(&depthStencilState, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthStencilState.BackFace.StencilPassOp = D3D11Mappings::get(mProperties.getStencilBackPassOp());
		depthStencilState.BackFace.StencilFailOp = D3D11Mappings::get(mProperties.getStencilBackFailOp());
		depthStencilState.BackFace.StencilDepthFailOp = D3D11Mappings::get(mProperties.getStencilBackZFailOp());
		depthStencilState.BackFace.StencilFunc = D3D11Mappings::get(mProperties.getStencilBackCompFunc());
		depthStencilState.FrontFace.StencilPassOp = D3D11Mappings::get(mProperties.getStencilFrontPassOp());
		depthStencilState.FrontFace.StencilFailOp = D3D11Mappings::get(mProperties.getStencilFrontFailOp());
		depthStencilState.FrontFace.StencilDepthFailOp = D3D11Mappings::get(mProperties.getStencilFrontZFailOp());
		depthStencilState.FrontFace.StencilFunc = D3D11Mappings::get(mProperties.getStencilFrontCompFunc());
		depthStencilState.DepthEnable = mProperties.getDepthReadEnable();
		depthStencilState.DepthWriteMask = mProperties.getDepthWriteEnable() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilState.DepthFunc = D3D11Mappings::get(mProperties.getDepthComparisonFunc());
		depthStencilState.StencilEnable = mProperties.getStencilEnable();
		depthStencilState.StencilReadMask = mProperties.getStencilReadMask();
		depthStencilState.StencilWriteMask = mProperties.getStencilWriteMask();

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		HRESULT hr = device.getD3D11Device()->CreateDepthStencilState(&depthStencilState, &mDepthStencilState);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create depth stencil state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_DepthStencilState);

		DepthStencilStateCore::createInternal();
	}
}