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
		float X, Y, Z, W;

	public:
		Vector4() = default;

		constexpr Vector4(BS_ZERO)
			: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{}

		constexpr Vector4(float x, float y, float z, float w)
			: X(x), Y(y), Z(z), W(w)
		{}

		constexpr explicit Vector4(const Vector3& vec, float w = 0.0f)
			: X(vec.X), Y(vec.Y), Z(vec.Z), W(w)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(Vector4& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
			std::swap(W, other.W);
		}

		float operator[](u32 i) const
		{
			assert(i < 4);

			return *(&X + i);
		}

		float& operator[](u32 i)
		{
			assert(i < 4);

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

		Vector4& operator=(float rhs)
		{
			X = rhs;
			Y = rhs;
			Z = rhs;
			W = rhs;

			return *this;
		}

		bool operator==(const Vector4& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W);
		}

		bool operator!=(const Vector4& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y || Z != rhs.Z || W != rhs.W);
		}

		Vector4& operator=(const Vector3& rhs)
		{
			X = rhs.X;
			Y = rhs.Y;
			Z = rhs.Z;
			W = 1.0f;

			return *this;
		}

		Vector4 operator+(const Vector4& rhs) const
		{
			return Vector4(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
		}

		Vector4 operator-(const Vector4& rhs) const
		{
			return Vector4(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
		}

		Vector4 operator*(float rhs) const
		{
			return Vector4(X * rhs, Y * rhs, Z * rhs, W * rhs);
		}

		Vector4 operator*(const Vector4& rhs) const
		{
			return Vector4(rhs.X * X, rhs.Y * Y, rhs.Z * Z, rhs.W * W);
		}

		Vector4 operator/(float rhs) const
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;
			return Vector4(X * inv, Y * inv, Z * inv, W * inv);
		}

		Vector4 operator/(const Vector4& rhs) const
		{
			return Vector4(X / rhs.X, Y / rhs.Y, Z / rhs.Z, W / rhs.W);
		}

		const Vector4& operator+() const
		{
			return *this;
		}

		Vector4 operator-() const
		{
			return Vector4(-X, -Y, -Z, -W);
		}

		friend Vector4 operator*(float lhs, const Vector4& rhs)
		{
			return Vector4(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
		}

		friend Vector4 operator/(float lhs, const Vector4& rhs)
		{
			return Vector4(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z, lhs / rhs.W);
		}

		friend Vector4 operator+(const Vector4& lhs, float rhs)
		{
			return Vector4(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs, lhs.W + rhs);
		}

		friend Vector4 operator+(float lhs, const Vector4& rhs)
		{
			return Vector4(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z, lhs + rhs.W);
		}

		friend Vector4 operator-(const Vector4& lhs, float rhs)
		{
			return Vector4(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs, lhs.W - rhs);
		}

		friend Vector4 operator-(float lhs, Vector4& rhs)
		{
			return Vector4(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z, lhs - rhs.W);
		}

		Vector4& operator+=(const Vector4& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			W += rhs.W;

			return *this;
		}

		Vector4& operator-=(const Vector4& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			W -= rhs.W;

			return *this;
		}

		Vector4& operator*=(float rhs)
		{
			X *= rhs;
			Y *= rhs;
			Z *= rhs;
			W *= rhs;

			return *this;
		}

		Vector4& operator+=(float rhs)
		{
			X += rhs;
			Y += rhs;
			Z += rhs;
			W += rhs;

			return *this;
		}

		Vector4& operator-=(float rhs)
		{
			X -= rhs;
			Y -= rhs;
			Z -= rhs;
			W -= rhs;

			return *this;
		}

		Vector4& operator*=(Vector4& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;
			W *= rhs.W;

			return *this;
		}

		Vector4& operator/=(float rhs)
		{
			assert(rhs != 0.0f);

			float inv = 1.0f / rhs;

			X *= inv;
			Y *= inv;
			Z *= inv;
			W *= inv;

			return *this;
		}

		Vector4& operator/=(const Vector4& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;
			W /= rhs.W;

			return *this;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		float Dot(const Vector4& vec) const
		{
			return X * vec.X + Y * vec.Y + Z * vec.Z + W * vec.W;
		}

		/** Checks are any of the vector components NaN. */
		inline bool IsNaN() const;

		static const Vector4 kZero;
	};

	/** @} */
} // namespace bs
