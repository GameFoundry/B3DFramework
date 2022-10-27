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
		i32 X = 0;
		i32 Y = 0;

		constexpr Vector2I() = default;

		constexpr Vector2I(i32 _x, i32 _y)
			: X(_x), Y(_y)
		{}

		constexpr explicit Vector2I(int val)
			: X(val), Y(val)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(Vector2I& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
		}

		/** Returns the manhattan distance between this and another point. */
		u32 ManhattanDist(const Vector2I& other) const
		{
			return (u32)std::abs(other.X - X) + (u32)std::abs(other.Y - Y);
		}

		i32 operator[](size_t i) const
		{
			assert(i < 2);

			return *(&X + i);
		}

		i32& operator[](size_t i)
		{
			assert(i < 2);

			return *(&X + i);
		}

		Vector2I& operator=(int val)
		{
			X = val;
			Y = val;

			return *this;
		}

		bool operator==(const Vector2I& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y);
		}

		bool operator!=(const Vector2I& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y);
		}

		Vector2I operator+(const Vector2I& rhs) const
		{
			return Vector2I(X + rhs.X, Y + rhs.Y);
		}

		Vector2I operator-(const Vector2I& rhs) const
		{
			return Vector2I(X - rhs.X, Y - rhs.Y);
		}

		Vector2I operator*(int val) const
		{
			return Vector2I(X * val, Y * val);
		}

		Vector2 operator*(float val) const
		{
			return Vector2(X * val, Y * val);
		}

		Vector2I operator*(const Vector2I& rhs) const
		{
			return Vector2I(X * rhs.X, Y * rhs.Y);
		}

		Vector2I operator/(int val) const
		{
			assert(val != 0);

			return Vector2I(X / val, Y / val);
		}

		Vector2 operator/(float val) const
		{
			assert(val != 0);

			return Vector2(X / val, Y / val);
		}

		Vector2I operator/(const Vector2I& rhs) const
		{
			return Vector2I(X / rhs.X, Y / rhs.Y);
		}

		const Vector2I& operator+() const
		{
			return *this;
		}

		Vector2I operator-() const
		{
			return Vector2I(-X, -Y);
		}

		friend Vector2I operator*(int lhs, const Vector2I& rhs)
		{
			return Vector2I(lhs * rhs.X, lhs * rhs.Y);
		}

		friend Vector2I operator/(int lhs, const Vector2I& rhs)
		{
			return Vector2I(lhs / rhs.X, lhs / rhs.Y);
		}

		Vector2I& operator+=(const Vector2I& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;

			return *this;
		}

		Vector2I& operator-=(const Vector2I& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;

			return *this;
		}

		Vector2I& operator*=(i32 val)
		{
			X *= val;
			Y *= val;

			return *this;
		}

		Vector2I& operator*=(const Vector2I& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;

			return *this;
		}

		Vector2I& operator/=(i32 val)
		{
			assert(val != 0);

			X /= val;
			Y /= val;

			return *this;
		}

		Vector2I& operator/=(const Vector2I& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;

			return *this;
		}

		/** Returns the square of the length(magnitude) of the vector. */
		i32 SquaredLength() const
		{
			return X * X + Y * Y;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		i32 Dot(const Vector2I& vec) const
		{
			return X * vec.X + Y * vec.Y;
		}

		static const Vector2I kZero;
	};

	/** @} */
} // namespace bs
