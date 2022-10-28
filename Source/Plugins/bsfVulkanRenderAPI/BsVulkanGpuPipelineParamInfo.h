//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs
{
	namespace ct
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
			u32 GetNumBindings(u32 layoutIdx) const { return mLayoutInfos[layoutIdx].NumBindings; }

			/** Returns a pointer to an array of bindings for the layout at the specified index. */
			VkDescriptorSetLayoutBinding* GetBindings(u32 layoutIdx) const { return mLayoutInfos[layoutIdx].Bindings; }

			/** Returns a pointer to any array of types expected by layout bindings. */
			GpuParamObjectType* GetLayoutTypes(u32 layoutIdx) const { return mLayoutInfos[layoutIdx].Types; }

			/** Returns a pointer to any array of underlying element types for textures/buffers. */
			GpuBufferFormat* GetLayoutElementTypes(u32 layoutIdx) const { return mLayoutInfos[layoutIdx].ElementTypes; }

			/** Returns the sequential index of the binding at the specificn set/slot. Returns -1 if slot is not used. */
			u32 GetBindingIdx(u32 set, u32 slot) const { return mSetExtraInfos[set].SlotIndices[slot]; }

			/**
			 * Returns a layout for the specified device, at the specified index. Returns null if no layout for the specified
			 * device index.
			 */
			VulkanDescriptorLayout* GetLayout(u32 deviceIdx, u32 layoutIdx) const;

		private:
			void Initialize() override;

			/** Data related to a single descriptor set layout. */
			struct LayoutInfo
			{
				VkDescriptorSetLayoutBinding* Bindings;
				GpuParamObjectType* Types;
				GpuBufferFormat* ElementTypes;
				u32 NumBindings;
			};

			/** Information about a single set in the param info object. Complements SetInfo. */
			struct SetExtraInfo
			{
				u32* SlotIndices;
			};

			GpuDeviceFlags mDeviceMask;

			SetExtraInfo* mSetExtraInfos = nullptr;
			VulkanDescriptorLayout** mLayouts[BS_MAX_DEVICES];
			LayoutInfo* mLayoutInfos;

			GroupAlloc mAlloc;
		};

		/** @} */
	} // namespace ct
} // namespace bs
