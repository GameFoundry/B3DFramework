//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
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
		 * Called after the owning allocator has moved the resource to another memory location.
		 * The allocator will update the resource's TGpuResourceLocation with the new location
		 * before calling this method.
		 */
		virtual void OnAllocationMoved() {}
	};

	/** @} */
} // namespace b3d
