//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderAPI.h"
#include "CoreThread/BsCoreThread.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanTextureManager.h"
#include "Managers/BsVulkanRenderWindowManager.h"
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "Managers/BsVulkanRenderStateManager.h"
#include "Managers/BsGpuProgramManager.h"
#include "Managers/BsVulkanQueryManager.h"
#include "Managers/BsVulkanGLSLProgramFactory.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanGpuParams.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanGpuParamBlockBuffer.h"

#include <vulkan/vulkan.h>

#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderPass.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"
#include "Win32/BsWin32RenderWindow.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
#	include "Win32/BsWin32VideoModeInfo.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
#	include "Linux/BsLinuxVideoModeInfo.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	include "MacOS/BsMacOSVideoModeInfo.h"
#	include <MoltenVK/vk_mvk_moltenvk.h>
#else
static_assert(false, "Other platform includes go here.");
#endif

#if B3D_PLATFORM != B3D_PLATFORM_ID_MACOS
#	define B3D_BUILD_WITH_VULKAN_VALIDATION_LAYERS 1
#else
#	define B3D_BUILD_WITH_VULKAN_VALIDATION_LAYERS 0
#endif

namespace bs { namespace ct {
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

/** When enabled the Vulkan backend will prefer an integrated GPU over a discrete one. */
static const bool kVulkanPreferIntegratedGPU = false;

/** Enables Vulkan validation layers. Ignored if the backend or platform does not support them. */
static const bool kEnableVulkanValidationLayers = B3D_DEBUG;

/** Enabled Vulkan debug labels for objects. */
static const bool kEnableVulkanDebugLabels = B3D_DEBUG;

/** If specified, allows you to select which is the primary GPU to use. If ~0u system will pick the best GPU according to other options. */
static const u32 kPreferredGPUIndex = ~0u;

}} // namespace bs::ct

using namespace bs;
using namespace bs::ct;

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

	const String message = StringUtil::Format("[{0}] Vulkan backend reported the following message (Code:{1} Layer:\"{2}\"):\n\t{3}", severity, msgCode, pLayerPrefix, pMsg);

	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		B3D_LOG(Error, RenderBackend, message);
	else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		B3D_LOG(Warning, RenderBackend, message);
	else
		B3D_LOG(Info, RenderBackend, message);

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
	message << StringUtil::Format("[{0}, {1}] Vulkan backend reported the following message (Name:\"{2}\" ID:{3}):\n\t{4}", severity, type, callbackData->pMessageIdName, callbackData->messageIdNumber, callbackData->pMessage);

	if(callbackData->objectCount > 0)
	{
		message << StringUtil::Format("\n\n\tAssociated objects (Count:{0}):", callbackData->objectCount);
		for(uint32_t objectIndex = 0; objectIndex < callbackData->objectCount; ++objectIndex)
		{
			const VkDebugUtilsObjectNameInfoEXT& objectInformation = callbackData->pObjects[objectIndex];
			message << StringUtil::Format("\n\t\t#{0}: Type:{1} Name:\"{2}\" Handle:\"{3}\"", objectIndex, objectInformation.pObjectName, GetVulkanObjectTypeName(objectInformation.objectType), (u32)objectInformation.objectHandle);
		}
	}

	if(callbackData->cmdBufLabelCount > 0)
	{
		message << StringUtil::Format("\n\n\tAssociated command buffer labels (Count:{0}):", callbackData->cmdBufLabelCount);
		for(uint32_t labelIndex = 0; labelIndex < callbackData->cmdBufLabelCount; ++labelIndex)
		{
			const VkDebugUtilsLabelEXT& commandBufferLabel = callbackData->pCmdBufLabels[labelIndex];
			message << StringUtil::Format("\n\t\t#{0}: Name:\"{1}\"", labelIndex, commandBufferLabel.pLabelName);
		}
	}

	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		B3D_LOG(Error, RenderBackend, message.str());
	else if(messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		B3D_LOG(Warning, RenderBackend, message.str());
	else
		B3D_LOG(Info, RenderBackend, message.str());

	// Don't abort calls that caused a validation message
	return VK_FALSE;
}

VulkanRenderAPI::VulkanRenderAPI()
{
	mDebugReportCallback = nullptr;
	mDebugUtilsMessenger = nullptr;
}

const StringID& VulkanRenderAPI::GetName() const
{
	static StringID strName("VulkanRenderAPI");
	return strName;
}

void VulkanRenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	// Create instance
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "bs::framework app";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "bs::framework";
	appInfo.engineVersion = (B3D_FRAMEWORK_VERSION_MAJOR << 24) | (B3D_FRAMEWORK_VERSION_MINOR << 16) | B3D_FRAMEWORK_VERSION_PATCH;

	// MoltenVK doesn't support 1.1, but we don't need it since the only feature we use from it right now is SPIR-V 1.3,
	// and that's not relevant for MoltenVK as SPIR-V gets translated to MSL anyway.
#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	appInfo.apiVersion = VK_API_VERSION_1_0;
#else
	appInfo.apiVersion = VK_API_VERSION_1_1;
#endif

	// Check supported extensions
	bool isDebugUtilsExtensionSupported = false;

	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	if(availableExtensionCount > 0)
	{
		FrameScope frameScope;
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

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM == B3D_PLATFORM_ID_ANDROID
	extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	extensions[1] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
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

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
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

	// For now always initialize a single device, as otherwise we run into problems with RenderDoc
	mNumDevices = 1;
	mDevices.resize(mNumDevices);

	for(uint32_t i = 0; i < mNumDevices; i++)
		mDevices[i] = B3DMakeShared<VulkanDevice>(physicalDevices[i], i);

	// Find primary device
	uint32_t primaryDeviceIndex = ~0u;

	if(kPreferredGPUIndex != ~0u && kPreferredGPUIndex < physicalDeviceCount)
		primaryDeviceIndex = kPreferredGPUIndex;

	if(primaryDeviceIndex == ~0u)
	{
		for(uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevices[deviceIndex], &deviceProperties);

			const bool isPrimary = kVulkanPreferIntegratedGPU ? deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU : deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

			if(isPrimary)
			{
				primaryDeviceIndex = deviceIndex;
				break;
			}
		}

		if(primaryDeviceIndex == ~0u)
			primaryDeviceIndex = 0;
	}

	mDevices[0] = B3DMakeShared<VulkanDevice>(physicalDevices[primaryDeviceIndex], 0);
	mDevices[0]->SetIsPrimary();
	mPrimaryDevices.push_back(mDevices[0]);

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	mVideoModeInfo = B3DMakeShared<LinuxVideoModeInfo>();
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	mVideoModeInfo = B3DMakeShared<MacOSVideoModeInfo>();
#else
	static_assert(false, "mVideoModeInfo needs to be created.");
#endif

	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = std::min(5U, mNumDevices);

	for(u32 i = 0; i < gpuInfo.NumGpUs; i++)
		gpuInfo.Names[i] = mDevices[i]->GetDeviceProperties().deviceName;

	PlatformUtility::SetGPUInfoInternal(gpuInfo);

	// Get required extension functions
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfacePresentModesKHR);

	VkDevice presentDevice = GetPresentDevice()->GetLogical();
	GET_DEVICE_PROC_ADDR(presentDevice, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(presentDevice, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(presentDevice, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(presentDevice, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(presentDevice, QueuePresentKHR);

	// Create command buffer manager
	CommandBufferManager::StartUp<VulkanCommandBufferManager>(*this);

	// Create main command buffer
	mMainCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));

	// Create the texture manager for use by others
	bs::TextureManager::StartUp<bs::VulkanTextureManager>();
	TextureManager::StartUp<VulkanTextureManager>();

	// Create the render pass manager
	VulkanRenderPassCache::StartUp();
	VulkanFramebufferCache::StartUp();

	// Create hardware buffer manager
	bs::HardwareBufferManager::StartUp();
	HardwareBufferManager::StartUp<VulkanHardwareBufferManager>();

	// Start the submit thread
	VulkanSubmitThread::StartUp();

	// Create render window manager
	bs::RenderWindowManager::StartUp<bs::VulkanRenderWindowManager>();
	RenderWindowManager::StartUp();

	// Create query manager
	QueryManager::StartUp<VulkanQueryManager>(*this);

	// Create vertex input manager
	VulkanVertexInputManager::StartUp();

	// Create & register GPU program factories
	mGLSLFactory = B3DNew<VulkanGLSLProgramFactory>();

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	GpuProgramManager::Instance().addFactory("mvksl", mGLSLFactory);
#else
	GpuProgramManager::Instance().AddFactory("vksl", mGLSLFactory);
#endif

	// Create render state manager
	RenderStateManager::StartUp<VulkanRenderStateManager>();

	InitCapabilites();

	RenderAPI::Initialize();
}

void VulkanRenderAPI::DestroyCore()
{
	THROW_IF_NOT_CORE_THREAD;

	if(mMainCommandBuffer != nullptr)
	{
		SubmitCommandBuffer(mMainCommandBuffer);
	}
		

	GetVulkanSubmitThread().WaitUntilIdle(true);
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			SPtr<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(*it);
			vulkanCommandBuffer->OnDidComplete();

			it = mSubmittedCommandBuffers.erase(it);
		}
		else
			++it;
	}

	if(mGLSLFactory != nullptr)
	{
		B3DDelete(mGLSLFactory);
		mGLSLFactory = nullptr;
	}

	VulkanSubmitThread::ShutDown();
	VulkanVertexInputManager::ShutDown();
	QueryManager::ShutDown();
	RenderStateManager::ShutDown();
	RenderWindowManager::ShutDown();
	bs::RenderWindowManager::ShutDown();
	HardwareBufferManager::ShutDown();
	bs::HardwareBufferManager::ShutDown();
	VulkanFramebufferCache::ShutDown();
	VulkanRenderPassCache::ShutDown();
	TextureManager::ShutDown();
	bs::TextureManager::ShutDown();

	mMainCommandBuffer = nullptr;

	CommandBufferManager::ShutDown();

	mPrimaryDevices.clear();
	mDevices.clear();

	if(mDebugReportCallback != nullptr)
		vkDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, gVulkanAllocator);

	if(mDebugUtilsMessenger != nullptr)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, gVulkanAllocator);

	vkDestroyInstance(mInstance, gVulkanAllocator);

	RenderAPI::DestroyCore();
}

void VulkanRenderAPI::SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetPipelineState(pipelineState);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetPipelineState(pipelineState);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	u32 globalQueueIdx = CommandSyncMask::GetGlobalQueueIdx(cb->GetType(), cb->GetQueueIdx());

	for(u32 i = 0; i < GPT_COUNT; i++)
	{
		SPtr<GpuParamDesc> paramDesc = gpuParams->GetParamDesc((GpuProgramType)i);
		if(paramDesc == nullptr)
			continue;

		// Flush all param block buffers
		for(auto iter = paramDesc->ParamBlocks.begin(); iter != paramDesc->ParamBlocks.end(); ++iter)
		{
			SPtr<GpuParamBlockBuffer> buffer = gpuParams->GetParamBlockBuffer(iter->second.Set, iter->second.Slot);

			if(buffer != nullptr)
				buffer->FlushToGpu(globalQueueIdx);
		}
	}

	vkCB->SetGpuParams(gpuParams);

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void VulkanRenderAPI::SetViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetNormalizedViewportArea(vp);
}

void VulkanRenderAPI::SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetVertexBuffers(index, buffers, numBuffers);

	B3D_INCREMENT_RENDER_STATISTIC(NumVertexBufferBinds);
}

void VulkanRenderAPI::SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetIndexBuffer(buffer);

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void VulkanRenderAPI::SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetVertexDeclaration(vertexDeclaration);
}

void VulkanRenderAPI::SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetDrawOp(op);
}

void VulkanRenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->Draw(vertexOffset, vertexCount, instanceCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void VulkanRenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->DrawIndexed(startIndex, indexCount, vertexOffset, instanceCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void VulkanRenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void VulkanRenderAPI::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* internalCommandBuffer = vulkanCommandBuffer->GetInternal();

	Rect2I area(left, top, right - left, bottom - top);
	internalCommandBuffer->EnableScissorTest(area);
}

void VulkanRenderAPI::DisableScissorTest(const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* internalCommandBuffer = vulkanCommandBuffer->GetInternal();

	internalCommandBuffer->DisableScissorTest();
}

void VulkanRenderAPI::SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetStencilRef(value);
}

void VulkanRenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->ClearViewport(buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanRenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->ClearRenderTarget(buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanRenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* vkCB = cb->GetInternal();

	vkCB->SetRenderTarget(target, readOnlyFlags, loadMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void VulkanRenderAPI::BeginFrame()
{
	THROW_IF_NOT_CORE_THREAD

	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			SPtr<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(*it);
			vulkanCommandBuffer->OnDidComplete();

			it = mSubmittedCommandBuffers.erase(it);
		}
		else
			++it;
	}
}

void VulkanRenderAPI::EndFrame()
{
	for(const auto& entry : mPrimaryDevices)
	{
		if(entry == nullptr)
			continue;

		// Submit transfer buffers
		VulkanCommandBufferManager& commandBufferManager = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::Instance());
		commandBufferManager.FlushTransferBuffers(entry->GetIndex());

		GetVulkanSubmitThread().QueueRefreshCommandBufferCompletionStates(entry.get());
	}
}

void VulkanRenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD

	if(target == nullptr || !target->GetProperties().IsWindow)
		return;

	// Retrieve the swap chain before command buffer submit, as the submit might internally rebuild the swap chain.
	VulkanSwapChain* swapChain = nullptr;

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	Win32RenderWindow* window = static_cast<Win32RenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	LinuxRenderWindow* window = static_cast<LinuxRenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	MacOSRenderWindow* window = static_cast<MacOSRenderWindow*>(target.get());
#endif

	window->SwapBuffers();
	swapChain = window->GetSwapChain();

	VulkanQueue* const presentQueue = GetPresentDevice()->GetQueue(GQT_GRAPHICS, 0);
	GetVulkanSubmitThread().QueuePresent(*presentQueue, *swapChain, syncMask);

	// Ensure the acquire operation we queued the previous frame has finished. This also means the old image was presented.
	swapChain->WaitUntilFirstImageAcquired();

	GetVulkanSubmitThread().QueueImageAcquire(*swapChain);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

void VulkanRenderAPI::AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
{
	B3D_EXCEPT(NotImplementedException, "Secondary command buffers not implemented");
}

void VulkanRenderAPI::BeginLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* internalCommandBuffer = vulkanCommandBuffer->GetInternal();

	internalCommandBuffer->BeginLabel(name);
}

void VulkanRenderAPI::EndLabel(const SPtr<CommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* internalCommmandBuffer = vulkanCommandBuffer->GetInternal();

	internalCommmandBuffer->EndLabel();
}

void VulkanRenderAPI::InsertLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanCommandBuffer* vulkanCOmmandBuffer = EnsureCommandBuffer(commandBuffer);
	VulkanInternalCommandBuffer* internalCommandBuffer = vulkanCOmmandBuffer->GetInternal();

	internalCommandBuffer->InsertLabel(name);
}

void VulkanRenderAPI::SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	VulkanCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);

	// Submit all transfer buffers first
	VulkanCommandBufferManager& commandBufferManager = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::Instance());
	commandBufferManager.FlushTransferBuffers(vulkanCommandBuffer->GetDeviceIdx());

	vulkanCommandBuffer->Submit(syncMask);

	if(vulkanCommandBuffer == mMainCommandBuffer.get())
	{
		mSubmittedCommandBuffers.push_back(mMainCommandBuffer);
		mMainCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));
	}
	else
	{
		mSubmittedCommandBuffers.push_back(commandBuffer);
	}
}

void VulkanRenderAPI::WaitUntilIdle() const
{
	GetVulkanSubmitThread().WaitUntilIdle();
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			(*it)->OnDidComplete();
			it = mSubmittedCommandBuffers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

SPtr<CommandBuffer> VulkanRenderAPI::GetMainCommandBuffer() const
{
	return mMainCommandBuffer;
}

void VulkanRenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	dest = matrix;

	// Flip Y axis
	dest[1][1] = -dest[1][1];

	// Convert depth range from [-1,1] to [0,1]
	dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
	dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
	dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
	dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
}

GpuParameterBlockInformation VulkanRenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params)
{
	GpuParameterBlockInformation block;
	block.BlockSize = 0;
	block.IsShareable = true;
	block.Name = name;
	block.Slot = 0;
	block.Set = 0;

	for(auto& param : params)
	{
		u32 size;
		if(param.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4
			size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;
		}
		else
			size = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(param.Type, param.ArraySize, block.BlockSize);

		param.ElementSize = size;
		param.ArrayElementStride = size;
		param.CpuMemOffset = block.BlockSize;
		param.GpuMemOffset = 0;
		block.BlockSize += size * param.ArraySize;
		param.ParamBlockSlot = 0;
		param.ParamBlockSet = 0;
	}

	// Constant buffer size must always be a multiple of 16
	if(block.BlockSize % 4 != 0)
		block.BlockSize += (4 - (block.BlockSize % 4));

	return block;
}

void VulkanRenderAPI::InitCapabilites()
{
	mNumDevices = (u32)mDevices.size();
	mCurrentCapabilities = B3DNewMultiple<RenderAPICapabilities>(mNumDevices);

	u32 deviceIdx = 0;
	for(auto& device : mDevices)
	{
		RenderAPICapabilities& caps = mCurrentCapabilities[deviceIdx];

		const VkPhysicalDeviceProperties& deviceProps = device->GetDeviceProperties();
		const VkPhysicalDeviceFeatures& deviceFeatures = device->GetDeviceFeatures();
		const VkPhysicalDeviceLimits& deviceLimits = deviceProps.limits;

		DriverVersion driverVersion;
		driverVersion.Major = ((uint32_t)(deviceProps.apiVersion) >> 22);
		driverVersion.Minor = ((uint32_t)(deviceProps.apiVersion) >> 12) & 0x3ff;
		driverVersion.Release = (uint32_t)(deviceProps.apiVersion) & 0xfff;
		driverVersion.Build = 0;

		caps.DriverVersion = driverVersion;
		caps.DeviceName = deviceProps.deviceName;

		// Determine vendor
		switch(deviceProps.vendorID)
		{
		case 0x10DE:
			caps.DeviceVendor = GPU_NVIDIA;
			break;
		case 0x1002:
			caps.DeviceVendor = GPU_AMD;
			break;
		case 0x163C:
		case 0x8086:
			caps.DeviceVendor = GPU_INTEL;
			break;
		default:
			caps.DeviceVendor = GPU_UNKNOWN;
			break;
		};

		caps.RenderApiName = GetName();

		if(deviceFeatures.textureCompressionBC)
			caps.SetCapability(RSC_TEXTURE_COMPRESSION_BC);

		if(deviceFeatures.textureCompressionETC2)
			caps.SetCapability(RSC_TEXTURE_COMPRESSION_ETC2);

		if(deviceFeatures.textureCompressionASTC_LDR)
			caps.SetCapability(RSC_TEXTURE_COMPRESSION_ASTC);

		caps.SetCapability(RSC_COMPUTE_PROGRAM);
		caps.SetCapability(RSC_LOAD_STORE);
		caps.SetCapability(RSC_LOAD_STORE_MSAA);
		caps.SetCapability(RSC_BYTECODE_CACHING);
		caps.SetCapability(RSC_TEXTURE_VIEWS);
		caps.SetCapability(RSC_RENDER_TARGET_LAYERS);
		caps.SetCapability(RSC_MULTI_THREADED_CB);

		caps.Conventions.NdcYAxis = Conventions::Axis::Down;
		caps.Conventions.MatrixOrder = Conventions::MatrixOrder::ColumnMajor;

		caps.MaxBoundVertexBuffers = deviceLimits.maxVertexInputBindings;
		caps.NumMultiRenderTargets = deviceLimits.maxColorAttachments;

		caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		caps.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

		caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		caps.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;

		caps.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;
		caps.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;

		if(deviceFeatures.geometryShader)
		{
			caps.SetCapability(RSC_GEOMETRY_PROGRAM);
			caps.AddShaderProfile("gs_5_0");
			caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
			caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
			caps.GeometryProgramNumOutputVertices = deviceLimits.maxGeometryOutputVertices;
		}

		if(deviceFeatures.tessellationShader)
		{
			caps.SetCapability(RSC_TESSELLATION_PROGRAM);

			caps.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
			caps.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

			caps.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
			caps.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		}

		caps.NumCombinedTextureUnits = caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];

		caps.NumCombinedParamBlockBuffers = caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM];

		caps.NumCombinedLoadStoreTextureUnits = caps.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];
		caps.MinimumUniformBufferOffsetAlignment = (u32)deviceLimits.minUniformBufferOffsetAlignment;

		caps.AddShaderProfile("glsl");

		deviceIdx++;
	}
}

VulkanCommandBuffer* VulkanRenderAPI::EnsureCommandBuffer(const SPtr<CommandBuffer>& buffer)
{
	if(buffer != nullptr)
		return static_cast<VulkanCommandBuffer*>(buffer.get());

	return static_cast<VulkanCommandBuffer*>(mMainCommandBuffer.get());
}

namespace bs { namespace ct {
VulkanRenderAPI& GetVulkanRenderAPI()
{
	return static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
}
}} // namespace bs::ct
