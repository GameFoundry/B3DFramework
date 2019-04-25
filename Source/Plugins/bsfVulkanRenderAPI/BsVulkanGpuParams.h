//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuParams.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Vulkan implementation of GpuParams, containing resource descriptors for all shader stages. */
	class VulkanGpuParams : public GpuParams
	{
	public:
		~VulkanGpuParams();

		/** @copydoc GpuParams::setParamBlockBuffer(UINT32, UINT32, const ParamsBufferType&) */
		void setParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBuffer>& paramBlockBuffer) override;

		/** @copydoc GpuParams::setTexture */
		void setTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture, 
						const TextureSurface& surface = TextureSurface::COMPLETE) override;

		/** @copydoc GpuParams::setLoadStoreTexture */
		void setLoadStoreTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture,
			const TextureSurface& surface) override;

		/** @copydoc GpuParams::setBuffer */
		void setBuffer(UINT32 set, UINT32 slot, const SPtr<GpuBuffer>& buffer) override;

		/** @copydoc GpuParams::setSamplerState */
		void setSamplerState(UINT32 set, UINT32 slot, const SPtr<SamplerState>& sampler) override;

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
			VkBufferView bufferView;
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

		friend class VulkanHardwareBufferManager;

		VulkanGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask);

		/** @copydoc GpuParams::initialize */
		void initialize() override;

		PerDeviceData mPerDeviceData[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
		bool* mSetsDirty = nullptr;

		GroupAlloc mAlloc;
		Mutex mMutex;
	};

	/** @} */
}}