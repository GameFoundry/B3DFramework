//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuParams.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of GpuParams, containing resource descriptors for all shader stages. */
		class VulkanGpuParams : public GpuParams
		{
		public:
			~VulkanGpuParams() override;

			void SetParamBlockBuffer(u32 set, u32 slot, const SPtr<GpuParamBlockBuffer>& paramBlockBuffer) override;
			void SetTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete) override;
			void SetLoadStoreTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface) override;
			void SetBuffer(u32 set, u32 slot, const SPtr<GpuBuffer>& buffer) override;
			void SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler) override;

			/** Returns the total number of descriptor sets used by this object. */
			u32 GetNumSets() const;

			/**
			 * Prepares the internal descriptor sets for a bind operation on the provided command buffer. It generates and/or
			 * updates and descriptor sets, and registers the relevant resources with the command buffer.
			 *
			 * Caller must perform external locking if some other thread could write to this object while it is being bound.
			 * The same applies to any resources held by this object.
			 *
			 * @param[in]	buffer	Buffer on which the parameters will be bound to.
			 * @param[out]	sets	Pre-allocated buffer in which the descriptor set handled will be written. Must be of
			 *						getNumSets() size.
			 *
			 * @note	Thread safe.
			 */
			void PrepareForBind(VulkanCmdBuffer& buffer, VkDescriptorSet* sets);

		protected:
			/** Contains data about writing to either buffer or a texture descriptor. */
			union WriteInfo
			{
				VkDescriptorImageInfo Image;
				VkDescriptorBufferInfo Buffer;
				VkBufferView BufferView;
			};

			/** All GPU param data related to a single descriptor set. */
			struct PerSetData
			{
				VulkanDescriptorSet* LatestSet;
				Vector<VulkanDescriptorSet*> Sets;

				VkWriteDescriptorSet* WriteSetInfos;
				WriteInfo* WriteInfos;

				u32 NumElements;
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

			friend class VulkanHardwareBufferManager;

			VulkanGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask);

			void Initialize() override;

			PerDeviceData mPerDeviceData[BS_MAX_DEVICES];
			GpuDeviceFlags mDeviceMask;
			bool* mSetsDirty = nullptr;

			GroupAlloc mAlloc;
			Mutex mMutex;
		};

		/** @} */
	} // namespace ct
} // namespace bs
