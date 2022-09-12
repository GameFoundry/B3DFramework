//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullBlendState.h"

namespace bs { namespace ct
{
	NullBlendState::NullBlendState(const BLEND_STATE_DESC& desc, UINT32 id)
		:BlendState(desc, id)
	{ }

	void NullBlendState::CreateInternal()
	{
		D3D11_BLEND_DESC blendStateDesc;
		ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

		blendStateDesc.AlphaToCoverageEnable = mProperties.GetAlphaToCoverageEnabled();
		blendStateDesc.IndependentBlendEnable = mProperties.GetIndependantBlendEnable();
		
		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			blendStateDesc.RenderTarget[i].BlendEnable = mProperties.GetBlendEnabled(i);
			blendStateDesc.RenderTarget[i].BlendOp = D3D11Mappings::get(mProperties.GetBlendOperation(i));
			blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11Mappings::get(mProperties.GetAlphaBlendOperation(i));
			blendStateDesc.RenderTarget[i].DestBlend = D3D11Mappings::get(mProperties.GetDstBlend(i));
			blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11Mappings::get(mProperties.GetAlphaDstBlend(i));
			blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0xf & (mProperties.GetRenderTargetWriteMask(i)); // Mask out all but last 4 bits
			blendStateDesc.RenderTarget[i].SrcBlend = D3D11Mappings::get(mProperties.GetSrcBlend(i));
			blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11Mappings::get(mProperties.GetAlphaSrcBlend(i));
		}

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = rs->GetPrimaryDevice();
		HRESULT hr = device.GetD3D11Device()->CreateBlendState(&blendStateDesc, &mBlendState);

		if(FAILED(hr) || device.HasError())
		{
			String errorDescription = device.GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create blend state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_BlendState);

		BlendState::createInternal();
	}
}}
