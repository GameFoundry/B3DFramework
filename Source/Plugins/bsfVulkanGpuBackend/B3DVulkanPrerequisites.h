//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#if B3D_PLATFORM_WIN32
#	define VK_USE_PLATFORM_WIN32_KHR
#elif B3D_PLATFORM_LINUX
#	define VK_USE_PLATFORM_XLIB_KHR
#elif B3D_PLATFORM_ANDROID
#	define VK_USE_PLATFORM_ANDROID_KHR
#elif B3D_PLATFORM_MACOS
#	define VK_USE_PLATFORM_MACOS_MVK
#endif

#include "vulkan/vulkan.h"
#undef MemoryBarrier // Conflicting define from winnt.h
#undef None // Conflicting define from Xlib

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup Vulkan Vulkan
 *	Wrapper around the Vulkan render API.
 */

/** @} */

namespace b3d
{
	extern VkAllocationCallbacks* gVulkanAllocator;

	namespace render
	{
		class Win32RenderWindow;
		class VulkanTexture;
		class Win32VideoMode;
		class VulkanGpuBuffer;
		class VulkanGpuDevice;
		class VulkanSwapChain;
		class VulkanFramebuffer;
		class VulkanDescriptorLayout;
		class VulkanDescriptorSet;
		class VulkanDescriptorManager;
		class VulkanGpuCommandBufferPool;
		class VulkanGpuCommandBuffer;
		class VulkanGpuQueue;
		class VulkanResourceManager;
		class VulkanBuffer;
		class VulkanImage;
		class VulkanGpuParameterSet;
		class VulkanEvent;
		class VulkanVertexInput;
		class VulkanSemaphore;

		/**	Vulkan specific types to track resource statistics for. */
		enum VulkanRenderStatResourceType
		{
			RenderStatObject_PipelineState = 100
		};

		/** Contains lists of images and buffers that require pipeline barrier transitions. */
		struct TransitionInfo
		{
			TArray<VkImageMemoryBarrier> ImageBarriers;
			TArray<VkBufferMemoryBarrier> BufferBarriers;
		};
	} // namespace render
} // namespace b3d

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk" #name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk" #name));
