//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTextureManager.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderTexture.h"
#include "BsVulkanResource.h"
#include "BsVulkanUtility.h"

namespace bs
{
	struct DummyTexFormat
	{
		TextureType type;
		int arraySize;
		int width;
		int height;
		int depth;
	};

	const static DummyTexFormat DummyTexTypes[] =
	{
		{ TEX_TYPE_1D,			1, 2, 1, 1 },
		{ TEX_TYPE_1D,			2, 2, 1, 1 },
		{ TEX_TYPE_2D,			1, 2, 2, 1 },
		{ TEX_TYPE_2D,			2, 2, 2, 1 },
		{ TEX_TYPE_3D,			1, 2, 2, 2 },
		{ TEX_TYPE_CUBE_MAP,	1, 2, 2, 1 },
		{ TEX_TYPE_CUBE_MAP,	2, 2, 2, 1 }
	};

	SPtr<RenderTexture> VulkanTextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		VulkanRenderTexture* tex = new (bs_alloc<VulkanRenderTexture>()) VulkanRenderTexture(desc);

		return bs_core_ptr<VulkanRenderTexture>(tex);
	}

	PixelFormat VulkanTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		PixelUtil::checkFormat(format, ttype, usage);

		if (ct::VulkanUtility::getPixelFormat(format, hwGamma) == VK_FORMAT_UNDEFINED)
			return PF_RGBA8;

		return format;
	}

	namespace ct
	{
	void VulkanTextureManager::onStartUp()
	{
		TextureManager::onStartUp();

		int idx = 0;
		for(auto& entry : DummyTexTypes)
		{
			SPtr<PixelData> pixelData = PixelData::create(entry.width, entry.height, entry.depth, PF_RGBA8);

			for(int depth = 0; depth < entry.depth; depth++)
				for(int height = 0; height < entry.height; height++)
					for(int width = 0; width < entry.width; width++)
						pixelData->setColorAt(Color::White, width, height, depth);

			TEXTURE_DESC desc;
			desc.type = entry.type;
			desc.width = entry.width;
			desc.height = entry.height;
			desc.depth = entry.depth;
			desc.format = PF_RGBA8;
			desc.usage = TU_STATIC;

			mDummyReadTextures[idx] = std::static_pointer_cast<VulkanTexture>(createTexture(desc));
			mDummyReadTextures[idx]->writeData(*pixelData);

			desc.usage = TU_LOADSTORE;
			mDummyStorageTextures[idx] = std::static_pointer_cast<VulkanTexture>(createTexture(desc));

			idx++;
		}
	}

	VkImageView VulkanTextureManager::getDummyImageView(GpuParamObjectType type, UINT32 deviceIdx) const
	{
		SPtr<VulkanTexture> texture;
		switch(type)
		{
		case GPOT_TEXTURE2DMS:
		case GPOT_TEXTURE2D:
			texture = mDummyReadTextures[2];
			break;
		case GPOT_RWTEXTURE2D:
		case GPOT_RWTEXTURE2DMS:
			texture = mDummyStorageTextures[2];
			break;
		case GPOT_TEXTURECUBE:
			texture = mDummyReadTextures[5];
			break;
		case GPOT_TEXTURECUBEARRAY:
			texture = mDummyReadTextures[6];
			break;
		case GPOT_TEXTURE2DARRAY:
		case GPOT_TEXTURE2DMSARRAY:
			texture = mDummyReadTextures[3];
			break;
		case GPOT_RWTEXTURE2DARRAY:
		case GPOT_RWTEXTURE2DMSARRAY:
			texture = mDummyStorageTextures[3];
			break;
		case GPOT_TEXTURE3D:
			texture = mDummyReadTextures[4];
			break;
		case GPOT_RWTEXTURE3D:
			texture = mDummyStorageTextures[4];
			break;
		case GPOT_TEXTURE1D:
			texture = mDummyReadTextures[0];
			break;
		case GPOT_TEXTURE1DARRAY:
			texture = mDummyReadTextures[1];
			break;
		case GPOT_RWTEXTURE1D:
			texture = mDummyStorageTextures[0];
			break;
		case GPOT_RWTEXTURE1DARRAY:
			texture = mDummyStorageTextures[1];
			break;
		}

		return texture->getResource(deviceIdx)->getView(false);
	}

	SPtr<Texture> VulkanTextureManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		VulkanTexture* tex = new (bs_alloc<VulkanTexture>()) VulkanTexture(desc, initialData, deviceMask);

		SPtr<VulkanTexture> texPtr = bs_shared_ptr<VulkanTexture>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTexture> VulkanTextureManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc,
																				  UINT32 deviceIdx)
	{
		SPtr<VulkanRenderTexture> texPtr = bs_shared_ptr_new<VulkanRenderTexture>(desc, deviceIdx);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
	}
}