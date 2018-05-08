//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cmath>

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsRadian.h"

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
		void swap(Vector3& other)
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
		float* ptr()
		{
			return &x;
		}

		/** Pointer accessor for direct copying. */
		const float* ptr() const
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
		float length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		float squaredLength() const
		{
			return x * x + y * y + z * z;
		}

		/**	Returns the distance to another vector. */
		float distance(const Vector3& rhs) const
		{
			return (*this - rhs).length();
		}

		/** Returns the square of the distance to another vector. */
		float squaredDistance(const Vector3& rhs) const
		{
			return (*this - rhs).squaredLength();
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		float dot(const Vector3& vec) const
		{
			return x * vec.x + y * vec.y + z * vec.z;
		}

		/** Normalizes the vector. */
		float normalize()
		{
			float len = length();

			// Will also work for zero-sized vectors, but will change nothing
			if (len > 1e-08f)
			{
				float invLen = 1.0f / len;
				x *= invLen;
				y *= invLen;
				z *= invLen;
			}
			return len;
		}


		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		Vector3 cross(const Vector3& other) const
		{
			return Vector3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void floor(const Vector3& cmp)
		{
			if (cmp.x < x) x = cmp.x;
			if (cmp.y < y) y = cmp.y;
			if (cmp.z < z) z = cmp.z;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void ceil(const Vector3& cmp)
		{
			if (cmp.x > x) x = cmp.x;
			if (cmp.y > y) y = cmp.y;
			if (cmp.z > z) z = cmp.z;
		}

		/** Generates a vector perpendicular to this vector. */
		Vector3 perpendicular() const
		{
			static const float squareZero = (float)(1e-06 * 1e-06);

			Vector3 perp = this->cross(Vector3::UNIT_X);

			if (perp.squaredLength() < squareZero)
				perp = this->cross(Vector3::UNIT_Y);

			perp.normalize();
			return perp;
		}

		/** Gets the angle between 2 vectors. */
		inline Radian angleBetween(const Vector3& dest) const;

		/** Returns true if this vector is zero length. */
		bool isZeroLength() const
		{
			float sqlen = (x * x) + (y * y) + (z * z);
			return (sqlen < (1e-06f * 1e-06f));
		}

		/** Calculates a reflection vector to the plane with the given normal. */
		Vector3 reflect(const Vector3& normal) const
		{
			return Vector3(*this - (2 * this->dot(normal) * normal));
		}

		/** Calculates two vectors orthonormal to the current vector, and normalizes the current vector if not already. */
		void orthogonalComplement(Vector3& a, Vector3& b)
		{
			if (std::abs(x) > std::abs(y))
				a = Vector3(-z, 0, x);
			else
				a = Vector3(0, z, -y);

			b = cross(a);

			orthonormalize(*this, a, b);
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void orthonormalize(Vector3& vec0, Vector3& vec1, Vector3& vec2)
		{
			vec0.normalize();

			float dot0 = vec0.dot(vec1);
			vec1 -= dot0*vec0;
			vec1.normalize();

			float dot1 = vec1.dot(vec2);
			dot0 = vec0.dot(vec2);
			vec2 -= dot0*vec0 + dot1*vec1;
			vec2.normalize();
		}

		/** Calculates the dot (scalar) product of two vectors. */
		static float dot(const Vector3& a, const Vector3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		/** Normalizes the provided vector and returns a new normalized instance. */
		static Vector3 normalize(const Vector3& val);

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		static Vector3 cross(const Vector3& a, const Vector3& b)
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
		static Vector3 lerp(float t, const Vector3& a, const Vector3& b)
		{
			return (1.0f - t) * a + t * b;
		}

		/** Checks are any of the vector components not a number. */
		inline bool isNaN() const;

		/** Returns the minimum of all the vector components as a new vector. */
		static Vector3 min(const Vector3& a, const Vector3& b)
		{
			return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static Vector3 max(const Vector3& a, const Vector3& b)
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

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector3);
	/** @endcond */
}
