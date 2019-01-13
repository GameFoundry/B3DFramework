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
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(pl:true,m:Rendering) SubMesh
	{
		SubMesh() = default;

		SubMesh(UINT32 indexOffset, UINT32 indexCount, DrawOperationType drawOp):
			indexOffset(indexOffset), indexCount(indexCount), drawOp(drawOp)
		{ }

		UINT32 indexOffset = 0;
		UINT32 indexCount = 0;
		DrawOperationType drawOp = DOT_TRIANGLE_LIST;
	};

	/** @} */
}