//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"
#include "Managers/BsVulkanDescriptorManager.h"
#include "Managers/BsVulkanQueryManager.h"

#define VMA_IMPLEMENTATION
#include "ThirdParty/vk_mem_alloc.h"

namespace bs { namespace ct
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice device, UINT32 deviceIdx)
		: mPhysicalDevice(device), mDeviceIdx(deviceIdx), mQueueInfos()
	{
		// Set to default
		for (UINT32 i = 0; i < GQT_COUNT; i++)
			mQueueInfos[i].familyIdx = (UINT32)-1;

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

		auto populateQueueInfo = [&](GpuQueueType type, uint32_t familyIdx)
		{
			queueCreateInfos.push_back(VkDeviceQueueCreateInfo());

			VkDeviceQueueCreateInfo& createInfo = queueCreateInfos.back();
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.queueFamilyIndex = familyIdx;
			createInfo.queueCount = std::min(queueFamilyProperties[familyIdx].queueCount, (uint32_t)BS_MAX_QUEUES_PER_TYPE);
			createInfo.pQueuePriorities = defaultQueuePriorities;

			mQueueInfos[type].familyIdx = familyIdx;
			mQueueInfos[type].queues.resize(createInfo.queueCount, nullptr);
		};

		// Look for dedicated compute queues
		for (UINT32 i = 0; i < (UINT32)queueFamilyProperties.size(); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
			{
				populateQueueInfo(GQT_COMPUTE, i);
				break;
			}
		}

		// Look for dedicated upload queues
		for (UINT32 i = 0; i < (UINT32)queueFamilyProperties.size(); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				populateQueueInfo(GQT_UPLOAD, i);
				break;
			}
		}

		// Looks for graphics queues
		for (UINT32 i = 0; i < (UINT32)queueFamilyProperties.size(); i++)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				populateQueueInfo(GQT_GRAPHICS, i);
				break;
			}
		}

		// Set up extensions
		const char* extensions[5];
		uint32_t numExtensions = 0;

		extensions[numExtensions++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		extensions[numExtensions++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
		extensions[numExtensions++] = VK_KHR_MAINTENANCE2_EXTENSION_NAME;

		// Enumerate supported extensions
		bool dedicatedAllocExt = false;
		bool getMemReqExt = false;

		uint32_t numAvailableExtensions = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &numAvailableExtensions, nullptr);
		if (numAvailableExtensions > 0)
		{
			Vector<VkExtensionProperties> availableExtensions(numAvailableExtensions);
			if (vkEnumerateDeviceExtensionProperties(device, nullptr, &numAvailableExtensions, availableExtensions.data()) == VK_SUCCESS)
			{
				for (auto entry : extensions)
				{
					if(entry == VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
					{
						extensions[numExtensions++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
						dedicatedAllocExt = true;
					}
					else if(entry == VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)
					{
						extensions[numExtensions++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
						getMemReqExt = true;
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
		deviceInfo.enabledExtensionCount = numExtensions;
		deviceInfo.ppEnabledExtensionNames = extensions;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;

		VkResult result = vkCreateDevice(device, &deviceInfo, gVulkanAllocator, &mLogicalDevice);
		assert(result == VK_SUCCESS);

		// Retrieve queues
		for(UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 numQueues = (UINT32)mQueueInfos[i].queues.size();
			for (UINT32 j = 0; j < numQueues; j++)
			{
				VkQueue queue;
				vkGetDeviceQueue(mLogicalDevice, mQueueInfos[i].familyIdx, j, &queue);

				mQueueInfos[i].queues[j] = bs_new<VulkanQueue>(*this, queue, (GpuQueueType)i, j);
			}
		}

		// Set up the memory allocator
		VmaAllocatorCreateInfo allocatorCI = {};
		allocatorCI.physicalDevice = device;
		allocatorCI.device = mLogicalDevice;
		allocatorCI.pAllocationCallbacks = gVulkanAllocator;

		if(dedicatedAllocExt && getMemReqExt)
			allocatorCI.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

		vmaCreateAllocator(&allocatorCI, &mAllocator);

		// Create pools/managers
		mCommandBufferPool = bs_new<VulkanCmdBufferPool>(*this);
		mQueryPool = bs_new<VulkanQueryPool>(*this);
		mDescriptorManager = bs_new<VulkanDescriptorManager>(*this);
		mResourceManager = bs_new<VulkanResourceManager>(*this);
	}

	VulkanDevice::~VulkanDevice()
	{
		VkResult result = vkDeviceWaitIdle(mLogicalDevice);
		assert(result == VK_SUCCESS);

		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 numQueues = (UINT32)mQueueInfos[i].queues.size();
			for (UINT32 j = 0; j < numQueues; j++)
			{
				mQueueInfos[i].queues[j]->refreshStates(true, true);
				bs_delete(mQueueInfos[i].queues[j]);
			}
		}

		bs_delete(mDescriptorManager);
		bs_delete(mQueryPool);
		bs_delete(mCommandBufferPool);

		// Needs to happen after query pool & command buffer pool shutdown, to ensure their resources are destroyed
		bs_delete(mResourceManager);
		
		vmaDestroyAllocator(mAllocator);
		vkDestroyDevice(mLogicalDevice, gVulkanAllocator);
	}

	void VulkanDevice::waitIdle()
	{
		VkResult result = vkDeviceWaitIdle(mLogicalDevice);
		assert(result == VK_SUCCESS);

		refreshStates(true);
	}

	void VulkanDevice::refreshStates(bool forceWait)
	{
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 numQueues = getNumQueues((GpuQueueType)i);
			for (UINT32 j = 0; j < numQueues; j++)
			{
				VulkanQueue* queue = getQueue((GpuQueueType)i, j);
				queue->refreshStates(forceWait, false);
			}
		}
	}

	UINT32 VulkanDevice::getQueueMask(GpuQueueType type, UINT32 queueIdx) const
	{
		UINT32 numQueues = getNumQueues(type);
		if (numQueues == 0)
			return 0;

		UINT32 idMask = 0;
		UINT32 curIdx = queueIdx % numQueues;
		while (curIdx < BS_MAX_QUEUES_PER_TYPE)
		{
			idMask |= CommandSyncMask::getGlobalQueueMask(type, curIdx);
			curIdx += numQueues;
		}

		return idMask;
	}

	SurfaceFormat VulkanDevice::getSurfaceFormat(const VkSurfaceKHR& surface, bool gamma) const
	{
		uint32_t numFormats;
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, nullptr);
		assert(result == VK_SUCCESS);
		assert(numFormats > 0);

		VkSurfaceFormatKHR* surfaceFormats = bs_stack_alloc<VkSurfaceFormatKHR>(numFormats);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, surfaceFormats);
		assert(result == VK_SUCCESS);

		SurfaceFormat output;
		output.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
		output.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		output.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

		// If there is no preferred format, use standard RGBA
		if ((numFormats == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			if (gamma)
				output.colorFormat = VK_FORMAT_R8G8B8A8_SRGB;
			else
				output.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

			output.colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool foundFormat = false;

			VkFormat wantedFormatsUNORM[] =
			{
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FORMAT_B8G8R8A8_UNORM,
				VK_FORMAT_A8B8G8R8_UNORM_PACK32,
				VK_FORMAT_A8B8G8R8_UNORM_PACK32,
				VK_FORMAT_R8G8B8_UNORM,
				VK_FORMAT_B8G8R8_UNORM
			};

			VkFormat wantedFormatsSRGB[] =
			{
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_FORMAT_B8G8R8A8_SRGB,
				VK_FORMAT_A8B8G8R8_SRGB_PACK32,
				VK_FORMAT_A8B8G8R8_SRGB_PACK32,
				VK_FORMAT_R8G8B8_SRGB,
				VK_FORMAT_B8G8R8_SRGB
			};

			UINT32 numWantedFormats;
			VkFormat* wantedFormats;
			if (gamma)
			{
				numWantedFormats = sizeof(wantedFormatsSRGB) / sizeof(wantedFormatsSRGB[0]);
				wantedFormats = wantedFormatsSRGB;
			}
			else
			{
				numWantedFormats = sizeof(wantedFormatsUNORM) / sizeof(wantedFormatsUNORM[0]);
				wantedFormats = wantedFormatsUNORM;
			}

			for(UINT32 i = 0; i < numWantedFormats; i++)
			{
				for(UINT32 j = 0; j < numFormats; j++)
				{
					if(surfaceFormats[j].format == wantedFormats[i])
					{
						output.colorFormat = surfaceFormats[j].format;
						output.colorSpace = surfaceFormats[j].colorSpace;

						foundFormat = true;
						break;
					}
				}

				if (foundFormat)
					break;
			}

			// If we haven't found anything, fall back to first available
			if(!foundFormat)
			{
				output.colorFormat = surfaceFormats[0].format;
				output.colorSpace = surfaceFormats[0].colorSpace;

				if (gamma)
					LOGERR("Cannot find a valid sRGB format for a render window surface, falling back to a default format.");
			}
		}

		bs_stack_free(surfaceFormats);
		return output;
	}

	VmaAllocation VulkanDevice::allocateMemory(VkImage image, VkMemoryPropertyFlags flags)
	{
		VmaAllocationCreateInfo allocCI = {};
		allocCI.requiredFlags = flags;

		VmaAllocationInfo allocInfo;
		VmaAllocation allocation;
		VkResult result = vmaAllocateMemoryForImage(mAllocator, image, &allocCI, &allocation, &allocInfo);
		assert(result == VK_SUCCESS);

		result = vkBindImageMemory(mLogicalDevice, image, allocInfo.deviceMemory, allocInfo.offset);
		assert(result == VK_SUCCESS);

		return allocation;
	}

	VmaAllocation VulkanDevice::allocateMemory(VkBuffer buffer, VkMemoryPropertyFlags flags)
	{
		VmaAllocationCreateInfo allocCI = {};
		allocCI.requiredFlags = flags;

		VmaAllocationInfo allocInfo;
		VmaAllocation memory;
		VkResult result = vmaAllocateMemoryForBuffer(mAllocator, buffer, &allocCI, &memory, &allocInfo);
		assert(result == VK_SUCCESS);

		result = vkBindBufferMemory(mLogicalDevice, buffer, allocInfo.deviceMemory, allocInfo.offset);
		assert(result == VK_SUCCESS);

		return memory;
	}

	void VulkanDevice::freeMemory(VmaAllocation allocation)
	{
		vmaFreeMemory(mAllocator, allocation);
	}

	void VulkanDevice::getAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset)
	{
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(mAllocator, allocation, &allocInfo);

		memory = allocInfo.deviceMemory;
		offset = allocInfo.offset;
	}

	uint32_t VulkanDevice::findMemoryType(uint32_t requirementBits, VkMemoryPropertyFlags wantedFlags)
	{
		for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
		{
			if (requirementBits & (1 << i))
			{
				if ((mMemoryProperties.memoryTypes[i].propertyFlags & wantedFlags) == wantedFlags)
					return i;
			}
		}

		return -1;
	}
}}
