//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
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

		/** Holds meta-data about a single GPU parameter set. */
		class VulkanGpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
		{
		public:
			VulkanGpuPipelineParameterSetLayout(VulkanGpuDevice& gpuDevice, const GpuProgramParameterDescription& parameterDescription);
			~VulkanGpuPipelineParameterSetLayout() = default;

			/** Returns a pointer to an array of bindings for the set. */
			TArrayView<const VkDescriptorSetLayoutBinding> GetBindings() const { return mBindings; }

			/** Returns a pointer to any array of types expected by layout bindings. */
			TArrayView<const GpuParameterObjectType> GetTypes() const { return mTypes; }

			/** Returns a pointer to any array of underlying element types for textures/buffers. */
			TArrayView<const GpuBufferFormat> GetElementTypes() const { return mElementTypes; }

			/** Returns a pointer to any array of underlying element array sizes for textures/buffers. */
			TArrayView<const u32> GetElementArraySizes() const { return mArraySizes; }

			/** Returns the sequential index of the binding at the specific slot. Returns ~0u if slot is not used. */
			u32 GetUsedBindingSequentialIndex(u32 slot) const { return mSlotToUsedBindingSequentialIndex[slot]; }

			/** Returns the sequential index of the resource at the specific slot. Returns ~0u if slot is not used. Similar to GetUsedBindingSequentialIndex(), but also accounts for array sizes of each binding. */
			u32 GetUsedResourceSequentialIndex(u32 slot, u32 arrayIndex) const { return mSlotToUsedResourceSequentialIndex[slot] != ~0u ? mSlotToUsedResourceSequentialIndex[slot] + arrayIndex : ~0u; }

			/** Returns a layout for the set. */
			VulkanDescriptorLayout* GetLayout() const { return mLayout; }

		private:
			VulkanGpuDevice& mGpuDevice;
			GroupAllocator mAllocator;

			TArrayView<VkDescriptorSetLayoutBinding> mBindings;
			TArrayView<GpuParameterObjectType> mTypes;
			TArrayView<GpuBufferFormat> mElementTypes;
			TArrayView<u32> mArraySizes;
			TArrayView<u32> mSlotToUsedBindingSequentialIndex;
			TArrayView<u32> mSlotToUsedResourceSequentialIndex;

			VulkanDescriptorLayout* mLayout = nullptr;
		};

		/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
		class VulkanGpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			VulkanGpuPipelineParameterLayout(VulkanGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation);
			~VulkanGpuPipelineParameterLayout() = default;
		};

		/** @} */
	} // namespace render
} // namespace b3d
