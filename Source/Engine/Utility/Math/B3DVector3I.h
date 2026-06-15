//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A three dimensional vector with integer coordinates. */
	template<class T>
	struct TVector3I
	{
		T X = (T)0;
		T Y = (T)0;
		T Z = (T)0;

		constexpr TVector3I() = default;

		constexpr TVector3I(T x, T y, T z)
			: X(x), Y(y), Z(z)
		{}

		constexpr explicit TVector3I(T value)
			: X(value), Y(value), Z(value)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(TVector3I& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
		}

		Vector3 ToFloat() const
		{
			return Vector3((float)X, (float)Y, (float)Z);
		}

		/** Returns the manhattan distance between this and another point. */
		T CalculateManhattanDistance(const TVector3I& other) const
		{
			return (T)std::abs((i32)(other.X - X)) + (T)std::abs((i32)(other.Y - Y)) + (T)std::abs((i32)(other.Z - Z));
		}

		T operator[](size_t index) const
		{
			B3D_ASSERT(index < 3);

			return *(&X + index);
		}

		T& operator[](size_t index)
		{
			B3D_ASSERT(index < 3);

			return *(&X + index);
		}

		TVector3I& operator=(T value)
		{
			X = value;
			Y = value;
			Z = value;

			return *this;
		}

		bool operator==(const TVector3I& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z);
		}

		bool operator!=(const TVector3I& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y) || Z != rhs.Z;
		}

		TVector3I operator+(const TVector3I& rhs) const
		{
			return TVector3I(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
		}

		TVector3I operator-(const TVector3I& rhs) const
		{
			return TVector3I(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
		}

		TVector3I operator*(T value) const
		{
			return TVector3I(X * value, Y * value, Z * value);
		}

		Vector3 operator*(float value) const
		{
			return Vector3((float)X * value, (float)Y * value, (float)Z * value);
		}

		TVector3I operator*(const TVector3I& rhs) const
		{
			return TVector3I(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
		}

		TVector3I operator/(T value) const
		{
			B3D_ASSERT(value != 0);

			return TVector3I(X / value, Y / value, Z / value);
		}

		Vector3 operator/(float value) const
		{
			B3D_ASSERT(value != 0.0f);

			return Vector3((float)X / value, (float)Y / value, (float)Z / value);
		}

		TVector3I operator/(const TVector3I& rhs) const
		{
			return TVector3I(X / rhs.X, Y / rhs.Y, Z / rhs.Z);
		}

		const TVector3I& operator+() const
		{
			return *this;
		}

		template<typename U = T, typename = std::enable_if_t<std::is_signed_v<U>, i32>>
		TVector3I operator-() const
		{
			return TVector3I(-X, -Y, -Z);
		}

		friend TVector3I operator*(T lhs, const TVector3I& rhs)
		{
			return TVector3I(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
		}

		friend TVector3I operator/(T lhs, const TVector3I& rhs)
		{
			return TVector3I(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z);
		}

		TVector3I& operator+=(const TVector3I& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;

			return *this;
		}

		TVector3I& operator-=(const TVector3I& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;

			return *this;
		}

		TVector3I& operator*=(T val)
		{
			X *= val;
			Y *= val;
			Z *= val;

			return *this;
		}

		TVector3I& operator*=(const TVector3I& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;

			return *this;
		}

		TVector3I& operator/=(T value)
		{
			B3D_ASSERT(value != 0);

			X /= value;
			Y /= value;
			Z /= value;

			return *this;
		}

		TVector3I& operator/=(const TVector3I& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;

			return *this;
		}

		/** Returns the square of the length(magnitude) of the vector. */
		T SquaredLength() const
		{
			return X * X + Y * Y + Z * Z;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector3I& other) const
		{
			return X * other.X + Y * other.Y + Z * other.Z;
		}

		static const TVector3I kZero;
	};

	template<> inline const TVector3I<i32> TVector3I<i32>::kZero{0, 0, 0};
	template<> inline const TVector3I<u32> TVector3I<u32>::kZero{0u, 0u, 0u};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector3I)) TVector3I<i32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector3UI)) TVector3I<u32>;

	/** @} */
} // namespace b3d
