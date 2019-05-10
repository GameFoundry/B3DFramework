//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#if BS_PLATFORM == BS_PLATFORM_WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#elif BS_PLATFORM == BS_PLATFORM_LINUX
	#define VK_USE_PLATFORM_XLIB_KHR
#elif BS_PLATFORM == BS_PLATFORM_ANDROID
	#define VK_USE_PLATFORM_ANDROID_KHR
#elif BS_PLATFORM == BS_PLATFORM_OSX
	#define VK_USE_PLATFORM_MACOS_MVK
#endif

/** Maximum number of GPU queues that may exist at once. */
#define BS_MAX_UNIQUE_QUEUES BS_MAX_QUEUES_PER_TYPE * bs::GQT_COUNT // Must fit within 4 bytes

#include "vulkan/vulkan.h"
#undef None // Conflicting define from Xlib

#include "ThirdParty/vk_mem_alloc.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup Vulkan bsfVulkanRenderAPI
 *	Wrapper around the Vulkan render API.
 */

/** @} */

namespace bs { namespace ct
{
	class VulkanRenderAPI;
	class Win32RenderWindow;
	class VulkanTexture;
	class Win32VideoMode;
	class VulkanVertexDeclaration;
	class VulkanHardwareBuffer;
	class VulkanDevice;
	class VulkanGLSLProgramFactory;
	class VulkanSwapChain;
	class VulkanFramebuffer;
	class VulkanDescriptorLayout;
	class VulkanDescriptorSet;
	class VulkanDescriptorManager;
	class VulkanCmdBufferPool;
	class VulkanCmdBuffer;
	class VulkanCommandBuffer;
	class VulkanQueue;
	class VulkanResourceManager;
	class VulkanGpuParamBlockBuffer;
	class VulkanBuffer;
	class VulkanImage;
	class VulkanDescriptorPool;
	class VulkanGpuParams;
	class VulkanTransferBuffer;
	class VulkanEvent;
	class VulkanQuery;
	class VulkanQueryPool;
	class VulkanVertexInput;
	class VulkanSemaphore;

	extern VkAllocationCallbacks* gVulkanAllocator;

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

	/** Bits that map to a specific part of a render target and signify whether it should be cleared or not. */
	enum ClearMaskBits
	{
		CLEAR_NONE = 0,
		CLEAR_COLOR0 = 1 << 0,
		CLEAR_COLOR1 = 1 << 1,
		CLEAR_COLOR2 = 1 << 2,
		CLEAR_COLOR3 = 1 << 3,
		CLEAR_COLOR4 = 1 << 4,
		CLEAR_COLOR5 = 1 << 5,
		CLEAR_COLOR6 = 1 << 6,
		CLEAR_COLOR7 = 1 << 7,
		CLEAR_STENCIL = 1 << 30,
		CLEAR_DEPTH = 1 << 31,
		CLEAR_ALL = 0xFF
	};

	typedef Flags<ClearMaskBits> ClearMask;
	BS_FLAGS_OPERATORS(ClearMaskBits);
}}

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));
