//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextureManager.h"
#include "BsException.h"
#include "BsPixelUtil.h"
#include "BsRenderAPI.h"

namespace bs 
{
    SPtr<Texture> TextureManager::createTexture(const TEXTURE_DESC& desc)
    {
		Texture* tex = new (bs_alloc<Texture>()) Texture(desc);
		SPtr<Texture> ret = bs_core_ptr<Texture>(tex);

		ret->_setThisPtr(ret);
		ret->initialize();

		return ret;
    }

	SPtr<Texture> TextureManager::createTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData)
    {
		Texture* tex = new (bs_alloc<Texture>()) Texture(desc, pixelData);
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

	SPtr<RenderTexture> TextureManager::createRenderTexture(const TEXTURE_DESC& colorDesc, bool createDepth,
		PixelFormat depthStencilFormat)
	{
		TEXTURE_DESC textureDesc = colorDesc;
		textureDesc.usage = TU_RENDERTARGET;
		textureDesc.numMips = 0;

		HTexture texture = Texture::create(textureDesc);

		HTexture depthStencil;
		if(createDepth)
		{
			textureDesc.format = depthStencilFormat;
			textureDesc.hwGamma = false;
			textureDesc.usage = TU_DEPTHSTENCIL;

			depthStencil = Texture::create(textureDesc);
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

	namespace ct
	{
	void TextureManager::onStartUp()
    {
		TEXTURE_DESC desc;
		desc.type = TEX_TYPE_2D;
		desc.width = 2;
		desc.height = 2;
		desc.format = PF_R8G8B8A8;
		desc.usage = TU_STATIC;

		// White built-in texture
		SPtr<TextureCore> whiteTexture = createTexture(desc);

		SPtr<PixelData> whitePixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		whitePixelData->setColorAt(Color::White, 0, 0);
		whitePixelData->setColorAt(Color::White, 0, 1);
		whitePixelData->setColorAt(Color::White, 1, 0);
		whitePixelData->setColorAt(Color::White, 1, 1);

		whiteTexture->writeData(*whitePixelData);
		TextureCore::WHITE = whiteTexture;

		// Black built-in texture
		SPtr<TextureCore> blackTexture = createTexture(desc);

		SPtr<PixelData> blackPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		blackPixelData->setColorAt(Color::Black, 0, 0);
		blackPixelData->setColorAt(Color::Black, 0, 1);
		blackPixelData->setColorAt(Color::Black, 1, 0);
		blackPixelData->setColorAt(Color::Black, 1, 1);

		blackTexture->writeData(*blackPixelData);
		TextureCore::BLACK = blackTexture;

		// Normal (Y = Up) built-in texture
		SPtr<TextureCore> normalTexture = createTexture(desc);
		SPtr<PixelData> normalPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);

		Color encodedNormal(0.5f, 0.5f, 1.0f);
		normalPixelData->setColorAt(encodedNormal, 0, 0);
		normalPixelData->setColorAt(encodedNormal, 0, 1);
		normalPixelData->setColorAt(encodedNormal, 1, 0);
		normalPixelData->setColorAt(encodedNormal, 1, 1);

		normalTexture->writeData(*normalPixelData);
		TextureCore::NORMAL = normalTexture;
    }

	void TextureManager::onShutDown()
    {
		// Need to make sure these are freed while still on the core thread
		TextureCore::WHITE = nullptr;
		TextureCore::BLACK = nullptr;
		TextureCore::NORMAL = nullptr;
    }

	SPtr<TextureCore> TextureManager::createTexture(const TEXTURE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<TextureCore> newRT = createTextureInternal(desc, nullptr, deviceMask);
		newRT->initialize();

		return newRT;
	}

	SPtr<RenderTextureCore> TextureManager::createRenderTexture(const RENDER_TEXTURE_DESC& desc, 
																	UINT32 deviceIdx)
	{
		SPtr<RenderTextureCore> newRT = createRenderTextureInternal(desc, deviceIdx);
		newRT->initialize();

		return newRT;
	}
	}
}
