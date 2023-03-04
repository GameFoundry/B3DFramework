//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuBackend.h"
#include "BsVulkanDevice.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	// TODO - Doc
	class VulkanGpuBackend : public GpuBackend
	{
		using Super = GpuBackend;
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		u32 GetDeviceCount() const override { return (u32)mDevices.size(); }
		SPtr<GpuDevice> GetDevice(u32 index) const override { return mDevices[index]; }

		/** Returns the internal Vulkan instance object. */
		VkInstance GetVkInstance() const { return mInstance; }

		/** Returns a Vulkan device at the specified index. Must be in range [0, GetDeviceCount()) */
		const SPtr<ct::VulkanDevice>& GetVulkanDevice(u32 index) const { return mDevices[index]; }

		/** Returns the primary device that supports swap chain present operations. */
		const SPtr<ct::VulkanDevice>& GetPresentDevice() const { return mPresentDevice; }
	private:
		VkInstance mInstance = nullptr;

		SmallVector<SPtr<ct::VulkanDevice>, 2> mDevices;
		SPtr<ct::VulkanDevice> mPresentDevice;

		ct::VulkanGLSLProgramFactory* mGLSLFactory = nullptr;

		VkDebugReportCallbackEXT mDebugReportCallback = nullptr;
		VkDebugUtilsMessengerEXT mDebugUtilsMessenger = nullptr;
	};

	/**	Provides easy access to the VulkanGpuBackend. */
	VulkanGpuBackend& GetVulkanGpuBackend();

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

	/** @} */
} // namespace bs
