//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullBlendState.h"

namespace bs { namespace ct
{
	NullBlendState::NullBlendState(const BLEND_STATE_DESC& desc, UINT32 id)
		:BlendState(desc, id)
	{ }

	void NullBlendState::createInternal()
	{
		D3D11_BLEND_DESC blendStateDesc;
		ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

		blendStateDesc.AlphaToCoverageEnable = mProperties.getAlphaToCoverageEnabled();
		blendStateDesc.IndependentBlendEnable = mProperties.getIndependantBlendEnable();
		
		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			blendStateDesc.RenderTarget[i].BlendEnable = mProperties.getBlendEnabled(i);
			blendStateDesc.RenderTarget[i].BlendOp = D3D11Mappings::get(mProperties.getBlendOperation(i));
			blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11Mappings::get(mProperties.getAlphaBlendOperation(i));
			blendStateDesc.RenderTarget[i].DestBlend = D3D11Mappings::get(mProperties.getDstBlend(i));
			blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11Mappings::get(mProperties.getAlphaDstBlend(i));
			blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0xf & (mProperties.getRenderTargetWriteMask(i)); // Mask out all but last 4 bits
			blendStateDesc.RenderTarget[i].SrcBlend = D3D11Mappings::get(mProperties.getSrcBlend(i));
			blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11Mappings::get(mProperties.getAlphaSrcBlend(i));
		}

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		HRESULT hr = device.getD3D11Device()->CreateBlendState(&blendStateDesc, &mBlendState);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create blend state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_BlendState);

		BlendState::createInternal();
	}
}}