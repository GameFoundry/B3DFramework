//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "RenderAPI/B3DSamplerState.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan sampler object that manages its usage and lifetime. */
		class VulkanSampler : public VulkanResource
		{
		public:
			VulkanSampler(VulkanResourceManager* owner, VkSampler sampler, const StringView& name = "");
			~VulkanSampler();

			/** Returns the internal handle to the Vulkan object. */
			VkSampler GetVulkanHandle() const { return mSampler; }

		private:
			VkSampler mSampler;
		};

		/**	Vulkan implementation of a sampler state. Wraps a Vulkan sampler object. */
		class VulkanSamplerState : public SamplerState
		{
		public:
			~VulkanSamplerState();

			/** Gets the resource wrapping the sampler object; */
			VulkanSampler* GetVulkanResource() const { return mSampler; }

		protected:
			friend class VulkanGpuDevice;

			VulkanSamplerState(VulkanGpuDevice& gpuDevice, const SamplerStateCreateInformation& createInformation);

			void Initialize() override;

			VulkanGpuDevice& mGpuDevice;
			VulkanSampler* mSampler = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
