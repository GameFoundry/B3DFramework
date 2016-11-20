//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsSamplerState.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan sampler object that manages its usage and lifetime. */
	class VulkanSampler : public VulkanResource
	{
	public:
		VulkanSampler(VulkanResourceManager* owner, VkSampler sampler);
		~VulkanSampler();

		/** Returns the internal handle to the Vulkan object. */
		VkSampler getHandle() const { return mSampler; }

	private:
		VkSampler mSampler;
	};

	/**	Vulkan implementation of a sampler state. Wraps a Vulkan sampler object. */
	class VulkanSamplerStateCore : public SamplerStateCore
	{
	public:
		~VulkanSamplerStateCore();

		/** 
		 * Gets the resource wrapping the sampler object, on the specified device. If sampler state device mask doesn't 
		 * include the provided device, null is returned. 
		 */
		VulkanSampler* getResource(UINT32 deviceIdx) const { return mSamplers[deviceIdx]; }

		/* 
		 * Returns a set sampler handles for the devices matching the provided mask. 
		 * 
		 * @param[in]	mask		Mask which determines for which devices we want the handles for. The device must exist 
		 *							in both the provided mask and the mask of the sampler state was created with.
		 * @param[out]	handles		Output array holding up to BS_MAX_LINKED_DEVICES handles. Only the first @p numHandles
		 *							entries of the array are defined.
		 * @param[out]	numHandles	Number of entries in the @p handles array. 
		 */
		void getHandles(GpuDeviceFlags mask, VkSampler(&handles)[BS_MAX_LINKED_DEVICES], UINT32& numHandles);

	protected:
		friend class VulkanRenderStateCoreManager;

		VulkanSamplerStateCore(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc SamplerStateCore::createInternal */
		void createInternal() override;

		VulkanSampler* mSamplers[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
}