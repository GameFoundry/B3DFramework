//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "BsVector2.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A two dimensional vector with integer coordinates.*/
	struct BS_UTILITY_EXPORT Vector2I
	{
		INT32 x = 0;
		INT32 y = 0;

		constexpr Vector2I() = default;

		constexpr Vector2I(INT32 _x, INT32 _y )
			:x(_x), y(_y)
		{ }

		constexpr explicit Vector2I(int val)
			:x(val), y(val)
		{ }

		/** Exchange the contents of this vector with another. */
		void swap(Vector2I& other)
		{
			std::swap(x, other.x);
			std::swap(y, other.y);
		}

		/** Returns the manhattan distance between this and another point. */
		UINT32 manhattanDist(const Vector2I& other) const
		{
			return (UINT32)std::abs(other.x - x) + (UINT32)std::abs(other.y - y);
		}

		INT32 operator[] (size_t i) const
		{
			assert(i < 2);

			return *(&x+i);
		}

		INT32& operator[] (size_t i)
		{
			assert(i < 2);

			return *(&x+i);
		}

		Vector2I& operator= (int val)
		{
			x = val;
			y = val;

			return *this;
		}

		bool operator== (const Vector2I& rhs) const
		{
			return (x == rhs.x && y == rhs.y);
		}

		bool operator!= (const Vector2I& rhs) const
		{
			return (x != rhs.x || y != rhs.y);
		}

		Vector2I operator+ (const Vector2I& rhs) const
		{
			return Vector2I(x + rhs.x, y + rhs.y);
		}

		Vector2I operator- (const Vector2I& rhs) const
		{
			return Vector2I(x - rhs.x, y - rhs.y);
		}

		Vector2I operator* (int val) const
		{
			return Vector2I(x * val, y * val);
		}

		Vector2 operator* (float val) const
		{
			return Vector2(x * val, y * val);
		}

		Vector2I operator* (const Vector2I& rhs) const
		{
			return Vector2I(x * rhs.x, y * rhs.y);
		}

		Vector2I operator/ (int val) const
		{
			assert(val != 0);

			return Vector2I(x / val, y / val);
		}

		Vector2 operator/ (float val) const
		{
			assert(val != 0);

			return Vector2(x / val, y / val);
		}

		Vector2I operator/ (const Vector2I& rhs) const
		{
			return Vector2I(x / rhs.x, y / rhs.y);
		}

		const Vector2I& operator+ () const
		{
			return *this;
		}

		Vector2I operator- () const
		{
			return Vector2I(-x, -y);
		}

		friend Vector2I operator* (int lhs, const Vector2I& rhs)
		{
			return Vector2I(lhs * rhs.x, lhs * rhs.y);
		}

		friend Vector2I operator/ (int lhs, const Vector2I& rhs)
		{
			return Vector2I(lhs / rhs.x, lhs / rhs.y);
		}

		Vector2I& operator+= (const Vector2I& rhs)
		{
			x += rhs.x;
			y += rhs.y;

			return *this;
		}

		Vector2I& operator-= (const Vector2I& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;

			return *this;
		}

		Vector2I& operator*= (INT32 val)
		{
			x *= val;
			y *= val;

			return *this;
		}

		Vector2I& operator*= (const Vector2I& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;

			return *this;
		}

		Vector2I& operator/= (INT32 val)
		{
			assert(val != 0);

			x /= val;
			y /= val;

			return *this;
		}

		Vector2I& operator/= (const Vector2I& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;

			return *this;
		}

		/** Returns the square of the length(magnitude) of the vector. */
		INT32 squaredLength() const
		{
			return x * x + y * y;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		INT32 dot(const Vector2I& vec) const
		{
			return x * vec.x + y * vec.y;
		}

		static const Vector2I ZERO;
	};

	/** @} */
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector2I)
}
