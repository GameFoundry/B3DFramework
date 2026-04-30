//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuTimelineFence.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuBackend.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuTimelineFence::VulkanGpuTimelineFence(VulkanGpuDevice& device)
	: mDevice(&device) , mLogicalDevice(device.GetLogical())
{
	if (!device.SupportsTimelineSemaphores())
		return;

	VkSemaphoreTypeCreateInfoKHR timelineCreateInfo = {};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
	timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
	timelineCreateInfo.initialValue = 0;

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = &timelineCreateInfo;

	const VkResult result = vkCreateSemaphore(mLogicalDevice, &semaphoreCreateInfo, gVulkanAllocator, &mTimeline);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanGpuTimelineFence::~VulkanGpuTimelineFence()
{
	if (mTimeline != VK_NULL_HANDLE)
		vkDestroySemaphore(mLogicalDevice, mTimeline, gVulkanAllocator);
}

u64 VulkanGpuTimelineFence::GetCompletedValue() const
{
	if (mTimeline == VK_NULL_HANDLE)
		return 0;

	u64 current = 0;
	const VkResult result = vkGetSemaphoreCounterValueKHR(mLogicalDevice, mTimeline, &current);
	B3D_ASSERT(result == VK_SUCCESS);

	return current;
}
