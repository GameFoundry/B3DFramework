//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuParams.h"
#include "BsGroupAlloc.h"

namespace bs
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

		/** Returns the total number of descriptor sets used by this object. */
		UINT32 getNumSets() const;

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
		void prepareForBind(VulkanCmdBuffer& buffer, VkDescriptorSet* sets);

	protected:
		/** Contains data about writing to either buffer or a texture descriptor. */
		union WriteInfo
		{
			VkDescriptorImageInfo image;
			VkDescriptorBufferInfo buffer;
		};

		/** All GPU param data related to a single descriptor set. */
		struct PerSetData
		{
			VulkanDescriptorSet* latestSet;
			Vector<VulkanDescriptorSet*> sets;

			VkWriteDescriptorSet* writeSetInfos;
			WriteInfo* writeInfos;

			UINT32 numElements;
		};

		/** All GPU param data beloning to a single device. */
		struct PerDeviceData
		{
			PerSetData* perSetData;

			VkImage* sampledImages;
			VkImage* storageImages;
			VkBuffer* uniformBuffers;
			VkBuffer* buffers;
			VkSampler* samplers;
		};

		friend class VulkanHardwareBufferCoreManager;

		VulkanGpuParams(const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask);

		/** @copydoc GpuParamsCore::initialize */
		void initialize() override;

		PerDeviceData mPerDeviceData[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
		bool* mSetsDirty;

		GroupAlloc mAlloc;
		Mutex mMutex;
	};

	/** @} */
}