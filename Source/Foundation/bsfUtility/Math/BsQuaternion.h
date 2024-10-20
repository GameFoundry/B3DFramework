//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a quaternion used for 3D rotations. */
	template<typename T>
	struct TQuaternion
	{
	private:
		struct EulerAngleOrderData
		{
			int A, B, C;
		};

	public:
		TQuaternion() = default;
		constexpr TQuaternion(const TQuaternion&) = default;
		constexpr TQuaternion& operator=(const TQuaternion&) = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TQuaternion(BS_ZERO zero)
			: X((T)0.0), Y((T)0.0), Z((T)0.0), W((T)0.0)
		{}

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TQuaternion(BS_IDENTITY)
			: X((T)0.0), Y((T)0.0), Z((T)0.0), W((T)1.0)
		{}

		constexpr TQuaternion(T w, T x, T y, T z)
			: X(x), Y(y), Z(z), W(w)
		{}

		/** Construct a quaternion from a rotation matrix. */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		B3D_SCRIPT_EXPORT(Exclude(true))
		explicit TQuaternion(const Matrix3& rot)
		{
			FromRotationMatrix(rot);
		}

		/** Construct a quaternion from an angle/axis. */
		B3D_SCRIPT_EXPORT(Exclude(true))
		explicit TQuaternion(const TVector3<T>& axis, const TRadian<T>& angle)
		{
			FromAxisAngle(axis, angle);
		}

		/** Construct a quaternion from 3 orthonormal local axes. */
		B3D_SCRIPT_EXPORT(Exclude(true))
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		explicit TQuaternion(const TVector3<T>& xaxis, const TVector3<T>& yaxis, const TVector3<T>& zaxis)
		{
			FromAxes(xaxis, yaxis, zaxis);
		}

		/**
		 * Construct a quaternion from euler angles, YXZ ordering.
		 *
		 * @see		Quaternion::FromEulerAngles
		 */
		B3D_SCRIPT_EXPORT(Exclude(true))
		explicit TQuaternion(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle)
		{
			FromEulerAngles(xAngle, yAngle, zAngle);
		}

		/**
		 * Construct a quaternion from euler angles, custom ordering.
		 *
		 * @see		Quaternion::FromEulerAngles
		 */
		B3D_SCRIPT_EXPORT(Exclude(true))
		explicit TQuaternion(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order)
		{
			FromEulerAngles(xAngle, yAngle, zAngle, order);
		}

		/** Exchange the contents of this quaternion with another. */
		void Swap(TQuaternion& other)
		{
			std::swap(W, other.W);
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
		}

		T operator[](const size_t i) const
		{
			B3D_ASSERT(i < 4);

			return *(&X + i);
		}

		T& operator[](const size_t i)
		{
			B3D_ASSERT(i < 4);

			return *(&X + i);
		}

		/**
		 * Initializes the quaternion from a 3x3 rotation matrix.
		 *
		 * @note	It's up to the caller to ensure the matrix is orthonormal.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void FromRotationMatrix(const Matrix3& mat);

		/**
		 * Initializes the quaternion from an angle axis pair. Quaternion will represent a rotation of "angle" radians
		 * around "axis".
		 */
		void FromAxisAngle(const TVector3<T>& axis, const TRadian<T>& angle);

		/**
		 * Initializes the quaternion from orthonormal set of axes. Quaternion will represent a rotation from base axes
		 * to the specified set of axes.
		 *
		 * @note	It's up to the caller to ensure the axes are orthonormal.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void FromAxes(const TVector3<T>& xAxis, const TVector3<T>& yAxis, const TVector3<T>& zAxis);

		/**
		 * Creates a quaternion from the provided Pitch/Yaw/Roll angles.
		 *
		 * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
		 *
		 * @note
		 * Since different values will be produced depending in which order are the rotations applied, this method assumes
		 * they are applied in YXZ order. If you need a specific order, use the overloaded fromEulerAngles() method instead.
		 */
		void FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle);

		/**
		 * Creates a quaternion from the provided Pitch/Yaw/Roll angles.
		 *
		 * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
		 * @param[in]	order 	The order in which rotations will be extracted. Different values can be retrieved depending
		 *						on the order.
		 */
		void FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order);

		/**
		 * Converts a quaternion to a rotation matrix.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void ToRotationMatrix(Matrix3& mat) const;

		/**
		 * Converts a quaternion to an angle axis pair.
		 *
		 * @param[out]	axis 	The axis around the which rotation takes place.
		 * @param[out]	angle	The angle in radians determining amount of rotation around the axis.
		 */
		void ToAxisAngle(TVector3<T>& axis, TRadian<T>& angle) const;

		/**
		 * Converts a quaternion to an orthonormal set of axes.
		 *
		 * @param[out]	xAxis	The X axis.
		 * @param[out]	yAxis	The Y axis.
		 * @param[out]	zAxis	The Z axis.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void ToAxes(TVector3<T>& xAxis, TVector3<T>& yAxis, TVector3<T>& zAxis) const;

		/**
		 * Extracts Pitch/Yaw/Roll rotations from this quaternion.
		 *
		 * @param[out]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[out]	yAngle  Rotation about y axis. (AKA Yaw)
		 * @param[out]	zAngle 	Rotation about z axis. (AKA Roll)
		 *
		 * @return	True if unique solution was found, false otherwise.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		bool ToEulerAngles(TRadian<T>& xAngle, TRadian<T>& yAngle, TRadian<T>& zAngle) const;

		/** Gets the positive x-axis of the coordinate system transformed by this quaternion. */
		TVector3<T> XAxis() const;

		/** Gets the positive y-axis of the coordinate system transformed by this quaternion. */
		TVector3<T> YAxis() const;

		/** Gets the positive z-axis of the coordinate system transformed by this quaternion. */
		TVector3<T> ZAxis() const;

		TQuaternion operator+(const TQuaternion& rhs) const
		{
			return TQuaternion(W + rhs.W, X + rhs.X, Y + rhs.Y, Z + rhs.Z);
		}

		TQuaternion operator-(const TQuaternion& rhs) const
		{
			return TQuaternion(W - rhs.W, X - rhs.X, Y - rhs.Y, Z - rhs.Z);
		}

		TQuaternion operator*(const TQuaternion& rhs) const
		{
			return TQuaternion(
				W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z,
				W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y,
				W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z,
				W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X);
		}

		TQuaternion operator*(T rhs) const
		{
			return TQuaternion(rhs * W, rhs * X, rhs * Y, rhs * Z);
		}

		TQuaternion operator/(T rhs) const
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inv = (T)1.0 / rhs;
			return TQuaternion(W * inv, X * inv, Y * inv, Z * inv);
		}

		TQuaternion operator-() const
		{
			return TQuaternion(-W, -X, -Y, -Z);
		}

		bool operator==(const TQuaternion& rhs) const
		{
			return (rhs.X == X) && (rhs.Y == Y) && (rhs.Z == Z) && (rhs.W == W);
		}

		bool operator!=(const TQuaternion& rhs) const
		{
			return !operator==(rhs);
		}

		TQuaternion& operator+=(const TQuaternion& rhs)
		{
			W += rhs.W;
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;

			return *this;
		}

		TQuaternion& operator-=(const TQuaternion& rhs)
		{
			W -= rhs.W;
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;

			return *this;
		}

		TQuaternion& operator*=(const TQuaternion& rhs)
		{
			T newW = W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z;
			T newX = W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y;
			T newY = W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z;
			T newZ = W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X;

			W = newW;
			X = newX;
			Y = newY;
			Z = newZ;

			return *this;
		}

		friend TQuaternion operator*(T lhs, const TQuaternion& rhs)
		{
			return TQuaternion(lhs * rhs.W, lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
		}

		/** Calculates the dot product of this quaternion and another. */
		T Dot(const TQuaternion& other) const
		{
			return W * other.W + X * other.X + Y * other.Y + Z * other.Z;
		}

		/**
		 * Normalizes this quaternion, and returns the previous length. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		T Normalize(T tolerance = (T)1e-04)
		{
			T len = Math::SquareRoot(Dot(*this, *this));
			if(!SAFE || len > (tolerance * tolerance))
				*this = *this * ((T)1.0 / len);

			return len;
		}

		/**
		 * Gets the inverse.
		 *
		 * @note	Quaternion must be non-zero.
		 */
		TQuaternion Inverse() const;

		/** Rotates the provided vector. */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		TVector3<T> Rotate(const TVector3<T>& vec) const;

		/**
		 * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
		 */
		void LookRotation(const TVector3<T>& forwardDir);

		/**
		 * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
		 * @param[in]	upDir		Constrains y axis orientation to a plane this vector lies on. This rule might be broken
		 *							if forward and up direction are nearly parallel.
		 */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void LookRotation(const TVector3<T>& forwardDir, const TVector3<T>& upDir);

		/** Query if any of the components of the quaternion are not a number. */
		bool IsNaN() const
		{
			return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z) || Math::IsNaN(W);
		}

		/** Calculates the dot product between two quaternions. */
		static T Dot(const TQuaternion& lhs, const TQuaternion& rhs)
		{
			return lhs.W * rhs.W + lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
		}

		/**
		 * Normalizes the provided quaternion and returns the result. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		static TQuaternion Normalize(const TQuaternion& q, T tolerance = (T)1e-04)
		{
			T sqrdLen = Dot(q, q);
			if(!SAFE || sqrdLen > tolerance)
				return q * Math::InverseSquareRoot(sqrdLen);

			return q;
		}

		/**
		 * Performs spherical interpolation between two quaternions. Spherical interpolation neatly interpolates between
		 * two rotations without modifying the size of the vector it is applied to (unlike linear interpolation).
		 */
		static TQuaternion Slerp(T t, const TQuaternion& p, const TQuaternion& q, bool shortestPath = true);

		/**
		 * Linearly interpolates between the two quaternions using @p t. t should be in [0, 1] range, where t = 0
		 * corresponds to the left vector, while t = 1 corresponds to the right vector.
		 */
		static TQuaternion Lerp(T t, const TQuaternion& a, const TQuaternion& b)
		{
			T d = Dot(a, b);
			T flip = d >= (T)0.0 ? (T)1.0 : (T)-1.0;

			TQuaternion output = flip * ((T)1.0 - t) * a + t * b;
			return Normalize(output);
		}

		/** Gets the shortest arc quaternion to rotate this vector to the destination vector. */
		static TQuaternion GetRotationFromTo(const TVector3<T>& from, const TVector3<T>& dest, const TVector3<T>& fallbackAxis = TVector3<T>::kZero);

		/** Returns the minimum of all the quaternion components as a new quaternion. */
		static TQuaternion Min(const TQuaternion& a, const TQuaternion& b)
		{
			return TQuaternion(std::min(a.X, b.X), std::min(a.Y, b.Y), std::min(a.Z, b.Z), std::min(a.W, b.W));
		}

		/** Returns the maximum of all the quaternion components as a new quaternion. */
		static TQuaternion Max(const TQuaternion& a, const TQuaternion& b)
		{
			return TQuaternion(std::max(a.X, b.X), std::max(a.Y, b.Y), std::max(a.Z, b.Z), std::max(a.W, b.W));
		}

		static constexpr const T kEpsilon = (T)1e-03;

		static const TQuaternion kZero;
		static const TQuaternion kIdentity;

		T X, Y, Z, W; // Note: Order is relevant, don't break it
	};

	template<> const TQuaternion<float> TQuaternion<float>::kZero{0.0f, 0.0f, 0.0f, 0.0f};
	template<> const TQuaternion<double> TQuaternion<double>::kZero{0.0, 0.0, 0.0, 0.0};

	template<> const TQuaternion<float> TQuaternion<float>::kIdentity{1.0f, 0.0f, 0.0f, 0.0f};
	template<> const TQuaternion<double> TQuaternion<double>::kIdentity{1.0, 0.0, 0.0, 0.0};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Quaternion)) TQuaternion<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(QuaternionD)) TQuaternion<double>;

	/** @} */
} // namespace bs

/** @cond SPECIALIZATIONS */
namespace std
{
	template <>
	class numeric_limits<bs::TQuaternion<float>>
	{
	public:
		constexpr static bs::TQuaternion<float> infinity() // NOLINT
		{
			return bs::TQuaternion<float>(
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity());
		}
	};

	template <>
	class numeric_limits<bs::TQuaternion<double>>
	{
	public:
		constexpr static bs::TQuaternion<double> infinity() // NOLINT
		{
			return bs::TQuaternion<double>(
				std::numeric_limits<double>::infinity(),
				std::numeric_limits<double>::infinity(),
				std::numeric_limits<double>::infinity(),
				std::numeric_limits<double>::infinity());
		}
	};
} // namespace std

/** @endcond */
