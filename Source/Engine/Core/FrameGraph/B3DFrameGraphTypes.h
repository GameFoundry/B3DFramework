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
	 * Currently resources are only imported from external textures/buffers.
	 * Transient resources will be added in a future update.
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

	/**
	 * Type of frame graph pass.
	 * Determines how the pass is executed and what automatic management is performed.
	 */
	enum class FrameGraphPassType
	{
		/**
		 * Generic pass with no automatic management.
		 * User must manually call BeginRenderPass/EndRenderPass if needed.
		 * Barriers are still automatically inserted.
		 */
		Generic,

		/**
		 * Render pass - renders to one or more render targets.
		 * Frame graph automatically:
		 * - Creates RenderTarget from declared color/depth attachments
		 * - Calls BeginRenderPass before execute lambda
		 * - Calls EndRenderPass after execute lambda
		 * - Issues barriers before BeginRenderPass
		 */
		Render,

		/**
		 * Compute pass - executes compute shaders.
		 * No render pass management, but barriers are automatically inserted.
		 * Validates that no render target resources are used.
		 */
		Compute
	};

	class FrameGraphPass;
	class FrameGraphPassResources;

	/**
	 * Tracks the lifetime of a resource within the frame graph.
	 * Used to determine when resources can be allocated and released.
	 */
	struct FrameGraphResourceLifetime
	{
		/** The resource being tracked */
		FrameGraphResourceId Resource;

		/** The pass where this resource is first used (or imported) */
		FrameGraphPass* FirstUse = nullptr;

		/** The pass where this resource is last used */
		FrameGraphPass* LastUse = nullptr;

		/** Whether this resource is written to (not just read) */
		bool IsWritten = false;

		/** Whether this resource is read from an external source (imported) */
		bool IsImported = false;

		/** Whether this resource is explicitly marked as an output */
		bool IsOutput = false;
	};

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
