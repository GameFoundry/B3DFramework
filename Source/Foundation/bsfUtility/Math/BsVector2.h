//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A two dimensional vector. */
	class BS_UTILITY_EXPORT Vector2
	{
	public:
		float X, Y;

	public:
		Vector2() = default;

		constexpr Vector2(BS_ZERO)
			: X(0.0f), Y(0.0f)
		{}

		constexpr Vector2(float x, float y)
			: X(x), Y(y)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(Vector2& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
		}

		float operator[](u32 i) const
		{
			assert(i < 2);

			return *(&X + i);
		}

		float& operator[](u32 i)
		{
			assert(i < 2);

			return *(&X + i);
		}

		/** Pointer accessor for direct copying. */
		float* Ptr()
		{
			return &X;
		}

		/** Pointer accessor for direct copying. */
		const float* Ptr() const
		{
			return &X;
		}

		Vector2& operator=(float rhs)
		{
			X = rhs;
			Y = rhs;

			return *this;
		}

		bool operator==(const Vector2& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y);
		}

		bool operator!=(const Vector2& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y);
		}

		Vector2 operator+(const Vector2& rhs) const
		{
			return Vector2(X + rhs.X, Y + rhs.Y);
		}

		Vector2 operator-(const Vector2& rhs) const
		{
			return Vector2(X - rhs.X, Y - rhs.Y);
		}

		Vector2 operator*(const float rhs) const
		{
			return Vector2(X * rhs, Y * rhs);
		}

		Vector2 operator*(const Vector2& rhs) const
		{
			return Vector2(X * rhs.X, Y * rhs.Y);
		}

		Vector2 operator/(const float rhs) const
		{
			assert(rhs != 0.0);

			float fInv = 1.0f / rhs;

			return Vector2(X * fInv, Y * fInv);
		}

		Vector2 operator/(const Vector2& rhs) const
		{
			return Vector2(X / rhs.X, Y / rhs.Y);
		}

		const Vector2& operator+() const
		{
			return *this;
		}

		Vector2 operator-() const
		{
			return Vector2(-X, -Y);
		}

		friend Vector2 operator*(float lhs, const Vector2& rhs)
		{
			return Vector2(lhs * rhs.X, lhs * rhs.Y);
		}

		friend Vector2 operator/(float lhs, const Vector2& rhs)
		{
			return Vector2(lhs / rhs.X, lhs / rhs.Y);
		}

		friend Vector2 operator+(Vector2& lhs, float rhs)
		{
			return Vector2(lhs.X + rhs, lhs.Y + rhs);
		}

		friend Vector2 operator+(float lhs, const Vector2& rhs)
		{
			return Vector2(lhs + rhs.X, lhs + rhs.Y);
		}

		friend Vector2 operator-(const Vector2& lhs, float rhs)
		{
			return Vector2(lhs.X - rhs, lhs.Y - rhs);
		}

		friend Vector2 operator-(const float lhs, const Vector2& rhs)
		{
			return Vector2(lhs - rhs.X, lhs - rhs.Y);
		}

		Vector2& operator+=(const Vector2& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;

			return *this;
		}

		Vector2& operator+=(float rhs)
		{
			X += rhs;
			Y += rhs;

			return *this;
		}

		Vector2& operator-=(const Vector2& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;

			return *this;
		}

		Vector2& operator-=(float rhs)
		{
			X -= rhs;
			Y -= rhs;

			return *this;
		}

		Vector2& operator*=(float rhs)
		{
			X *= rhs;
			Y *= rhs;

			return *this;
		}

		Vector2& operator*=(const Vector2& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;

			return *this;
		}

		Vector2& operator/=(float rhs)
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;

			X *= inv;
			Y *= inv;

			return *this;
		}

		Vector2& operator/=(const Vector2& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;

			return *this;
		}

		/** Returns the length (magnitude) of the vector. */
		float Length() const
		{
			return Math::Sqrt(X * X + Y * Y);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		float SquaredLength() const
		{
			return X * X + Y * Y;
		}

		/** Returns the distance to another vector. */
		float Distance(const Vector2& rhs) const
		{
			return (*this - rhs).Length();
		}

		/** Returns the square of the distance to another vector. */
		float SqrdDistance(const Vector2& rhs) const
		{
			return (*this - rhs).SquaredLength();
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		float Dot(const Vector2& vec) const
		{
			return X * vec.X + Y * vec.Y;
		}

		/**
		 * Normalizes this vector, and returns the previous length. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		float Normalize(float tolerance = 1e-04f)
		{
			float len = Length();
			if(!SAFE || len > (tolerance * tolerance))
				*this *= 1.0f / len;

			return len;
		}

		/** Generates a vector perpendicular to this vector. */
		Vector2 Perpendicular() const
		{
			return Vector2(-Y, X);
		}

		/**
		 * Calculates the 2 dimensional cross-product of 2 vectors, which results in a single floating point value which
		 * is 2 times the area of the triangle.
		 */
		float Cross(const Vector2& other) const
		{
			return X * other.Y - Y * other.X;
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void Floor(const Vector2& cmp)
		{
			if(cmp.X < X) X = cmp.X;
			if(cmp.Y < Y) Y = cmp.Y;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void Ceil(const Vector2& cmp)
		{
			if(cmp.X > X) X = cmp.X;
			if(cmp.Y > Y) Y = cmp.Y;
		}

		/** Returns true if this vector is zero length. */
		bool IsZeroLength(float tolerance = 1e-04f) const
		{
			float sqrdLen = X * X + Y * Y;
			return sqrdLen < tolerance;
		}

		/** Calculates a reflection vector to the plane with the given normal. */
		Vector2 Reflect(const Vector2& normal) const
		{
			return Vector2(*this - (2 * this->Dot(normal) * normal));
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void Orthonormalize(Vector2& u, Vector2& v)
		{
			u.Normalize();

			float dot = u.Dot(v);
			v -= u * dot;
			v.Normalize();
		}

		/**
		 * Normalizes the provided vector and returns the result. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		static Vector2 Normalize(const Vector2& v, float tolerance = 1e-04f)
		{
			float sqrdLen = v.X * v.X + v.Y * v.Y;
			if(!SAFE || sqrdLen > tolerance)
				return v * Math::InvSqrt(sqrdLen);

			return v;
		}

		/** Checks are any of the vector components NaN. */
		bool IsNaN() const
		{
			return Math::IsNaN(X) || Math::IsNaN(Y);
		}

		/** Returns the minimum of all the vector components as a new vector. */
		static Vector2 Min(const Vector2& a, const Vector2& b)
		{
			return Vector2(std::min(a.X, b.X), std::min(a.Y, b.Y));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static Vector2 Max(const Vector2& a, const Vector2& b)
		{
			return Vector2(std::max(a.X, b.X), std::max(a.Y, b.Y));
		}

		static const Vector2 kZero;
		static const Vector2 kOne;
		static const Vector2 kUnitX;
		static const Vector2 kUnitY;
	};

	/** @} */
} // namespace bs
