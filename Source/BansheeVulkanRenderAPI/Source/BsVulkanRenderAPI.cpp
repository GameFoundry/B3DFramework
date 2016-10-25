//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderAPI.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsGpuParamDesc.h"
#include "BsVulkanDevice.h"
#include "BsVulkanTextureManager.h"
#include "BsVulkanRenderWindowManager.h"
#include "BsVulkanHardwareBufferManager.h"
#include "BsVulkanRenderStateManager.h"
#include "BsGpuProgramManager.h"
#include "BsVulkanQueryManager.h"
#include "BsVulkanGLSLProgramFactory.h"
#include "BsVulkanCommandBufferManager.h"
#include "Win32/BsWin32VideoModeInfo.h"

namespace BansheeEngine
{
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
		:mInstance(nullptr), mDebugCallback(nullptr)
	{ }

	VulkanRenderAPI::~VulkanRenderAPI()
	{

	}

	const StringID& VulkanRenderAPI::getName() const
	{
		static StringID strName("VulkanRenderAPI");
		return strName;
	}

	const String& VulkanRenderAPI::getShadingLanguageName() const
	{
		static String strName("glsl");
		return strName;
	}

	void VulkanRenderAPI::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		// Create instance
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Banshee3D App";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "Banshee3D";
		appInfo.engineVersion = (0 << 24) | (4 << 16) | 0;
		appInfo.apiVersion = VK_API_VERSION_1_0;

#if BS_DEBUG_MODE
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
#else
		const char** layers = nullptr;
		const char* extensions[] =
		{
			nullptr, /** Surface extension */
			nullptr, /** OS specific surface extension */
		};
#endif

		extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#if BS_PLATFORM == BS_PLATFORM_WIN32
		extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif BS_PLATFORM == BS_PLATFORM_ANDROID
		extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#else
		extensions[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#endif

		uint32_t numLayers = sizeof(layers) / sizeof(layers[0]);
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
#if BS_DEBUG_MODE
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
		uint32_t numDevices;

		result = vkEnumeratePhysicalDevices(mInstance, &numDevices, nullptr);
		assert(result == VK_SUCCESS);

		Vector<VkPhysicalDevice> physicalDevices(numDevices);
		result = vkEnumeratePhysicalDevices(mInstance, &numDevices, physicalDevices.data());
		assert(result == VK_SUCCESS);

		mDevices.resize(numDevices);
		for(uint32_t i = 0; i < numDevices; i++)
			mDevices[i] = bs_shared_ptr_new<VulkanDevice>(physicalDevices[i]);

		// Find primary device
		// Note: MULTIGPU - Detect multiple similar devices here if supporting multi-GPU
		for (uint32_t i = 0; i < numDevices; i++)
		{
			if (mDevices[i]->getDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				mPrimaryDevices.push_back(mDevices[i]);
				break;
			}
		}

		if (mPrimaryDevices.size() == 0)
			mPrimaryDevices.push_back(mDevices[0]);

#if BS_PLATFORM == BS_PLATFORM_WIN32
		mVideoModeInfo = bs_shared_ptr_new<Win32VideoModeInfo>();
#else
		static_assert(false, "mVideoModeInfo needs to be created.")
#endif

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

		// Create the texture manager for use by others		
		TextureManager::startUp<VulkanTextureManager>();
		TextureCoreManager::startUp<VulkanTextureCoreManager>();

		// Create hardware buffer manager		
		HardwareBufferManager::startUp();
		HardwareBufferCoreManager::startUp<VulkanHardwareBufferCoreManager>();

		// Create render window manager
		RenderWindowManager::startUp<VulkanRenderWindowManager>();
		RenderWindowCoreManager::startUp<VulkanRenderWindowCoreManager>(*this);

		// Create query manager 
		QueryManager::startUp<VulkanQueryManager>();

		// Create & register HLSL factory		
		mGLSLFactory = bs_new<VulkanGLSLProgramFactory>();

		// Create render state manager
		RenderStateCoreManager::startUp<VulkanRenderStateCoreManager>();
		GpuProgramCoreManager::instance().addFactory(mGLSLFactory);

		// TODO - Create and populate capabilities, per device
		mCurrentCapabilities->addShaderProfile("glsl");

		RenderAPICore::initialize();
	}

    void VulkanRenderAPI::destroyCore()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mGLSLFactory != nullptr)
		{
			bs_delete(mGLSLFactory);
			mGLSLFactory = nullptr;
		}

		QueryManager::shutDown();
		RenderStateCoreManager::shutDown();
		RenderWindowCoreManager::shutDown();
		RenderWindowManager::shutDown();
		HardwareBufferCoreManager::shutDown();
		HardwareBufferManager::shutDown();
		TextureCoreManager::shutDown();
		TextureManager::shutDown();
		CommandBufferManager::shutDown();

		mPrimaryDevices.clear();
		mDevices.clear();

#if BS_DEBUG_MODE
		if (mDebugCallback != nullptr)
			vkDestroyDebugReportCallbackEXT(mInstance, mDebugCallback, gVulkanAllocator);
#endif

		vkDestroyInstance(mInstance, gVulkanAllocator);

		RenderAPICore::destroyCore();
	}

	void VulkanRenderAPI::setGraphicsPipeline(const SPtr<GpuPipelineStateCore>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setComputePipeline(const SPtr<GpuProgramCore>& computeProgram,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setGpuParams(const SPtr<GpuParamsCore>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumGpuParamBinds);
	}

	void VulkanRenderAPI::beginFrame(const SPtr<CommandBuffer>& commandBuffer)
	{

	}

	void VulkanRenderAPI::endFrame(const SPtr<CommandBuffer>& commandBuffer)
	{

	}

	void VulkanRenderAPI::setViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumVertexBufferBinds);
	}

	void VulkanRenderAPI::setIndexBuffer(const SPtr<IndexBufferCore>& buffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void VulkanRenderAPI::setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
		UINT32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumComputeCalls);
	}

	void VulkanRenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil,
		UINT8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void VulkanRenderAPI::swapBuffers(const SPtr<RenderTargetCore>& target, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPresents);
	}

	void VulkanRenderAPI::addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
	{

	}

	void VulkanRenderAPI::executeCommands(const SPtr<CommandBuffer>& commandBuffer)
	{

	}
	
	void VulkanRenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{

	}

	const RenderAPIInfo& VulkanRenderAPI::getAPIInfo() const
	{
		static RenderAPIInfo info(0.0f, 0.0f, 0.0f, 1.0f, VET_COLOR_ABGR, false, true, false, false);

		return info;
	}

	GpuParamBlockDesc VulkanRenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		
		return block;
	}
}