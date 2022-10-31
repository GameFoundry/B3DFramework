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
#include "BsVulkanUtility.h"
#include "BsVulkanRenderPass.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#	include "Win32/BsWin32VideoModeInfo.h"
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#	include "Linux/BsLinuxVideoModeInfo.h"
#elif BS_PLATFORM == BS_PLATFORM_OSX
#	include "MacOS/BsMacOSVideoModeInfo.h"
#	include <MoltenVK/vk_mvk_moltenvk.h>
#else
static_assert(false, "Other platform includes go here.");
#endif

#if BS_PLATFORM != BS_PLATFORM_OSX
#	define USE_VALIDATION_LAYERS 1
#else
#	define USE_VALIDATION_LAYERS 0
#endif

VkAllocationCallbacks* gVulkanAllocator = nullptr;

namespace bs { namespace ct {
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;

PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
}} // namespace bs::ct

using namespace bs;
using namespace bs::ct;

VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
	StringStream message;

	// Determine prefix
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		message << "ERROR";

	if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		message << "WARNING";

	if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		message << "PERFORMANCE";

	if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		message << "INFO";

	if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		message << "DEBUG";

	message << ": [" << pLayerPrefix << "] Code " << msgCode << ": " << pMsg << std::endl;

	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		BS_EXCEPT(RenderingAPIException, message.str())
	else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		BS_LOG(Warning, RenderBackend, message.str());
	else
		BS_LOG(Info, RenderBackend, message.str());

	// Don't abort calls that caused a validation message
	return VK_FALSE;
}

VulkanRenderAPI::VulkanRenderAPI()
{
#if BS_DEBUG_MODE
	mDebugCallback = nullptr;
#endif
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
	appInfo.engineVersion = (BS_VERSION_MAJOR << 24) | (BS_VERSION_MINOR << 16) | BS_VERSION_PATCH;

	// MoltenVK doesn't support 1.1, but we don't need it since the only feature we use from it right now is SPIR-V 1.3,
	// and that's not relevant for MoltenVK as SPIR-V gets translated to MSL anyway.
#if BS_PLATFORM == BS_PLATFORM_OSX
	appInfo.apiVersion = VK_API_VERSION_1_0;
#else
	appInfo.apiVersion = VK_API_VERSION_1_1;
#endif

#if BS_DEBUG_MODE && USE_VALIDATION_LAYERS
	const char* layers[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	const char* extensions[] = {
		nullptr, /** Surface extension */
		nullptr, /** OS specific surface extension */
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	uint32_t numLayers = sizeof(layers) / sizeof(layers[0]);
#else
	const char** layers = nullptr;
	const char* extensions[] = {
		nullptr, /** Surface extension */
		nullptr, /** OS specific surface extension */
	};

	uint32_t numLayers = 0;
#endif

	extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#if BS_PLATFORM == BS_PLATFORM_WIN32
	extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif BS_PLATFORM == BS_PLATFORM_ANDROID
	extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif BS_PLATFORM == BS_PLATFORM_LINUX
	extensions[1] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif BS_PLATFORM == BS_PLATFORM_OSX
	extensions[1] = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#else
	static_assert(false, "Other platform includes go here.");
#endif

	uint32_t numExtensions = sizeof(extensions) / sizeof(extensions[0]);

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = numLayers;
	instanceInfo.ppEnabledLayerNames = layers;
	instanceInfo.enabledExtensionCount = numExtensions;
	instanceInfo.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateInstance(&instanceInfo, gVulkanAllocator, &mInstance);
	assert(result == VK_SUCCESS);

	// Set up debugging
#if BS_DEBUG_MODE && USE_VALIDATION_LAYERS
	VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

	GET_INSTANCE_PROC_ADDR(mInstance, CreateDebugReportCallbackEXT);
	GET_INSTANCE_PROC_ADDR(mInstance, DestroyDebugReportCallbackEXT);

	VkDebugReportCallbackCreateInfoEXT debugInfo;
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugInfo.pNext = nullptr;
	debugInfo.flags = 0;
	debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMsgCallback;
	debugInfo.flags = debugFlags;

	result = vkCreateDebugReportCallbackEXT(mInstance, &debugInfo, nullptr, &mDebugCallback);
	assert(result == VK_SUCCESS);
#endif

#if BS_PLATFORM == BS_PLATFORM_OSX
	MVKConfiguration mvkConfig;
	size_t mvkConfigSize = sizeof(MVKConfiguration);
	vkGetMoltenVKConfigurationMVK(mInstance, &mvkConfig, &mvkConfigSize);

#	if BS_DEBUG_MODE
	mvkConfig.debugMode = VK_TRUE;
#	endif

	vkSetMoltenVKConfigurationMVK(mInstance, &mvkConfig, &mvkConfigSize);
#endif

	// Enumerate all devices
	result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, nullptr);
	assert(result == VK_SUCCESS);

	Vector<VkPhysicalDevice> physicalDevices(mNumDevices);
	result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, physicalDevices.data());
	assert(result == VK_SUCCESS);

	mDevices.resize(mNumDevices);
	for(uint32_t i = 0; i < mNumDevices; i++)
		mDevices[i] = B3DMakeShared<VulkanDevice>(physicalDevices[i], i);

	// Find primary device
	// Note: MULTIGPU - Detect multiple similar devices here if supporting multi-GPU
	for(uint32_t i = 0; i < mNumDevices; i++)
	{
		bool isPrimary = mDevices[i]->GetDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		if(isPrimary)
		{
			mDevices[i]->SetIsPrimary();
			mPrimaryDevices.push_back(mDevices[i]);

			// Make sure the primary device is first in the list
			if(i != 0)
			{
				mDevices[0]->SetIndex(i);
				mDevices[i]->SetIndex(0);

				std::swap(mDevices[0], mDevices[i]);
			}

			break;
		}
	}

	if(mPrimaryDevices.size() == 0)
	{
		mDevices[0]->SetIsPrimary();
		mPrimaryDevices.push_back(mDevices[0]);
	}

#if BS_PLATFORM == BS_PLATFORM_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#elif BS_PLATFORM == BS_PLATFORM_LINUX
	mVideoModeInfo = B3DMakeShared<LinuxVideoModeInfo>();
#elif BS_PLATFORM == BS_PLATFORM_OSX
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

	VkDevice presentDevice = GetPresentDeviceInternal()->GetLogical();
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
	VulkanRenderPasses::StartUp();

	// Create hardware buffer manager
	bs::HardwareBufferManager::StartUp();
	HardwareBufferManager::StartUp<VulkanHardwareBufferManager>();

	// Create render window manager
	bs::RenderWindowManager::StartUp<bs::VulkanRenderWindowManager>();
	RenderWindowManager::StartUp();

	// Create query manager
	QueryManager::StartUp<VulkanQueryManager>(*this);

	// Create vertex input manager
	VulkanVertexInputManager::StartUp();

	// Create & register GPU program factories
	mGLSLFactory = B3DNew<VulkanGLSLProgramFactory>();

#if BS_PLATFORM == BS_PLATFORM_OSX
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

	if(mGLSLFactory != nullptr)
	{
		B3DDelete(mGLSLFactory);
		mGLSLFactory = nullptr;
	}

	VulkanVertexInputManager::ShutDown();
	QueryManager::ShutDown();
	RenderStateManager::ShutDown();
	RenderWindowManager::ShutDown();
	bs::RenderWindowManager::ShutDown();
	HardwareBufferManager::ShutDown();
	bs::HardwareBufferManager::ShutDown();
	VulkanRenderPasses::ShutDown();
	TextureManager::ShutDown();
	bs::TextureManager::ShutDown();

	mMainCommandBuffer = nullptr;

	// Make sure everything finishes and all resources get freed
	for(u32 i = 0; i < (u32)mDevices.size(); i++)
		mDevices[i]->WaitIdle();

	CommandBufferManager::ShutDown();

	mPrimaryDevices.clear();
	mDevices.clear();

#if BS_DEBUG_MODE
	if(mDebugCallback != nullptr)
		vkDestroyDebugReportCallbackEXT(mInstance, mDebugCallback, gVulkanAllocator);
#endif

	vkDestroyInstance(mInstance, gVulkanAllocator);

	RenderAPI::DestroyCore();
}

void VulkanRenderAPI::SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetPipelineState(pipelineState);

	BS_INC_RENDER_STAT(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetPipelineState(pipelineState);

	BS_INC_RENDER_STAT(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

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

	BS_INC_RENDER_STAT(NumGpuParamBinds);
}

void VulkanRenderAPI::SetViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetViewport(vp);
}

void VulkanRenderAPI::SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetVertexBuffers(index, buffers, numBuffers);

	BS_INC_RENDER_STAT(NumVertexBufferBinds);
}

void VulkanRenderAPI::SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetIndexBuffer(buffer);

	BS_INC_RENDER_STAT(NumIndexBufferBinds);
}

void VulkanRenderAPI::SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetVertexDeclaration(vertexDeclaration);
}

void VulkanRenderAPI::SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetDrawOp(op);
}

void VulkanRenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->Draw(vertexOffset, vertexCount, instanceCount);

	BS_INC_RENDER_STAT(NumDrawCalls);
	BS_ADD_RENDER_STAT(NumVertices, vertexCount);
	BS_ADD_RENDER_STAT(NumPrimitives, primCount);
}

void VulkanRenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->DrawIndexed(startIndex, indexCount, vertexOffset, instanceCount);

	BS_INC_RENDER_STAT(NumDrawCalls);
	BS_ADD_RENDER_STAT(NumVertices, vertexCount);
	BS_ADD_RENDER_STAT(NumPrimitives, primCount);
}

void VulkanRenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

	BS_INC_RENDER_STAT(NumComputeCalls);
}

void VulkanRenderAPI::SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	Rect2I area(left, top, right - left, bottom - top);
	vkCB->SetScissorRect(area);
}

void VulkanRenderAPI::SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetStencilRef(value);
}

void VulkanRenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->ClearViewport(buffers, color, depth, stencil, targetMask);

	BS_INC_RENDER_STAT(NumClears);
}

void VulkanRenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->ClearRenderTarget(buffers, color, depth, stencil, targetMask);

	BS_INC_RENDER_STAT(NumClears);
}

void VulkanRenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanCommandBuffer* cb = GetCb(commandBuffer);
	VulkanCmdBuffer* vkCB = cb->GetInternal();

	vkCB->SetRenderTarget(target, readOnlyFlags, loadMask);

	BS_INC_RENDER_STAT(NumRenderTargetChanges);
}

void VulkanRenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	SubmitCommandBuffer(mMainCommandBuffer, syncMask);
	target->SwapBuffers(syncMask);

	// See if any command buffers finished executing
	for(u32 i = 0; i < (u32)mDevices.size(); i++)
		mDevices[i]->RefreshStates();

	BS_INC_RENDER_STAT(NumPresents);
}

void VulkanRenderAPI::AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
{
	BS_EXCEPT(NotImplementedException, "Secondary command buffers not implemented");
}

void VulkanRenderAPI::SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	VulkanCommandBuffer* cmdBuffer = GetCb(commandBuffer);

	// Submit all transfer buffers first
	VulkanCommandBufferManager& cbm = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::Instance());
	cbm.FlushTransferBuffers(cmdBuffer->GetDeviceIdx());

	cmdBuffer->Submit(syncMask);

	if(cmdBuffer == mMainCommandBuffer.get())
		mMainCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));
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

GpuParamBlockDesc VulkanRenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
{
	GpuParamBlockDesc block;
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

		caps.AddShaderProfile("glsl");

		deviceIdx++;
	}
}

VulkanCommandBuffer* VulkanRenderAPI::GetCb(const SPtr<CommandBuffer>& buffer)
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
