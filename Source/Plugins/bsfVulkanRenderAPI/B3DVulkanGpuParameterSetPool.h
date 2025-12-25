//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuParameterSetPool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of GpuParameterSetPool. */
		class VulkanGpuParameterSetPool final : public GpuParameterSetPool
		{
			friend class VulkanGpuParameterSet;
			friend class VulkanDescriptorSet;

		public:
			VulkanGpuParameterSetPool(VulkanGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation);
			~VulkanGpuParameterSetPool() override;

			SPtr<GpuParameterSet> Create(const SPtr<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize = false) override;
			void Reset() override;

			/** Returns the underlying Vulkan descriptor pool handle. */
			VkDescriptorPool GetVulkanHandle() const { return mPool; }

		private:
			/**
			 * Allocates a descriptor set from the pool, including the VulkanDescriptorSet wrapper.
			 *
			 * @param layout	The descriptor set layout.
			 * @return			The allocated VulkanDescriptorSet, or nullptr on failure.
			 */
			VulkanDescriptorSet* AllocateDescriptorSet(VkDescriptorSetLayout layout);

			/** Frees the Vulkan descriptor set allocated by AllocateDescriptorSet. */
			void NotifyDescriptorSetDestroyed(VulkanDescriptorSet* set);

			VulkanGpuDevice& mDevice;
			VkDescriptorPool mPool = VK_NULL_HANDLE;

#if B3D_BUILD_TYPE_DEVELOPMENT
			UnorderedSet<VulkanDescriptorSet*> mLiveDescriptorSets;
#endif
		};

		/** @} */
	} // namespace render
} // namespace b3d
