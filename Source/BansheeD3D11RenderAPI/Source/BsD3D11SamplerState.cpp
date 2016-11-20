//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11SamplerState.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11Mappings.h"
#include "BsRenderStats.h"

namespace bs
{
	D3D11SamplerStateCore::D3D11SamplerStateCore(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:SamplerStateCore(desc, deviceMask), mSamplerState(nullptr)
	{ }

	D3D11SamplerStateCore::~D3D11SamplerStateCore()
	{
		SAFE_RELEASE(mSamplerState);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_SamplerState);
	}

	void D3D11SamplerStateCore::createInternal()
	{
		D3D11_SAMPLER_DESC samplerState;
		ZeroMemory(&samplerState, sizeof(D3D11_SAMPLER_DESC));

		samplerState.AddressU = D3D11Mappings::get(mProperties.getTextureAddressingMode().u);
		samplerState.AddressV = D3D11Mappings::get(mProperties.getTextureAddressingMode().v);
		samplerState.AddressW = D3D11Mappings::get(mProperties.getTextureAddressingMode().w);
		samplerState.BorderColor[0] = mProperties.getBorderColor()[0];
		samplerState.BorderColor[1] = mProperties.getBorderColor()[1];
		samplerState.BorderColor[2] = mProperties.getBorderColor()[2];
		samplerState.BorderColor[3] = mProperties.getBorderColor()[3];
		samplerState.ComparisonFunc = D3D11Mappings::get(mProperties.getComparisonFunction());
		samplerState.MaxAnisotropy = mProperties.getTextureAnisotropy();
		samplerState.MaxLOD = mProperties.getMaximumMip();
		samplerState.MinLOD = mProperties.getMinimumMip();
		samplerState.MipLODBias = mProperties.getTextureMipmapBias();

		bool isComparison = ((mProperties.getTextureFiltering(FT_MIN) & FO_USE_COMPARISON) & 
			(mProperties.getTextureFiltering(FT_MAG) & FO_USE_COMPARISON) &
			(mProperties.getTextureFiltering(FT_MIP) & FO_USE_COMPARISON)) != 0;

		FilterOptions minFilter = (FilterOptions)(mProperties.getTextureFiltering(FT_MIN) & ~FO_USE_COMPARISON);
		FilterOptions magFilter = (FilterOptions)(mProperties.getTextureFiltering(FT_MAG) & ~FO_USE_COMPARISON);
		FilterOptions mipFilter = (FilterOptions)(mProperties.getTextureFiltering(FT_MIP) & ~FO_USE_COMPARISON);

		if (minFilter == FO_ANISOTROPIC && magFilter == FO_ANISOTROPIC && mipFilter == FO_ANISOTROPIC)
		{
			samplerState.Filter = D3D11_FILTER_ANISOTROPIC;
		}
		else
		{
			if(minFilter == FO_POINT || minFilter == FO_NONE)
			{
				if(magFilter == FO_POINT || magFilter == FO_NONE)
				{
					if(mipFilter == FO_POINT || mipFilter == FO_NONE)
						samplerState.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					else if(mipFilter == FO_LINEAR)
						samplerState.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				}
				else if(magFilter == FO_LINEAR)
				{
					if(mipFilter == FO_POINT || mipFilter == FO_NONE)
						samplerState.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
					else if(mipFilter == FO_LINEAR)
						samplerState.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				}
			}
			else if(minFilter == FO_LINEAR)
			{
				if(magFilter == FO_POINT || magFilter == FO_NONE)
				{
					if(mipFilter == FO_POINT || mipFilter == FO_NONE)
						samplerState.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
					else if(mipFilter == FO_LINEAR)
						samplerState.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				}
				else if(magFilter == FO_LINEAR)
				{
					if(mipFilter == FO_POINT || mipFilter == FO_NONE)
						samplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
					else if(mipFilter == FO_LINEAR)
						samplerState.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				}
			}
		}

		if(isComparison)
		{
			// Adds COMPARISON flag to the filter
			// See: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476132(v=vs.85).aspx
			samplerState.Filter = (D3D11_FILTER)(0x80 | samplerState.Filter);
		}

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		HRESULT hr = device.getD3D11Device()->CreateSamplerState(&samplerState, &mSamplerState);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create sampler state.\nError Description:" + errorDescription);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_SamplerState);

		SamplerStateCore::createInternal();
	}
}