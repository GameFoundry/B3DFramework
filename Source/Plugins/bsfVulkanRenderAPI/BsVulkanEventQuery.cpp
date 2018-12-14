//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanEventQuery.h"
#include "BsVulkanDevice.h"
#include "BsVulkanCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanEvent::VulkanEvent(VulkanResourceManager* owner)
		:VulkanResource(owner, false)
	{
		VkDevice vkDevice = owner->getDevice().getLogical();

		VkEventCreateInfo eventCI;
		eventCI.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		eventCI.pNext = nullptr;
		eventCI.flags = 0;

		VkResult result = vkCreateEvent(vkDevice, &eventCI, gVulkanAllocator, &mEvent);
		assert(result == VK_SUCCESS);
	}

	VulkanEvent::~VulkanEvent()
	{
		VkDevice vkDevice = mOwner->getDevice().getLogical();
		vkDestroyEvent(vkDevice, mEvent, gVulkanAllocator);
	}

	bool VulkanEvent::isSignaled() const
	{
		VkDevice vkDevice = mOwner->getDevice().getLogical();
		return vkGetEventStatus(vkDevice, mEvent) == VK_EVENT_SET;
	}

	void VulkanEvent::reset()
	{
		VkDevice vkDevice = mOwner->getDevice().getLogical();

		VkResult result = vkResetEvent(vkDevice, mEvent);
		assert(result == VK_SUCCESS);
	}

	VulkanEventQuery::VulkanEventQuery(VulkanDevice& device)
		:mDevice(device), mEvent(nullptr)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanEventQuery::~VulkanEventQuery()
	{
		if (mEvent != nullptr)
			mEvent->destroy();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanEventQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		if (mEvent != nullptr)
		{
			if (mEvent->isBound())
			{
				// Clear current event and create a new one
				mEvent->destroy();
				mEvent = mDevice.getResourceManager().create<VulkanEvent>();
			}
			else
			{
				// Re-use existing event
				mEvent->reset();
			}
		}
		else // Create new event
			mEvent = mDevice.getResourceManager().create<VulkanEvent>();

		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->getInternal(); 
		internalCB->registerResource(mEvent, VulkanAccessFlag::Read);

		internalCB->setEvent(mEvent);

		setActive(true);
	}

	bool VulkanEventQuery::isReady() const
	{
		if (mEvent == nullptr)
			return false;

		return mEvent->isSignaled();
	}
}}