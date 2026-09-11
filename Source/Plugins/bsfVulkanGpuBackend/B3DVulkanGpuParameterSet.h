//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuShaderBindings.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d::render
{
	class VulkanBarrierHelper;
	class VulkanResourceTracker;
	class VulkanGpuParameterSetPool;
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
			VulkanGpuParameterSet(VulkanGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 set, VulkanGpuParameterSetPool& pool);
			~VulkanGpuParameterSet() override;

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

			/**
			 * Prepares the descriptor set for a bind operation on a command buffer. Refreshes descriptor contents for resources whose internal handles changed,
			 * allocates or updates the descriptor set as needed, and retains it and its samplers through @p resourceTracker. Shader accesses are appended to
			 * @p outBindings and must be tracked once every set participating in the bind has been prepared.
			 *
			 * Caller must prevent concurrent changes to this set and its resources until the bindings have been tracked.
			 *
			 * @param		resourceTracker		Tracker of the command buffer the set is bound on, used to resolve image layouts and retain resources.
			 * @param		outBindings			Receives this set's image and buffer accesses, appended to any existing bindings.
			 * @param		outDynamicOffsets	Receives this set's dynamic buffer offsets in descriptor binding order, appended to any existing offsets.
			 * @param		outSet				Receives the descriptor set handle to bind.
			 */
			void PrepareForBind(VulkanResourceTracker& resourceTracker, GpuShaderBindings& outBindings, TInlineArray<u32, 4>& outDynamicOffsets, VkDescriptorSet& outSet);

		protected:
			/** All GPU param data related to a single descriptor set. */
			struct SetInformation
			{
				VulkanDescriptorSet* DescriptorSet = nullptr;

				TArrayView<VkWriteDescriptorSet> WriteSetInfos;
				TArrayView<VkDescriptorImageInfo> ImageWriteInfos;
				TArrayView<VkDescriptorBufferInfo> BufferWriteInfos;
				TArrayView<VkBufferView> BufferViews;

				u32 ElementCount;
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
