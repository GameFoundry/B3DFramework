//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RasterizerState.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11Mappings.h"
#include "BsRenderStats.h"
#include "BsMath.h"

namespace bs
{
	D3D11RasterizerStateCore::D3D11RasterizerStateCore(const RASTERIZER_STATE_DESC& desc, UINT32 id)
		:RasterizerStateCore(desc, id), mRasterizerState(nullptr)
	{ }

	D3D11RasterizerStateCore::~D3D11RasterizerStateCore()
	{
		SAFE_RELEASE(mRasterizerState);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_RasterizerState);
	}

	void D3D11RasterizerStateCore::createInternal()
	{
		INT32 scaledDepthBias = Math::floorToInt(-mProperties.getDepthBias() * float((1 << 24))); // Note: Assumes 24-bit depth buffer

		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerStateDesc.AntialiasedLineEnable = mProperties.getAntialiasedLineEnable();
		rasterizerStateDesc.CullMode = D3D11Mappings::get(mProperties.getCullMode());
		rasterizerStateDesc.DepthBias = scaledDepthBias;
		rasterizerStateDesc.DepthBiasClamp = mProperties.getDepthBiasClamp();
		rasterizerStateDesc.DepthClipEnable = mProperties.getDepthClipEnable();
		rasterizerStateDesc.FillMode = D3D11Mappings::get(mProperties.getPolygonMode());
		rasterizerStateDesc.MultisampleEnable = mProperties.getMultisampleEnable();
		rasterizerStateDesc.ScissorEnable = mProperties.getScissorEnable();
		rasterizerStateDesc.SlopeScaledDepthBias = mProperties.getSlopeScaledDepthBias();
		rasterizerStateDesc.FrontCounterClockwise = false;

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		HRESULT hr = device.getD3D11Device()->CreateRasterizerState(&rasterizerStateDesc, &mRasterizerState);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create rasterizer state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_RasterizerState);
		RasterizerStateCore::createInternal();
	}
}