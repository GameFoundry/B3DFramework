//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Mappings.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

D3D11DepthStencilState::D3D11DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, u32 id)
	: DepthStencilState(desc, id)
{}

D3D11DepthStencilState::~D3D11DepthStencilState()
{
	SAFE_RELEASE(mDepthStencilState);

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_DepthStencilState);
}

void D3D11DepthStencilState::CreateInternal()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilState;
	ZeroMemory(&depthStencilState, sizeof(D3D11_DEPTH_STENCIL_DESC));

	bool depthEnable = mProperties.GetDepthWriteEnable() || mProperties.GetDepthReadEnable();
	CompareFunction compareFunc;
	if(mProperties.GetDepthReadEnable())
		compareFunc = mProperties.GetDepthComparisonFunc();
	else
		compareFunc = CMPF_ALWAYS_PASS;

	depthStencilState.BackFace.StencilPassOp = D3D11Mappings::Get(mProperties.GetStencilBackPassOp());
	depthStencilState.BackFace.StencilFailOp = D3D11Mappings::Get(mProperties.GetStencilBackFailOp());
	depthStencilState.BackFace.StencilDepthFailOp = D3D11Mappings::Get(mProperties.GetStencilBackZFailOp());
	depthStencilState.BackFace.StencilFunc = D3D11Mappings::Get(mProperties.GetStencilBackCompFunc());
	depthStencilState.FrontFace.StencilPassOp = D3D11Mappings::Get(mProperties.GetStencilFrontPassOp());
	depthStencilState.FrontFace.StencilFailOp = D3D11Mappings::Get(mProperties.GetStencilFrontFailOp());
	depthStencilState.FrontFace.StencilDepthFailOp = D3D11Mappings::Get(mProperties.GetStencilFrontZFailOp());
	depthStencilState.FrontFace.StencilFunc = D3D11Mappings::Get(mProperties.GetStencilFrontCompFunc());
	depthStencilState.DepthEnable = depthEnable;
	depthStencilState.DepthWriteMask = mProperties.GetDepthWriteEnable() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilState.DepthFunc = D3D11Mappings::Get(compareFunc);
	depthStencilState.StencilEnable = mProperties.GetStencilEnable();
	depthStencilState.StencilReadMask = mProperties.GetStencilReadMask();
	depthStencilState.StencilWriteMask = mProperties.GetStencilWriteMask();

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	HRESULT hr = device.GetD3D11Device()->CreateDepthStencilState(&depthStencilState, &mDepthStencilState);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Cannot create depth stencil state.\nError Description:" + errorDescription);
	}

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_DepthStencilState);

	DepthStencilState::CreateInternal();
}
