//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		:RenderTextureCore(desc, deviceIdx), mProperties(desc, false)
	{ 
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");
	}

	void D3D11RenderTexture::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "RTV")
		{
			ID3D11RenderTargetView** rtvs = (ID3D11RenderTargetView**)data;
			for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; ++i)
			{
				if (mColorSurfaces[i] == nullptr)
					continue;

				D3D11TextureView* textureView = static_cast<D3D11TextureView*>(mColorSurfaces[i].get());
				rtvs[i] = textureView->getRTV();
			}
		}
		else if(name == "DSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

			*dsv = depthStencilView->getDSV(false);
		}
		else if (name == "RODSV")
		{
			if (mDepthStencilSurface == nullptr)
				return;

			ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
			D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

			*dsv = depthStencilView->getDSV(true);
		}
	}
}}