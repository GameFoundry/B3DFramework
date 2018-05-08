//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A four dimensional vector. */
	class BS_UTILITY_EXPORT Vector4
	{
	public:
		float x, y, z, w;

	public:
		Vector4() = default;

		constexpr Vector4(BS_ZERO)
			:x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{ }

		constexpr Vector4(float x, float y, float z, float w)
			:x(x), y(y), z(z), w(w)
		{ }

		constexpr explicit Vector4(const Vector3& vec, float w = 0.0f)
			:x(vec.x), y(vec.y), z(vec.z), w(w)
		{ }

		/** Exchange the contents of this vector with another. */
		void swap(Vector4& other)
		{
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
			std::swap(w, other.w);
		}

		float operator[] (UINT32 i) const
		{
			assert (i < 4);

			return *(&x+i);
		}

		float& operator[] (UINT32 i)
		{
			assert(i < 4);

			return *(&x+i);
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

		Vector4& operator= (float rhs)
		{
			x = rhs;
			y = rhs;
			z = rhs;
			w = rhs;

			return *this;
		}

		bool operator== (const Vector4& rhs) const
		{
			return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
		}

		bool operator!= (const Vector4& rhs) const
		{
			return (x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w);
		}

		Vector4& operator= (const Vector3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = 1.0f;

			return *this;
		}

		Vector4 operator+ (const Vector4& rhs) const
		{
			return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}

		Vector4 operator- (const Vector4& rhs) const
		{
			return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}

		Vector4 operator* (float rhs) const
		{
			return Vector4(x * rhs, y * rhs, z * rhs, w * rhs);
		}

		Vector4 operator* (const Vector4& rhs) const
		{
			return Vector4(rhs.x * x, rhs.y * y, rhs.z * z, rhs.w * w);
		}

		Vector4 operator/ (float rhs) const
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;
			return Vector4(x * inv, y * inv, z * inv, w * inv);
		}

		Vector4 operator/ (const Vector4& rhs) const
		{
			return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
		}

		const Vector4& operator+ () const
		{
			return *this;
		}

		Vector4 operator- () const
		{
			return Vector4(-x, -y, -z, -w);
		}

		friend Vector4 operator* (float lhs, const Vector4& rhs)
		{
			return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
		}

		friend Vector4 operator/ (float lhs, const Vector4& rhs)
		{
			return Vector4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
		}

		friend Vector4 operator+ (const Vector4& lhs, float rhs)
		{
			return Vector4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
		}

		friend Vector4 operator+ (float lhs, const Vector4& rhs)
		{
			return Vector4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
		}

		friend Vector4 operator- (const Vector4& lhs, float rhs)
		{
			return Vector4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
		}

		friend Vector4 operator- (float lhs, Vector4& rhs)
		{
			return Vector4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
		}

		Vector4& operator+= (const Vector4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;

			return *this;
		}

		Vector4& operator-= (const Vector4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;

			return *this;
		}

		Vector4& operator*= (float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;

			return *this;
		}

		Vector4& operator+= (float rhs)
		{
			x += rhs;
			y += rhs;
			z += rhs;
			w += rhs;

			return *this;
		}

		Vector4& operator-= (float rhs)
		{
			x -= rhs;
			y -= rhs;
			z -= rhs;
			w -= rhs;

			return *this;
		}

		Vector4& operator*= (Vector4& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;

			return *this;
		}

		Vector4& operator/= (float rhs)
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;

			x *= inv;
			y *= inv;
			z *= inv;
			w *= inv;

			return *this;
		}

		Vector4& operator/= (const Vector4& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;

			return *this;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		float dot(const Vector4& vec) const
		{
			return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
		}

		/** Checks are any of the vector components NaN. */
		inline bool isNaN() const;

		static const Vector4 ZERO;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector4);
	/** @endcond */
}

