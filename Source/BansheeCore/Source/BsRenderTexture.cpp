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
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			HTexture texture = desc.colorSurfaces[i].texture;

			if (texture.isLoaded())
			{
				const TextureProperties& texProps = texture->getProperties();
				construct(&texProps, requiresFlipping);

				break;
			}
		}
	}

	RenderTextureProperties::RenderTextureProperties(const RENDER_TEXTURE_DESC_CORE& desc, bool requiresFlipping)
	{
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			SPtr<TextureCore> texture = desc.colorSurfaces[i].texture;

			if (texture != nullptr)
			{
				const TextureProperties& texProps = texture->getProperties();
				construct(&texProps, requiresFlipping);

				break;
			}
		}
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

	RenderTextureCore::RenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask)
		:mDesc(desc)
	{ }

	RenderTextureCore::~RenderTextureCore()
	{ 
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (mColorSurfaces[i] != nullptr)
				TextureCore::releaseView(mColorSurfaces[i]);
		}

		if (mDepthStencilSurface != nullptr)
			TextureCore::releaseView(mDepthStencilSurface);
	}

	void RenderTextureCore::initialize()
	{
		RenderTargetCore::initialize();

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (mDesc.colorSurfaces[i].texture != nullptr)
			{
				SPtr<TextureCore> texture = mDesc.colorSurfaces[i].texture;

				if (texture->getProperties().getUsage() != TU_RENDERTARGET)
					BS_EXCEPT(InvalidParametersException, "Provided texture is not created with render target usage.");

				mColorSurfaces[i] = TextureCore::requestView(texture, mDesc.colorSurfaces[i].mipLevel, 1,
					mDesc.colorSurfaces[i].face, mDesc.colorSurfaces[i].numFaces, GVU_RENDERTARGET);
			}
		}

		if (mDesc.depthStencilSurface.texture != nullptr)
		{
			SPtr<TextureCore> texture = mDesc.depthStencilSurface.texture;

			if (texture->getProperties().getUsage() != TU_DEPTHSTENCIL)
				BS_EXCEPT(InvalidParametersException, "Provided texture is not created with depth stencil usage.");

			mDepthStencilSurface = TextureCore::requestView(texture, mDesc.depthStencilSurface.mipLevel, 1,
				mDesc.depthStencilSurface.face, 0, GVU_DEPTHSTENCIL);
		}

		throwIfBuffersDontMatch();
	}

	SPtr<RenderTextureCore> RenderTextureCore::create(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask)
	{
		return TextureCoreManager::instance().createRenderTexture(desc, deviceMask);
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
		SPtr<TextureView> firstSurfaceDesc = nullptr;
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (mColorSurfaces[i] == nullptr)
				continue;

			if (firstSurfaceDesc == nullptr)
			{
				firstSurfaceDesc = mColorSurfaces[i];
				continue;
			}

			const TextureProperties& curTexProps = mColorSurfaces[i]->getTexture()->getProperties();
			const TextureProperties& firstTexProps = firstSurfaceDesc->getTexture()->getProperties();

			UINT32 curMsCount = curTexProps.getMultisampleCount();
			UINT32 firstMsCount = firstTexProps.getMultisampleCount();

			if (curMsCount == 0)
				curMsCount = 1;

			if (firstMsCount == 0)
				firstMsCount = 1;

			if (curTexProps.getWidth() != firstTexProps.getWidth() ||
				curTexProps.getHeight() != firstTexProps.getHeight() ||
				curMsCount != firstMsCount)
			{
				String errorInfo = "\nWidth: " + toString(curTexProps.getWidth()) + "/" + toString(firstTexProps.getWidth());
				errorInfo += "\nHeight: " + toString(curTexProps.getHeight()) + "/" + toString(firstTexProps.getHeight());
				errorInfo += "\nMultisample Count: " + toString(curMsCount) + "/" + toString(firstMsCount);

				BS_EXCEPT(InvalidParametersException, "Provided color textures don't match!" + errorInfo);
			}
		}

		if (firstSurfaceDesc != nullptr)
		{
			const TextureProperties& firstTexProps = firstSurfaceDesc->getTexture()->getProperties();

			UINT32 numSlices;
			if (firstTexProps.getTextureType() == TEX_TYPE_3D)
				numSlices = firstTexProps.getDepth();
			else
				numSlices = firstTexProps.getNumFaces();

			if ((firstSurfaceDesc->getFirstArraySlice() + firstSurfaceDesc->getNumArraySlices()) > numSlices)
			{
				BS_EXCEPT(InvalidParametersException, "Provided number of faces is out of range. Face: " +
					toString(firstSurfaceDesc->getFirstArraySlice() + firstSurfaceDesc->getNumArraySlices()) + ". Max num faces: " + toString(numSlices));
			}

			if (firstSurfaceDesc->getMostDetailedMip() > firstTexProps.getNumMipmaps())
			{
				BS_EXCEPT(InvalidParametersException, "Provided number of mip maps is out of range. Mip level: " +
					toString(firstSurfaceDesc->getMostDetailedMip()) + ". Max num mipmaps: " + toString(firstTexProps.getNumMipmaps()));
			}

			if (mDepthStencilSurface == nullptr)
				return;

			const TextureProperties& depthTexProps = mDepthStencilSurface->getTexture()->getProperties();
			UINT32 depthMsCount = depthTexProps.getMultisampleCount();
			UINT32 colorMsCount = firstTexProps.getMultisampleCount();

			if (depthMsCount == 0)
				depthMsCount = 1;

			if (colorMsCount == 0)
				colorMsCount = 1;

			if (depthTexProps.getWidth() != firstTexProps.getWidth() ||
				depthTexProps.getHeight() != firstTexProps.getHeight() ||
				depthMsCount != colorMsCount)
			{
				String errorInfo = "\nWidth: " + toString(depthTexProps.getWidth()) + "/" + toString(firstTexProps.getWidth());
				errorInfo += "\nHeight: " + toString(depthTexProps.getHeight()) + "/" + toString(firstTexProps.getHeight());
				errorInfo += "\nMultisample Count: " + toString(depthMsCount) + "/" + toString(colorMsCount);

				BS_EXCEPT(InvalidParametersException, "Provided texture and depth stencil buffer don't match!" + errorInfo);
			}
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
		:mDesc(desc)
	{
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (desc.colorSurfaces[i].texture != nullptr)
				mBindableColorTex[i] = desc.colorSurfaces[i].texture;
		}

		if (desc.depthStencilSurface.texture != nullptr)
			mBindableDepthStencilTex = desc.depthStencilSurface.texture;
	}

	SPtr<CoreObjectCore> RenderTexture::createCore() const
	{
		RENDER_TEXTURE_DESC_CORE coreDesc;

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			RENDER_SURFACE_DESC_CORE surfaceDesc;
			if (mDesc.colorSurfaces[i].texture.isLoaded())
				surfaceDesc.texture = mDesc.colorSurfaces[i].texture->getCore();

			surfaceDesc.face = mDesc.colorSurfaces[i].face;
			surfaceDesc.numFaces = mDesc.colorSurfaces[i].numFaces;
			surfaceDesc.mipLevel = mDesc.colorSurfaces[i].mipLevel;

			coreDesc.colorSurfaces[i] = surfaceDesc;
		}

		if (mDesc.depthStencilSurface.texture.isLoaded())
			coreDesc.depthStencilSurface.texture = mDesc.depthStencilSurface.texture->getCore();

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
