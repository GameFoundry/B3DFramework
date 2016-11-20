//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderTexture.h"

namespace bs
{
	VulkanRenderTextureCore::VulkanRenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask)
		:RenderTextureCore(desc, deviceMask), mProperties(desc, false)
	{
		
	}

	void VulkanRenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "FB")
		{
			VkFramebuffer* fb = (VkFramebuffer*)data;
			// TODO - Assign framebuffer
			return;
		}

		if (name == "RP")
		{
			VkRenderPass* renderPass = (VkRenderPass*)data;
			// TODO - Assign render pass
			return;
		}
	}

	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{ 

	}
}