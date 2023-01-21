//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup bsfSL bsfSL
 *	Implementation of the BSF Shading Language.
 */

/** @} */

namespace bs
{
	extern const char* SystemName;

	/**	Contains the results of BSL parsing or compilation. */
	struct BSLResult
	{
		String ErrorMessage; /**< Error message if compilation failed. */
		int ErrorLine = 0; /**< Line of the error if one occurred. */
		int ErrorColumn = 0; /**< Column of the error if one occurred. */
		String ErrorFile; /**< File in which the error occurred. Empty if root file. */
	};
}
