//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTextureManager.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderTexture.h"
#include "BsVulkanResource.h"

namespace bs
{
	SPtr<RenderTexture> VulkanTextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		VulkanRenderTexture* tex = new (bs_alloc<VulkanRenderTexture>()) VulkanRenderTexture(desc);

		return bs_core_ptr<VulkanRenderTexture>(tex);
	}

	PixelFormat VulkanTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		return PF_R8G8B8A8;;
	}

	void VulkanTextureCoreManager::onStartUp()
	{
		TextureCoreManager::onStartUp();

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
		mDummyReadTexture = std::static_pointer_cast<VulkanTextureCore>(createTexture(desc));
		mDummyReadTexture->writeData(*whitePixelData);

		desc.usage = TU_LOADSTORE;

		mDummyStorageTexture = std::static_pointer_cast<VulkanTextureCore>(createTexture(desc));
	}

	VkImageView VulkanTextureCoreManager::getDummyReadImageView(UINT32 deviceIdx) const
	{
		return mDummyReadTexture->getResource(deviceIdx)->getView(false);
	}

	VkImageView VulkanTextureCoreManager::getDummyStorageImageView(UINT32 deviceIdx) const
	{
		return mDummyStorageTexture->getResource(deviceIdx)->getView(false);
	}

	SPtr<TextureCore> VulkanTextureCoreManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		VulkanTextureCore* tex = new (bs_alloc<VulkanTextureCore>()) VulkanTextureCore(desc, initialData, deviceMask);

		SPtr<VulkanTextureCore> texPtr = bs_shared_ptr<VulkanTextureCore>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> VulkanTextureCoreManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC_CORE& desc,
																				  UINT32 deviceIdx)
	{
		SPtr<VulkanRenderTextureCore> texPtr = bs_shared_ptr_new<VulkanRenderTextureCore>(desc, deviceIdx);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
}