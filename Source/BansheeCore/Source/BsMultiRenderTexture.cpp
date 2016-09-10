//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMultiRenderTexture.h"
#include "BsTexture.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsCoreThread.h"
#include "BsTextureManager.h"
#include "BsFrameAlloc.h"
#include "BsResources.h"

namespace BansheeEngine
{
	MultiRenderTextureProperties::MultiRenderTextureProperties(const MULTI_RENDER_TEXTURE_DESC& desc)
	{
		for (size_t i = 0; i < desc.colorSurfaces.size(); i++)
		{
			HTexture texture = desc.colorSurfaces[i].texture;

			if (texture != nullptr)
			{
				const TextureProperties& texProps = texture->getProperties();
				construct(&texProps);

				break;
			}
		}
	}

	MultiRenderTextureProperties::MultiRenderTextureProperties(const MULTI_RENDER_TEXTURE_CORE_DESC& desc)
	{
		for (size_t i = 0; i < desc.colorSurfaces.size(); i++)
		{
			SPtr<TextureCore> texture = desc.colorSurfaces[i].texture;

			if (texture != nullptr)
			{
				const TextureProperties& texProps = texture->getProperties();
				construct(&texProps);

				break;
			}
		}
	}

	void MultiRenderTextureProperties::construct(const TextureProperties* props)
	{
		if (props == nullptr)
			return;

		mWidth = props->getWidth();
		mHeight = props->getHeight();
		mColorDepth = PixelUtil::getNumElemBits(props->getFormat());
		mActive = true;
		mHwGamma = props->isHardwareGammaEnabled();
		mMultisampleCount = props->getMultisampleCount();
		mIsWindow = false;
		mRequiresTextureFlipping = requiresTextureFlipping();
	}

	MultiRenderTextureCore::MultiRenderTextureCore(const MULTI_RENDER_TEXTURE_CORE_DESC& desc)
		:mDesc(desc)
	{ }

	MultiRenderTextureCore::~MultiRenderTextureCore()
	{
		for (auto iter = mColorSurfaces.begin(); iter != mColorSurfaces.end(); ++iter)
		{
			if (*iter != nullptr)
				TextureCore::releaseView(*iter);
		}

		if (mDepthStencilSurface != nullptr)
			TextureCore::releaseView(mDepthStencilSurface);
	}

	void MultiRenderTextureCore::initialize()
	{
		RenderTargetCore::initialize();

		mColorSurfaces.resize(BS_MAX_MULTIPLE_RENDER_TARGETS);

		for (size_t i = 0; i < mDesc.colorSurfaces.size(); i++)
		{
			if (mDesc.colorSurfaces[i].texture != nullptr)
			{
				if (i >= BS_MAX_MULTIPLE_RENDER_TARGETS)
				{
					LOGWRN("Render texture index is larger than the maximum number of supported render targets. Index: " + toString((int)i) +
						". Max. number of render targets: " + toString(BS_MAX_MULTIPLE_RENDER_TARGETS));

					continue;
				}

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

	void MultiRenderTextureCore::syncToCore(const CoreSyncData& data)
	{
		MultiRenderTextureProperties& props = const_cast<MultiRenderTextureProperties&>(getProperties());
		props = data.getData<MultiRenderTextureProperties>();
	}

	const MultiRenderTextureProperties& MultiRenderTextureCore::getProperties() const
	{
		return static_cast<const MultiRenderTextureProperties&>(getPropertiesInternal());
	}

	SPtr<MultiRenderTextureCore> MultiRenderTextureCore::create(const MULTI_RENDER_TEXTURE_CORE_DESC& desc)
	{
		return TextureCoreManager::instance().createMultiRenderTexture(desc);
	}

	void MultiRenderTextureCore::throwIfBuffersDontMatch() const
	{
		SPtr<TextureView> firstSurfaceDesc = nullptr;
		for(size_t i = 0; i < mColorSurfaces.size(); i++)
		{
			if(mColorSurfaces[i] == nullptr)
				continue;

			if(firstSurfaceDesc == nullptr)
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

			if (firstTexProps.getTextureType() != TEX_TYPE_2D)
				BS_EXCEPT(NotImplementedException, "Render textures are currently only implemented for 2D surfaces.");

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

	void MultiRenderTextureCore::copyToMemory(PixelData &dst, FrameBuffer buffer)
	{
		BS_EXCEPT(InternalErrorException,"The method or operation is not implemented.");
	}

	MultiRenderTexture::MultiRenderTexture(const MULTI_RENDER_TEXTURE_DESC& desc)
		:mDesc(desc)
	{
		for (UINT32 i = 0; i < (UINT32)desc.colorSurfaces.size(); i++)
		{
			if (desc.colorSurfaces[i].texture != nullptr)
				mBindableColorTex.push_back(desc.colorSurfaces[i].texture);
		}

		if (desc.depthStencilSurface.texture != nullptr)
			mBindableDepthStencilTex = desc.depthStencilSurface.texture;
	}

	SPtr<CoreObjectCore> MultiRenderTexture::createCore() const
	{
		MULTI_RENDER_TEXTURE_CORE_DESC coreDesc;

		for (auto& colorSurface : mDesc.colorSurfaces)
		{
			RENDER_SURFACE_CORE_DESC surfaceDesc;
			if (colorSurface.texture.isLoaded())
				surfaceDesc.texture = colorSurface.texture->getCore();

			surfaceDesc.face = colorSurface.face;
			surfaceDesc.numFaces = colorSurface.numFaces;
			surfaceDesc.mipLevel = colorSurface.mipLevel;

			coreDesc.colorSurfaces.push_back(surfaceDesc);
		}

		if (mDesc.depthStencilSurface.texture.isLoaded())
			coreDesc.depthStencilSurface.texture = mDesc.depthStencilSurface.texture->getCore();

		coreDesc.depthStencilSurface.face = mDesc.depthStencilSurface.face;
		coreDesc.depthStencilSurface.numFaces = mDesc.depthStencilSurface.numFaces;
		coreDesc.depthStencilSurface.mipLevel = mDesc.depthStencilSurface.mipLevel;

		return TextureCoreManager::instance().createMultiRenderTextureInternal(coreDesc);
	}

	SPtr<MultiRenderTextureCore> MultiRenderTexture::getCore() const
	{
		return std::static_pointer_cast<MultiRenderTextureCore>(mCoreSpecific);
	}

	SPtr<MultiRenderTexture> MultiRenderTexture::create(const MULTI_RENDER_TEXTURE_DESC& desc)
	{
		return TextureManager::instance().createMultiRenderTexture(desc);
	}

	CoreSyncData MultiRenderTexture::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = sizeof(MultiRenderTextureProperties);
		UINT8* buffer = allocator->alloc(size);

		MultiRenderTextureProperties& props = const_cast<MultiRenderTextureProperties&>(getProperties());

		memcpy(buffer, (void*)&props, size);
		return CoreSyncData(buffer, size);
	}

	const MultiRenderTextureProperties& MultiRenderTexture::getProperties() const
	{
		return static_cast<const MultiRenderTextureProperties&>(getPropertiesInternal());
	}

	UINT32 MultiRenderTexture::getColorSurfaceCount() const
	{
		return (UINT32)mDesc.colorSurfaces.size();
	}
}