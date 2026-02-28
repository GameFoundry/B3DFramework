//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cmath>

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DRadian.h"
#include "B3DMath.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A three dimensional vector. */
	template<class T>
	struct TVector3
	{
		T X, Y, Z;

		constexpr TVector3() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TVector3(ZeroTag)
			: X((T)0.0), Y((T)0.0), Z((T)0.0)
		{}

		constexpr TVector3(T value)
			: X(value), Y(value), Z(value)
		{}
		constexpr TVector3(T x, T y, T z)
			: X(x), Y(y), Z(z)
		{}

		explicit TVector3(const Vector4& vec);

		/** Exchange the contents of this vector with another. */
		void Swap(TVector3& other)
		{
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
		}

		T operator[](u32 i) const
		{
			B3D_ASSERT(i < 3);

			return *(&X + i);
		}

		T& operator[](u32 i)
		{
			B3D_ASSERT(i < 3);

			return *(&X + i);
		}

		TVector3& operator=(T rhs)
		{
			X = rhs;
			Y = rhs;
			Z = rhs;

			return *this;
		}

		bool operator==(const TVector3& rhs) const
		{
			return (X == rhs.X && Y == rhs.Y && Z == rhs.Z);
		}

		bool operator!=(const TVector3& rhs) const
		{
			return (X != rhs.X || Y != rhs.Y || Z != rhs.Z);
		}

		TVector3 operator+(const TVector3& rhs) const
		{
			return TVector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
		}

		TVector3 operator-(const TVector3& rhs) const
		{
			return TVector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
		}

		TVector3 operator*(T rhs) const
		{
			return TVector3(X * rhs, Y * rhs, Z * rhs);
		}

		TVector3 operator*(const TVector3& rhs) const
		{
			return TVector3(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
		}

		TVector3 operator/(T value) const
		{
			B3D_ASSERT(value != (T)0.0);

			const T inverse = (T)1.0 / value;
			return TVector3(X * inverse, Y * inverse, Z * inverse);
		}

		TVector3 operator/(const TVector3& rhs) const
		{
			return TVector3(X / rhs.X, Y / rhs.Y, Z / rhs.Z);
		}

		const TVector3& operator+() const
		{
			return *this;
		}

		TVector3 operator-() const
		{
			return TVector3(-X, -Y, -Z);
		}

		friend TVector3 operator*(T lhs, const TVector3& rhs)
		{
			return TVector3(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
		}

		friend TVector3 operator/(T lhs, const TVector3& rhs)
		{
			return TVector3(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z);
		}

		friend TVector3 operator+(const TVector3& lhs, T rhs)
		{
			return TVector3(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs);
		}

		friend TVector3 operator+(T lhs, const TVector3& rhs)
		{
			return TVector3(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z);
		}

		friend TVector3 operator-(const TVector3& lhs, T rhs)
		{
			return TVector3(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs);
		}

		friend TVector3 operator-(T lhs, const TVector3& rhs)
		{
			return TVector3(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z);
		}

		TVector3& operator+=(const TVector3& rhs)
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;

			return *this;
		}

		TVector3& operator+=(T rhs)
		{
			X += rhs;
			Y += rhs;
			Z += rhs;

			return *this;
		}

		TVector3& operator-=(const TVector3& rhs)
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;

			return *this;
		}

		TVector3& operator-=(T rhs)
		{
			X -= rhs;
			Y -= rhs;
			Z -= rhs;

			return *this;
		}

		TVector3& operator*=(T rhs)
		{
			X *= rhs;
			Y *= rhs;
			Z *= rhs;

			return *this;
		}

		TVector3& operator*=(const TVector3& rhs)
		{
			X *= rhs.X;
			Y *= rhs.Y;
			Z *= rhs.Z;

			return *this;
		}

		TVector3& operator/=(T rhs)
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverse = (T)1.0 / rhs;
			X *= inverse;
			Y *= inverse;
			Z *= inverse;

			return *this;
		}

		TVector3& operator/=(const TVector3& rhs)
		{
			X /= rhs.X;
			Y /= rhs.Y;
			Z /= rhs.Z;

			return *this;
		}

		/** Returns the length (magnitude) of the vector. */
		T Length() const
		{
			return std::sqrt(X * X + Y * Y + Z * Z);
		}

		/** Returns the square of the length(magnitude) of the vector. */
		T SquaredLength() const
		{
			return X * X + Y * Y + Z * Z;
		}

		/**	Returns the distance to another vector. */
		T Distance(const TVector3& rhs) const
		{
			return (*this - rhs).Length();
		}

		/** Returns the square of the distance to another vector. */
		T SquaredDistance(const TVector3& rhs) const
		{
			return (*this - rhs).SquaredLength();
		}

		/** Calculates the dot (scalar) product of this vector with another. */
		T Dot(const TVector3& rhs) const
		{
			return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
		}

		/** Normalizes this vector, and returns the previous length. */
		T Normalize()
		{
			const T length = Length();
			*this *= (T)1.0 / length;

			return length;
		}

		/** Normalizes this vector, and returns the previous length. Checks if the magnitude is above @p tolerance to avoid division by zero or precision issues. If false, no checks are made. */
		T NormalizeChecked(T tolerance = (T)1e-04)
		{
			const T length = Length();
			if(length > (tolerance * tolerance))
				*this *= (T)1.0 / length;

			return length;
		}

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		TVector3 Cross(const TVector3& other) const
		{
			return TVector3(
				Y * other.Z - Z * other.Y,
				Z * other.X - X * other.Z,
				X * other.Y - Y * other.X);
		}

		/** Sets this vector's components to the minimum of its own and the ones of the passed in vector. */
		void Min(const TVector3& other)
		{
			if(other.X < X) X = other.X;
			if(other.Y < Y) Y = other.Y;
			if(other.Z < Z) Z = other.Z;
		}

		/** Sets this vector's components to the maximum of its own and the ones of the passed in vector. */
		void Max(const TVector3& other)
		{
			if(other.X > X) X = other.X;
			if(other.Y > Y) Y = other.Y;
			if(other.Z > Z) Z = other.Z;
		}

		/** Generates a vector perpendicular to this vector. */
		TVector3 Perpendicular() const
		{
			static const T kSquareZero = (T)(1e-06 * 1e-06);

			TVector3 perpendicular = this->Cross(TVector3::kUnitX);
			if(perpendicular.SquaredLength() < kSquareZero)
				perpendicular = this->Cross(TVector3::kUnitY);

			perpendicular.Normalize();
			return perpendicular;
		}

		/** Gets the angle between 2 vectors. */
		TRadian<T> AngleBetween(const TVector3& other) const
		{
			T lengthProduct = Length() * other.Length();

			// Divide by zero check
			if(lengthProduct < (T)1e-6)
				lengthProduct = (T)1e-6;

			T f = Dot(other) / lengthProduct;

			f = Math::Clamp(f, (T)-1.0, (T)1.0);
			return Math::Acos(f);
		}

		/** Returns true if this vector is zero length. */
		bool IsZeroLength(T tolerance = (T)1e-04) const
		{
			const T squaredLength = X * X + Y * Y + Z * Z;
			return squaredLength < tolerance;
		}

		/** Calculates a reflection vector to the plane with the given normal. */
		TVector3 Reflect(const TVector3& normal) const
		{
			return TVector3(*this - ((T)2.0 * this->Dot(normal) * normal));
		}

		/** Calculates two vectors orthonormal to the current vector, and normalizes the current vector if not already. */
		void OrthogonalComplement(TVector3& a, TVector3& b)
		{
			if(std::abs(X) > std::abs(Y))
				a = TVector3(-Z, 0, X);
			else
				a = TVector3(0, Z, -Y);

			b = Cross(a);

			Orthonormalize(*this, a, b);
		}

		/** Performs Gram-Schmidt orthonormalization. */
		static void Orthonormalize(TVector3& vec0, TVector3& vec1, TVector3& vec2)
		{
			vec0.Normalize();

			T dot0 = vec0.Dot(vec1);
			vec1 -= dot0 * vec0;
			vec1.Normalize();

			const T dot1 = vec1.Dot(vec2);
			dot0 = vec0.Dot(vec2);
			vec2 -= dot0 * vec0 + dot1 * vec1;
			vec2.Normalize();
		}

		/** Calculates the dot (scalar) product of two vectors. */
		static T Dot(const TVector3& a, const TVector3& b)
		{
			return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
		}

		/** Normalizes the provided vector and returns the result. */
		static TVector3 Normalize(const TVector3& rhs)
		{
			const T squaredLength = Dot(rhs, rhs);
			return rhs * Math::InverseSquareRoot(squaredLength);
		}

		/** Normalizes the provided vector and returns the result. Checks if the magnitude is above @p tolerance to avoid division by zero or precision issues. If false, no checks are made. */
		static TVector3 NormalizeChecked(const TVector3& rhs, T tolerance = (T)1e-04)
		{
			const T squaredLength = Dot(rhs, rhs);
			if(squaredLength > tolerance)
				return rhs * Math::InverseSquareRoot(squaredLength);

			return rhs;
		}

		/** Calculates the cross-product of 2 vectors, that is, the vector that lies perpendicular to them both. */
		static TVector3 Cross(const TVector3& a, const TVector3& b)
		{
			return TVector3(
				a.Y * b.Z - a.Z * b.Y,
				a.Z * b.X - a.X * b.Z,
				a.X * b.Y - a.Y * b.X);
		}

		/**
		 * Linearly interpolates between the two vectors using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to the left vector, while t = 1 corresponds to the right vector.
		 */
		static TVector3 Lerp(T t, const TVector3& a, const TVector3& b)
		{
			return ((T)1.0 - t) * a + t * b;
		}

		/** Checks are any of the vector components not a number. */
		bool IsNaN() const
		{
			return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z);
		}

		/** Returns the minimum of all the vector components as a new vector. */
		static TVector3 Min(const TVector3& a, const TVector3& b)
		{
			return TVector3(std::min(a.X, b.X), std::min(a.Y, b.Y), std::min(a.Z, b.Z));
		}

		/** Returns the maximum of all the vector components as a new vector. */
		static TVector3 Max(const TVector3& a, const TVector3& b)
		{
			return TVector3(std::max(a.X, b.X), std::max(a.Y, b.Y), std::max(a.Z, b.Z));
		}

		static const TVector3 kZero;
		static const TVector3 kOne;
		static const TVector3 kInfinite;
		static const TVector3 kUnitX;
		static const TVector3 kUnitY;
		static const TVector3 kUnitZ;
	};

	template<> const TVector3<float> TVector3<float>::kZero{kZeroTag};
	template<> const TVector3<double> TVector3<double>::kZero{kZeroTag};

	template<> const TVector3<float> TVector3<float>::kOne{1.0f, 1.0f, 1.0f};
	template<> const TVector3<double> TVector3<double>::kOne{1.0, 1.0, 1.0};

	template<> const TVector3<float> TVector3<float>::kInfinite{ std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity() };
	template<> const TVector3<double> TVector3<double>::kInfinite{ std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity() };

	template<> const TVector3<float> TVector3<float>::kUnitX{1.0f, 0.0f, 0.0f};
	template<> const TVector3<double> TVector3<double>::kUnitX{1.0, 0.0, 0.0};

	template<> const TVector3<float> TVector3<float>::kUnitY{0.0f, 1.0f, 0.0f};
	template<> const TVector3<double> TVector3<double>::kUnitY{0.0, 1.0, 0.0};

	template<> const TVector3<float> TVector3<float>::kUnitZ{0.0f, 0.0f, 1.0f};
	template<> const TVector3<double> TVector3<double>::kUnitZ{0.0, 0.0, 1.0};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector3)) TVector3<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Vector3D)) TVector3<double>;

	/** @} */
} // namespace b3d

/** @cond SPECIALIZATIONS */
namespace std
{
	template <>
	class numeric_limits<b3d::TVector3<float>>
	{
	public:
		constexpr static b3d::TVector3<float> infinity() // NOLINT
		{
			return b3d::TVector3<float>(
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity());
		}
	};

	template <>
	class numeric_limits<b3d::TVector3<double>>
	{
	public:
		constexpr static b3d::TVector3<double> infinity() // NOLINT
		{
			return b3d::TVector3<double>(
				std::numeric_limits<double>::infinity(),
				std::numeric_limits<double>::infinity(),
				std::numeric_limits<double>::infinity());
		}
	};

	/** Hash value generator for TVector3<T>. */
	template<class T>
	struct hash<b3d::TVector3<T>>
	{
		size_t operator()(const b3d::TVector3<T>& value) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, value.X);
			b3d::B3DCombineHash(hash, value.Y);
			b3d::B3DCombineHash(hash, value.Z);

			return hash;
		}
	};
} // namespace std

/** @endcond */
