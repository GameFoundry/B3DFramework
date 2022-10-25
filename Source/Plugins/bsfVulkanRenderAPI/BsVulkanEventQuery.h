//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsEventQuery.h"

namespace bs
{
	namespace ct
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
			VkEvent GetHandle() const { return mEvent; }

			/** Checks if the event has been signaled on the device. */
			bool IsSignaled() const;

			/** Resets an event back to unsignaled state, making it re-usable. */
			void Reset();

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
			void Begin(const SPtr<CommandBuffer>& cb);

			/** @copydoc EventQuery::isReady */
			bool IsReady() const override;

		private:
			VulkanDevice& mDevice;
			VulkanEvent* mEvent;
		};

		/** @} */
	} // namespace ct
} // namespace bs
