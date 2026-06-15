//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "B3DVulkanGpuDevice.h"

namespace b3d
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Handles initialization and shutdown of Vulkan GPU backend, and provides access to GPU device objects. */
	class VulkanGpuBackend : public GpuBackend
	{
		using Super = GpuBackend;
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		u32 GetDeviceCount() const override { return (u32)mDevices.size(); }
		TShared<GpuDevice> GetDevice(u32 index) const override
		{
			B3D_ASSERT(index < mDevices.size());
			return mDevices[index];
		}

		/** Returns the internal Vulkan instance object. */
		VkInstance GetVkInstance() const { return mInstance; }

		/** Returns a Vulkan device at the specified index. Must be in range [0, GetDeviceCount()) */
		const TShared<render::VulkanGpuDevice>& GetVulkanDevice(u32 index) const
		{
			B3D_ASSERT(index < mDevices.size());
			return mDevices[index];
		}

		/** Returns the primary device that supports swap chain present operations. */
		const TShared<render::VulkanGpuDevice>& GetPresentDevice() const { return mPresentDevice; }
	private:
		VkInstance mInstance = nullptr;

		TInlineArray<TShared<render::VulkanGpuDevice>, 2> mDevices;
		TShared<render::VulkanGpuDevice> mPresentDevice;

		VkDebugReportCallbackEXT mDebugReportCallback = nullptr;
		VkDebugUtilsMessengerEXT mDebugUtilsMessenger = nullptr;
	};

	/**	Provides easy access to the VulkanGpuBackend. */
	B3D_EXPORT VulkanGpuBackend& GetVulkanGpuBackend();

	extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	extern PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
	extern PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

	extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

	extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	extern PFN_vkQueuePresentKHR vkQueuePresentKHR;

	extern PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR;
	extern PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;

	extern PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirementsKHR;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Diagnostics (VK_KHR_pipeline_executable_properties): used by the optional gpu.DumpPipelineStats occupancy dump.
	extern PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR;
	extern PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR;
#endif

	/** @} */
} // namespace b3d
