//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cmath>

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsRadian.h"
#include "BsMath.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A three dimensional vector. */
	class BS_UTILITY_EXPORT Vector3
	{
	public:
		float X, Y, Z;

	public:
		Vector3() = default;

		constexpr Vector3(BS_ZERO)
			: X(0.0f), Y(0.0f), Z(0.0f)
		{}

		constexpr Vector3(float x, float y, float z)
			: X(x), Y(y), Z(z)
		{}

		explicit Vector3(const Vector4& vec);

		/** Exchange the contents of this vector with another. */
		void Swap(Vector3& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
		}

		float operator[](u32 i) const
		{
			assert(i < 3);

			return *(&X + i);
		}

		float& operator[](u32 i)
		{
			assert(i < 3);

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

		Vector3& operator=(float rhs)
		{
			X = rhs;
			Y = rhs;
			Z = rhs;

			return *this;
		}

		bool operator==(const Vector3& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z);
		}

		bool operator!=(const Vector3& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y || Z != rhs.Z);
		}

		Vector3 operator+(const Vector3& rhs) const
		{
			return Vector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
		}

		Vector3 operator-(const Vector3& rhs) const
		{
			return Vector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
		}

		Vector3 operator*(float rhs) const
		{
			return Vector3(X * rhs, Y * rhs, Z * rhs);
		}

		Vector3 operator*(const Vector3& rhs) const
		{
			return Vector3(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
		}

		Vector3 operator/(float val) const
		{
			assert(val != 0.0);

			float fInv = 1.0f / val;
			return Vector3(X * fInv, Y * fInv, Z * fInv);
		}

		Vector3 operator/(const Vector3& rhs) const
		{
			return Vector3(X / rhs.X, Y / rhs.Y, Z / rhs.Z);
		}

		const Vector3& operator+() const
		{
			return *this;
		}

		Vector3 operator-() const
		{
			return Vector3(-X, -Y, -Z);
		}

		friend Vector3 operator*(float lhs, const Vector3& rhs)
		{
			return Vector3(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
		}

		friend Vector3 operator/(float lhs, const Vector3& rhs)
		{
			return Vector3(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z);
		}

		friend Vector3 operator+(const Vector3& lhs, float rhs)
		{
			return Vector3(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs);
		}

		friend Vector3 operator+(float lhs, const Vector3& rhs)
		{
			return Vector3(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z);
		}

		friend Vector3 operator-(const Vector3& lhs, float rhs)
		{
			return Vector3(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs);
		}

		friend Vector3 operator-(float lhs, const Vector3& rhs)
		{
			return Vector3(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z);
		}

		Vector3& operator+=(const Vector3& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;

			return *this;
		}

		Vector3& operator+=(float rhs)
		{
			X += rhs;
			Y += rhs;
			Z += rhs;

			return *this;
		}

		Vector3& operator-=(const Vector3& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;

			return *this;
		}

		Vector3& operator-=(float rhs)
		{
			X -= rhs;
			Y -= rhs;
			Z -= rhs;

			return *this;
		}

		Vector3& operator*=(float rhs)
		{
			X *= rhs;
			Y *= rhs;
			Z *= rhs;

			return *this;
		}

		Vector3& operator*=(const Vector3& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;

			return *this;
		}

		Vector3& operator/=(float rhs)
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;

			X *= inv;
			Y *= inv;
			Z *= inv;

			return *this;
		}

		Vector3& operator/=(const Vector3& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;

			return *this;
		}

		/** Returns the length (magnitude) of the vector. */
		float Length() const
		{
			return std::sqrt(X * X + Y * Y + Z * Z);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		float SquaredLength() const
		{
			return X * X + Y * Y + Z * Z;
		}

		/**	Returns the distance to another vector. */
		float Distance(const Vector3& rhs) const
		{
			return (*this - rhs).Length();
		}

		/** Returns the square of the distance to another vector. */
		float SquaredDistance(const Vector3& rhs) const
		{
			return (*this - rhs).SquaredLength();
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		float Dot(const Vector3& vec) const
		{
			return X * vec.X + Y * vec.Y + Z * vec.Z;
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

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		Vector3 Cross(const Vector3& other) const
		{
			return Vector3(
				Y * other.Z - Z * other.Y,
				Z * other.X - X * other.Z,
				X * other.Y - Y * other.X);
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void Min(const Vector3& cmp)
		{
			if(cmp.X < X) X = cmp.X;
			if(cmp.Y < Y) Y = cmp.Y;
			if(cmp.Z < Z) Z = cmp.Z;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void Max(const Vector3& cmp)
		{
			if(cmp.X > X) X = cmp.X;
			if(cmp.Y > Y) Y = cmp.Y;
			if(cmp.Z > Z) Z = cmp.Z;
		}

		/** Generates a vector perpendicular to this vector. */
		Vector3 Perpendicular() const
		{
			static const float kSquareZero = (float)(1e-06 * 1e-06);

			Vector3 perp = this->Cross(Vector3::kUnitX);

			if(perp.SquaredLength() < kSquareZero)
				perp = this->Cross(Vector3::kUnitY);

			perp.Normalize();
			return perp;
		}

		/** Gets the angle between 2 vectors. */
		inline Radian AngleBetween(const Vector3& dest) const;

		/** Returns true if this vector is zero length. */
		bool IsZeroLength(float tolerance = 1e-04f) const
		{
			float sqrdLen = X * X + Y * Y + Z * Z;
			return sqrdLen < tolerance;
		}

		/** Calculates a reflection vector to the plane with the given normal. */
		Vector3 Reflect(const Vector3& normal) const
		{
			return Vector3(*this - (2 * this->Dot(normal) * normal));
		}

		/** Calculates two vectors orthonormal to the current vector, and normalizes the current vector if not already. */
		void OrthogonalComplement(Vector3& a, Vector3& b)
		{
			if(std::abs(X) > std::abs(Y))
				a = Vector3(-Z, 0, X);
			else
				a = Vector3(0, Z, -Y);

			b = Cross(a);

			Orthonormalize(*this, a, b);
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void Orthonormalize(Vector3& vec0, Vector3& vec1, Vector3& vec2)
		{
			vec0.Normalize();

			float dot0 = vec0.Dot(vec1);
			vec1 -= dot0 * vec0;
			vec1.Normalize();

			float dot1 = vec1.Dot(vec2);
			dot0 = vec0.Dot(vec2);
			vec2 -= dot0 * vec0 + dot1 * vec1;
			vec2.Normalize();
		}

		/** Calculates the dot (scalar) product of two vectors. */
		static float Dot(const Vector3& a, const Vector3& b)
		{
			return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
		}

		/**
		 * Normalizes the provided vector and returns the result. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		static Vector3 Normalize(const Vector3& v, float tolerance = 1e-04f)
		{
			float sqrdLen = Dot(v, v);
			if(!SAFE || sqrdLen > tolerance)
				return v * Math::InvSqrt(sqrdLen);

			return v;
		}

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		static Vector3 Cross(const Vector3& a, const Vector3& b)
		{
			return Vector3(
				a.Y * b.Z - a.Z * b.Y,
				a.Z * b.X - a.X * b.Z,
				a.X * b.Y - a.Y * b.X);
		}

		/**
		 * Linearly interpolates between the two vectors using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to the left vector, while t = 1 corresponds to the right vector.
		 */
		static Vector3 Lerp(float t, const Vector3& a, const Vector3& b)
		{
			return (1.0f - t) * a + t * b;
		}

		/** Checks are any of the vector components not a number. */
		inline bool IsNaN() const;

		/** Returns the minimum of all the vector components as a new vector. */
		static Vector3 Min(const Vector3& a, const Vector3& b)
		{
			return Vector3(std::min(a.X, b.X), std::min(a.Y, b.Y), std::min(a.Z, b.Z));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static Vector3 Max(const Vector3& a, const Vector3& b)
		{
			return Vector3(std::max(a.X, b.X), std::max(a.Y, b.Y), std::max(a.Z, b.Z));
		}

		static const Vector3 kZero;
		static const Vector3 kOne;
		static const Vector3 kInf;
		static const Vector3 kUnitX;
		static const Vector3 kUnitY;
		static const Vector3 kUnitZ;
	};

	/** @} */
} // namespace bs

/** @cond SPECIALIZATIONS */
namespace std
{
	template <>
	class numeric_limits<bs::Vector3>
	{
	public:
		constexpr static bs::Vector3 infinity() // NOLINT
		{
			return bs::Vector3(
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity());
		}
	};
} // namespace std

/** @endcond */
