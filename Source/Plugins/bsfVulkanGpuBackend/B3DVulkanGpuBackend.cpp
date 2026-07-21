//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "Managers/B3DVulkanTextureManager.h"
#include "Managers/B3DVulkanRenderWindowManager.h"
#include "Managers/B3DVulkanVertexInputManager.h"

#include <vulkan/vulkan.h>

#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuProgram.h"
#include "B3DVulkanRenderPass.h"
#include "CoreObject/B3DRenderThread.h"
#include "Utility/B3DCommandLine.h"
#include "Utility/B3DConfigVariable.h"
#if B3D_PLATFORM_WIN32
#	include "Win32/B3DRenderDocFrameCapture.h"
#endif

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#elif B3D_PLATFORM_LINUX
#	include "Private/Linux/B3DLinuxVideoModeInfo.h"
#elif B3D_PLATFORM_MACOS
#	include "MacOS/B3DMacOSVideoModeInfo.h"
#	include <MoltenVK/vk_mvk_moltenvk.h>
#else
static_assert(false, "Other platform includes go here.");
#endif

#if !B3D_PLATFORM_MACOS
#	define B3D_BUILD_WITH_VULKAN_VALIDATION_LAYERS 1
#else
#	define B3D_BUILD_WITH_VULKAN_VALIDATION_LAYERS 0
#endif

namespace b3d {
VkAllocationCallbacks* gVulkanAllocator = nullptr;

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;

PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;

PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;

PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR = nullptr;
PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR = nullptr;

PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirementsKHR = nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
// Diagnostics (VK_KHR_pipeline_executable_properties): used by the optional gpu.DumpPipelineStats occupancy dump.
PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR = nullptr;
PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR = nullptr;
#endif

/** Enables Vulkan validation layers. Ignored if the backend or platform does not support them. */
static const bool kEnableVulkanValidationLayers = B3D_DEBUG;

/** Enabled Vulkan debug labels for objects. */
static const bool kEnableVulkanDebugLabels = B3D_DEBUG;

} // namespace b3d

using namespace b3d;
using namespace b3d::render;

/** Converts a Vulkan object type into its string representation. */
static const char* GetVulkanObjectTypeName(VkObjectType objectType)
{
#define EMIT_CASE_FOR_OBJECT_TYPE(x) \
	case VK_OBJECT_TYPE_##x: return #x;

	switch(objectType)
	{
		EMIT_CASE_FOR_OBJECT_TYPE(BUFFER)
		EMIT_CASE_FOR_OBJECT_TYPE(BUFFER_VIEW)
		EMIT_CASE_FOR_OBJECT_TYPE(COMMAND_BUFFER)
		EMIT_CASE_FOR_OBJECT_TYPE(COMMAND_POOL)
		EMIT_CASE_FOR_OBJECT_TYPE(DESCRIPTOR_POOL)
		EMIT_CASE_FOR_OBJECT_TYPE(DESCRIPTOR_SET)
		EMIT_CASE_FOR_OBJECT_TYPE(DESCRIPTOR_SET_LAYOUT)
		EMIT_CASE_FOR_OBJECT_TYPE(DESCRIPTOR_UPDATE_TEMPLATE)
		EMIT_CASE_FOR_OBJECT_TYPE(DEVICE)
		EMIT_CASE_FOR_OBJECT_TYPE(DEVICE_MEMORY)
		EMIT_CASE_FOR_OBJECT_TYPE(DISPLAY_KHR)
		EMIT_CASE_FOR_OBJECT_TYPE(DISPLAY_MODE_KHR)
		EMIT_CASE_FOR_OBJECT_TYPE(EVENT)
		EMIT_CASE_FOR_OBJECT_TYPE(FENCE)
		EMIT_CASE_FOR_OBJECT_TYPE(FRAMEBUFFER)
		EMIT_CASE_FOR_OBJECT_TYPE(IMAGE)
		EMIT_CASE_FOR_OBJECT_TYPE(IMAGE_VIEW)
		EMIT_CASE_FOR_OBJECT_TYPE(PHYSICAL_DEVICE)
		EMIT_CASE_FOR_OBJECT_TYPE(PIPELINE)
		EMIT_CASE_FOR_OBJECT_TYPE(PIPELINE_CACHE)
		EMIT_CASE_FOR_OBJECT_TYPE(PIPELINE_LAYOUT)
		EMIT_CASE_FOR_OBJECT_TYPE(QUERY_POOL)
		EMIT_CASE_FOR_OBJECT_TYPE(QUEUE)
		EMIT_CASE_FOR_OBJECT_TYPE(RENDER_PASS)
		EMIT_CASE_FOR_OBJECT_TYPE(SAMPLER)
		EMIT_CASE_FOR_OBJECT_TYPE(SAMPLER_YCBCR_CONVERSION)
		EMIT_CASE_FOR_OBJECT_TYPE(SEMAPHORE)
		EMIT_CASE_FOR_OBJECT_TYPE(SHADER_MODULE)
		EMIT_CASE_FOR_OBJECT_TYPE(SURFACE_KHR)
		EMIT_CASE_FOR_OBJECT_TYPE(SWAPCHAIN_KHR)
	default: break;
	}
#undef EMIT_CASE_FOR_OBJECT_TYPE

	return "Unknown type";
}

/** Callback triggered when using the VK_EXT_debug_report debugging extension. */
static VkBool32 DebugReportMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
	// Determine prefix
	const char* severity;
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		severity = "ERROR";
	else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		severity = "WARNING";
	else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		severity = "PERFORMANCE";
	else if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		severity = "INFO";
	else if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		severity = "DEBUG";

	const String message = StringUtility::Format("[{0}] Vulkan backend reported the following message (Code:{1} Layer:\"{2}\"):\n\t{3}", severity, msgCode, pLayerPrefix, pMsg);

	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		B3D_LOG(Error, LogRenderBackend, "{0}", message);
	else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		B3D_LOG(Warning, LogRenderBackend, "{0}", message);
	else
		B3D_LOG(Info, LogRenderBackend, "{0}", message);

	// Don't abort calls that caused a validation message
	return VK_FALSE;
}

/** Callback triggered when using the VK_EXT_debug_utils debugging extension. */
VkBool32 DebugUtilsMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
	const char* severity;
	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		severity = "ERROR";
	else if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		severity = "WARNING";
	else if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		severity = "INFO";
	else
		severity = "VERBOSE";

	const char* type;
	if((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0)
		type = "VALIDATION";
	else if((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0)
		type = "PERFORMANCE";
	else
		type = "GENERAL";

	StringStream message;
	message << StringUtility::Format("[{0}, {1}] Vulkan backend reported the following message (Name:\"{2}\" ID:{3}):\n\t{4}", severity, type, callbackData->pMessageIdName, callbackData->messageIdNumber, callbackData->pMessage);

	if(callbackData->objectCount > 0)
	{
		message << StringUtility::Format("\n\n\tAssociated objects (Count:{0}):", callbackData->objectCount);
		for(uint32_t objectIndex = 0; objectIndex < callbackData->objectCount; ++objectIndex)
		{
			const VkDebugUtilsObjectNameInfoEXT& objectInformation = callbackData->pObjects[objectIndex];
			message << StringUtility::Format("\n\t\t#{0}: Type:{1} Name:\"{2}\" Handle:\"{3}\"", objectIndex, objectInformation.pObjectName, GetVulkanObjectTypeName(objectInformation.objectType), (u32)objectInformation.objectHandle);
		}
	}

	if(callbackData->cmdBufLabelCount > 0)
	{
		message << StringUtility::Format("\n\n\tAssociated command buffer labels (Count:{0}):", callbackData->cmdBufLabelCount);
		for(uint32_t labelIndex = 0; labelIndex < callbackData->cmdBufLabelCount; ++labelIndex)
		{
			const VkDebugUtilsLabelEXT& commandBufferLabel = callbackData->pCmdBufLabels[labelIndex];
			message << StringUtility::Format("\n\t\t#{0}: Name:\"{1}\"", labelIndex, commandBufferLabel.pLabelName);
		}
	}

	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		B3D_LOG(Error, LogRenderBackend, "{0}", message.str());
	else if(messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		B3D_LOG(Warning, LogRenderBackend, "{0}", message.str());
	else
		B3D_LOG(Info, LogRenderBackend, "{0}", message.str());

	// Don't abort calls that caused a validation message
	return VK_FALSE;
}

void VulkanGpuBackend::OnStartUp()
{
	// Create instance
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "B3D Framework app";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "B3D Framework";
	appInfo.engineVersion = (B3D_FRAMEWORK_VERSION_MAJOR << 24) | (B3D_FRAMEWORK_VERSION_MINOR << 16) | B3D_FRAMEWORK_VERSION_PATCH;
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// Check supported extensions
	bool isDebugUtilsExtensionSupported = false;

	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	if(availableExtensionCount > 0)
	{
		FrameAllocatorScope frameScope;
		FrameVector<VkExtensionProperties> availableExtensions(availableExtensionCount);

		if(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data()) == VK_SUCCESS)
		{
			for(const auto& extensionEntry : availableExtensions)
			{
				if(strcmp(extensionEntry.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
				{
					isDebugUtilsExtensionSupported = true;
				}
			}
		}
	}

	const bool isVulkanValidationEnabled = B3D_BUILD_WITH_VULKAN_VALIDATION_LAYERS && kEnableVulkanValidationLayers;
	const u32 layerCount = isVulkanValidationEnabled ? 1 : 0;
	const char* layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	u32 extensionCount = 2; // Two surface extensions are always enabled
	const char* extensions[] = {
		nullptr, /** Surface extension */
		nullptr, /** OS specific surface extension */
		nullptr, /** Debugging extension */
	};

	extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#if B3D_PLATFORM_WIN32
	extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM_ANDROID
	extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM_LINUX
	extensions[1] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM_MACOS
	extensions[1] = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#else
	static_assert(false, "Other platform includes go here.");
#endif

	if(isVulkanValidationEnabled || kEnableVulkanDebugLabels)
	{
		if(isDebugUtilsExtensionSupported)
			extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		else
			extensions[2] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

		extensionCount++;
	}

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = layerCount;
	instanceInfo.ppEnabledLayerNames = layers;
	instanceInfo.enabledExtensionCount = extensionCount;
	instanceInfo.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateInstance(&instanceInfo, gVulkanAllocator, &mInstance);
	B3D_ASSERT(result == VK_SUCCESS);

	// Set up debugging
	if(isVulkanValidationEnabled)
	{
		if(isDebugUtilsExtensionSupported)
		{
			vkCreateDebugUtilsMessengerEXT = GET_INSTANCE_PROC_ADDR(mInstance, CreateDebugUtilsMessengerEXT);
			vkDestroyDebugUtilsMessengerEXT = GET_INSTANCE_PROC_ADDR(mInstance, DestroyDebugUtilsMessengerEXT);

			VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.pNext = nullptr;
			debugMessengerCreateInfo.flags = 0;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = DebugUtilsMessageCallback;
			debugMessengerCreateInfo.pUserData = nullptr;

			result = vkCreateDebugUtilsMessengerEXT(mInstance, &debugMessengerCreateInfo, nullptr, &mDebugUtilsMessenger);
			B3D_ASSERT(result == VK_SUCCESS);
		}
		else // Use the older report extension
		{
			VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

			GET_INSTANCE_PROC_ADDR(mInstance, CreateDebugReportCallbackEXT)
			GET_INSTANCE_PROC_ADDR(mInstance, DestroyDebugReportCallbackEXT)

			VkDebugReportCallbackCreateInfoEXT debugInfo;
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			debugInfo.pNext = nullptr;
			debugInfo.flags = 0;
			debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugReportMessageCallback;
			debugInfo.flags = debugFlags;

			result = vkCreateDebugReportCallbackEXT(mInstance, &debugInfo, nullptr, &mDebugReportCallback);
			B3D_ASSERT(result == VK_SUCCESS);
		}
	}

	if(kEnableVulkanDebugLabels && isDebugUtilsExtensionSupported)
	{
		vkCmdBeginDebugUtilsLabelEXT = GET_INSTANCE_PROC_ADDR(mInstance, CmdBeginDebugUtilsLabelEXT)
		vkCmdEndDebugUtilsLabelEXT = GET_INSTANCE_PROC_ADDR(mInstance, CmdEndDebugUtilsLabelEXT)
		vkCmdInsertDebugUtilsLabelEXT = GET_INSTANCE_PROC_ADDR(mInstance, CmdInsertDebugUtilsLabelEXT)
		vkSetDebugUtilsObjectNameEXT = GET_INSTANCE_PROC_ADDR(mInstance, SetDebugUtilsObjectNameEXT)
	}

#if B3D_PLATFORM_MACOS
	MVKConfiguration mvkConfig;
	size_t mvkConfigSize = sizeof(MVKConfiguration);
	vkGetMoltenVKConfigurationMVK(mInstance, &mvkConfig, &mvkConfigSize);

#	if B3D_DEBUG
	mvkConfig.debugMode = VK_TRUE;
#	endif

	vkSetMoltenVKConfigurationMVK(mInstance, &mvkConfig, &mvkConfigSize);
#endif

	// Enumerate all devices
	u32 physicalDeviceCount = 0;
	result = vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
	B3D_ASSERT(result == VK_SUCCESS);

	Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());
	B3D_ASSERT(result == VK_SUCCESS);

	// Find primary device
	uint32_t primaryDeviceIndex = ~0u;

	if(gGpuPreferredDeviceIndex >= 0 && gGpuPreferredDeviceIndex < (i32)physicalDeviceCount)
		primaryDeviceIndex = (u32)gGpuPreferredDeviceIndex;

	if(primaryDeviceIndex == ~0u)
	{
		for(uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevices[deviceIndex], &deviceProperties);

			const bool isPrimary = gGpuPreferIntegrated ? deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU : deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

			if(isPrimary)
			{
				primaryDeviceIndex = deviceIndex;
				break;
			}
		}

		if(primaryDeviceIndex == ~0u)
			primaryDeviceIndex = 0;
	}

	// For now always initialize a single device, as otherwise we run into problems with RenderDoc
	mDevices.resize(1);

	mDevices[0] = B3DMakeShared<VulkanGpuDevice>(physicalDevices[primaryDeviceIndex]);
	mDevices[0]->SetIsPrimary();

	mPresentDevice = mDevices[0];

	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = std::min(5U, (u32)mDevices.size());

	for(u32 i = 0; i < gpuInfo.NumGpUs; i++)
		gpuInfo.Names[i] = mDevices[i]->GetDeviceProperties().deviceName;

	PlatformUtility::SetGPUInfo(gpuInfo);

	// Get required extension functions
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceSupportKHR)
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceFormatsKHR)
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR)
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfacePresentModesKHR)

	VkDevice presentDevice = GetPresentDevice()->GetLogical();
	GET_DEVICE_PROC_ADDR(presentDevice, CreateSwapchainKHR)
	GET_DEVICE_PROC_ADDR(presentDevice, DestroySwapchainKHR)
	GET_DEVICE_PROC_ADDR(presentDevice, GetSwapchainImagesKHR)
	GET_DEVICE_PROC_ADDR(presentDevice, AcquireNextImageKHR)
	GET_DEVICE_PROC_ADDR(presentDevice, QueuePresentKHR)

	// Create the render pass manager
	VulkanRenderPassCache::StartUp();
	VulkanFramebufferCache::StartUp();

	// Create the texture manager for use by others. Must come after device creation (which starts the
	// submit thread): its startup uploads the built-in/dummy textures through a worker GpuWorkContext,
	// whose teardown submits the recorded transfers and waits for them on the GPU queue.
	TextureManager::StartUp<VulkanTextureManager>();
	render::TextureManager::StartUp<render::VulkanTextureManager>(*mDevices[0]);

	// Create render window manager
	RenderWindowManager::StartUp<VulkanRenderWindowManager>();

	// Create vertex input manager
	VulkanVertexInputManager::StartUp();

#if B3D_PLATFORM_WIN32
	mFrameCapture = B3DMakeShared<RenderDocFrameCapture>(mInstance); // TODO - This should included in the build for development only, but it's currently always bundled with the application
#endif

	Super::OnStartUp();
}

void VulkanGpuBackend::OnShutDown()
{
	for (const auto& device : mDevices)
	{
		if (!device->IsInitialized())
			continue;

		device->WaitUntilIdle();
	}

	VulkanVertexInputManager::ShutDown();
	RenderWindowManager::ShutDown();
	VulkanFramebufferCache::ShutDown();
	VulkanRenderPassCache::ShutDown();
	render::TextureManager::ShutDown();
	TextureManager::ShutDown();

	mPresentDevice = nullptr;
	mDevices.clear();

	if (mDebugReportCallback != nullptr)
		vkDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, gVulkanAllocator);

	if (mDebugUtilsMessenger != nullptr)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, gVulkanAllocator);

	vkDestroyInstance(mInstance, gVulkanAllocator);

	Super::OnShutDown();
}

namespace b3d {
VulkanGpuBackend& GetVulkanGpuBackend()
{
	return static_cast<VulkanGpuBackend&>(VulkanGpuBackend::Instance());
}
} // namespace b3d
