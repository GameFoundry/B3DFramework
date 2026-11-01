//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12DescriptorHeap.h"
#include "Managers/B3DD3D12DescriptorManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Bindless heaps of a D3D12 device. The resource heap is the bindless region at the start of the shader-visible CBV/SRV/UAV heaps,
		 * which are its native heaps, and the sampler heap is the bindless region of the shader-visible sampler heap. Slot updates switch
		 * native heaps.
		 *
		 * The descriptor table region of the CBV/SRV/UAV heaps follows the bindless region, so switching native heaps also brings the
		 * tables up to date, see D3D12DescriptorManager.
		 */
		class D3D12BindlessHeaps final : public GpuBindlessHeaps
		{
		public:
			/**
			 * @param	device				Device that owns the heaps.
			 * @param	completionTracker	Tracker whose current marker is stamped onto released slots.
			 * @param	createInformation	Capacities of the heaps.
			 */
			D3D12BindlessHeaps(D3D12GpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation);
			~D3D12BindlessHeaps() override;

			/** Returns the current native resource heap. Tables written into it reach every command buffer that gets it. */
			D3D12BindlessHeap* GetCurrentResourceHeap() const;

			/**
			 * Marks @p count descriptors of the table region, starting at @p offset, as written into the current native heap only, so
			 * every other native heap copies them before it is current again. Must be called with the table region mutex held, see
			 * D3D12DescriptorManager::GetResourceTableMutex().
			 */
			void MarkTableDescriptorsDirty(u32 offset, u32 count);

		protected:
			/**
			 * @name GpuBindlessHeaps Interface
			 * @{
			 */

			bool SwitchResourceHeap(u32 capacity, TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots) override;
			GpuBindlessHeap* CreateResourceHeap(u32 capacity) override;
			void CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges) override;
			void WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) override;
			void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) override;
			void ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType) override;
			void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) override;

			/** @} */

		private:
			/** Returns the CPU-only shader resource view that resource slots holding @p view copy. A null view if it cannot be created. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetViewHandle(const GpuBindlessResourceView& view) const;

			/** Writes @p source into resource slot @p slotIndex of the mirror and of the current native heap. Nothing may read the slot. */
			void WriteIntoCurrentHeap(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source);

			D3D12GpuDevice& mDevice;
			D3D12DescriptorManager& mDescriptorManager;
			D3D12DescriptorHeap mMirror; /**< CPU-only copy of the resource heap, holding the latest descriptor of every slot. Native heaps are filled from it. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
