//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A four dimensional vector with integer coordinates. */
	struct BS_SCRIPT_EXPORT(m:Math,pl:true) Vector4I
	{
		INT32 x = 0;
		INT32 y = 0;
		INT32 z = 0;
		INT32 w = 0;

		constexpr Vector4I() = default;

		constexpr Vector4I(INT32 x, INT32 y, INT32 z, INT32 w)
			:x(x), y(y), z(z), w(w)
		{ }

		INT32 operator[] (size_t i) const
		{
			assert(i < 4);

			switch (i)
			{
				case 0: return x;
				case 1: return y;
				case 2: return z;
				case 3: return w;
				default: return 0;
			}
		}

		INT32& operator[] (size_t i)
		{
			assert(i < 4);

			switch (i)
			{
				case 0: return x;
				case 1: return y;
				case 2: return z;
				case 3: return w;
				default: return x;
			}
		}

		bool operator== (const Vector4I& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
		}

		bool operator!= (const Vector4I& rhs) const
		{
			return !operator==(rhs);
		}
	};

	/** @} */

	BS_ALLOW_MEMCPY_SERIALIZATION(Vector4I)
}
