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

#if BS_PLATFORM == BS_PLATFORM_WIN32
	#include "Win32/BsWin32VideoModeInfo.h"
#elif BS_PLATFORM == BS_PLATFORM_LINUX
	#include "Linux/BsLinuxVideoModeInfo.h"
#else
	static_assert(false, "Other platform includes go here.");
#endif

#define USE_VALIDATION_LAYERS 1

namespace bs { namespace ct
{
	VkAllocationCallbacks* gVulkanAllocator = nullptr;

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

	VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
		size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
	{
		StringStream message;

		// Determine prefix
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			message << "ERROR";

		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			message << "WARNING";

		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			message << "PERFORMANCE";

		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			message << "INFO";

		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			message << "DEBUG";

		message << ": [" << pLayerPrefix << "] Code " << msgCode << ": " << pMsg << std::endl;

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			BS_EXCEPT(RenderingAPIException, message.str())
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			LOGWRN(message.str())
		else
			LOGDBG(message.str())

		// Don't abort calls that caused a validation message
		return VK_FALSE;
	}

	VulkanRenderAPI::VulkanRenderAPI()
	{
#if BS_DEBUG_MODE
		mDebugCallback = nullptr;
#endif
	}

	const StringID& VulkanRenderAPI::getName() const
	{
		static StringID strName("VulkanRenderAPI");
		return strName;
	}

	void VulkanRenderAPI::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		// Create instance
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "bs::framework app";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "bs::framework";
		appInfo.engineVersion = (0 << 24) | (4 << 16) | 0;
		appInfo.apiVersion = VK_API_VERSION_1_0;

#if BS_DEBUG_MODE && USE_VALIDATION_LAYERS
		const char* layers[] =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};

		const char* extensions[] =
		{
			nullptr, /** Surface extension */
			nullptr, /** OS specific surface extension */
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		};

		uint32_t numLayers = sizeof(layers) / sizeof(layers[0]);
#else
		const char** layers = nullptr;
		const char* extensions[] =
		{
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
#else
		extensions[1] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
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

		// Enumerate all devices
		result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, nullptr);
		assert(result == VK_SUCCESS);

		Vector<VkPhysicalDevice> physicalDevices(mNumDevices);
		result = vkEnumeratePhysicalDevices(mInstance, &mNumDevices, physicalDevices.data());
		assert(result == VK_SUCCESS);

		mDevices.resize(mNumDevices);
		for(uint32_t i = 0; i < mNumDevices; i++)
			mDevices[i] = bs_shared_ptr_new<VulkanDevice>(physicalDevices[i], i);

		// Find primary device
		// Note: MULTIGPU - Detect multiple similar devices here if supporting multi-GPU
		for (uint32_t i = 0; i < mNumDevices; i++)
		{
			bool isPrimary = mDevices[i]->getDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

			if (isPrimary)
			{
				mDevices[i]->setIsPrimary();
				mPrimaryDevices.push_back(mDevices[i]);

				// Make sure the primary device is first in the list
				if(i != 0)
				{
					mDevices[0]->setIndex(i);
					mDevices[i]->setIndex(0);

					std::swap(mDevices[0], mDevices[i]);
				}

				break;
			}
		}

		if (mPrimaryDevices.size() == 0)
		{
			mDevices[0]->setIsPrimary();
			mPrimaryDevices.push_back(mDevices[0]);
		}

#if BS_PLATFORM == BS_PLATFORM_WIN32
		mVideoModeInfo = bs_shared_ptr_new<Win32VideoModeInfo>();
#elif BS_PLATFORM == BS_PLATFORM_LINUX
		mVideoModeInfo = bs_shared_ptr_new<LinuxVideoModeInfo>();
#else
		static_assert(false, "mVideoModeInfo needs to be created.");
#endif

		GPUInfo gpuInfo;
		gpuInfo.numGPUs = std::min(5U, mNumDevices);

		for(UINT32 i = 0; i < gpuInfo.numGPUs; i++)
			gpuInfo.names[i] = mDevices[i]->getDeviceProperties().deviceName;

		PlatformUtility::_setGPUInfo(gpuInfo);

		// Get required extension functions
		GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(mInstance, GetPhysicalDeviceSurfacePresentModesKHR);

		VkDevice presentDevice = _getPresentDevice()->getLogical();
		GET_DEVICE_PROC_ADDR(presentDevice, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(presentDevice, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(presentDevice, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(presentDevice, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(presentDevice, QueuePresentKHR);

		// Create command buffer manager
		CommandBufferManager::startUp<VulkanCommandBufferManager>(*this);

		// Create main command buffer
		mMainCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(CommandBuffer::create(GQT_GRAPHICS));

		// Create the texture manager for use by others		
		bs::TextureManager::startUp<bs::VulkanTextureManager>();
		TextureManager::startUp<VulkanTextureManager>();

		// Create hardware buffer manager		
		bs::HardwareBufferManager::startUp();
		HardwareBufferManager::startUp<VulkanHardwareBufferManager>();

		// Create render window manager
		bs::RenderWindowManager::startUp<bs::VulkanRenderWindowManager>();
		RenderWindowManager::startUp();

		// Create query manager 
		QueryManager::startUp<VulkanQueryManager>(*this);

		// Create vertex input manager
		VulkanVertexInputManager::startUp();

		// Create & register HLSL factory		
		mGLSLFactory = bs_new<VulkanGLSLProgramFactory>();

		// Create render state manager
		RenderStateManager::startUp<VulkanRenderStateManager>();
		GpuProgramManager::instance().addFactory("vksl", mGLSLFactory);

		initCapabilites();
		
		RenderAPI::initialize();
	}

	void VulkanRenderAPI::destroyCore()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mGLSLFactory != nullptr)
		{
			bs_delete(mGLSLFactory);
			mGLSLFactory = nullptr;
		}

		VulkanVertexInputManager::shutDown();
		QueryManager::shutDown();
		RenderStateManager::shutDown();
		RenderWindowManager::shutDown();
		bs::RenderWindowManager::shutDown();
		HardwareBufferManager::shutDown();
		bs::HardwareBufferManager::shutDown();
		TextureManager::shutDown();
		bs::TextureManager::shutDown();

		mMainCommandBuffer = nullptr;

		// Make sure everything finishes and all resources get freed
		for (UINT32 i = 0; i < (UINT32)mDevices.size(); i++)
			mDevices[i]->waitIdle();

		CommandBufferManager::shutDown();

		mPrimaryDevices.clear();
		mDevices.clear();

#if BS_DEBUG_MODE
		if (mDebugCallback != nullptr)
			vkDestroyDebugReportCallbackEXT(mInstance, mDebugCallback, gVulkanAllocator);
#endif

		vkDestroyInstance(mInstance, gVulkanAllocator);

		RenderAPI::destroyCore();
	}

	void VulkanRenderAPI::setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setPipelineState(pipelineState);

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setPipelineState(pipelineState);

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		UINT32 globalQueueIdx = CommandSyncMask::getGlobalQueueIdx(cb->getType(), cb->getQueueIdx());

		for (UINT32 i = 0; i < GPT_COUNT; i++)
		{
			SPtr<GpuParamDesc> paramDesc = gpuParams->getParamDesc((GpuProgramType)i);
			if (paramDesc == nullptr)
				continue;

			// Flush all param block buffers
			for (auto iter = paramDesc->paramBlocks.begin(); iter != paramDesc->paramBlocks.end(); ++iter)
			{
				SPtr<GpuParamBlockBuffer> buffer = gpuParams->getParamBlockBuffer(iter->second.set, iter->second.slot);

				if (buffer != nullptr)
					buffer->flushToGPU(globalQueueIdx);
			}
		}

		vkCB->setGpuParams(gpuParams);

		BS_INC_RENDER_STAT(NumGpuParamBinds);
	}

	void VulkanRenderAPI::setViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setViewport(vp);
	}

	void VulkanRenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setVertexBuffers(index, buffers, numBuffers);

		BS_INC_RENDER_STAT(NumVertexBufferBinds);
	}

	void VulkanRenderAPI::setIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setIndexBuffer(buffer);

		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void VulkanRenderAPI::setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setVertexDeclaration(vertexDeclaration);
	}

	void VulkanRenderAPI::setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setDrawOp(op);
	}

	void VulkanRenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->draw(vertexOffset, vertexCount, instanceCount);

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
		UINT32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->drawIndexed(startIndex, indexCount, vertexOffset, instanceCount);

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->dispatch(numGroupsX, numGroupsY, numGroupsZ);

		BS_INC_RENDER_STAT(NumComputeCalls);
	}

	void VulkanRenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		Rect2I area(left, top, right - left, bottom - top);
		vkCB->setScissorRect(area);
	}

	void VulkanRenderAPI::setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setStencilRef(value);
	}

	void VulkanRenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->clearViewport(buffers, color, depth, stencil, targetMask);

		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil,
		UINT8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->clearRenderTarget(buffers, color, depth, stencil, targetMask);

		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags,
		RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanCommandBuffer* cb = getCB(commandBuffer);
		VulkanCmdBuffer* vkCB = cb->getInternal();

		vkCB->setRenderTarget(target, readOnlyFlags, loadMask);
		
		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void VulkanRenderAPI::swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		submitCommandBuffer(mMainCommandBuffer, syncMask);
		target->swapBuffers(syncMask);

		// See if any command buffers finished executing
		for (UINT32 i = 0; i < (UINT32)mDevices.size(); i++)
			mDevices[i]->refreshStates();

		BS_INC_RENDER_STAT(NumPresents);
	}

	void VulkanRenderAPI::addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
	{
		BS_EXCEPT(NotImplementedException, "Secondary command buffers not implemented");
	}

	void VulkanRenderAPI::submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		VulkanCommandBuffer* cmdBuffer = getCB(commandBuffer);

		// Submit all transfer buffers first
		VulkanCommandBufferManager& cbm = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());
		cbm.flushTransferBuffers(cmdBuffer->getDeviceIdx());

		cmdBuffer->submit(syncMask);
	}

	void VulkanRenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
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

	GpuParamBlockDesc VulkanRenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		block.blockSize = 0;
		block.isShareable = true;
		block.name = name;
		block.slot = 0;
		block.set = 0;

		for (auto& param : params)
		{
			UINT32 size;
			if(param.type == GPDT_STRUCT)
			{
				// Structs are always aligned and rounded up to vec4
				size = Math::divideAndRoundUp(param.elementSize, 16U) * 4;
				block.blockSize = Math::divideAndRoundUp(block.blockSize, 4U) * 4;
			}
			else
				size = VulkanUtility::calcInterfaceBlockElementSizeAndOffset(param.type, param.arraySize, block.blockSize);

			param.elementSize = size;
			param.arrayElementStride = size;
			param.cpuMemOffset = block.blockSize;
			param.gpuMemOffset = 0;
			block.blockSize += size * param.arraySize;
			param.paramBlockSlot = 0;
			param.paramBlockSet = 0;
		}

		// Constant buffer size must always be a multiple of 16
		if (block.blockSize % 4 != 0)
			block.blockSize += (4 - (block.blockSize % 4));

		return block;
	}

	void VulkanRenderAPI::initCapabilites()
	{
		mNumDevices = (UINT32)mDevices.size();
		mCurrentCapabilities = bs_newN<RenderAPICapabilities>(mNumDevices);

		UINT32 deviceIdx = 0;
		for (auto& device : mDevices)
		{
			RenderAPICapabilities& caps = mCurrentCapabilities[deviceIdx];

			const VkPhysicalDeviceProperties& deviceProps = device->getDeviceProperties();
			const VkPhysicalDeviceFeatures& deviceFeatures = device->getDeviceFeatures();
			const VkPhysicalDeviceLimits& deviceLimits = deviceProps.limits;

			DriverVersion driverVersion;
			driverVersion.major = ((uint32_t)(deviceProps.apiVersion) >> 22);
			driverVersion.minor = ((uint32_t)(deviceProps.apiVersion) >> 12) & 0x3ff;
			driverVersion.release = (uint32_t)(deviceProps.apiVersion) & 0xfff;
			driverVersion.build = 0;

			caps.driverVersion = driverVersion;
			caps.deviceName = deviceProps.deviceName;
			
			// Determine vendor
			switch (deviceProps.vendorID)
			{
			case 0x10DE:
				caps.deviceVendor = GPU_NVIDIA;
				break;
			case 0x1002:
				caps.deviceVendor = GPU_AMD;
				break;
			case 0x163C:
			case 0x8086:
				caps.deviceVendor = GPU_INTEL;
				break;
			default:
				caps.deviceVendor = GPU_UNKNOWN;
				break;
			};
			
			caps.renderAPIName = getName();

			if(deviceFeatures.textureCompressionBC)
				caps.setCapability(RSC_TEXTURE_COMPRESSION_BC);

			if (deviceFeatures.textureCompressionETC2)
				caps.setCapability(RSC_TEXTURE_COMPRESSION_ETC2);

			if (deviceFeatures.textureCompressionASTC_LDR)
				caps.setCapability(RSC_TEXTURE_COMPRESSION_ASTC);

			caps.setCapability(RSC_COMPUTE_PROGRAM);
			caps.setCapability(RSC_LOAD_STORE);
			caps.setCapability(RSC_LOAD_STORE_MSAA);
			caps.setCapability(RSC_BYTECODE_CACHING);
			caps.setCapability(RSC_TEXTURE_VIEWS);
			caps.setCapability(RSC_RENDER_TARGET_LAYERS);
			caps.setCapability(RSC_MULTI_THREADED_CB);

			caps.conventions.ndcYAxis = Conventions::Axis::Down;
			caps.conventions.matrixOrder = Conventions::MatrixOrder::ColumnMajor;

			caps.maxBoundVertexBuffers = deviceLimits.maxVertexInputBindings;
			caps.numMultiRenderTargets = deviceLimits.maxColorAttachments;

			caps.numTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
			caps.numTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
			caps.numTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

			caps.numGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
			caps.numGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
			caps.numGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;

			caps.numLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;
			caps.numLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;

			if(deviceFeatures.geometryShader)
			{
				caps.setCapability(RSC_GEOMETRY_PROGRAM);
				caps.addShaderProfile("gs_5_0");
				caps.numTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
				caps.numGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
				caps.geometryProgramNumOutputVertices = deviceLimits.maxGeometryOutputVertices;
			}

			if (deviceFeatures.tessellationShader)
			{
				caps.setCapability(RSC_TESSELLATION_PROGRAM);

				caps.numTextureUnitsPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
				caps.numTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
				
				caps.numGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
				caps.numGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
			}

			caps.numCombinedTextureUnits
				= caps.numTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM]
				+ caps.numTextureUnitsPerStage[GPT_VERTEX_PROGRAM] 
				+ caps.numTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM]
				+ caps.numTextureUnitsPerStage[GPT_HULL_PROGRAM] 
				+ caps.numTextureUnitsPerStage[GPT_DOMAIN_PROGRAM]
				+ caps.numTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];

			caps.numCombinedParamBlockBuffers
				= caps.numGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM]
				+ caps.numGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] 
				+ caps.numGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM]
				+ caps.numGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] 
				+ caps.numGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM]
				+ caps.numGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM];

			caps.numCombinedLoadStoreTextureUnits
				= caps.numLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM]
				+ caps.numLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];

			caps.addShaderProfile("glsl");

			deviceIdx++;
		}
	}

	VulkanCommandBuffer* VulkanRenderAPI::getCB(const SPtr<CommandBuffer>& buffer)
	{
		if (buffer != nullptr)
			return static_cast<VulkanCommandBuffer*>(buffer.get());

		return static_cast<VulkanCommandBuffer*>(mMainCommandBuffer.get());
	}

	VulkanRenderAPI& gVulkanRenderAPI()
	{
		return static_cast<VulkanRenderAPI&>(RenderAPI::instance());
	}
}}