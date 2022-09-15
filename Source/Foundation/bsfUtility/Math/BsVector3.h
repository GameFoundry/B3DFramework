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
		float x, y, z;

	public:
		Vector3() = default;

		constexpr Vector3(BS_ZERO)
			:x(0.0f), y(0.0f), z(0.0f)
		{ }

		constexpr Vector3(float x, float y, float z)
			:x(x), y(y), z(z)
		{ }

		explicit Vector3(const Vector4& vec);

		/** Exchange the contents of this vector with another. */
		void Swap(Vector3& other)
		{
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
		}

		float operator[] (UINT32 i) const
		{
			assert(i < 3);

			return *(&x + i);
		}

		float& operator[] (UINT32 i)
		{
			assert(i < 3);

			return *(&x + i);
		}

		/** Pointer accessor for direct copying. */
		float* Ptr()
		{
			return &x;
		}

		/** Pointer accessor for direct copying. */
		const float* Ptr() const
		{
			return &x;
		}

		Vector3& operator= (float rhs)
		{
			x = rhs;
			y = rhs;
			z = rhs;

			return *this;
		}

		bool operator== (const Vector3& rhs) const
		{
			return (x == rhs.x && y == rhs.y && z == rhs.z);
		}

		bool operator!= (const Vector3& rhs) const
		{
			return (x != rhs.x || y != rhs.y || z != rhs.z);
		}

		Vector3 operator+ (const Vector3& rhs) const
		{
			return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		Vector3 operator- (const Vector3& rhs) const
		{
			return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Vector3 operator* (float rhs) const
		{
			return Vector3(x * rhs, y * rhs, z * rhs);
		}

		Vector3 operator* (const Vector3& rhs) const
		{
			return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
		}

		Vector3 operator/ (float val) const
		{
			assert(val != 0.0);

			float fInv = 1.0f / val;
			return Vector3(x * fInv, y * fInv, z * fInv);
		}

		Vector3 operator/ (const Vector3& rhs) const
		{
			return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
		}

		const Vector3& operator+ () const
		{
			return *this;
		}

		Vector3 operator- () const
		{
			return Vector3(-x, -y, -z);
		}

		friend Vector3 operator* (float lhs, const Vector3& rhs)
		{
			return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
		}

		friend Vector3 operator/ (float lhs, const Vector3& rhs)
		{
			return Vector3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
		}

		friend Vector3 operator+ (const Vector3& lhs, float rhs)
		{
			return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
		}

		friend Vector3 operator+ (float lhs, const Vector3& rhs)
		{
			return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
		}

		friend Vector3 operator- (const Vector3& lhs, float rhs)
		{
			return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
		}

		friend Vector3 operator- (float lhs, const Vector3& rhs)
		{
			return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
		}

		Vector3& operator+= (const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}

		Vector3& operator+= (float rhs)
		{
			x += rhs;
			y += rhs;
			z += rhs;

			return *this;
		}

		Vector3& operator-= (const Vector3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}

		Vector3& operator-= (float rhs)
		{
			x -= rhs;
			y -= rhs;
			z -= rhs;

			return *this;
		}

		Vector3& operator*= (float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;

			return *this;
		}

		Vector3& operator*= (const Vector3& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;

			return *this;
		}

		Vector3& operator/= (float rhs)
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;

			x *= inv;
			y *= inv;
			z *= inv;

			return *this;
		}

		Vector3& operator/= (const Vector3& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;

			return *this;
		}

		/** Returns the length (magnitude) of the vector. */
		float Length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		float SquaredLength() const
		{
			return x * x + y * y + z * z;
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
			return x * vec.x + y * vec.y + z * vec.z;
		}

		/** 
		 * Normalizes this vector, and returns the previous length. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template<bool SAFE = true>
		float Normalize(float tolerance = 1e-04f)
		{
			float len = Length();
			if (!SAFE || len > (tolerance * tolerance))
				*this *= 1.0f / len;

			return len;
		}

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		Vector3 Cross(const Vector3& other) const
		{
			return Vector3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void Min(const Vector3& cmp)
		{
			if (cmp.x < x) x = cmp.x;
			if (cmp.y < y) y = cmp.y;
			if (cmp.z < z) z = cmp.z;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void Max(const Vector3& cmp)
		{
			if (cmp.x > x) x = cmp.x;
			if (cmp.y > y) y = cmp.y;
			if (cmp.z > z) z = cmp.z;
		}

		/** Generates a vector perpendicular to this vector. */
		Vector3 Perpendicular() const
		{
			static const float squareZero = (float)(1e-06 * 1e-06);

			Vector3 perp = this->Cross(Vector3::UNIT_X);

			if (perp.SquaredLength() < squareZero)
				perp = this->Cross(Vector3::UNIT_Y);

			perp.Normalize();
			return perp;
		}

		/** Gets the angle between 2 vectors. */
		inline Radian AngleBetween(const Vector3& dest) const;

		/** Returns true if this vector is zero length. */
		bool IsZeroLength(float tolerance = 1e-04f) const
		{
			float sqrdLen = x * x + y * y + z * z;
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
			if (std::abs(x) > std::abs(y))
				a = Vector3(-z, 0, x);
			else
				a = Vector3(0, z, -y);

			b = Cross(a);

			Orthonormalize(*this, a, b);
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void Orthonormalize(Vector3& vec0, Vector3& vec1, Vector3& vec2)
		{
			vec0.Normalize();

			float dot0 = vec0.Dot(vec1);
			vec1 -= dot0*vec0;
			vec1.Normalize();

			float dot1 = vec1.Dot(vec2);
			dot0 = vec0.Dot(vec2);
			vec2 -= dot0*vec0 + dot1*vec1;
			vec2.Normalize();
		}

		/** Calculates the dot (scalar) product of two vectors. */
		static float Dot(const Vector3& a, const Vector3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		/**
		 * Normalizes the provided vector and returns the result. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template<bool SAFE = true>
		static Vector3 Normalize(const Vector3& v, float tolerance = 1e-04f)
		{
			float sqrdLen = Dot(v, v);
			if (!SAFE || sqrdLen > tolerance)
				return v * Math::InvSqrt(sqrdLen);

			return v;
		}

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		static Vector3 Cross(const Vector3& a, const Vector3& b)
		{
			return Vector3(
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x);
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
			return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static Vector3 Max(const Vector3& a, const Vector3& b)
		{
			return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
		}

		static const Vector3 ZERO;
		static const Vector3 ONE;
		static const Vector3 INF;
		static const Vector3 UNIT_X;
		static const Vector3 UNIT_Y;
		static const Vector3 UNIT_Z;
	};

	/** @} */
}

/** @cond SPECIALIZATIONS */
namespace std
{
	template<> class numeric_limits<bs::Vector3>
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
}
/** @endcond */
