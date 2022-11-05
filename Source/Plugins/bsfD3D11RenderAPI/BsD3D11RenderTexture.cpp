//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderTexture.h"
#include "BsD3D11TextureView.h"

using namespace bs;

D3D11RenderTexture::D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc)
	: RenderTexture(desc), mProperties(desc, false)
{
}

namespace bs { namespace ct {
D3D11RenderTexture::D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
	: RenderTexture(desc, deviceIdx), mProperties(desc, false)
{
	B3D_ASSERT(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");
}

void D3D11RenderTexture::GetCustomAttribute(const String& name, void* data) const
{
	if(name == "RTV")
	{
		ID3D11RenderTargetView** rtvs = (ID3D11RenderTargetView**)data;
		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; ++i)
		{
			if(mColorSurfaces[i] == nullptr)
				continue;

			D3D11TextureView* textureView = static_cast<D3D11TextureView*>(mColorSurfaces[i].get());
			rtvs[i] = textureView->GetRtv();
		}
	}
	else if(name == "DSV")
	{
		if(mDepthStencilSurface == nullptr)
			return;

		ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
		D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

		*dsv = depthStencilView->GetDsv(false, false);
	}
	else if(name == "RODSV")
	{
		if(mDepthStencilSurface == nullptr)
			return;

		ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
		D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

		*dsv = depthStencilView->GetDsv(true, true);
	}
	else if(name == "RODWSV")
	{
		if(mDepthStencilSurface == nullptr)
			return;

		ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
		D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

		*dsv = depthStencilView->GetDsv(true, false);
	}
	else if(name == "WDROSV")
	{
		if(mDepthStencilSurface == nullptr)
			return;

		ID3D11DepthStencilView** dsv = (ID3D11DepthStencilView**)data;
		D3D11TextureView* depthStencilView = static_cast<D3D11TextureView*>(mDepthStencilSurface.get());

		*dsv = depthStencilView->GetDsv(false, true);
	}
}
}} // namespace bs::ct
