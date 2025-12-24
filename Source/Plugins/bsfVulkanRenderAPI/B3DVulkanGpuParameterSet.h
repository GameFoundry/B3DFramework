//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d::render
{
	class VulkanBarrierHelper;
	class VulkanResourceTracker;
}

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of GpuParameterSet, containing resource descriptors for a single descriptor set. */
		class VulkanGpuParameterSet : public GpuParameterSet
		{
			friend class VulkanGpuParameterSetPool;

		public:
			VulkanGpuParameterSet(VulkanGpuDevice& gpuDevice, const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 set);
			~VulkanGpuParameterSet() override;

			bool SetUniformBuffer(u32 slot, const SPtr<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const SPtr<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex = 0) override;

			/**
			 * Prepares the internal descriptor sets for a bind operation on the provided command buffer. It generates and/or
			 * updates and descriptor sets, and registers the relevant resources with the command buffer.
			 *
			 * Caller must perform external locking if some other thread could write to this object while it is being bound.
			 * The same applies to any resources held by this object.
			 *
			 * @param		commandBuffer		Buffer on which the parameters will be bound to.
			 * @param		resourceTracker		Tracker to track usages of resources used on the command buffer.
			 * @param		barrierHelper		Barrier helper into which to queue any required barriers.
			 * @param		outSet				Parameter into which the descriptor set handle will be written.
			 * @param		outDynamicOffsets	Dynamic offsets required for binding the descriptor sets.
			 *
			 * @note	Thread safe.
			 */
			void PrepareForBind(VulkanGpuCommandBuffer& commandBuffer, VulkanResourceTracker& resourceTracker, VulkanBarrierHelper& barrierHelper, VkDescriptorSet& outSet, TInlineArray<u32, 4>& outDynamicOffsets);

		protected:
			/** All GPU param data related to a single descriptor set. */
			struct SetInformation
			{
				VulkanDescriptorSet* LastUsedSet;
				TInlineArray<VulkanDescriptorSet*, 4> SetCache;

				TArrayView<VkWriteDescriptorSet> WriteSetInfos;
				TArrayView<VkDescriptorImageInfo> ImageWriteInfos;
				TArrayView<VkDescriptorBufferInfo> BufferWriteInfos;
				TArrayView<VkBufferView> BufferViews;

				u32 ElementCount;
				u32 LastFreeSetIndex = ~0u;
			};

			void Initialize() override;

			VulkanGpuDevice& mGpuDevice;

			SetInformation mSetInformation;
			bool mSetDirty = false;

			TArrayView<VkImage> mSampledImages;
			TArrayView<VkImage> mStorageImages;
			TArrayView<VkBuffer> mUniformBuffers;
			TArrayView<VkBuffer> mBuffers;
			TArrayView<VkSampler> mSamplers;

			GroupAllocator mAllocator;
			Mutex mMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
