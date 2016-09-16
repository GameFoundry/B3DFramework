//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsSubMesh.h"

namespace BansheeEngine
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** Contains all information needed for rendering a single sub-mesh. Closely tied with Renderer. */
	class BS_EXPORT RenderableElement
	{
	public:
		/**	Reference to the mesh to render. */
		SPtr<MeshCore> mesh;

		/**	Portion of the mesh to render. */
		SubMesh subMesh;

		/**	Material to render the mesh with. */
		SPtr<MaterialCore> material;

		/** GPU buffer containing element's bone matrices, if it requires any. */
		SPtr<GpuBufferCore> boneMatrixBuffer;

		/** Vertex buffer containing element's morph shape vertices, if it has any. */
		SPtr<VertexBufferCore> morphShapeBuffer;
	};

	/** @} */
}