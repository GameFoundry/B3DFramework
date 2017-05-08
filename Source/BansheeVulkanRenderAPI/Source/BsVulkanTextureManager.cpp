//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTextureManager.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderTexture.h"
#include "BsVulkanResource.h"
#include "BsVulkanUtility.h"

namespace bs
{
	SPtr<RenderTexture> VulkanTextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		VulkanRenderTexture* tex = new (bs_alloc<VulkanRenderTexture>()) VulkanRenderTexture(desc);

		return bs_core_ptr<VulkanRenderTexture>(tex);
	}

	PixelFormat VulkanTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		PixelUtil::checkFormat(format, ttype, usage);

		if (ct::VulkanUtility::getPixelFormat(format, hwGamma) == VK_FORMAT_UNDEFINED)
			return PF_R8G8B8A8;

		return format;
	}

	namespace ct
	{
	void VulkanTextureManager::onStartUp()
	{
		TextureManager::onStartUp();

		SPtr<PixelData> whitePixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		whitePixelData->setColorAt(Color::White, 0, 0);
		whitePixelData->setColorAt(Color::White, 0, 1);
		whitePixelData->setColorAt(Color::White, 1, 0);
		whitePixelData->setColorAt(Color::White, 1, 1);

		TEXTURE_DESC desc;
		desc.type = TEX_TYPE_2D;
		desc.width = 2;
		desc.height = 2;
		desc.format = PF_R8G8B8A8;
		desc.usage = TU_STATIC;

		// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
		mDummyReadTexture = std::static_pointer_cast<VulkanTexture>(createTexture(desc));
		mDummyReadTexture->writeData(*whitePixelData);

		desc.usage = TU_LOADSTORE;

		mDummyStorageTexture = std::static_pointer_cast<VulkanTexture>(createTexture(desc));
	}

	VkImageView VulkanTextureManager::getDummyReadImageView(UINT32 deviceIdx) const
	{
		return mDummyReadTexture->getResource(deviceIdx)->getView(false);
	}

	VkImageView VulkanTextureManager::getDummyStorageImageView(UINT32 deviceIdx) const
	{
		return mDummyStorageTexture->getResource(deviceIdx)->getView(false);
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