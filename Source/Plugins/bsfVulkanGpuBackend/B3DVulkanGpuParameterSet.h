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
			 * Appends shader accesses and dynamic offsets, refreshes cached descriptor data for changed resource handles, and retains samplers through @p resourceTracker.
			 * Prepares descriptor resource handles and views; image layouts and descriptor writes are completed by FinalizeDescriptorSet() after tracking accesses from all sets.
			 *
			 * @param		resourceTracker		Command-buffer tracker used to retain samplers. Shader accesses are appended to @p outBindings for the caller to track.
			 * @param		outBindings			Receives this set's image and buffer accesses, appended to any existing bindings.
			 * @param		outDynamicOffsets	Receives this set's dynamic buffer offsets in descriptor binding order, appended to any existing offsets.
			 */
			void PrepareBindingResources(VulkanResourceTracker& resourceTracker, GpuShaderBindings& outBindings, TInlineArray<u32, 4>& outDynamicOffsets);

			/**
			 * Resolves descriptor image layouts from tracked accesses, allocates or updates the descriptor set as needed, and retains it for the command buffer.
			 *
			 * @param		resourceTracker		Command-buffer tracker supplying required image layouts and retaining the descriptor set.
			 * @param		outSet				Receives the descriptor set handle to bind.
			 */
			void FinalizeDescriptorSet(VulkanResourceTracker& resourceTracker, VkDescriptorSet& outSet);

		protected:
			/** Image descriptors awaiting layout resolution in FinalizeDescriptorSet(). */
			struct PendingImageDescriptor
			{
				PendingImageDescriptor(IGpuImageResource* image, const GpuTextureSubresourceRange& range, u32 index) : Image(image), Range(range), Index(index) { }
				IGpuImageResource* Image;
				GpuTextureSubresourceRange Range;
				u32 Index;
			};

			Vector<PendingImageDescriptor> mPendingImageDescriptors;

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
