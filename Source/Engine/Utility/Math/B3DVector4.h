//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A four dimensional vector. */
	template<class T>
	struct TVector4
	{
		T X, Y, Z, W;

		TVector4() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TVector4(ZeroTag)
			: X((T)0.0), Y((T)0.0), Z((T)0.0), W((T)0.0)
		{}

		constexpr TVector4(T x, T y, T z, T w)
			: X(x), Y(y), Z(z), W(w)
		{}

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr explicit TVector4(const TVector3<T>& other, T w = (T)0.0)
			: X(other.X), Y(other.Y), Z(other.Z), W(w)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(TVector4& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
			std::swap(W, other.W);
		}

		T operator[](u32 i) const
		{
			B3D_ASSERT(i < 4);

			return *(&X + i);
		}

		T& operator[](u32 i)
		{
			B3D_ASSERT(i < 4);

			return *(&X + i);
		}

		TVector4& operator=(T rhs)
		{
			X = rhs;
			Y = rhs;
			Z = rhs;
			W = rhs;

			return *this;
		}

		bool operator==(const TVector4& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W);
		}

		bool operator!=(const TVector4& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y || Z != rhs.Z || W != rhs.W);
		}

		TVector4& operator=(const TVector3<T>& rhs)
		{
			X = rhs.X;
			Y = rhs.Y;
			Z = rhs.Z;
			W = (T)1.0;

			return *this;
		}

		TVector4 operator+(const TVector4& rhs) const
		{
			return TVector4(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
		}

		TVector4 operator-(const TVector4& rhs) const
		{
			return TVector4(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
		}

		TVector4 operator*(T rhs) const
		{
			return TVector4(X * rhs, Y * rhs, Z * rhs, W * rhs);
		}

		TVector4 operator*(const TVector4& rhs) const
		{
			return TVector4(rhs.X * X, rhs.Y * Y, rhs.Z * Z, rhs.W * W);
		}

		TVector4 operator/(T rhs) const
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverse = (T)1.0 / rhs;
			return TVector4(X * inverse, Y * inverse, Z * inverse, W * inverse);
		}

		TVector4 operator/(const TVector4& rhs) const
		{
			return TVector4(X / rhs.X, Y / rhs.Y, Z / rhs.Z, W / rhs.W);
		}

		const TVector4& operator+() const
		{
			return *this;
		}

		TVector4 operator-() const
		{
			return TVector4(-X, -Y, -Z, -W);
		}

		friend TVector4 operator*(T lhs, const TVector4& rhs)
		{
			return TVector4(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
		}

		friend TVector4 operator/(T lhs, const TVector4& rhs)
		{
			return TVector4(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z, lhs / rhs.W);
		}

		friend TVector4 operator+(const TVector4& lhs, T rhs)
		{
			return TVector4(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs, lhs.W + rhs);
		}

		friend TVector4 operator+(T lhs, const TVector4& rhs)
		{
			return TVector4(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z, lhs + rhs.W);
		}

		friend TVector4 operator-(const TVector4& lhs, T rhs)
		{
			return TVector4(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs, lhs.W - rhs);
		}

		friend TVector4 operator-(T lhs, TVector4& rhs)
		{
			return TVector4(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z, lhs - rhs.W);
		}

		TVector4& operator+=(const TVector4& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			W += rhs.W;

			return *this;
		}

		TVector4& operator-=(const TVector4& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			W -= rhs.W;

			return *this;
		}

		TVector4& operator*=(T rhs)
		{
			X *= rhs;
			Y *= rhs;
			Z *= rhs;
			W *= rhs;

			return *this;
		}

		TVector4& operator+=(T rhs)
		{
			X += rhs;
			Y += rhs;
			Z += rhs;
			W += rhs;

			return *this;
		}

		TVector4& operator-=(T rhs)
		{
			X -= rhs;
			Y -= rhs;
			Z -= rhs;
			W -= rhs;

			return *this;
		}

		TVector4& operator*=(TVector4& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;
			W *= rhs.W;

			return *this;
		}

		TVector4& operator/=(T rhs)
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverse = (T)1.0 / rhs;

			X *= inverse;
			Y *= inverse;
			Z *= inverse;
			W *= inverse;

			return *this;
		}

		TVector4& operator/=(const TVector4& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;
			W /= rhs.W;

			return *this;
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector4& rhs) const
		{
			return X * rhs.X + Y * rhs.Y + Z * rhs.Z + W * rhs.W;
		}

		/** Checks are any of the vector components NaN. */
		inline bool IsNaN() const;

		static const TVector4 kZero;
	};

	template<> const TVector4<float> TVector4<float>::kZero{kZeroTag};
	template<> const TVector4<double> TVector4<double>::kZero{kZeroTag};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector4)) TVector4<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector4D)) TVector4<double>;

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TVector4<T>. */
template<typename T>
struct hash<b3d::TVector4<T>>
{
	size_t operator()(const b3d::TVector4<T>& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.X);
		b3d::B3DCombineHash(hash, value.Y);
		b3d::B3DCombineHash(hash, value.Z);
		b3d::B3DCombineHash(hash, value.W);

		return hash;
	}
};
} // namespace std

/** @endcond */
