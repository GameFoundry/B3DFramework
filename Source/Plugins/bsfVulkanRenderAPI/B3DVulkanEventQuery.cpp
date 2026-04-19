//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanEventQuery.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

VulkanEvent::VulkanEvent(VulkanResourceManager* owner, const StringView& name)
	: VulkanResource(owner, false, name)
{
	VkDevice vkDevice = owner->GetDevice().GetLogical();

	VkEventCreateInfo eventCI;
	eventCI.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	eventCI.pNext = nullptr;
	eventCI.flags = 0;

	VkResult result = vkCreateEvent(vkDevice, &eventCI, gVulkanAllocator, &mEvent);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanEvent::~VulkanEvent()
{
	VkDevice vkDevice = mOwner->GetDevice().GetLogical();
	vkDestroyEvent(vkDevice, mEvent, gVulkanAllocator);
}

bool VulkanEvent::IsSignaled() const
{
	VkDevice vkDevice = mOwner->GetDevice().GetLogical();
	return vkGetEventStatus(vkDevice, mEvent) == VK_EVENT_SET;
}

void VulkanEvent::Reset()
{
	VkDevice vkDevice = mOwner->GetDevice().GetLogical();

	VkResult result = vkResetEvent(vkDevice, mEvent);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanEventQuery::VulkanEventQuery(VulkanGpuDevice& device)
	: mDevice(device), mEvent(nullptr)
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

VulkanEventQuery::~VulkanEventQuery()
{
	if(mEvent != nullptr)
		mEvent->Destroy();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void VulkanEventQuery::Begin(GpuCommandBuffer& commandBuffer)
{
	if(mEvent != nullptr)
	{
		if(mEvent->IsBound())
		{
			// Clear current event and create a new one
			mEvent->Destroy();
			mEvent = mDevice.GetResourceManager().Create<VulkanEvent>();
		}
		else
		{
			// Re-use existing event
			mEvent->Reset();
		}
	}
	else // Create new event
		mEvent = mDevice.GetResourceManager().Create<VulkanEvent>();

	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);
	vulkanCommandBuffer.SetEvent(mEvent);
}

bool VulkanEventQuery::IsReady() const
{
	if(mEvent == nullptr)
		return false;

	return mEvent->IsSignaled();
}
