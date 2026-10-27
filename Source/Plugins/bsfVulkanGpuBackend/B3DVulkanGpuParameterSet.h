//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d::render
{
	class GpuResourceTracker;
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
			void Flush(TArrayView<const GpuRenderPassAttachmentUsage> attachments = {}) override;

			/**
			 * Prepares the descriptor set for a bind on a command buffer, and tracks the resources it references with the command buffer. On
			 * the thread that owns the set, flushes the set first (see Flush()), for the render pass of the command buffer.
			 *
			 * @param		resourceTracker		Tracker of the command buffer the set is bound on, used to resolve image layouts and retain resources.
			 * @param		outDynamicOffsets	Receives this set's dynamic buffer offsets in descriptor binding order, appended to any existing offsets.
			 * @param		outSet				Receives the descriptor set handle to bind.
			 * @return							False if shader accesses overlap a writable attachment.
			 */
			bool PrepareForBind(GpuResourceTracker& resourceTracker, TInlineArray<u32, 4>& outDynamicOffsets, VkDescriptorSet& outSet);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Tracks the selected suballocations after resource use has been tracked, without changing descriptors. */
			void TrackDynamicUniformBufferOffsets(GpuResourceTracker& resourceTracker, TArrayView<const u32> offsets) const;
#endif

		protected:
			/**
			 * Resolves the native handles of the bound resources, and the layouts of sampled images in a render pass with @p attachments, and
			 * compares them to what the descriptor set holds. If @p canWrite is true, writes the differences and the earlier Set* calls into
			 * the descriptor set, or into a new one if a command buffer uses the current one. Returns true if the descriptor set needed
			 * changes. Caller holds mMutex.
			 */
			bool UpdateDescriptors(TArrayView<const GpuRenderPassAttachmentUsage> attachments, bool canWrite);

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

			// TODO - Remove. Parameter sets are only used on the thread that owns their pool (see GpuParameterSetPool), so
			// nothing accesses a set concurrently. The Metal and D3D12 parameter sets don't lock.
			Mutex mMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
