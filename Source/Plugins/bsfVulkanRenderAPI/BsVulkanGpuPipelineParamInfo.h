//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
	class VulkanGpuPipelineParamInfo : public GpuPipelineParamInfo
	{
	public:
		VulkanGpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);
		~VulkanGpuPipelineParamInfo() = default;

		/** Returns the number of bindings present at the layout at the specified index. */
		UINT32 getNumBindings(UINT32 layoutIdx) const { return mLayoutInfos[layoutIdx].numBindings; }

		/** Returns a pointer to an array of bindings for the layout at the specified index. */
		VkDescriptorSetLayoutBinding* getBindings(UINT32 layoutIdx) const { return mLayoutInfos[layoutIdx].bindings; }

		/** Returns a pointer to any array of types expected by layout bindings. */
		GpuParamObjectType* getLayoutTypes(UINT32 layoutIdx) const { return mLayoutInfos[layoutIdx].types; }

		/** Returns the sequential index of the binding at the specificn set/slot. Returns -1 if slot is not used. */
		UINT32 getBindingIdx(UINT32 set, UINT32 slot) const { return mSetExtraInfos[set].slotIndices[slot]; }

		/** 
		 * Returns a layout for the specified device, at the specified index. Returns null if no layout for the specified 
		 * device index. 
		 */
		VulkanDescriptorLayout* getLayout(UINT32 deviceIdx, UINT32 layoutIdx) const;

	private:
		/**	@copydoc GpuPipelineParamInfo::initialize */
		void initialize() override;

		/** Data related to a single descriptor set layout. */
		struct LayoutInfo
		{
			VkDescriptorSetLayoutBinding* bindings;
			GpuParamObjectType* types;
			UINT32 numBindings;
		};

		/** Information about a single set in the param info object. Complements SetInfo. */
		struct SetExtraInfo
		{
			UINT32* slotIndices;
		};

		GpuDeviceFlags mDeviceMask;

		SetExtraInfo* mSetExtraInfos = nullptr;
		VulkanDescriptorLayout** mLayouts[BS_MAX_DEVICES];
		LayoutInfo* mLayoutInfos;

		GroupAlloc mAlloc;
	};

	/** @} */
}}