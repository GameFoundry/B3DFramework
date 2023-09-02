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
	template<class T>
	struct TVector2
	{
		T X;
		T Y;

		constexpr TVector2() = default;

		constexpr TVector2(BS_ZERO)
			: X((T)0.0), Y((T)0.0)
		{}

		constexpr TVector2(T x, T y)
			: X(x), Y(y)
		{}

		/** Exchange the contents of this vector with another. */
		void Swap(TVector2& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
		}

		T operator[](u32 i) const
		{
			B3D_ASSERT(i < 2);

			return *(&X + i);
		}

		T& operator[](u32 i)
		{
			B3D_ASSERT(i < 2);

			return *(&X + i);
		}

		TVector2& operator=(T rhs)
		{
			X = rhs;
			Y = rhs;

			return *this;
		}

		bool operator==(const TVector2& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y);
		}

		bool operator!=(const TVector2& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y);
		}

		TVector2 operator+(const TVector2& rhs) const
		{
			return TVector2(X + rhs.X, Y + rhs.Y);
		}

		TVector2 operator-(const TVector2& rhs) const
		{
			return TVector2(X - rhs.X, Y - rhs.Y);
		}

		TVector2 operator*(const T rhs) const
		{
			return TVector2(X * rhs, Y * rhs);
		}

		TVector2 operator*(const TVector2& rhs) const
		{
			return TVector2(X * rhs.X, Y * rhs.Y);
		}

		TVector2 operator/(const T rhs) const
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			return TVector2(X * inverseRHS, Y * inverseRHS);
		}

		TVector2 operator/(const TVector2& rhs) const
		{
			return TVector2(X / rhs.X, Y / rhs.Y);
		}

		const TVector2& operator+() const
		{
			return *this;
		}

		TVector2 operator-() const
		{
			return TVector2(-X, -Y);
		}

		friend TVector2 operator*(T lhs, const TVector2& rhs)
		{
			return TVector2(lhs * rhs.X, lhs * rhs.Y);
		}

		friend TVector2 operator/(T lhs, const TVector2& rhs)
		{
			return TVector2(lhs / rhs.X, lhs / rhs.Y);
		}

		friend TVector2 operator+(TVector2& lhs, T rhs)
		{
			return TVector2(lhs.X + rhs, lhs.Y + rhs);
		}

		friend TVector2 operator+(T lhs, const TVector2& rhs)
		{
			return TVector2(lhs + rhs.X, lhs + rhs.Y);
		}

		friend TVector2 operator-(const TVector2& lhs, T rhs)
		{
			return TVector2(lhs.X - rhs, lhs.Y - rhs);
		}

		friend TVector2 operator-(const T lhs, const TVector2& rhs)
		{
			return TVector2(lhs - rhs.X, lhs - rhs.Y);
		}

		TVector2& operator+=(const TVector2& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;

			return *this;
		}

		TVector2& operator+=(T rhs)
		{
			X += rhs;
			Y += rhs;

			return *this;
		}

		TVector2& operator-=(const TVector2& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;

			return *this;
		}

		TVector2& operator-=(T rhs)
		{
			X -= rhs;
			Y -= rhs;

			return *this;
		}

		TVector2& operator*=(T rhs)
		{
			X *= rhs;
			Y *= rhs;

			return *this;
		}

		TVector2& operator*=(const TVector2& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;

			return *this;
		}

		TVector2& operator/=(T rhs)
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			X *= inverseRHS;
			Y *= inverseRHS;

			return *this;
		}

		TVector2& operator/=(const TVector2& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;

			return *this;
		}

		/** Returns the length (magnitude) of the vector. */
		T Length() const
		{
			return Math::SquareRoot(X * X + Y * Y);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		T SquaredLength() const
		{
			return X * X + Y * Y;
		}

		/** Returns the distance to another vector. */
		T Distance(const TVector2& rhs) const
		{
			return (*this - rhs).Length();
		}

		/** Returns the square of the distance to another vector. */
		T SquaredDistance(const TVector2& rhs) const
		{
			return (*this - rhs).SquaredLength();
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector2& other) const
		{
			return X * other.X + Y * other.Y;
		}

		/** Normalizes this vector, and returns the previous length. */
		T Normalize()
		{
			const T length = Length();
			*this *= (T)1.0 / length;

			return length;
		}

		/** Normalizes this vector, and returns the previous length. Checks if the magnitude is above @p tolerance to avoid division by zero or precision issues. */
		T NormalizeChecked(T tolerance = (T)1e-04)
		{
			const T length = Length();
			if(length > (tolerance * tolerance))
				*this *= (T)1.0 / length;

			return length;
		}

		/** Generates a vector perpendicular to this vector. */
		TVector2 Perpendicular() const
		{
			return TVector2(-Y, X);
		}

		/**
		 * Calculates the 2 dimensional cross-product of 2 vectors, which results in a single floating point value which
		 * is 2 times the area of the triangle.
		 */
		T Cross(const TVector2& other) const
		{
			return X * other.Y - Y * other.X;
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void Floor(const TVector2& other)
		{
			if(other.X < X) X = other.X;
			if(other.Y < Y) Y = other.Y;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void Ceil(const TVector2& other)
		{
			if(other.X > X) X = other.X;
			if(other.Y > Y) Y = other.Y;
		}

		/** Returns true if this vector is zero length. */
		bool IsZeroLength(T tolerance = (T)1e-04) const
		{
			const T squaredLength = X * X + Y * Y;
			return squaredLength < tolerance;
		}

		/** Calculates a reflection vector to the plane with the given normal. */
		TVector2 Reflect(const TVector2& normal) const
		{
			return TVector2(*this - ((T)2.0 * this->Dot(normal) * normal));
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void Orthonormalize(TVector2& u, TVector2& v)
		{
			u.Normalize();

			const T dot = u.Dot(v);
			v -= u * dot;
			v.Normalize();
		}

		/** Normalizes the provided vector and returns the result. */
		static TVector2 Normalize(const TVector2& value)
		{
			const T squaredLength = value.X * value.X + value.Y * value.Y;
			return value * Math::InverseSquareRoot(squaredLength);
		}

		/** Normalizes the provided vector and returns the result. Checks if the magnitude is above @p tolerance to avoid division by zero or precision issues. */
		static TVector2 NormalizeChecked(const TVector2& value, T tolerance = (T)1e-04)
		{
			const T squaredLength = value.X * value.X + value.Y * value.Y;
			if(squaredLength > tolerance)
				return value * Math::InverseSquareRoot(squaredLength);

			return value;
		}

		/** Checks are any of the vector components NaN. */
		bool IsNaN() const
		{
			return Math::IsNaN(X) || Math::IsNaN(Y);
		}

		/** Returns the minimum of all the vector components as a new vector. */
		static TVector2 Min(const TVector2& a, const TVector2& b)
		{
			return TVector2(std::min(a.X, b.X), std::min(a.Y, b.Y));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static TVector2 Max(const TVector2& a, const TVector2& b)
		{
			return TVector2(std::max(a.X, b.X), std::max(a.Y, b.Y));
		}

		static const TVector2 kZero;
		static const TVector2 kOne;
		static const TVector2 kUnitX;
		static const TVector2 kUnitY;
	};

	template<> const TVector2<float> TVector2<float>::kZero{BsZero};
	template<> const TVector2<double> TVector2<double>::kZero{BsZero};

	template<> const TVector2<float> TVector2<float>::kOne{1.0f, 1.0f};
	template<> const TVector2<double> TVector2<double>::kOne{1.0, 1.0};

	template<> const TVector2<float> TVector2<float>::kUnitX{1.0f, 0.0f};
	template<> const TVector2<double> TVector2<double>::kUnitX{1.0, 0.0};

	template<> const TVector2<float> TVector2<float>::kUnitY{0.0f, 1.0f};
	template<> const TVector2<double> TVector2<double>::kUnitY{0.0, 1.0};

	extern template struct TVector2<float>;
	extern template struct TVector2<double>;

	/** @} */
} // namespace bs

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TVector2<T>. */
template<class T>
struct hash<bs::TVector2<T>>
{
	size_t operator()(const bs::TVector2<T>& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.X);
		bs::B3DCombineHash(hash, value.Y);

		return hash;
	}
};
} // namespace std

/** @endcond */
