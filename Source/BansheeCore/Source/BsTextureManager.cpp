//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextureManager.h"
#include "BsException.h"
#include "BsPixelUtil.h"
#include "BsRenderAPI.h"

namespace BansheeEngine 
{
    SPtr<Texture> TextureManager::createTexture(TextureType texType, UINT32 width, UINT32 height, UINT32 depth, int numMipmaps,
        PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount, UINT32 numArraySlices)
    {
		Texture* tex = new (bs_alloc<Texture>()) Texture(texType, width, height, depth, numMipmaps, format, usage, hwGamma, multisampleCount, numArraySlices);
		SPtr<Texture> ret = bs_core_ptr<Texture>(tex);

		ret->_setThisPtr(ret);
		ret->initialize();

		return ret;
    }

	SPtr<Texture> TextureManager::createTexture(const SPtr<PixelData>& pixelData, int usage , bool hwGammaCorrection)
    {
		Texture* tex = new (bs_alloc<Texture>()) Texture(pixelData, usage, hwGammaCorrection);
		SPtr<Texture> ret = bs_core_ptr<Texture>(tex);

		ret->_setThisPtr(ret);
		ret->initialize();

		return ret;
    }

	SPtr<Texture> TextureManager::_createEmpty()
	{
		Texture* tex = new (bs_alloc<Texture>()) Texture();
		SPtr<Texture> texture = bs_core_ptr<Texture>(tex);
		texture->_setThisPtr(texture);

		return texture;
	}

	SPtr<RenderTexture> TextureManager::createRenderTexture(TextureType textureType, UINT32 width, UINT32 height, 
			PixelFormat format, bool hwGamma, UINT32 multisampleCount, 
			bool createDepth, PixelFormat depthStencilFormat)
	{
		HTexture texture = Texture::create(textureType, width, height, 0, format, TU_RENDERTARGET, hwGamma, multisampleCount);

		HTexture depthStencil;
		if(createDepth)
		{
			depthStencil = Texture::create(TEX_TYPE_2D, width, height, 0, depthStencilFormat, TU_DEPTHSTENCIL, false, multisampleCount);
		}

		RENDER_TEXTURE_DESC desc;
		desc.colorSurfaces[0].texture = texture;
		desc.colorSurfaces[0].face = 0;
		desc.colorSurfaces[0].numFaces = 1;
		desc.colorSurfaces[0].mipLevel = 0;

		desc.depthStencilSurface.texture = depthStencil;
		desc.depthStencilSurface.face = 0;
		desc.depthStencilSurface.numFaces = 1;
		desc.depthStencilSurface.mipLevel = 0;

		SPtr<RenderTexture> newRT = createRenderTexture(desc);

		return newRT;
	}

	SPtr<RenderTexture> TextureManager::createRenderTexture(const RENDER_TEXTURE_DESC& desc)
	{
		SPtr<RenderTexture> newRT = createRenderTextureImpl(desc);
		newRT->_setThisPtr(newRT);
		newRT->initialize();

		return newRT;
	}

	void TextureCoreManager::onStartUp()
    {
		// White built-in texture
		SPtr<TextureCore> whiteTexture = createTexture(TEX_TYPE_2D, 2, 2, 1, 0, PF_R8G8B8A8, TU_STATIC);

		SPtr<PixelData> whitePixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		whitePixelData->setColorAt(Color::White, 0, 0);
		whitePixelData->setColorAt(Color::White, 0, 1);
		whitePixelData->setColorAt(Color::White, 1, 0);
		whitePixelData->setColorAt(Color::White, 1, 1);

		whiteTexture->writeData(*whitePixelData);
		TextureCore::WHITE = whiteTexture;

		// Black built-in texture
		SPtr<TextureCore> blackTexture = createTexture(TEX_TYPE_2D, 2, 2, 1, 0, PF_R8G8B8A8, TU_STATIC);

		SPtr<PixelData> blackPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		blackPixelData->setColorAt(Color::Black, 0, 0);
		blackPixelData->setColorAt(Color::Black, 0, 1);
		blackPixelData->setColorAt(Color::Black, 1, 0);
		blackPixelData->setColorAt(Color::Black, 1, 1);

		blackTexture->writeData(*blackPixelData);
		TextureCore::BLACK = blackTexture;

		// Normal (Y = Up) built-in texture
		SPtr<TextureCore> normalTexture = createTexture(TEX_TYPE_2D, 2, 2, 1, 0, PF_R8G8B8A8, TU_STATIC);
		SPtr<PixelData> normalPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);

		Color encodedNormal(0.5f, 0.5f, 1.0f);
		normalPixelData->setColorAt(encodedNormal, 0, 0);
		normalPixelData->setColorAt(encodedNormal, 0, 1);
		normalPixelData->setColorAt(encodedNormal, 1, 0);
		normalPixelData->setColorAt(encodedNormal, 1, 1);

		normalTexture->writeData(*normalPixelData);
		TextureCore::NORMAL = normalTexture;
    }

	void TextureCoreManager::onShutDown()
    {
		// Need to make sure these are freed while still on the core thread
		TextureCore::WHITE = nullptr;
		TextureCore::BLACK = nullptr;
		TextureCore::NORMAL = nullptr;
    }

	SPtr<TextureCore> TextureCoreManager::createTexture(TextureType texType, UINT32 width, UINT32 height, UINT32 depth,
		int numMips, PixelFormat format, int usage, bool hwGammaCorrection, UINT32 multisampleCount, UINT32 numArraySlices)
	{
		SPtr<TextureCore> newRT = createTextureInternal(texType, width, height, depth, numMips, format, 
			usage, hwGammaCorrection, multisampleCount, numArraySlices);
		newRT->initialize();

		return newRT;
	}

	SPtr<RenderTextureCore> TextureCoreManager::createRenderTexture(const RENDER_TEXTURE_DESC_CORE& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<RenderTextureCore> newRT = createRenderTextureInternal(desc, deviceMask);
		newRT->initialize();

		return newRT;
	}
}
