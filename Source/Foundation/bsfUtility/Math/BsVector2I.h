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
	template<class T>
	struct TVector2I
	{
		T X = 0;
		T Y = 0;

		constexpr TVector2I() = default;

		constexpr TVector2I(T x, T y)
			: X(x), Y(y)
		{}

		constexpr explicit TVector2I(T value)
			: X(value), Y(value)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(TVector2I& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
		}

		Vector2 ToFloat() const
		{
			return Vector2((float)X, (float)Y);
		}

		/** Returns the manhattan distance between this and another point. */
		T CalculateManhattanDistance(const TVector2I& other) const
		{
			return (T)std::abs((i32)(other.X - X)) + (T)std::abs((i32)(other.Y - Y));
		}

		T operator[](size_t index) const
		{
			B3D_ASSERT(index < 2);

			return *(&X + index);
		}

		T& operator[](size_t index)
		{
			B3D_ASSERT(index < 2);

			return *(&X + index);
		}

		TVector2I& operator=(T val)
		{
			X = val;
			Y = val;

			return *this;
		}

		bool operator==(const TVector2I& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y);
		}

		bool operator!=(const TVector2I& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y);
		}

		TVector2I operator+(const TVector2I& rhs) const
		{
			return TVector2I(X + rhs.X, Y + rhs.Y);
		}

		TVector2I operator-(const TVector2I& rhs) const
		{
			return TVector2I(X - rhs.X, Y - rhs.Y);
		}

		TVector2I operator*(T value) const
		{
			return TVector2I(X * value, Y * value);
		}

		Vector2 operator*(float value) const
		{
			return Vector2((float)X * value, (float)Y * value);
		}

		TVector2I operator*(const TVector2I& rhs) const
		{
			return TVector2I(X * rhs.X, Y * rhs.Y);
		}

		TVector2I operator/(T value) const
		{
			B3D_ASSERT(value != 0);

			return TVector2I(X / value, Y / value);
		}

		Vector2 operator/(float value) const
		{
			B3D_ASSERT(value != 0.0f);

			return Vector2((float)X / value, (float)Y / value);
		}

		TVector2I operator/(const TVector2I& rhs) const
		{
			return TVector2I(X / rhs.X, Y / rhs.Y);
		}

		const TVector2I& operator+() const
		{
			return *this;
		}

		TVector2I operator-() const
		{
			return TVector2I(-X, -Y);
		}

		friend TVector2I operator*(T lhs, const TVector2I& rhs)
		{
			return TVector2I(lhs * rhs.X, lhs * rhs.Y);
		}

		friend TVector2I operator/(T lhs, const TVector2I& rhs)
		{
			return TVector2I(lhs / rhs.X, lhs / rhs.Y);
		}

		TVector2I& operator+=(const TVector2I& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;

			return *this;
		}

		TVector2I& operator-=(const TVector2I& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;

			return *this;
		}

		TVector2I& operator*=(T val)
		{
			X *= val;
			Y *= val;

			return *this;
		}

		TVector2I& operator*=(const TVector2I& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;

			return *this;
		}

		TVector2I& operator/=(T value)
		{
			B3D_ASSERT(value != 0);

			X /= value;
			Y /= value;

			return *this;
		}

		TVector2I& operator/=(const TVector2I& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;

			return *this;
		}

		/** Returns the square of the length(magnitude) of the vector. */
		T SquaredLength() const
		{
			return X * X + Y * Y;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector2I& other) const
		{
			return X * other.X + Y * other.Y;
		}

		static const TVector2I kZero;
	};

	template<> const TVector2I<i32> TVector2I<i32>::kZero;
	template<> const TVector2I<u32> TVector2I<u32>::kZero;

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector2I)) TVector2I<i32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector2UI)) TVector2I<u32>;

	/** @} */
} // namespace bs
