//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuParameters.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d::render
{
	class VulkanResourceTracker;
}

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of GpuParameters, containing resource descriptors for all shader stages. */
		class VulkanGpuParameters : public GpuParameters
		{
		public:
			VulkanGpuParameters(VulkanGpuDevice& gpuDevice, const SPtr<GpuPipelineParameterLayout>& parameterLayout);
			~VulkanGpuParameters() override;

			bool SetUniformBuffer(u32 set, u32 slot, const SPtr<GpuBuffer>& paramBlockBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 set, u32 slot, const SPtr<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex = 0) override;

			/** Returns the total number of descriptor sets used by this object. */
			u32 GetSetCount() const;

			/**
			 * Prepares the internal descriptor sets for a bind operation on the provided command buffer. It generates and/or
			 * updates and descriptor sets, and registers the relevant resources with the command buffer.
			 *
			 * Caller must perform external locking if some other thread could write to this object while it is being bound.
			 * The same applies to any resources held by this object.
			 *
			 * @param		commandBuffer				Buffer on which the parameters will be bound to.
			 * @param		resourceTracker		Tracker to track usages of resources used on the command buffer.
			 * @param		outSets				Pre-allocated buffer in which the descriptor set handled will be written. Must be of GetSetCount() size.
			 * @param		outDynamicOffsets	Dynamic offsets required for binding the descriptor sets.
			 *
			 * @note	Thread safe.
			 */
			void PrepareForBind(VulkanGpuCommandBuffer& commandBuffer, VulkanResourceTracker& resourceTracker, VkDescriptorSet* outSets, Vector<u32>& outDynamicOffsets);

		protected:
			/** All GPU param data related to a single descriptor set. */
			struct PerSetData
			{
				VulkanDescriptorSet* LastUsedSet;
				Vector<VulkanDescriptorSet*> Sets;

				VkWriteDescriptorSet* WriteSetInfos;
				VkDescriptorImageInfo* ImageWriteInfos;
				VkDescriptorBufferInfo* BufferWriteInfos;
				VkBufferView* BufferViews;

				u32 ElementCount;
				u32 LastFreeSetIndex = ~0u;
			};

			/** All GPU param data beloning to a single device. */
			struct PerDeviceData
			{
				PerSetData* PerSetData;

				VkImage* SampledImages;
				VkImage* StorageImages;
				VkBuffer* UniformBuffers;
				VkBuffer* Buffers;
				VkSampler* Samplers;
			};

			void Initialize() override;

			VulkanGpuDevice& mGpuDevice;
			PerDeviceData mPerDeviceData;
			bool* mSetsDirty = nullptr;

			GroupAlloc mAlloc;
			Mutex mMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
