//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderTexture.h"
#include "BsD3D11TextureView.h"

namespace bs
{
	D3D11RenderTexture::D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{

	}

	namespace ct
	{
	D3D11RenderTexture::D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc, UINT32 deviceIdx)
		:RenderTexture(desc, deviceIdx), mProperties(desc, false)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");
	}

	void D3D11RenderTexture::GetCustomAttribute(const String& name, void* data) const
	{
		if(name == "RTV")
		{
			ID3D11RenderTargetView** rtvs = (ID3D11RenderTargetView**)data;
			for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; ++i)
			{
				if (mColorSurfaces[i] == nullptr)
					continue;

				D3D11TextureView* textureView = static_cast<D3D11TextureView*>(mColorSurfaces[i].Get());
				rtvs[i] = textureView->GetRTV();
			}
		}
		else If(name == "DSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.Get());

			*dsv = depthStencilView->GetDSV(false, false);
		}
		else if (name == "RODSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.Get());

			*dsv = depthStencilView->GetDSV(true, true);
		}
		else if (name == "RODWSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.Get());

			*dsv = depthStencilView->GetDSV(true, false);
		}
		else if (name == "WDROSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.Get());

			*dsv = depthStencilView->GetDSV(false, true);
		}
	}
}}
