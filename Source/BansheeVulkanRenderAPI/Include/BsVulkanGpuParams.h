//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuParams.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Vulkan implementation of GpuParams, containing resource descriptors for all shader stages. */
	class VulkanGpuParams : public GpuParamsCore
	{
	public:
		~VulkanGpuParams();

		/** @copydoc GpuParamsCore::setParamBlockBuffer(UINT32, UINT32, const ParamsBufferType&) */
		void setParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& paramBlockBuffer) override;

		/** @copydoc GpuParamsCore::setTexture */
		void setTexture(UINT32 set, UINT32 slot, const SPtr<TextureCore>& texture) override;

		/** @copydoc GpuParamsCore::setLoadStoreTexture */
		void setLoadStoreTexture(UINT32 set, UINT32 slot, const SPtr<TextureCore>& texture,
			const TextureSurface& surface) override;

		/** @copydoc GpuParamsCore::setBuffer */
		void setBuffer(UINT32 set, UINT32 slot, const SPtr<GpuBufferCore>& buffer) override;

		/** @copydoc GpuParamsCore::setSamplerState */
		void setSamplerState(UINT32 set, UINT32 slot, const SPtr<SamplerStateCore>& sampler) override;

		/** @copydoc GpuParamsCore::setLoadStoreSurface */
		void setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface) override;

		/** 
		 * Notifies the object that a command buffer containing the object is about to be submitted to a queue. 
		 *
		 * @param[in]	buffer			Command buffer on which we're about to submit the GPU params.
		 * @param[out]	transitionInfo	Contains barriers that transition resources to appropriate queues families
		 *								and/or transition image layouts.
		 */
		void prepareForSubmit(VulkanCmdBuffer* buffer, UnorderedMap<UINT32, TransitionInfo>& transitionInfo);

		/** Binds the internal descriptor sets to the provided command buffer. */
		void bind(VulkanCommandBuffer& buffer);

	protected:
		/** Contains data about writing to either buffer or a texture descriptor. */
		union WriteInfo
		{
			VkDescriptorImageInfo image;
			VkDescriptorBufferInfo buffer;
			VkBufferView bufferView;
		};

		/** All GPU param data related to a single descriptor set. */
		struct PerSetData
		{
			VulkanDescriptorLayout* layout;
			VulkanDescriptorSet* set;

			VkWriteDescriptorSet* writeSetInfos;
			WriteInfo* writeInfos;
			UINT32 numElements;
		};

		/** All GPU param data beloning to a single device. */
		struct PerDeviceData
		{
			PerSetData* perSetData;
			UINT32 numSets;
		};

		friend class VulkanHardwareBufferCoreManager;

		VulkanGpuParams(const GPU_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);

		PerDeviceData mPerDeviceData[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
		UINT8* mData;
		bool* mSetsDirty;
	};

	/** @} */
}