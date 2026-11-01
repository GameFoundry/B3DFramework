//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalResource.h"
#include "GpuBackend/B3DGpuBindlessHeapPool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Native heap of the bindless resource heap: a buffer of GetCapacity() 8-byte entries that shaders read as an argument buffer. */
		class MetalBindlessHeap : public TMetalResource<GpuBindlessHeap>
		{
		public:
			MetalBindlessHeap(MetalResourceManager* owner, u32 capacity, const TShared<GpuBuffer>& heapBuffer);

			/** Returns the buffer holding the entries. */
			const TShared<GpuBuffer>& GetHeapBuffer() const { return mHeapBuffer; }

		private:
			TShared<GpuBuffer> mHeapBuffer;
		};

		/**
		 * Bindless heaps of a Metal device. Both heaps are buffers of 8-byte entries in shared memory. The resource heap is created at its
		 * maximum capacity, and slot updates patch it in GPU order, so its native heap never switches.
		 */
		class MetalBindlessHeaps final : public GpuBindlessHeaps
		{
		public:
			/**
			 * @param	device				Device that owns the heaps.
			 * @param	completionTracker	Tracker whose current marker is stamped onto released slots.
			 * @param	createInformation	Capacities of the heaps. The resource heap's capacity must be its maximum capacity.
			 */
			MetalBindlessHeaps(MetalGpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation);
			~MetalBindlessHeaps() override;

			/**
			 * Returns the buffer of heap @p heap. The resource heap binds at buffer index kGpuBindlessParameterSet, and the sampler heap at
			 * kMetalSamplerHeapBufferIndex.
			 */
			id<MTLBuffer> GetHeapBuffer(GpuBindlessHeapType heap) const;

		protected:
			/**
			 * @name GpuBindlessHeaps Interface
			 * @{
			 */

			bool Initialize() override;
			GpuBindlessHeap* CreateResourceHeap(u32 capacity) override;
			void CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges) override;
			void WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) override;
			void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) override;
			void ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType) override;
			void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) override;
			bool PatchResourceSlots(TArrayView<const GpuBindlessSlotUpdate> updates) override;

			/** @} */

		private:
			/** Returns the resource heap entry that refers to @p view. */
			u64 GetResourceEntry(const GpuBindlessResourceView& view) const;

			MetalGpuDevice& mDevice;
			u32 mSamplerCapacity;
			TShared<GpuBuffer> mSamplerHeapBuffer;
			TShared<GpuCommandBufferPool> mPatchCommandBufferPool; /**< Created on first use by PatchResourceSlots(). */
		};

		/** @} */
	} // namespace render
} // namespace b3d
