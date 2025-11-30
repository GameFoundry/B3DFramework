//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DSubMesh.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Contains all information needed for rendering a single sub-mesh. Closely tied with Renderer. */
		class B3D_EXPORT RenderElement
		{
		public:
			/**	Reference to the mesh to render. */
			SPtr<Mesh> Mesh;

			/**	Portion of the mesh to render. */
			SubMesh SubMesh;

			/**	Material to render the mesh with. */
			SPtr<Material> Material;

			/** Index of the technique in the material to render the element with. */
			u32 DefaultVariationIndex = 0;

			/** Index of the technique in the material to render the element with when velocity writes are supported. */
			u32 WriteVelocityVariationIndex = ~0u;

			/** All GPU parameters from the material used by the renderable. */
			SPtr<MaterialParameterAdapter> ParameterAdapter;

			/** Shared parameter set for per-object data (set #1), bound at render time. */
			SPtr<GpuParameterSet> SharedPerObjectParameterSet;

			/** Renderer specific value that identifies the type of this renderable element. */
			u32 Type = 0;

			/** Encodes the draw call for the render element. */
			virtual void Draw(GpuCommandBuffer& commandBuffer) const = 0;

		protected:
			~RenderElement() = default;
		};

		/** @} */
	} // namespace render
} // namespace b3d
