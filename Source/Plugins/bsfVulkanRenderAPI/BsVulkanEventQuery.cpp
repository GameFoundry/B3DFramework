//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanEventQuery.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanEvent::VulkanEvent(VulkanResourceManager* owner)
	: VulkanResource(owner, false)
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

void VulkanEventQuery::Begin(const SPtr<GpuCommandBuffer>& cb)
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

	VulkanGpuCommandBuffer* vulkanCB;
	if(cb != nullptr)
		vulkanCB = static_cast<VulkanGpuCommandBuffer*>(cb.get());
	else
		vulkanCB = static_cast<VulkanGpuCommandBuffer*>(GetVulkanRenderAPI().GetMainVulkanCommandBuffer());

	VulkanInternalCommandBuffer* internalCB = vulkanCB->GetInternal();
	internalCB->RegisterResource(mEvent, VulkanAccessFlag::Read);

	internalCB->SetEvent(mEvent);
}

bool VulkanEventQuery::IsReady() const
{
	if(mEvent == nullptr)
		return false;

	return mEvent->IsSignaled();
}
