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
		void setParamBlockBuffer(UINT32 set, UINT32 slot, const ParamsBufferType& paramBlockBuffer) override;

		/** @copydoc GpuParamsCore::setTexture */
		void setTexture(UINT32 set, UINT32 slot, const TextureType& texture) override;

		/** @copydoc GpuParamsCore::setLoadStoreTexture */
		void setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, 
			const TextureSurface& surface) override;

		/** @copydoc GpuParamsCore::setBuffer */
		void setBuffer(UINT32 set, UINT32 slot, const BufferType& buffer) override;

		/** @copydoc GpuParamsCore::setSamplerState */
		void setSamplerState(UINT32 set, UINT32 slot, const SamplerType& sampler) override;

		/** @copydoc GpuParamsCore::setLoadStoreSurface */
		void setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface) override;

	protected:
		/** All GPU param data related to a single descriptor set. */
		struct PerSetData
		{
			VulkanDescriptorLayout* layout;
		};

		/** All GPU param data beloning to a single device. */
		struct PerDeviceData
		{
			PerSetData* perSetData;
			UINT32 numSets;
		};

		friend class VulkanHardwareBufferCoreManager;

		VulkanGpuParams(const GPU_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);

		PerDeviceData mPerDeviceData[BS_MAX_LINKED_DEVICES];
		UINT32 mNumDevices;
	};

	/** @} */
}