//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	namespace render { class GpuCommandBuffer; }

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Memory-layout category of a GPU allocation. Some APIs require different allocation granularity when
	 * linear and non-linear entries overlap (i.e. buffer image granularity), and this is used by the allocator
	 * to respect that.
	 */
	enum class GpuResourceKind : u8
	{
		Linear		= 0,
		NonLinear	= 1
	};

	/**
	 * Empty marker base for TGpuResourceLocation. Lets non-templated interfaces (e.g. IGpuResource::OnAllocationMoved) accept any backend's
	 * typed location through a common reference; the consumer downcasts to its concrete TGpuResourceLocation<HeapBackend>.
	 */
	struct GpuResourceLocation {};

	/**
	 * GPU memory allocation as returned by a GPU memory allocator. Used for freeing the allocation, as well
	 * as referencing the underlying memory. Each consumer owns their location and is the sole writer; the
	 * allocator only writes to the consumer's location once during the initial @c TryAllocate, and then
	 * supplies a fresh replacement location to @c IGpuResource::OnAllocationMoved when defragmentation
	 * moves the allocation.
	 *
	 * Inherits from @c GpuResourceLocation so any backend-specialised location can be passed through
	 * the non-templated @c IGpuResource::OnAllocationMoved hook and downcast back to its concrete type.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 */
	template <typename HeapBackend>
	struct TGpuResourceLocation : GpuResourceLocation
	{
		using HeapHandle = typename HeapBackend::HeapHandle;

		HeapHandle Heap{};
		u64 Offset = 0;
		u64 Size = 0;

		/** Type-erased pointer to the allocator strategy that owns this allocation. */
		void* Allocator = nullptr;

		// Strategy-private bookkeeping. Interpretation is private to the owning allocator.
		u32 AllocatorData0 = 0;
		u32 AllocatorData1 = 0;

		/** Returns @c true if the location currently refers to a live allocation owned by some allocator. */
		bool IsValid() const
		{
			return Allocator != nullptr;
		}

		/** Resets the location to the empty state. */
		void Reset()
		{
			Heap = HeapHandle{};
			Offset = 0;
			Size = 0;
			Allocator = nullptr;
			AllocatorData0 = 0;
			AllocatorData1 = 0;
		}
	};

	/**
	 * Common interface for a resource that can be stored on the GPU. Provides the use/bound counts
	 * that can be queried whether a resource is currently in flight, plus a relocation hook fired
	 * after an allocator has moved the resource allocation (i.e. during defragmentation).
	 */
	class B3D_EXPORT IGpuResource
	{
	public:
		virtual ~IGpuResource() = default;

		/**
		 * Number of recorded-but-not-yet-submitted command buffers currently referencing the given
		 * subresource. For buffers this is usually always 0, for textures this is a mip/face
		 * combination whose subresource index can that be retrieved from TextureProperties::MapToSubresourceIndex.
		 */
		virtual u32 GetBoundCount(u32 subresourceIdx = 0) const = 0;

		/**
		 * Number of in-flight submissions currently referencing the given subresource. For buffers this is usually
		 * always 0, for textures this is a mip/face combination whose subresource index can that be retrieved
		 * from TextureProperties::MapToSubresourceIndex.
		 */
		virtual u32 GetUseCount(u32 subresourceIdx = 0) const = 0;

		/**
		 * Called after the owning allocator has reserved a new home for this resource during defragmentation.
		 * Inside this call, the consumer's old TGpuResourceLocation is still intact — the implementation
		 * can read its source heap / offset / size off it. The implementation must:
		 *   1. Record a copy from the source range to the destination range using @p commandBuffer.
		 *   2. Queue the old backend object (e.g. VkBuffer / VkImage) for deferred-destroy on @p submissionIndex.
		 *   3. Replace its own TGpuResourceLocation with @p newLocation, so the location identifies the destination slot from now on.
		 *   4. Recreate any placed backend object bound to the new memory range.
		 *
		 * @p newLocation is a GpuResourceLocation reference; the consumer downcasts to its concrete
		 * TGpuResourceLocation<HeapBackend> to access the typed heap handle and slot identity.
		 *
		 * Must succeed; backends should not pick candidates whose recreation can fail.
		 */
		virtual void OnAllocationMoved(u64 submissionIndex, render::GpuCommandBuffer& commandBuffer, const GpuResourceLocation& newLocation)
		{
			(void)submissionIndex;
			(void)commandBuffer;
			(void)newLocation;
		}
	};

	/** @} */
} // namespace b3d
