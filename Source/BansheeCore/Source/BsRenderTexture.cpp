//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderTexture.h"
#include "BsException.h"
#include "BsTexture.h"
#include "BsTextureManager.h"
#include "BsResources.h"
#include "BsCoreThread.h"
#include "BsFrameAlloc.h"

namespace BansheeEngine
{
	RenderTextureProperties::RenderTextureProperties(const RENDER_TEXTURE_DESC& desc, bool requiresFlipping)
	{
		HTexture texture = desc.colorSurface.texture;

		if (texture.isLoaded())
		{
			const TextureProperties& props = texture->getProperties();
			construct(&props, requiresFlipping);
		}
		else
			construct(nullptr, requiresFlipping);
	}

	RenderTextureProperties::RenderTextureProperties(const RENDER_TEXTURE_CORE_DESC& desc, bool requiresFlipping)
	{
		SPtr<TextureCore> texture = desc.colorSurface.texture;

		if (texture != nullptr)
		{
			const TextureProperties& props = texture->getProperties();
			construct(&props, requiresFlipping);
		}
		else
			construct(nullptr, requiresFlipping);
	}

	void RenderTextureProperties::construct(const TextureProperties* textureProps, bool requiresFlipping)
	{
		if (textureProps != nullptr)
		{
			mWidth = textureProps->getWidth();
			mHeight = textureProps->getHeight();
			mColorDepth = BansheeEngine::PixelUtil::getNumElemBits(textureProps->getFormat());
			mHwGamma = textureProps->isHardwareGammaEnabled();
			mMultisampleCount = textureProps->getMultisampleCount();
		}

		mActive = true;
		mIsWindow = false;
		mRequiresTextureFlipping = requiresFlipping;
	}

	RenderTextureCore::RenderTextureCore(const RENDER_TEXTURE_CORE_DESC& desc)
		:mColorSurface(nullptr), mDepthStencilSurface(nullptr), mDesc(desc)
	{ }

	RenderTextureCore::~RenderTextureCore()
	{ 
		if (mColorSurface != nullptr)
			TextureCore::releaseView(mColorSurface);

		if (mDepthStencilSurface != nullptr)
			TextureCore::releaseView(mDepthStencilSurface);
	}

	void RenderTextureCore::initialize()
	{
		RenderTargetCore::initialize();

		const RENDER_SURFACE_CORE_DESC& colorSurface = mDesc.colorSurface;
		if (colorSurface.texture != nullptr)
		{
			SPtr<TextureCore> texture = colorSurface.texture;

			if (texture->getProperties().getUsage() != TU_RENDERTARGET)
				BS_EXCEPT(InvalidParametersException, "Provided texture is not created with render target usage.");

			mColorSurface = TextureCore::requestView(texture, colorSurface.mipLevel, 1,
				colorSurface.face, colorSurface.numFaces, GVU_RENDERTARGET);
		}

		const RENDER_SURFACE_CORE_DESC& depthStencilSurface = mDesc.depthStencilSurface;
		if (depthStencilSurface.texture != nullptr)
		{
			SPtr<TextureCore> texture = depthStencilSurface.texture;

			if (texture->getProperties().getUsage() != TU_DEPTHSTENCIL)
				BS_EXCEPT(InvalidParametersException, "Provided texture is not created with depth stencil usage.");

			mDepthStencilSurface = TextureCore::requestView(texture, depthStencilSurface.mipLevel, 1,
				depthStencilSurface.face, 0, GVU_DEPTHSTENCIL);
		}

		throwIfBuffersDontMatch();

		if (mColorSurface != nullptr)
		{
			assert(mColorSurface->getTexture() != nullptr);

			SPtr<TextureCore> colorTexture = mColorSurface->getTexture();
			const TextureProperties& texProps = colorTexture->getProperties();

			UINT32 numSlices;
			if (texProps.getTextureType() == TEX_TYPE_3D)
				numSlices = texProps.getDepth();
			else
				numSlices = texProps.getNumFaces();

			if ((mColorSurface->getFirstArraySlice() + mColorSurface->getNumArraySlices()) > numSlices)
			{
				BS_EXCEPT(InvalidParametersException, "Provided number of faces is out of range. Face: " +
					toString(mColorSurface->getFirstArraySlice() + mColorSurface->getNumArraySlices()) +
					". Max num faces: " + toString(numSlices));
			}

			if (mColorSurface->getMostDetailedMip() > texProps.getNumMipmaps())
			{
				BS_EXCEPT(InvalidParametersException, "Provided number of mip maps is out of range. Mip level: " +
					toString(mColorSurface->getMostDetailedMip()) + ". Max num mipmaps: " + toString(texProps.getNumMipmaps()));
			}
		}
	}

	SPtr<RenderTextureCore> RenderTextureCore::create(const RENDER_TEXTURE_CORE_DESC& desc)
	{
		return TextureCoreManager::instance().createRenderTexture(desc);
	}

	void RenderTextureCore::syncToCore(const CoreSyncData& data)
	{
		RenderTextureProperties& props = const_cast<RenderTextureProperties&>(getProperties());
		props = data.getData<RenderTextureProperties>();
	}

	const RenderTextureProperties& RenderTextureCore::getProperties() const
	{
		return static_cast<const RenderTextureProperties&>(getPropertiesInternal());
	}

	void RenderTextureCore::throwIfBuffersDontMatch() const
	{
		if (mColorSurface == nullptr || mDepthStencilSurface == nullptr)
			return;

		const TextureProperties& colorProps = mColorSurface->getTexture()->getProperties();
		const TextureProperties& depthProps = mDepthStencilSurface->getTexture()->getProperties();

		UINT32 colorMsCount = colorProps.getMultisampleCount();
		UINT32 depthMsCount = depthProps.getMultisampleCount();

		if (colorMsCount == 0)
			colorMsCount = 1;

		if (depthMsCount == 0)
			depthMsCount = 1;

		if (colorProps.getWidth() != depthProps.getWidth() ||
			colorProps.getHeight() != depthProps.getHeight() ||
			colorMsCount != depthMsCount)
		{
			String errorInfo = "\nWidth: " + toString(colorProps.getWidth()) + "/" + toString(depthProps.getWidth());
			errorInfo += "\nHeight: " + toString(colorProps.getHeight()) + "/" + toString(depthProps.getHeight());
			errorInfo += "\nMultisample Count: " + toString(colorMsCount) + "/" + toString(depthMsCount);

			BS_EXCEPT(InvalidParametersException, "Provided texture and depth stencil buffer don't match!" + errorInfo);
		}
	}

	SPtr<RenderTexture> RenderTexture::create(TextureType textureType, UINT32 width, UINT32 height, 
		PixelFormat format, bool hwGamma, UINT32 multisampleCount, 
		bool createDepth, PixelFormat depthStencilFormat)
	{
		return TextureManager::instance().createRenderTexture(textureType, width, height, format, hwGamma, 
			multisampleCount, createDepth, depthStencilFormat);
	}

	SPtr<RenderTexture> RenderTexture::create(const RENDER_TEXTURE_DESC& desc)
	{
		return TextureManager::instance().createRenderTexture(desc);
	}

	SPtr<RenderTextureCore> RenderTexture::getCore() const 
	{ 
		return std::static_pointer_cast<RenderTextureCore>(mCoreSpecific); 
	}

	RenderTexture::RenderTexture(const RENDER_TEXTURE_DESC& desc)
	{
		mDesc = desc;

		if (desc.colorSurface.texture != nullptr)
			mBindableColorTex = desc.colorSurface.texture;

		if (desc.depthStencilSurface.texture != nullptr)
			mBindableDepthStencilTex = desc.depthStencilSurface.texture;
	}

	SPtr<CoreObjectCore> RenderTexture::createCore() const
	{
		RENDER_TEXTURE_CORE_DESC coreDesc;

		if (mDesc.colorSurface.texture.isLoaded())
			coreDesc.colorSurface.texture = mDesc.colorSurface.texture->getCore();

		if (mDesc.depthStencilSurface.texture.isLoaded())
			coreDesc.depthStencilSurface.texture = mDesc.depthStencilSurface.texture->getCore();

		coreDesc.colorSurface.face = mDesc.colorSurface.face;
		coreDesc.colorSurface.numFaces = mDesc.colorSurface.numFaces;
		coreDesc.colorSurface.mipLevel = mDesc.colorSurface.mipLevel;
		coreDesc.depthStencilSurface.face = mDesc.depthStencilSurface.face;
		coreDesc.depthStencilSurface.numFaces = mDesc.depthStencilSurface.numFaces;
		coreDesc.depthStencilSurface.mipLevel = mDesc.depthStencilSurface.mipLevel;

		return TextureCoreManager::instance().createRenderTextureInternal(coreDesc);
	}

	CoreSyncData RenderTexture::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = sizeof(RenderTextureProperties);
		UINT8* buffer = allocator->alloc(size);

		RenderTextureProperties& props = const_cast<RenderTextureProperties&>(getProperties());

		memcpy(buffer, (void*)&props, size);
		return CoreSyncData(buffer, size);
	}

	const RenderTextureProperties& RenderTexture::getProperties() const
	{
		return static_cast<const RenderTextureProperties&>(getPropertiesInternal());
	}
}
