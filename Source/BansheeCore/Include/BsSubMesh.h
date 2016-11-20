//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**	Data about a sub-mesh range and the type of primitives contained in the range. */
	struct BS_CORE_EXPORT SubMesh
	{
		SubMesh()
			: indexOffset(0), indexCount(0), drawOp(DOT_TRIANGLE_LIST)
		{ }

		SubMesh(UINT32 indexOffset, UINT32 indexCount, DrawOperationType drawOp):
			indexOffset(indexOffset), indexCount(indexCount), drawOp(drawOp)
		{ }

		UINT32 indexOffset;
		UINT32 indexCount;
		DrawOperationType drawOp;
	};

	/** @} */
}