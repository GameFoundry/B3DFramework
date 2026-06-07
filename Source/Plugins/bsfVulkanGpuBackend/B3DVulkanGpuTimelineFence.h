//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

namespace b3d
{
	namespace render
	{
		class VulkanGpuDevice;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Vulkan implementation of @c GpuTimelineFence. */
	class VulkanGpuTimelineFence final : public GpuTimelineFence
	{
	public:
		explicit VulkanGpuTimelineFence(render::VulkanGpuDevice& device);
		~VulkanGpuTimelineFence() override;

		VulkanGpuTimelineFence(const VulkanGpuTimelineFence&) = delete;
		VulkanGpuTimelineFence& operator=(const VulkanGpuTimelineFence&) = delete;

		u64 GetCompletedValue() const final;

		/** Returns the underlying timeline semaphore handle, or @c VK_NULL_HANDLE if unavailable. */
		VkSemaphore GetTimelineSemaphore() const { return mTimeline; }

	protected:
		/** Native blocking wait via vkWaitSemaphores. Invoked by Wait(). */
		void WaitInternal(u64 value) final;

	private:
		render::VulkanGpuDevice* mDevice = nullptr;
		VkDevice mLogicalDevice = VK_NULL_HANDLE;
		VkSemaphore mTimeline = VK_NULL_HANDLE;
	};

	/** @} */
} // namespace b3d
