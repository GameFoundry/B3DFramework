//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RasterizerState.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11Mappings.h"
#include "Profiling/BsRenderStats.h"
#include "Math/BsMath.h"

namespace bs { namespace ct
{
	D3D11RasterizerState::D3D11RasterizerState(const RASTERIZER_STATE_DESC& desc, UINT32 id)
		:RasterizerState(desc, id)
	{ }

	D3D11RasterizerState::~D3D11RasterizerState()
	{
		SAFE_RELEASE(mRasterizerState);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_RasterizerState);
	}

	void D3D11RasterizerState::CreateInternal()
	{
		INT32 scaledDepthBias = Math::FloorToInt(-mProperties.GetDepthBias() * float((1 << 24))); // Note: Assumes 24-bit depth buffer

		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerStateDesc.AntialiasedLineEnable = mProperties.GetAntialiasedLineEnable();
		rasterizerStateDesc.CullMode = D3D11Mappings::get(mProperties.GetCullMode());
		rasterizerStateDesc.DepthBias = scaledDepthBias;
		rasterizerStateDesc.DepthBiasClamp = mProperties.GetDepthBiasClamp();
		rasterizerStateDesc.DepthClipEnable = mProperties.GetDepthClipEnable();
		rasterizerStateDesc.FillMode = D3D11Mappings::get(mProperties.GetPolygonMode());
		rasterizerStateDesc.MultisampleEnable = mProperties.GetMultisampleEnable();
		rasterizerStateDesc.ScissorEnable = mProperties.GetScissorEnable();
		rasterizerStateDesc.SlopeScaledDepthBias = mProperties.GetSlopeScaledDepthBias();
		rasterizerStateDesc.FrontCounterClockwise = false;

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = rs->GetPrimaryDevice();
		HRESULT hr = device.GetD3D11Device()->CreateRasterizerState(&rasterizerStateDesc, &mRasterizerState);

		if(FAILED(hr) || device.HasError())
		{
			String errorDescription = device.GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create rasterizer state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_RasterizerState);
		RasterizerState::createInternal();
	}
}}
