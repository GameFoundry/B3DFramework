//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Memory
	 *  @{
	 */

	/** Serializable blob of raw memory. */
	struct DataBlob
	{
		u8* Data = nullptr;
		u32 Size = 0;
	};

	/** @} */
} // namespace b3d
