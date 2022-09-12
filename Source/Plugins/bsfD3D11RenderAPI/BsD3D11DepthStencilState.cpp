//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Mappings.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	D3D11DepthStencilState::D3D11DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id)
		:DepthStencilState(desc, id)
	{ }

	D3D11DepthStencilState::~D3D11DepthStencilState()
	{
		SAFE_RELEASE(mDepthStencilState);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_DepthStencilState);
	}

	void D3D11DepthStencilState::CreateInternal()
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilState;
		ZeroMemory(&depthStencilState, sizeof(D3D11_DEPTH_STENCIL_DESC));

		bool depthEnable = mProperties.getDepthWriteEnable() || mProperties.getDepthReadEnable();
		CompareFunction compareFunc;
		if (mProperties.getDepthReadEnable())
			compareFunc = mProperties.getDepthComparisonFunc();
		else
			compareFunc = CMPF_ALWAYS_PASS;

		depthStencilState.BackFace.StencilPassOp = D3D11Mappings::get(mProperties.getStencilBackPassOp());
		depthStencilState.BackFace.StencilFailOp = D3D11Mappings::get(mProperties.getStencilBackFailOp());
		depthStencilState.BackFace.StencilDepthFailOp = D3D11Mappings::get(mProperties.getStencilBackZFailOp());
		depthStencilState.BackFace.StencilFunc = D3D11Mappings::get(mProperties.getStencilBackCompFunc());
		depthStencilState.FrontFace.StencilPassOp = D3D11Mappings::get(mProperties.getStencilFrontPassOp());
		depthStencilState.FrontFace.StencilFailOp = D3D11Mappings::get(mProperties.getStencilFrontFailOp());
		depthStencilState.FrontFace.StencilDepthFailOp = D3D11Mappings::get(mProperties.getStencilFrontZFailOp());
		depthStencilState.FrontFace.StencilFunc = D3D11Mappings::get(mProperties.getStencilFrontCompFunc());
		depthStencilState.DepthEnable = depthEnable;
		depthStencilState.DepthWriteMask = mProperties.getDepthWriteEnable() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilState.DepthFunc = D3D11Mappings::get(compareFunc);
		depthStencilState.StencilEnable = mProperties.getStencilEnable();
		depthStencilState.StencilReadMask = mProperties.getStencilReadMask();
		depthStencilState.StencilWriteMask = mProperties.getStencilWriteMask();

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		HRESULT hr = device.getD3D11Device()->CreateDepthStencilState(&depthStencilState, &mDepthStencilState);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create depth stencil state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_DepthStencilState);

		DepthStencilState::createInternal();
	}
}}
