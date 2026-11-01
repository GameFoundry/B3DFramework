//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuBindlessHeapPool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanDescriptorLayout;

		/**
		 * Native heap of the bindless resource heap: one copy of the bindless heap descriptor set, holding the sampler heap and
		 * GetCapacity() slots of the resource heap, allocated from its own pool. See GpuBindlessHeapPool.
		 */
		class VulkanBindlessHeap : public TVulkanResource<GpuBindlessHeap>
		{
		public:
			VulkanBindlessHeap(VulkanResourceManager* owner, u32 capacity, VkDevice device, VkDescriptorPool pool, VkDescriptorSet set);
			~VulkanBindlessHeap() override;

			/** Returns the descriptor set, bound at kGpuBindlessParameterSet. */
			VkDescriptorSet GetSet() const { return mSet; }

		private:
			VkDevice mDevice;
			VkDescriptorPool mPool;
			VkDescriptorSet mSet;
		};

		/**
		 * Bindless heaps of a Vulkan device. Each native heap is a descriptor set holding both heaps. Slot updates switch native heaps.
		 */
		class VulkanBindlessHeaps final : public GpuBindlessHeaps
		{
		public:
			/**
			 * @param	device					Device that owns the heaps.
			 * @param	completionTracker		Tracker whose current marker is stamped onto released slots.
			 * @param	heapLayout				Layout of the bindless heap set. Must outlive the heaps.
			 * @param	supportsNullDescriptor	True if cleared resource slots can hold null descriptors.
			 * @param	createInformation		Capacities of the heaps.
			 */
			VulkanBindlessHeaps(VulkanGpuDevice& device, const IGpuCompletionTracker& completionTracker, VulkanDescriptorLayout& heapLayout, bool supportsNullDescriptor,
				const GpuBindlessHeapsCreateInformation& createInformation);
			~VulkanBindlessHeaps() override;

		protected:
			/**
			 * @name GpuBindlessHeaps Interface
			 * @{
			 */

			GpuBindlessHeap* CreateResourceHeap(u32 capacity) override;
			void CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges) override;
			void WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) override;
			void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) override;
			void ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType) override;
			void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) override;

			/** @} */

		private:
			/** Writes the descriptor of @p view into resource slot @p slotIndex of @p heap, or of the current native heap if null. */
			void WriteResourceDescriptor(u32 slotIndex, const GpuBindlessResourceView& view, VulkanBindlessHeap* heap);

			/**
			 * Writes @p writeInfo, a resource slot write whose destination set is ignored, into the set of the current native heap, and
			 * marks the slot for the other native heaps to copy.
			 */
			void WriteIntoCurrentHeap(VkWriteDescriptorSet& writeInfo);

			VulkanGpuDevice& mDevice;
			VulkanDescriptorLayout& mHeapLayout;
			bool mSupportsNullDescriptor;
			u32 mSamplerCapacity;
			u32 mSamplerSlotCount = 0; /**< One past the highest sampler slot ever written. Every sampler slot below it holds a valid sampler. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
