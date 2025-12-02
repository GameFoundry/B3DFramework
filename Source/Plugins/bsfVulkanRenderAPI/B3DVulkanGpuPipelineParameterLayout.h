//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d
{
	namespace render
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

			/** Returns a pointer to an array of bindings for the set at the specified index. */
			TArrayView<const VkDescriptorSetLayoutBinding> GetBindings(u32 set) const { return mExtendedSetInformation[set].Bindings; }

			/** Returns a pointer to any array of types expected by layout bindings. */
			TArrayView<const GpuParameterObjectType> GetTypes(u32 set) const { return mExtendedSetInformation[set].Types; }

			/** Returns a pointer to any array of underlying element types for textures/buffers. */
			TArrayView<const GpuBufferFormat> GetElementTypes(u32 set) const { return mExtendedSetInformation[set].ElementTypes; }

			/** Returns a pointer to any array of underlying element array sizes for textures/buffers. */
			TArrayView<const u32> GetElementArraySizes(u32 set) const { return mExtendedSetInformation[set].ArraySizes; }

			/** Returns the sequential index of the binding at the specific set/slot. Returns ~0u if slot is not used. */
			u32 GetUsedBindingSequentialIndex(u32 set, u32 slot) const { return mExtendedSetInformation[set].SlotToUsedBindingSequentialIndex[slot]; }

			/** Returns the sequential index of the resource at the specific set/slot. Returns ~0u if slot is not used. Similar to GetUsedBindingSequentialIndex(), but also accounts for array sizes of each binding. */
			u32 GetUsedResourceSequentialIndex(u32 set, u32 slot, u32 arrayIndex) const { return mExtendedSetInformation[set].SlotToUsedResourceSequentialIndex[slot] != ~0u ? mExtendedSetInformation[set].SlotToUsedResourceSequentialIndex[slot] + arrayIndex : ~0u; }

			/** Returns a layout for the specified set. */
			VulkanDescriptorLayout* GetLayout(u32 set) const { return mLayouts[set]; }

		private:
			/** Data related to a single descriptor set layout. */
			struct ExtendedSetInformation
			{
				TArrayView<VkDescriptorSetLayoutBinding> Bindings;
				TArrayView<GpuParameterObjectType> Types;
				TArrayView<GpuBufferFormat> ElementTypes;
				TArrayView<u32> ArraySizes;
				TArrayView<u32> SlotToUsedBindingSequentialIndex;
				TArrayView<u32> SlotToUsedResourceSequentialIndex;
			};

			VulkanGpuDevice& mGpuDevice;

			TArrayView<VulkanDescriptorLayout*> mLayouts;
			TArrayView<ExtendedSetInformation> mExtendedSetInformation;

			GroupAllocator mAllocator;
		};

		/** @} */
	} // namespace render
} // namespace b3d
