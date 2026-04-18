//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "ECS/B3DEntity.h"

namespace b3d
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Persistent identifier for objects used by the renderer, such as renderables, lights, cameras, etc.
	 * These IDs are allocated on the main thread and passed to the render thread, where they are used to
	 * find PackedRendererId values for accessing packed arrays associated with the renderer object.
	 */
	using RendererId = ecs::Entity;

	/** Sentinel value representing an invalid RendererId. */
	inline const RendererId kInvalidRendererId{RendererId::kIdentifierMask, RendererId::kVersionMask};

	/** Identifier for a slot in the packed renderer arrays. It's always a sequential index into the packed renderer arrays. */
	using PackedRendererId = std::conditional_t<sizeof(RendererId) == 8, u64, u32>;

	/** Sentinel value representing an invalid PackedRendererId. */
	constexpr PackedRendererId kInvalidPackedRendererId = ~PackedRendererId(0);

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Allocator for RendererId values. Deallocated IDs may be re-used. Increments version on reuse. */
	class RendererIdAllocator
	{
	public:
		/** Allocates a new RendererId: pops from free list (with bumped version) or creates a fresh ID at version 0. */
		RendererId Allocate()
		{
			if(!mFreeList.Empty())
			{
				RendererId reused = mFreeList.back();
				mFreeList.Pop();
				return reused;
			}

			RendererId objectId(mNextIdentifier, 0);
			++mNextIdentifier;
			return objectId;
		}

		/** Deallocates a RendererId, pushing it to the free list and increments the version. */
		void Deallocate(RendererId objectId)
		{
			const RendererId::VersionType nextVersion = (objectId.GetVersion() + 1) & RendererId::kVersionMask;
			mFreeList.Add(RendererId(objectId.GetIdentifier(), nextVersion));
		}

	private:
		u32 mNextIdentifier = 0;
		TArray<RendererId> mFreeList;
	};

	/** @} */
} // namespace b3d
