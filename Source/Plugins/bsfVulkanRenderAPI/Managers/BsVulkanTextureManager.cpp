//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanTextureManager.h"
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

	SPtr<RenderTexture> VulkanTextureManager::CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		VulkanRenderTexture* tex = new (bs_alloc<VulkanRenderTexture>()) VulkanRenderTexture(desc);

		return bs_core_ptr<VulkanRenderTexture>(tex);
	}

	PixelFormat VulkanTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		PixelUtil::checkFormat(format, ttype, usage);

		if (ct::VulkanUtility::getPixelFormat(format, hwGamma) == VK_FORMAT_UNDEFINED)
			return PF_RGBA8;

		return format;
	}

	namespace ct
	{
	void VulkanTextureManager::OnStartUp()
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
			desc.numArraySlices = entry.arraySize;
			desc.format = PF_RGBA8;
			desc.usage = TU_STATIC | TU_MUTABLEFORMAT;

			mDummyReadTextures[idx] = std::static_pointer_cast<VulkanTexture>(createTexture(desc));
			mDummyReadTextures[idx]->writeData(*pixelData);

			desc.usage = TU_LOADSTORE;
			mDummyStorageTextures[idx] = std::static_pointer_cast<VulkanTexture>(createTexture(desc));

			idx++;
		}
	}

	VulkanTexture* VulkanTextureManager::getDummyTexture(GpuParamObjectType type) const
	{
		switch(type)
		{
		case GPOT_TEXTURE2DMS:
		case GPOT_TEXTURE2D:
			return mDummyReadTextures[2].get();
		case GPOT_RWTEXTURE2D:
		case GPOT_RWTEXTURE2DMS:
			return mDummyStorageTextures[2].get();
		case GPOT_TEXTURECUBE:
			return mDummyReadTextures[5].get();
		case GPOT_TEXTURECUBEARRAY:
			return mDummyReadTextures[6].get();
		case GPOT_TEXTURE2DARRAY:
		case GPOT_TEXTURE2DMSARRAY:
			return mDummyReadTextures[3].get();
		case GPOT_RWTEXTURE2DARRAY:
		case GPOT_RWTEXTURE2DMSARRAY:
			return mDummyStorageTextures[3].get();
		case GPOT_TEXTURE3D:
			return mDummyReadTextures[4].get();
		case GPOT_RWTEXTURE3D:
			return mDummyStorageTextures[4].get();
		case GPOT_TEXTURE1D:
			return mDummyReadTextures[0].get();
		case GPOT_TEXTURE1DARRAY:
			return mDummyReadTextures[1].get();
		case GPOT_RWTEXTURE1D:
			return mDummyStorageTextures[0].get();
		case GPOT_RWTEXTURE1DARRAY:
			return mDummyStorageTextures[1].get();
		default:
			return nullptr;
		}
	}

	VkFormat VulkanTextureManager::GetDummyViewFormat(GpuBufferFormat format)
	{
		switch(format)
		{
		case BF_16X1F:
		case BF_32X1F:
			return VK_FORMAT_R32_SFLOAT;
		case BF_16X2F:
		case BF_32X2F:
			return VK_FORMAT_R16G16_UNORM;
		case BF_32X3F:
		case BF_32X4F:
		case BF_16X4F:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case BF_16X1U:
		case BF_32X1U:
			return VK_FORMAT_R32_UINT;
		case BF_16X2U:
		case BF_32X2U:
			return VK_FORMAT_R16G16_UINT;
		case BF_32X3U:
		case BF_32X4U:
		case BF_16X4U:
			return VK_FORMAT_R8G8B8A8_UINT;
		case BF_16X1S:
		case BF_32X1S:
			return VK_FORMAT_R32_SINT;
		case BF_16X2S:
		case BF_32X2S:
			return VK_FORMAT_R16G16_SINT;
		case BF_32X3S:
		case BF_32X4S:
		case BF_16X4S:
			return VK_FORMAT_R8G8B8A8_SINT;
		default:
			return VK_FORMAT_UNDEFINED;
		}
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
