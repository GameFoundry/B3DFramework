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
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Math),ExportAsStruct(true)) Vector3I
	{
		i32 X = 0;
		i32 Y = 0;
		i32 Z = 0;

		constexpr Vector3I() = default;

		constexpr Vector3I(i32 x, i32 y, i32 z)
			:X(x), Y(y), Z(z)
		{ }

		i32 operator[] (size_t i) const
		{
			assert(i < 3);

			switch (i)
			{
				case 0: return X;
				case 1: return Y;
				case 2: return Z;
				default: return 0;
			}
		}

		i32& operator[] (size_t i)
		{
			assert(i < 3);

			switch (i)
			{
				case 0: return X;
				case 1: return Y;
				case 2: return Z;
				default: return X;
			}
		}

		bool operator== (const Vector3I& rhs) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z;
		}

		bool operator!= (const Vector3I& rhs) const
		{
			return !operator==(rhs);
		}
	};

	/** @} */
}
