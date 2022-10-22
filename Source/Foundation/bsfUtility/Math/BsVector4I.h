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
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Math),pl:true) Vector4I
	{
		i32 X = 0;
		i32 Y = 0;
		i32 Z = 0;
		i32 W = 0;

		constexpr Vector4I() = default;

		constexpr Vector4I(i32 x, i32 y, i32 z, i32 w)
			:X(x), Y(y), Z(z), W(w)
		{ }

		i32 operator[] (size_t i) const
		{
			assert(i < 4);

			switch (i)
			{
				case 0: return X;
				case 1: return Y;
				case 2: return Z;
				case 3: return W;
				default: return 0;
			}
		}

		i32& operator[] (size_t i)
		{
			assert(i < 4);

			switch (i)
			{
				case 0: return X;
				case 1: return Y;
				case 2: return Z;
				case 3: return W;
				default: return X;
			}
		}

		bool operator== (const Vector4I& rhs) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W;
		}

		bool operator!= (const Vector4I& rhs) const
		{
			return !operator==(rhs);
		}
	};

	/** @} */
}
