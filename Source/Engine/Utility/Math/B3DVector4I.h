//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DVector4.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A four dimensional vector with integer coordinates. */
	template<class T>
	struct TVector4I
	{
		T X = (T)0;
		T Y = (T)0;
		T Z = (T)0;
		T W = (T)0;

		constexpr TVector4I() = default;

		constexpr TVector4I(T x, T y, T z, T w)
			: X(x), Y(y), Z(z), W(w)
		{}

		constexpr explicit TVector4I(T value)
			: X(value), Y(value), Z(value), W(value)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(TVector4I& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
			std::swap(W, other.W);
		}

		Vector4 ToFloat() const
		{
			return Vector4((float)X, (float)Y, (float)Z, (float)W);
		}

		T operator[](size_t index) const
		{
			B3D_ASSERT(index < 4);

			return *(&X + index);
		}

		T& operator[](size_t index)
		{
			B3D_ASSERT(index < 4);

			return *(&X + index);
		}

		TVector4I& operator=(T value)
		{
			X = value;
			Y = value;
			Z = value;
			W = value;

			return *this;
		}

		bool operator==(const TVector4I& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W);
		}

		bool operator!=(const TVector4I& rhs) const
		{
			return X != rhs.X || Y != rhs.Y || Z != rhs.Z || W != rhs.W;
		}

		TVector4I operator+(const TVector4I& rhs) const
		{
			return TVector4I(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
		}

		TVector4I operator-(const TVector4I& rhs) const
		{
			return TVector4I(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W + rhs.W);
		}

		TVector4I operator*(T value) const
		{
			return TVector4I(X * value, Y * value, Z * value, W * value);
		}

		Vector4 operator*(float value) const
		{
			return Vector4((float)X * value, (float)Y * value, (float)Z * value, (float)W * value);
		}

		TVector4I operator*(const TVector4I& rhs) const
		{
			return TVector4I(X * rhs.X, Y * rhs.Y, Z * rhs.Z, W * rhs.W);
		}

		TVector4I operator/(T value) const
		{
			B3D_ASSERT(value != 0);

			return TVector4I(X / value, Y / value, Z / value, W / value);
		}

		Vector4 operator/(float value) const
		{
			B3D_ASSERT(value != 0.0f);

			return Vector4((float)X / value, (float)Y / value, (float)Z / value, (float)W / value);
		}

		TVector4I operator/(const TVector4I& rhs) const
		{
			return TVector4I(X / rhs.X, Y / rhs.Y, Z / rhs.Z, W / rhs.W);
		}

		const TVector4I& operator+() const
		{
			return *this;
		}

		template<typename U = T, typename = std::enable_if_t<std::is_signed_v<U>, i32>>
		TVector4I operator-() const
		{
			return TVector4I(-X, -Y, -Z, -W);
		}

		friend TVector4I operator*(T lhs, const TVector4I& rhs)
		{
			return TVector4I(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
		}

		friend TVector4I operator/(T lhs, const TVector4I& rhs)
		{
			return TVector4I(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z, lhs * rhs.W);
		}

		TVector4I& operator+=(const TVector4I& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			W += rhs.W;

			return *this;
		}

		TVector4I& operator-=(const TVector4I& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			W -= rhs.W;

			return *this;
		}

		TVector4I& operator*=(T val)
		{
			X *= val;
			Y *= val;
			Z *= val;
			W *= val;

			return *this;
		}

		TVector4I& operator*=(const TVector4I& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;
			W *= rhs.W;

			return *this;
		}

		TVector4I& operator/=(T value)
		{
			B3D_ASSERT(value != 0);

			X /= value;
			Y /= value;
			Z /= value;
			W /= value;

			return *this;
		}

		TVector4I& operator/=(const TVector4I& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;
			W /= rhs.W;

			return *this;
		}

		/** Returns the square of the length(magnitude) of the vector. */
		T SquaredLength() const
		{
			return X * X + Y * Y + Z * Z + W * W;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector4I& other) const
		{
			return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
		}

		static const TVector4I kZero;
	};

	template<> const TVector4I<i32> TVector4I<i32>::kZero{0, 0, 0, 0};
	template<> const TVector4I<u32> TVector4I<u32>::kZero{0u, 0u, 0u, 0u};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector4I)) TVector4I<i32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector4UI)) TVector4I<u32>;

	/** @} */
} // namespace b3d
