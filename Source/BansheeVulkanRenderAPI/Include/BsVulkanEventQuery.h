//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsEventQuery.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan event object that manages its usage and lifetime. */
	class VulkanEvent : public VulkanResource
	{
	public:
		VulkanEvent(VulkanResourceManager* owner);
		~VulkanEvent();

		/** Returns the internal handle to the Vulkan object. */
		VkEvent getHandle() const { return mEvent; }

		/** Checks if the event has been signaled on the device. */
		bool isSignaled() const;

		/** Resets an event back to unsignaled state, making it re-usable. */
		void reset();
	private:
		VkEvent mEvent;
	};

	/** @copydoc EventQuery */
	class VulkanEventQuery : public EventQuery
	{
	public:
		VulkanEventQuery(VulkanDevice& device);
		~VulkanEventQuery();

		/** @copydoc EventQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc EventQuery::isReady */
		bool isReady() const override;

	private:
		VulkanDevice& mDevice;
		VulkanEvent* mEvent;
	};

	/** @} */
}