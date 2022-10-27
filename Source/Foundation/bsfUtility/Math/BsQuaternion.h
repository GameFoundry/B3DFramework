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
	class BS_UTILITY_EXPORT Quaternion
	{
	private:
		struct EulerAngleOrderData
		{
			int A, B, C;
		};

	public:
		Quaternion() = default;
		constexpr Quaternion(const Quaternion&) = default;
		constexpr Quaternion& operator=(const Quaternion&) = default;

		constexpr Quaternion(BS_ZERO zero)
			: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{}

		constexpr Quaternion(BS_IDENTITY)
			: X(0.0f), Y(0.0f), Z(0.0f), W(1.0f)
		{}

		constexpr Quaternion(float w, float x, float y, float z)
			: X(x), Y(y), Z(z), W(w)
		{}

		/** Construct a quaternion from a rotation matrix. */
		explicit Quaternion(const Matrix3& rot)
		{
			FromRotationMatrix(rot);
		}

		/** Construct a quaternion from an angle/axis. */
		explicit Quaternion(const Vector3& axis, const Radian& angle)
		{
			FromAxisAngle(axis, angle);
		}

		/** Construct a quaternion from 3 orthonormal local axes. */
		explicit Quaternion(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
		{
			FromAxes(xaxis, yaxis, zaxis);
		}

		/**
		 * Construct a quaternion from euler angles, YXZ ordering.
		 *
		 * @see		Quaternion::fromEulerAngles
		 */
		explicit Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
		{
			FromEulerAngles(xAngle, yAngle, zAngle);
		}

		/**
		 * Construct a quaternion from euler angles, custom ordering.
		 *
		 * @see		Quaternion::fromEulerAngles
		 */
		explicit Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order)
		{
			FromEulerAngles(xAngle, yAngle, zAngle, order);
		}

		/** Exchange the contents of this quaternion with another. */
		void Swap(Quaternion& other)
		{
			std::swap(W, other.W);
			std::swap(X, other.X);
			std::swap(Y, other.Y);
			std::swap(Z, other.Z);
		}

		float operator[](const size_t i) const
		{
			assert(i < 4);

			return *(&X + i);
		}

		float& operator[](const size_t i)
		{
			assert(i < 4);

			return *(&X + i);
		}

		/**
		 * Initializes the quaternion from a 3x3 rotation matrix.
		 *
		 * @note	It's up to the caller to ensure the matrix is orthonormal.
		 */
		void FromRotationMatrix(const Matrix3& mat);

		/**
		 * Initializes the quaternion from an angle axis pair. Quaternion will represent a rotation of "angle" radians
		 * around "axis".
		 */
		void FromAxisAngle(const Vector3& axis, const Radian& angle);

		/**
		 * Initializes the quaternion from orthonormal set of axes. Quaternion will represent a rotation from base axes
		 * to the specified set of axes.
		 *
		 * @note	It's up to the caller to ensure the axes are orthonormal.
		 */
		void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

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
		void FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle);

		/**
		 * Creates a quaternion from the provided Pitch/Yaw/Roll angles.
		 *
		 * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
		 * @param[in]	order 	The order in which rotations will be extracted. Different values can be retrieved depending
		 *						on the order.
		 */
		void FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order);

		/**
		 * Converts a quaternion to a rotation matrix.
		 */
		void ToRotationMatrix(Matrix3& mat) const;

		/**
		 * Converts a quaternion to an angle axis pair.
		 *
		 * @param[out]	axis 	The axis around the which rotation takes place.
		 * @param[out]	angle	The angle in radians determining amount of rotation around the axis.
		 */
		void ToAxisAngle(Vector3& axis, Radian& angle) const;

		/**
		 * Converts a quaternion to an orthonormal set of axes.
		 *
		 * @param[out]	xAxis	The X axis.
		 * @param[out]	yAxis	The Y axis.
		 * @param[out]	zAxis	The Z axis.
		 */
		void ToAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;

		/**
		 * Extracts Pitch/Yaw/Roll rotations from this quaternion.
		 *
		 * @param[out]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[out]	yAngle  Rotation about y axis. (AKA Yaw)
		 * @param[out]	zAngle 	Rotation about z axis. (AKA Roll)
		 *
		 * @return	True if unique solution was found, false otherwise.
		 */
		bool ToEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const;

		/** Gets the positive x-axis of the coordinate system transformed by this quaternion. */
		Vector3 XAxis() const;

		/** Gets the positive y-axis of the coordinate system transformed by this quaternion. */
		Vector3 YAxis() const;

		/** Gets the positive z-axis of the coordinate system transformed by this quaternion. */
		Vector3 ZAxis() const;

		Quaternion operator+(const Quaternion& rhs) const
		{
			return Quaternion(W + rhs.W, X + rhs.X, Y + rhs.Y, Z + rhs.Z);
		}

		Quaternion operator-(const Quaternion& rhs) const
		{
			return Quaternion(W - rhs.W, X - rhs.X, Y - rhs.Y, Z - rhs.Z);
		}

		Quaternion operator*(const Quaternion& rhs) const
		{
			return Quaternion(
				W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z,
				W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y,
				W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z,
				W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X);
		}

		Quaternion operator*(float rhs) const
		{
			return Quaternion(rhs * W, rhs * X, rhs * Y, rhs * Z);
		}

		Quaternion operator/(float rhs) const
		{
			assert(rhs != 0.0);

			const float inv = 1.0f / rhs;
			return Quaternion(W * inv, X * inv, Y * inv, Z * inv);
		}

		Quaternion operator-() const
		{
			return Quaternion(-W, -X, -Y, -Z);
		}

		bool operator==(const Quaternion& rhs) const
		{
			return (rhs.X == X) && (rhs.Y == Y) && (rhs.Z == Z) && (rhs.W == W);
		}

		bool operator!=(const Quaternion& rhs) const
		{
			return !operator==(rhs);
		}

		Quaternion& operator+=(const Quaternion& rhs)
		{
			W += rhs.W;
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;

			return *this;
		}

		Quaternion& operator-=(const Quaternion& rhs)
		{
			W -= rhs.W;
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;

			return *this;
		}

		Quaternion& operator*=(const Quaternion& rhs)
		{
			float newW = W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z;
			float newX = W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y;
			float newY = W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z;
			float newZ = W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X;

			W = newW;
			X = newX;
			Y = newY;
			Z = newZ;

			return *this;
		}

		friend Quaternion operator*(float lhs, const Quaternion& rhs)
		{
			return Quaternion(lhs * rhs.W, lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
		}

		/** Calculates the dot product of this quaternion and another. */
		float Dot(const Quaternion& other) const
		{
			return W * other.W + X * other.X + Y * other.Y + Z * other.Z;
		}

		/**
		 * Normalizes this quaternion, and returns the previous length. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		float Normalize(float tolerance = 1e-04f)
		{
			float len = Math::Sqrt(Dot(*this, *this));
			if(!SAFE || len > (tolerance * tolerance))
				*this = *this * (1.0f / len);

			return len;
		}

		/**
		 * Gets the inverse.
		 *
		 * @note	Quaternion must be non-zero.
		 */
		Quaternion Inverse() const;

		/** Rotates the provided vector. */
		Vector3 Rotate(const Vector3& vec) const;

		/**
		 * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
		 */
		void LookRotation(const Vector3& forwardDir);

		/**
		 * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
		 * @param[in]	upDir		Constrains y axis orientation to a plane this vector lies on. This rule might be broken
		 *							if forward and up direction are nearly parallel.
		 */
		void LookRotation(const Vector3& forwardDir, const Vector3& upDir);

		/** Query if any of the components of the quaternion are not a number. */
		bool IsNaN() const
		{
			return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z) || Math::IsNaN(W);
		}

		/** Calculates the dot product between two quaternions. */
		static float Dot(const Quaternion& lhs, const Quaternion& rhs)
		{
			return lhs.W * rhs.W + lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
		}

		/**
		 * Normalizes the provided quaternion and returns the result. If @p SAFE is true, checks if the magnitude is
		 * above @p tolerance to avoid division by zero or precision issues. If false, no checks are made.
		 */
		template <bool SAFE = true>
		static Quaternion Normalize(const Quaternion& q, float tolerance = 1e-04f)
		{
			float sqrdLen = Dot(q, q);
			if(!SAFE || sqrdLen > tolerance)
				return q * Math::InvSqrt(sqrdLen);

			return q;
		}

		/**
		 * Performs spherical interpolation between two quaternions. Spherical interpolation neatly interpolates between
		 * two rotations without modifying the size of the vector it is applied to (unlike linear interpolation).
		 */
		static Quaternion Slerp(float t, const Quaternion& p, const Quaternion& q, bool shortestPath = true);

		/**
		 * Linearly interpolates between the two quaternions using @p t. t should be in [0, 1] range, where t = 0
		 * corresponds to the left vector, while t = 1 corresponds to the right vector.
		 */
		static Quaternion Lerp(float t, const Quaternion& a, const Quaternion& b)
		{
			float d = Dot(a, b);
			float flip = d >= 0.0f ? 1.0f : -1.0f;

			Quaternion output = flip * (1.0f - t) * a + t * b;
			return Normalize(output);
		}

		/** Gets the shortest arc quaternion to rotate this vector to the destination vector. */
		static Quaternion GetRotationFromTo(const Vector3& from, const Vector3& dest, const Vector3& fallbackAxis = Vector3::kZero);

		/** Returns the minimum of all the quaternion components as a new quaternion. */
		static Quaternion Min(const Quaternion& a, const Quaternion& b)
		{
			return Quaternion(std::min(a.X, b.X), std::min(a.Y, b.Y), std::min(a.Z, b.Z), std::min(a.W, b.W));
		}

		/** Returns the maximum of all the quaternion components as a new quaternion. */
		static Quaternion Max(const Quaternion& a, const Quaternion& b)
		{
			return Quaternion(std::max(a.X, b.X), std::max(a.Y, b.Y), std::max(a.Z, b.Z), std::max(a.W, b.W));
		}

		static constexpr const float kEpsilon = 1e-03f;

		static const Quaternion kZero;
		static const Quaternion kIdentity;

		float X, Y, Z, W; // Note: Order is relevant, don't break it
	};

	/** @} */
} // namespace bs

/** @cond SPECIALIZATIONS */
namespace std
{
	template <>
	class numeric_limits<bs::Quaternion>
	{
	public:
		constexpr static bs::Quaternion infinity() // NOLINT
		{
			return bs::Quaternion(
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity());
		}
	};
} // namespace std

/** @endcond */
