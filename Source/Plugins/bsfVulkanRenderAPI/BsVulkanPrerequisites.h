//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
#	define VK_USE_PLATFORM_WIN32_KHR
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
#	define VK_USE_PLATFORM_XLIB_KHR
#elif B3D_PLATFORM == B3D_PLATFORM_ID_ANDROID
#	define VK_USE_PLATFORM_ANDROID_KHR
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	define VK_USE_PLATFORM_MACOS_MVK
#endif

/** Maximum number of GPU queues that may exist at once. */
#define BS_MAX_UNIQUE_QUEUES BS_MAX_QUEUES_PER_TYPE* bs::GQT_COUNT // Must fit within 4 bytes

#include "vulkan/vulkan.h"
#undef MemoryBarrier // Conflicting define from winnt.h
#undef None // Conflicting define from Xlib

#include "ThirdParty/vk_mem_alloc.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup Vulkan bsfVulkanRenderAPI
 *	Wrapper around the Vulkan render API.
 */

/** @} */

namespace bs
{
	extern VkAllocationCallbacks* gVulkanAllocator;

	namespace ct
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
		class VulkanInternalCommandBuffer;
		class VulkanGpuCommandBuffer;
		class VulkanGpuQueue;
		class VulkanResourceManager;
		class VulkanBuffer;
		class VulkanImage;
		class VulkanDescriptorPool;
		class VulkanGpuParameters;
		class VulkanEvent;
		class VulkanQuery;
		class VulkanQueryPool;
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
			Vector<VkImageMemoryBarrier> ImageBarriers;
			Vector<VkBufferMemoryBarrier> BufferBarriers;
		};

		/** Contains the row pitch and slice height for an image subresource. */
		struct ImageSubresourcePitch
		{
			ImageSubresourcePitch(u32 rowPitch = 0, u32 sliceHeight = 0)
				: RowPitch(rowPitch), SliceHeight(sliceHeight)
			{ }

			/** Number of blocks before advancing to the next row. For non-compressed formats this is equal to the number of pixels. For compressed it depends on the block size. */
			u32 RowPitch = 0;

			/** Number of block columns before advancing to the next slice. For non-compressed formats this is equal to the number of pixels. For compressed it depends on the block size. */
			u32 SliceHeight = 0;
		};
	} // namespace ct
} // namespace bs

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk" #name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk" #name));
