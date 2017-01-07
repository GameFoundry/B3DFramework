//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11TextureManager.h"
#include "BsD3D11Texture.h"
#include "BsD3D11RenderTexture.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11RenderAPI.h"

namespace bs
{
	SPtr<RenderTexture> D3D11TextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		D3D11RenderTexture* tex = new (bs_alloc<D3D11RenderTexture>()) D3D11RenderTexture(desc);

		return bs_core_ptr<D3D11RenderTexture>(tex);
	}

	PixelFormat D3D11TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		DXGI_FORMAT d3dPF = D3D11Mappings::getPF(D3D11Mappings::getClosestSupportedPF(format, ttype, usage), hwGamma);

		return D3D11Mappings::getPF(d3dPF);
	}

	SPtr<TextureCore> D3D11TextureCoreManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		D3D11TextureCore* tex = new (bs_alloc<D3D11TextureCore>()) D3D11TextureCore(desc, initialData, deviceMask);

		SPtr<D3D11TextureCore> texPtr = bs_shared_ptr<D3D11TextureCore>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> D3D11TextureCoreManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC_CORE& desc,
																				 UINT32 deviceIdx)
	{
		SPtr<D3D11RenderTextureCore> texPtr = bs_shared_ptr_new<D3D11RenderTextureCore>(desc, deviceIdx);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
}