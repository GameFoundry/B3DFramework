//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#if BS_PLATFORM == BS_PLATFORM_WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "vulkan/vulkan.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup Vulkan BansheeVulkanRenderAPI
 *	Wrapper around the Vulkan render API.
 */

/** @} */

namespace BansheeEngine
{
	class VulkanRenderAPI;
	class Win32RenderWindow;
	class VulkanTextureCore;
	class Win32VideoMode;
	class VulkanIndexBuffer;
	class VulkanVertexDeclaration;
	class VulkanHardwareBuffer;
	class VulkanDevice;
	class VulkanGLSLProgramFactory;
	class VulkanSwapChain;
	class VulkanDescriptorLayout;
	class VulkanDescriptorSet;
	class VulkanDescriptorManager;
	class VulkanCmdBufferPool;
	class VulkanCmdBuffer;
	class VulkanCommandBuffer;
	class VulkanQueue;
	class VulkanResourceManager;
	class VulkanGpuParamBlockBufferCore;
	class VulkanBuffer;
	class VulkanDescriptorPool;
	class VulkanGpuParams;

	VkAllocationCallbacks* gVulkanAllocator = nullptr;

	/**	Vulkan specific types to track resource statistics for. */
	enum VulkanRenderStatResourceType
	{
		RenderStatObject_PipelineState = 100
	};

	/** Contains lists of images and buffers that require pipeline barrier transitions. */
	struct TransitionInfo
	{
		Vector<VkImageMemoryBarrier> imageBarriers;
		Vector<VkBufferMemoryBarrier> bufferBarriers;
	};
}

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));