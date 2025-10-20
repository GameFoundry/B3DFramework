//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Opaque handle to a resource in the frame graph.
	 *
	 * Resources are identified by index into internal storage. This is a lightweight handle type
	 * that can be copied freely. Use FrameGraph::GetResource() to look up the actual resource.
	 *
	 * Phase 1 Note: Resources are only imported from external textures/buffers.
	 * Transient resources will be added in Phase 4.
	 */
	struct FrameGraphResourceId
	{
		u32 Index = ~0u;

		/** Returns true if this is a valid resource ID */
		bool IsValid() const { return Index != ~0u; }

		/** Comparison operators for use in containers */
		bool operator==(const FrameGraphResourceId& other) const { return Index == other.Index; }
		bool operator!=(const FrameGraphResourceId& other) const { return Index != other.Index; }
		bool operator<(const FrameGraphResourceId& other) const { return Index < other.Index; }
	};

	/** Invalid resource ID constant - use to indicate no resource or initialize handles */
	static constexpr FrameGraphResourceId kInvalidFrameGraphResourceId = FrameGraphResourceId{~0u};

	/** @} */
}

/** Hash function for FrameGraphResourceId for use in unordered containers (std::unordered_map, std::unordered_set, etc.) */
namespace std
{
	template<>
	struct hash<b3d::render::FrameGraphResourceId>
	{
		size_t operator()(const b3d::render::FrameGraphResourceId& id) const noexcept
		{
			return hash<b3d::u32>()(id.Index);
		}
	};
}
