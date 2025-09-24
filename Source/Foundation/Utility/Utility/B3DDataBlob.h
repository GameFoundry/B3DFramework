//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Error/B3DException.h"

namespace b3d
{
	/** @addtogroup General
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
