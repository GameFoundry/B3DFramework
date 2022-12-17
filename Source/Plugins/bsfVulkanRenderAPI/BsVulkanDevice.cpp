//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanUtility.h"
#include "Managers/BsVulkanDescriptorManager.h"
#include "Managers/BsVulkanQueryManager.h"

#define VMA_IMPLEMENTATION
#include "BsVulkanSubmitThread.h"
#include "ThirdParty/vk_mem_alloc.h"

using namespace bs;
using namespace bs::ct;

VulkanDevice::VulkanDevice(VkPhysicalDevice device, u32 deviceIdx)
	: mPhysicalDevice(device), mDeviceIdx(deviceIdx), mQueueInfos()
{
	// Set to default
	for(u32 i = 0; i < GQT_COUNT; i++)
		mQueueInfos[i].FamilyIndex = (u32)-1;

	vkGetPhysicalDeviceProperties(device, &mDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &mDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(device, &mMemoryProperties);

	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilyProperties.data());

	// Create queues
	const float defaultQueuePriorities[BS_MAX_QUEUES_PER_TYPE] = { 0.0f };
	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	auto fnPopulateQueueInfo = [&](GpuQueueType type, uint32_t familyIdx)
	{
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo());

		VkDeviceQueueCreateInfo& createInfo = queueCreateInfos.back();
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = familyIdx;
		createInfo.queueCount = std::min(queueFamilyProperties[familyIdx].queueCount, (uint32_t)BS_MAX_QUEUES_PER_TYPE);
		createInfo.pQueuePriorities = defaultQueuePriorities;

		mQueueInfos[type].FamilyIndex = familyIdx;
		mQueueInfos[type].Queues.resize(createInfo.queueCount, nullptr);
	};

	// Look for dedicated compute queues
	for(u32 i = 0; i < (u32)queueFamilyProperties.size(); i++)
	{
		if((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
		{
			fnPopulateQueueInfo(GQT_COMPUTE, i);
			break;
		}
	}

	// Look for dedicated upload queues
	for(u32 i = 0; i < (u32)queueFamilyProperties.size(); i++)
	{
		if((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0 &&
		   ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
		   ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
		{
			fnPopulateQueueInfo(GQT_UPLOAD, i);
			break;
		}
	}

	// Looks for graphics queues
	for(u32 i = 0; i < (u32)queueFamilyProperties.size(); i++)
	{
		if((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			fnPopulateQueueInfo(GQT_GRAPHICS, i);
			break;
		}
	}

	// If we haven't found a dedicated upload queue, look for any non-graphics queue
	if(mQueueInfos[GQT_UPLOAD].Queues.empty())
	{
		for(UINT32 i = 0; i < (UINT32)queueFamilyProperties.size(); i++)
		{
			if(!mQueueInfos[GQT_GRAPHICS].Queues.empty() && mQueueInfos[GQT_GRAPHICS].FamilyIndex == i)
				continue;

			if((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				fnPopulateQueueInfo(GQT_UPLOAD, i);
				break;
			}
		}
	}

	// If we haven't found a dedicated compue queue, look for any that aren't yet used by the graphics and upload queues
	if(mQueueInfos[GQT_COMPUTE].Queues.empty())
	{
		for(UINT32 i = 0; i < (UINT32)queueFamilyProperties.size(); i++)
		{
			if((!mQueueInfos[GQT_GRAPHICS].Queues.empty() && mQueueInfos[GQT_GRAPHICS].FamilyIndex == i) || (!mQueueInfos[GQT_UPLOAD].Queues.empty() && mQueueInfos[GQT_UPLOAD].FamilyIndex == i))
				continue;

			if((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
			{
				fnPopulateQueueInfo(GQT_COMPUTE, i);
				break;
			}
		}
	}

	// Set up extensions
	const char* extensions[6];
	uint32_t extensionCount = 0;

	extensions[extensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE2_EXTENSION_NAME;

	// Enumerate supported extensions
	bool dedicatedAllocExt = false;
	bool getMemReqExt = false;

	uint32_t availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
	if(availableExtensionCount > 0)
	{
		Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		if(vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data()) == VK_SUCCESS)
		{
			for(auto entry : availableExtensions)
			{
				if(strcmp(entry.extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
					dedicatedAllocExt = true;
				}
				else if(strcmp(entry.extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
					getMemReqExt = true;
				}
				else if(strcmp(entry.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME;
				}
			}
		}
	}

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.pEnabledFeatures = &mDeviceFeatures;
	deviceInfo.enabledExtensionCount = extensionCount;
	deviceInfo.ppEnabledExtensionNames = extensions;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	VkResult result = vkCreateDevice(device, &deviceInfo, gVulkanAllocator, &mLogicalDevice);
	B3D_ASSERT(result == VK_SUCCESS);

	// Retrieve queues
	for(u32 i = 0; i < GQT_COUNT; i++)
	{
		u32 numQueues = (u32)mQueueInfos[i].Queues.size();
		for(u32 j = 0; j < numQueues; j++)
		{
			VkQueue queue;
			vkGetDeviceQueue(mLogicalDevice, mQueueInfos[i].FamilyIndex, j, &queue);

			mQueueInfos[i].Queues[j] = B3DNew<VulkanQueue>(*this, queue, (GpuQueueType)i, j);
		}
	}

	// Set up the memory allocator
	VmaAllocatorCreateInfo allocatorCI = {};
	allocatorCI.physicalDevice = device;
	allocatorCI.device = mLogicalDevice;
	allocatorCI.instance = GetVulkanRenderAPI().GetVkInstance();
	allocatorCI.pAllocationCallbacks = gVulkanAllocator;

	if(dedicatedAllocExt && getMemReqExt)
		allocatorCI.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

	vmaCreateAllocator(&allocatorCI, &mAllocator);

	// Create pools/managers
	mCommandBufferPool = B3DNew<VulkanCommandBufferPool>(*this, VulkanThread::Render);

	mQueryPool = B3DNew<VulkanQueryPool>(*this);
	mDescriptorManager = B3DNew<VulkanDescriptorManager>(*this);
	mResourceManager = B3DNew<VulkanResourceManager>(*this);
}

VulkanDevice::~VulkanDevice()
{
	B3DDelete(mDescriptorManager);
	B3DDelete(mQueryPool);
	B3DDelete(mCommandBufferPool);

	// Needs to happen after query pool & command buffer pool shutdown, to ensure their resources are destroyed
	B3DDelete(mResourceManager);

	vmaDestroyAllocator(mAllocator);
	vkDestroyDevice(mLogicalDevice, gVulkanAllocator);
}

void VulkanDevice::WaitUntilIdle() const
{
	AssertIfNotVulkanSubmitThread();

	VkResult result = vkDeviceWaitIdle(mLogicalDevice);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanDevice::DoForEachQueue(const std::function<void(VulkanQueue&)>&& callback) const
{
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		GpuQueueType queueType = (GpuQueueType)queueTypeIndex;

		const u32 queueCount = GetQueueCountForType(queueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			VulkanQueue* const queue = GetQueue(queueType, queueIndex);
			callback(*queue);
		}
	}
}

u32 VulkanDevice::GetQueueMask(GpuQueueType type, u32 queueIdx) const
{
	u32 numQueues = GetQueueCountForType(type);
	if(numQueues == 0)
		return 0;

	u32 idMask = 0;
	u32 curIdx = queueIdx % numQueues;
	while(curIdx < BS_MAX_QUEUES_PER_TYPE)
	{
		idMask |= CommandSyncMask::GetGlobalQueueMask(type, curIdx);
		curIdx += numQueues;
	}

	return idMask;
}

SurfaceFormat VulkanDevice::GetSurfaceFormat(const VkSurfaceKHR& surface, bool gamma) const
{
	uint32_t numFormats;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, nullptr);
	B3D_ASSERT(result == VK_SUCCESS);
	B3D_ASSERT(numFormats > 0);

	VkSurfaceFormatKHR* surfaceFormats = B3DStackAllocate<VkSurfaceFormatKHR>(numFormats);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, surfaceFormats);
	B3D_ASSERT(result == VK_SUCCESS);

	SurfaceFormat output;
	output.ColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	output.ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	PixelFormat depthFormat = VulkanUtility::GetClosestSupportedPixelFormat(*this, PF_D24S8, TEX_TYPE_2D, TU_DEPTHSTENCIL, true, false);

	output.DepthFormat = VulkanUtility::GetPixelFormat(depthFormat);

	// If there is no preferred format, use standard RGBA
	if((numFormats == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		if(gamma)
			output.ColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
		else
			output.ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;

		output.ColorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		bool foundFormat = false;

		VkFormat wantedFormatsUNORM[] = {
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			VK_FORMAT_R8G8B8_UNORM,
			VK_FORMAT_B8G8R8_UNORM
		};

		VkFormat wantedFormatsSRGB[] = {
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			VK_FORMAT_R8G8B8_SRGB,
			VK_FORMAT_B8G8R8_SRGB
		};

		u32 numWantedFormats;
		VkFormat* wantedFormats;
		if(gamma)
		{
			numWantedFormats = sizeof(wantedFormatsSRGB) / sizeof(wantedFormatsSRGB[0]);
			wantedFormats = wantedFormatsSRGB;
		}
		else
		{
			numWantedFormats = sizeof(wantedFormatsUNORM) / sizeof(wantedFormatsUNORM[0]);
			wantedFormats = wantedFormatsUNORM;
		}

		for(u32 i = 0; i < numWantedFormats; i++)
		{
			for(u32 j = 0; j < numFormats; j++)
			{
				if(surfaceFormats[j].format == wantedFormats[i])
				{
					output.ColorFormat = surfaceFormats[j].format;
					output.ColorSpace = surfaceFormats[j].colorSpace;

					foundFormat = true;
					break;
				}
			}

			if(foundFormat)
				break;
		}

		// If we haven't found anything, fall back to first available
		if(!foundFormat)
		{
			output.ColorFormat = surfaceFormats[0].format;
			output.ColorSpace = surfaceFormats[0].colorSpace;

			if(gamma)
			{
				B3D_LOG(Error, RenderBackend, "Cannot find a valid sRGB format for a render window surface, "
											 "falling back to a default format.");
			}
		}
	}

	B3DStackFree(surfaceFormats);
	return output;
}

VmaAllocation VulkanDevice::AllocateMemory(VkImage image, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocationCreateInformation = {};
	allocationCreateInformation.usage = usage;

	VmaAllocationInfo allocationInfo;
	VmaAllocation allocation;
	VkResult result = vmaAllocateMemoryForImage(mAllocator, image, &allocationCreateInformation, &allocation, &allocationInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	result = vkBindImageMemory(mLogicalDevice, image, allocationInfo.deviceMemory, allocationInfo.offset);
	B3D_ASSERT(result == VK_SUCCESS);

	return allocation;
}

VmaAllocation VulkanDevice::AllocateMemory(VkBuffer buffer, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocationCreateInformation = {};
	allocationCreateInformation.usage = usage;

	VmaAllocationInfo allocationInfo;
	VmaAllocation memory;
	VkResult result = vmaAllocateMemoryForBuffer(mAllocator, buffer, &allocationCreateInformation, &memory, &allocationInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	result = vkBindBufferMemory(mLogicalDevice, buffer, allocationInfo.deviceMemory, allocationInfo.offset);
	B3D_ASSERT(result == VK_SUCCESS);

	return memory;
}

void VulkanDevice::FreeMemory(VmaAllocation allocation)
{
	vmaFreeMemory(mAllocator, allocation);
}

void* VulkanDevice::MapMemory(const VmaAllocation& allocation) const
{
	void* data;
	VkResult result = vmaMapMemory(mAllocator, allocation, &data);
	B3D_ASSERT(result == VK_SUCCESS);

	return data;
}

void VulkanDevice::UnmapMemory(const VmaAllocation& allocation) const
{
	vmaUnmapMemory(mAllocator, allocation);
}

void VulkanDevice::InvalidateMemory(const VmaAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	VkResult result = vmaInvalidateAllocation(mAllocator, allocation, offset, size);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanDevice::FlushMemory(const VmaAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	VkResult result = vmaFlushAllocation(mAllocator, allocation, offset, size);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanDevice::GetAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset)
{
	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(mAllocator, allocation, &allocInfo);

	memory = allocInfo.deviceMemory;
	offset = allocInfo.offset;
}

uint32_t VulkanDevice::FindMemoryType(uint32_t requirementBits, VkMemoryPropertyFlags wantedFlags)
{
	for(uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
	{
		if(requirementBits & (1 << i))
		{
			if((mMemoryProperties.memoryTypes[i].propertyFlags & wantedFlags) == wantedFlags)
				return i;
		}
	}

	return -1;
}
