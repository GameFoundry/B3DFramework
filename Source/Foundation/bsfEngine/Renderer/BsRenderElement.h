//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "RenderAPI/BsSubMesh.h"

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** Contains all information needed for rendering a single sub-mesh. Closely tied with Renderer. */
	class BS_EXPORT RenderElement
	{
	public:
		/**	Reference to the mesh to render. */
		SPtr<Mesh> mesh;

		/**	Portion of the mesh to render. */
		SubMesh subMesh;

		/**	Material to render the mesh with. */
		SPtr<Material> material;

		/** Index of the technique in the material to render the element with. */
		UINT32 techniqueIdx;

		/** All GPU parameters from the material used by the renderable. */
		SPtr<GpuParamsSet> params;

		/** Renderer specific value that identifies the type of this renderable element. */
		UINT32 type = 0;

		/** Executes the draw call for the render element. */
		virtual void draw() const = 0;

	protected:
		~RenderElement() = default;
	};

	/** @} */
}}