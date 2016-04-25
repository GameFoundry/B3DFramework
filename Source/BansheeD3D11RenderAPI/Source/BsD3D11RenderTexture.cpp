//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderTexture.h"
#include "BsD3D11TextureView.h"

namespace BansheeEngine
{
	D3D11RenderTextureCore::D3D11RenderTextureCore(const RENDER_TEXTURE_CORE_DESC& desc)
		:RenderTextureCore(desc), mProperties(desc, false)
	{ 

	}

	void D3D11RenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "RTV")
		{
			ID3D11RenderTargetView** rtvs = (ID3D11RenderTargetView **)data;			
			D3D11TextureView* textureView = static_cast<D3D11TextureView*>(mColorSurface.get());
			*rtvs = textureView->getRTV();		
		}
		else if(name == "DSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView **)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

			*dsv = depthStencilView->getDSV(false);
		}
		else if (name == "RODSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView **)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

			*dsv = depthStencilView->getDSV(true);
		}
	}

	D3D11RenderTexture::D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{ 

	}
}