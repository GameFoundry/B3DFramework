//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuPipelineParamInfo.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
	class VulkanGpuPipelineParamInfo : public GpuPipelineParamInfoCore
	{
	public:
		VulkanGpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);
		~VulkanGpuPipelineParamInfo();

		/** Returns the number of bindings present at the layout at the specified index. */
		UINT32 getNumBindings(UINT32 layoutIdx) const { return mLayoutInfos[layoutIdx].numBindings; }

		/** Returns a pointer to an array of bindings for the layout at the specified index. */
		VkDescriptorSetLayoutBinding* getBindings(UINT32 layoutIdx) const { return mLayoutInfos[layoutIdx].bindings; }

		/** 
		 * Returns a layout for the specified device, at the specified index. Returns null if no layout for the specified 
		 * device index. 
		 */
		VulkanDescriptorLayout* getLayout(UINT32 deviceIdx, UINT32 layoutIdx) const;

	private:
		/**	@copydoc GpuPipelineParamInfoCore::initialize */
		void initialize() override;

		/** Data related to a single descriptor set layout. */
		struct LayoutInfo
		{
			VkDescriptorSetLayoutBinding* bindings;
			UINT32 numBindings;
		};

		GpuDeviceFlags mDeviceMask;

		VulkanDescriptorLayout** mLayouts[BS_MAX_DEVICES];
		LayoutInfo* mLayoutInfos;

		UINT8* mData;
	};

	/** @} */
}