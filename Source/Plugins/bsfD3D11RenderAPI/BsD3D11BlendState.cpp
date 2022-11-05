//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11BlendState.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

D3D11BlendState::D3D11BlendState(const BLEND_STATE_DESC& desc, u32 id)
	: BlendState(desc, id)
{}

D3D11BlendState::~D3D11BlendState()
{
	SAFE_RELEASE(mBlendState);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_BlendState);
}

void D3D11BlendState::CreateInternal()
{
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.AlphaToCoverageEnable = mProperties.GetAlphaToCoverageEnabled();
	blendStateDesc.IndependentBlendEnable = mProperties.GetIndependantBlendEnable();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		blendStateDesc.RenderTarget[i].BlendEnable = mProperties.GetBlendEnabled(i);
		blendStateDesc.RenderTarget[i].BlendOp = D3D11Mappings::Get(mProperties.GetBlendOperation(i));
		blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11Mappings::Get(mProperties.GetAlphaBlendOperation(i));
		blendStateDesc.RenderTarget[i].DestBlend = D3D11Mappings::Get(mProperties.GetDstBlend(i));
		blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11Mappings::Get(mProperties.GetAlphaDstBlend(i));
		blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0xf & (mProperties.GetRenderTargetWriteMask(i)); // Mask out all but last 4 bits
		blendStateDesc.RenderTarget[i].SrcBlend = D3D11Mappings::Get(mProperties.GetSrcBlend(i));
		blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11Mappings::Get(mProperties.GetAlphaSrcBlend(i));
	}

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	HRESULT hr = device.GetD3D11Device()->CreateBlendState(&blendStateDesc, &mBlendState);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create blend state.\nError Description:" + errorDescription);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_BlendState);

	BlendState::CreateInternal();
}
