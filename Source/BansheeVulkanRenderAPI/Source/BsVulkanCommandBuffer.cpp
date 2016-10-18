//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	VulkanCommandBuffer::VulkanCommandBuffer(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask, bool secondary)
		: CommandBuffer(type, syncMask, secondary)
	{ }
}