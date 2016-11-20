//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTextureManager.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderTexture.h"

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

	SPtr<TextureCore> VulkanTextureCoreManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		VulkanTextureCore* tex = new (bs_alloc<VulkanTextureCore>()) VulkanTextureCore(desc, initialData, deviceMask);

		SPtr<VulkanTextureCore> texPtr = bs_shared_ptr<VulkanTextureCore>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> VulkanTextureCoreManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC_CORE& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanRenderTextureCore> texPtr = bs_shared_ptr_new<VulkanRenderTextureCore>(desc, deviceMask);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
}