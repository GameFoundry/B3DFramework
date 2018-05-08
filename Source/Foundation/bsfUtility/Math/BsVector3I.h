//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A three dimensional vector with integer coordinates. */
	struct BS_SCRIPT_EXPORT(m:Math,pl:true) Vector3I
	{
		INT32 x = 0;
		INT32 y = 0;
		INT32 z = 0;

		constexpr Vector3I() = default;

		constexpr Vector3I(INT32 x, INT32 y, INT32 z)
			:x(x), y(y), z(z)
		{ }

		INT32 operator[] (size_t i) const
		{
			assert(i < 3);

			switch (i)
			{
				case 0: return x;
				case 1: return y;
				case 2: return z;
				default: return 0;
			}
		}

		INT32& operator[] (size_t i)
		{
			assert(i < 3);

			switch (i)
			{
				case 0: return x;
				case 1: return y;
				case 2: return z;
				default: return x;
			}
		}

		bool operator== (const Vector3I& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}

		bool operator!= (const Vector3I& rhs) const
		{
			return !operator==(rhs);
		}
	};

	/** @} */

	BS_ALLOW_MEMCPY_SERIALIZATION(Vector3I)
}
