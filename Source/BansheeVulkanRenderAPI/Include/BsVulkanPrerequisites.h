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
	class VulkanTexture;
	class Win32VideoMode;
	class VulkanIndexBuffer;
	class VulkanVertexDeclaration;
	class VulkanHardwareBuffer;
	class VulkanDevice;
	class VulkanGLSLProgramFactory;
	class VulkanSwapChain;

	VkAllocationCallbacks* gVulkanAllocator = nullptr;

	/**	Vulkan specific types to track resource statistics for. */
	enum VulkanRenderStatResourceType
	{
		RenderStatObject_PipelineState = 100
	};

	/** Types of GPU queues. */
	enum VulkanQueueType
	{
		/**
		 * Queue used for rendering. Allows the use of draw commands, but also all commands supported by compute
		 * or upload buffers.
		 */
		VQT_GRAPHICS,
		/** Discrete queue used for compute operations. Allows the use of dispatch and upload commands. */
		VQT_COMPUTE,
		/** Queue used for memory transfer operations only. No rendering or compute dispatch allowed. */
		VQT_UPLOAD,
		VQT_COUNT // Keep at end
	};
}

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));

/** Maximum number of hardware devices usable at once. */
#define BS_MAX_VULKAN_DEVICES 5