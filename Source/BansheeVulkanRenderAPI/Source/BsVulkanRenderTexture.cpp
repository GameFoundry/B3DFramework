//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderTexture.h"

namespace BansheeEngine
{
	VulkanRenderTextureCore::VulkanRenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask)
		:RenderTextureCore(desc, deviceMask), mProperties(desc, false)
	{
		
	}

	void VulkanRenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		
	}

	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{ 

	}
}