//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector2I.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A N dimensional vector with integer coordinates. */
	template<int N>
	struct VectorNI
	{
		INT32 v[N];

		VectorNI()
		{ 
			memset(v, 0, sizeof(v));
		}

		VectorNI(INT32 val[N])
		{ 
			memcpy(v, val, sizeof(v));
		}

		VectorNI(std::initializer_list<INT32> list)
		{
			assert(list.size() <= N);
			std::copy(list.begin(), list.end(), v);
		}

		INT32 operator[] (size_t i) const
		{
			assert(i < N);

			return v[i];
		}

		INT32& operator[] (size_t i)
		{
			assert(i < N);

			return v[i];
		}

		VectorNI& operator= (const VectorNI& rhs)
		{
			memcpy(v, rhs.v, sizeof(v));

			return *this;
		}

		bool operator== (const Vector2I& rhs) const
		{
			for (UINT32 i = 0; i < N; i++)
			{
				if (v[i] != rhs[i])
					return false;
			}

			return true;
		}

		bool operator!= (const Vector2I& rhs) const
		{
			return !operator==(*this, rhs);
		}
	};

	/** @} */

	typedef VectorNI<3> Vector3I;
	typedef VectorNI<4> Vector4I;

	BS_ALLOW_MEMCPY_SERIALIZATION(Vector3I)
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector4I)
}
