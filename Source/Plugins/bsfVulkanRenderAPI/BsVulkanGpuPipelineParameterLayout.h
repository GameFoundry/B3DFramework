//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
		class VulkanGpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			VulkanGpuPipelineParameterLayout(VulkanGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation);
			~VulkanGpuPipelineParameterLayout() = default;

			void Initialize() override;

			/** Returns the number of bindings present at the layout at the specified index. */
			u32 GetLayoutBindingCount(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].BindingCount; }

			/** Returns the number of resource present at the layout at the specified index. Similar to GetBindingCount(), but also accounts for array sizes of each binding. */
			u32 GetLayoutResourceCount(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].ResourceCount; }

			/** Returns a pointer to an array of bindings for the layout at the specified index. */
			VkDescriptorSetLayoutBinding* GetLayoutBindings(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].Bindings; }

			/** Returns a pointer to any array of types expected by layout bindings. */
			GpuParameterObjectType* GetLayoutTypes(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].Types; }

			/** Returns a pointer to any array of underlying element types for textures/buffers. */
			GpuBufferFormat* GetLayoutElementTypes(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].ElementTypes; }

			/** Returns a pointer to any array of underlying element array sizes for textures/buffers. */
			u32* GetLayoutElementArraySizes(u32 layoutIndex) const { return mLayoutInfos[layoutIndex].ArraySizes; }

			/** Returns the sequential index of the binding at the specific set/slot. Returns ~0u if slot is not used. */
			u32 GetUsedBindingSequentialIndex(u32 set, u32 slot) const { return mSetExtraInfos[set].SlotToUsedBindingSequentialIndex[slot]; }

			/** Returns the sequential index of the resource at the specific set/slot. Returns ~0u if slot is not used. Similar to GetUsedBindingSequentialIndex(), but also accounts for array sizes of each binding. */
			u32 GetUsedResourceSequentialIndex(u32 set, u32 slot, u32 arrayIndex) const { return mSetExtraInfos[set].SlotToUsedResourceSequentialIndex[slot] != ~0u ? mSetExtraInfos[set].SlotToUsedResourceSequentialIndex[slot] + arrayIndex : ~0u; }

			/**
			 * Returns a layout for the specified device, at the specified index. Returns null if no layout for the specified
			 * device index.
			 */
			VulkanDescriptorLayout* GetLayout(u32 deviceIdx, u32 layoutIndex) const;

		private:
			/** Data related to a single descriptor set layout. */
			struct LayoutInfo
			{
				VkDescriptorSetLayoutBinding* Bindings = nullptr;
				GpuParameterObjectType* Types = nullptr;
				GpuBufferFormat* ElementTypes = nullptr;
				u32* ArraySizes = nullptr;
				u32 BindingCount = 0;
				u32 ResourceCount = 0;
			};

			/** Information about a single set in the param info object. Complements SetInfo. */
			struct SetExtraInfo
			{
				u32* SlotToUsedBindingSequentialIndex;
				u32* SlotToUsedResourceSequentialIndex;
			};

			VulkanGpuDevice& mGpuDevice;

			SetExtraInfo* mSetExtraInfos = nullptr;
			VulkanDescriptorLayout** mLayouts;
			LayoutInfo* mLayoutInfos;

			GroupAlloc mAlloc;
		};

		/** @} */
	} // namespace ct
} // namespace bs
