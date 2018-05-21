//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/** Rectangle represented in the form of offsets from some parent rectangle. */
	struct RectOffset
	{
		RectOffset()
			:left(0), right(0), top(0), bottom(0)
		{ }

		RectOffset(INT32 left, INT32 right, INT32 top, INT32 bottom)
			:left(left), right(right), top(top), bottom(bottom)
		{ }

		bool operator==(const RectOffset& rhs) const
		{
			return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
		}

		bool operator!=(const RectOffset& rhs) const
		{
			return !operator==(rhs);
		}

		INT32 left, right, top, bottom;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(RectOffset);
	/** @endcond */
}