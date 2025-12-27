//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphTypes.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	class FrameGraph;
	class FrameGraphPass;
	class Texture;
	class GpuBuffer;

	/**
	 * Provides access to allocated resources during pass execution.
	 *
	 * This class is passed to the execute callback to allow retrieval of
	 * allocated transient and imported resources. Resources are guaranteed to be
	 * allocated if they're in the pass's declared access list.
	 *
	 * Usage:
	 * @code
	 * graph.DeclareRenderPass("MyPass",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.WriteColor(transientTexture);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         auto texture = resources.GetTexture(transientTexture);
	 *         params->SetTexture("paramName", texture);
	 *         cmd.SetGpuParameterSet(params);
	 *         cmd.Draw(3);
	 *     });
	 * @endcode
	 */
	class B3D_EXPORT FrameGraphPassResources
	{
	public:
		/**
		 * Constructor for FrameGraph internal use only.
		 *
		 * @param frameGraph	Reference to the owning frame graph
		 * @param pass			Pointer to the current pass being executed
		 */
		FrameGraphPassResources(
			FrameGraph& frameGraph,
			const FrameGraphPass* pass);

		/**
		 * Gets an allocated texture resource.
		 *
		 * Use this to retrieve textures (both imported and transient) during pass execution.
		 * The resource must have been declared in the pass setup callback using Read(),
		 * Write(), ReadWrite(), WriteColor(), WriteDepth(), or ReadDepth().
		 *
		 * For transient resources, the texture is guaranteed to be allocated (non-null)
		 * if it was properly declared. For imported resources, returns the original
		 * imported texture.
		 *
		 * @param id		Resource ID returned from ImportTexture or DeclareTransientTexture
		 * @return			Allocated texture (never null if resource exists)
		 *
		 * @throws			Error (B3D_ENSURE) if:
		 *					- Resource doesn't exist
		 *					- Resource is not a texture (type mismatch)
		 *					- Resource is not allocated (transient not yet allocated)
		 */
		SPtr<Texture> GetTexture(FrameGraphResourceId id) const;

		/**
		 * Gets an allocated buffer resource.
		 *
		 * Use this to retrieve buffers (both imported and transient) during pass execution.
		 * The resource must have been declared in the pass setup callback using Read(),
		 * Write(), or ReadWrite().
		 *
		 * For transient resources, the buffer is guaranteed to be allocated (non-null)
		 * if it was properly declared. For imported resources, returns the original
		 * imported buffer.
		 *
		 * @param id		Resource ID returned from ImportBuffer or DeclareTransientBuffer
		 * @return			Allocated buffer (never null if resource exists)
		 *
		 * @throws			Error (B3D_ENSURE) if:
		 *					- Resource doesn't exist
		 *					- Resource is not a buffer (type mismatch)
		 *					- Resource is not allocated (transient not yet allocated)
		 */
		SPtr<GpuBuffer> GetBuffer(FrameGraphResourceId id) const;

		/**
		 * Tries to get a texture resource, returns null if not found or not allocated.
		 *
		 * This is a non-throwing variant of GetTexture() useful for optional resources
		 * or when you want to check if a resource is available without triggering errors.
		 *
		 * Returns null if:
		 * - Resource ID is invalid
		 * - Resource doesn't exist
		 * - Resource is not a texture
		 * - Resource is not yet allocated (transient before first use)
		 *
		 * @param id		Resource ID
		 * @return			Texture or null if not found/not allocated/not a texture
		 */
		SPtr<Texture> TryGetTexture(FrameGraphResourceId id) const;

		/**
		 * Tries to get a buffer resource, returns null if not found or not allocated.
		 *
		 * This is a non-throwing variant of GetBuffer() useful for optional resources
		 * or when you want to check if a resource is available without triggering errors.
		 *
		 * Returns null if:
		 * - Resource ID is invalid
		 * - Resource doesn't exist
		 * - Resource is not a buffer
		 * - Resource is not yet allocated (transient before first use)
		 *
		 * @param id		Resource ID
		 * @return			Buffer or null if not found/not allocated/not a buffer
		 */
		SPtr<GpuBuffer> TryGetBuffer(FrameGraphResourceId id) const;

	private:
		FrameGraph& mFrameGraph;
		const FrameGraphPass* mPass;
	};

	/** @} */
}
