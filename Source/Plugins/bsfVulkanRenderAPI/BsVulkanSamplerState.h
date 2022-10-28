//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsSamplerState.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan sampler object that manages its usage and lifetime. */
		class VulkanSampler : public VulkanResource
		{
		public:
			VulkanSampler(VulkanResourceManager* owner, VkSampler sampler);
			~VulkanSampler();

			/** Returns the internal handle to the Vulkan object. */
			VkSampler GetHandle() const { return mSampler; }

		private:
			VkSampler mSampler;
		};

		/**	Vulkan implementation of a sampler state. Wraps a Vulkan sampler object. */
		class VulkanSamplerState : public SamplerState
		{
		public:
			~VulkanSamplerState();

			/**
			 * Gets the resource wrapping the sampler object, on the specified device. If sampler state device mask doesn't
			 * include the provided device, null is returned.
			 */
			VulkanSampler* GetResource(u32 deviceIdx) const { return mSamplers[deviceIdx]; }

		protected:
			friend class VulkanRenderStateManager;

			VulkanSamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

			void CreateInternal() override;

			VulkanSampler* mSamplers[BS_MAX_DEVICES];
			GpuDeviceFlags mDeviceMask;
		};

		/** @} */
	} // namespace ct
} // namespace bs
