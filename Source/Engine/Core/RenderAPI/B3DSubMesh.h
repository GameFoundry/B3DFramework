//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**	Data about a sub-mesh range and the type of primitives contained in the range. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SubMesh
	{
		SubMesh() = default;

		SubMesh(u32 indexOffset, u32 indexCount, DrawOperationType drawOp)
			: IndexOffset(indexOffset), IndexCount(indexCount), DrawOp(drawOp)
		{}

		u32 IndexOffset = 0;
		u32 IndexCount = 0;
		DrawOperationType DrawOp = DOT_TRIANGLE_LIST;
	};

	/** @} */
} // namespace b3d
