//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsSamplerState.h"

namespace bs { namespace ct
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
	class VulkanSamplerState : public SamplerStateCore
	{
	public:
		~VulkanSamplerState();

		/** 
		 * Gets the resource wrapping the sampler object, on the specified device. If sampler state device mask doesn't 
		 * include the provided device, null is returned. 
		 */
		VulkanSampler* getResource(UINT32 deviceIdx) const { return mSamplers[deviceIdx]; }

	protected:
		friend class VulkanRenderStateManager;

		VulkanSamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc SamplerStateCore::createInternal */
		void createInternal() override;

		VulkanSampler* mSamplers[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
}}