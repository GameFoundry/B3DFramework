//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**	Data about a sub-mesh range and the type of primitives contained in the range. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(pl:true,DocumentationGroup(Rendering)) SubMesh
	{
		SubMesh() = default;

		SubMesh(u32 indexOffset, u32 indexCount, DrawOperationType drawOp):
			IndexOffset(indexOffset), IndexCount(indexCount), DrawOp(drawOp)
		{ }

		u32 IndexOffset = 0;
		u32 IndexCount = 0;
		DrawOperationType DrawOp = DOT_TRIANGLE_LIST;
	};

	/** @} */
}
