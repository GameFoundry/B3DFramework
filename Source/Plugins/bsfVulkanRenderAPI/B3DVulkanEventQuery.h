//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "RenderAPI/B3DEventQuery.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan event object that manages its usage and lifetime. */
		class VulkanEvent : public VulkanResource
		{
		public:
			VulkanEvent(VulkanResourceManager* owner, const StringView& name = "");
			~VulkanEvent() override;

			/** Returns the internal handle to the Vulkan object. */
			VkEvent GetVulkanHandle() const { return mEvent; }

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
			VulkanEventQuery(VulkanGpuDevice& device);
			~VulkanEventQuery() override;

			void Begin(GpuCommandBuffer& commandBuffer) override;
			bool IsReady() const override;

		private:
			VulkanGpuDevice& mDevice;
			VulkanEvent* mEvent;
		};

		/** @} */
	} // namespace render
} // namespace b3d
