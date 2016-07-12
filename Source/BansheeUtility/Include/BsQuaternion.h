//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsMath.h"
#include "BsVector3.h"

namespace BansheeEngine 
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
			int a, b, c;
		};

    public:
        Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
			:x(x), y(y), z(z), w(w)
		{ }

        /** Construct a quaternion from a rotation matrix. */
        explicit Quaternion(const Matrix3& rot)
        {
            fromRotationMatrix(rot);
        }

        /** Construct a quaternion from an angle/axis. */
        explicit Quaternion(const Vector3& axis, const Radian& angle)
        {
            fromAxisAngle(axis, angle);
        }

        /** Construct a quaternion from 3 orthonormal local axes. */
        explicit Quaternion(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
        {
            fromAxes(xaxis, yaxis, zaxis);
        }

        /**
         * Construct a quaternion from euler angles, YXZ ordering.
         * 			
		 * @see		Quaternion::fromEulerAngles
         */
		explicit Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
		{
			fromEulerAngles(xAngle, yAngle, zAngle);
		}

        /**
         * Construct a quaternion from euler angles, custom ordering.
         * 			
		 * @see		Quaternion::fromEulerAngles
         */
		explicit Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order)
		{
			fromEulerAngles(xAngle, yAngle, zAngle, order);
		}

		/** Exchange the contents of this quaternion with another. */
		void swap(Quaternion& other)
		{
			std::swap(w, other.w);
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
		}

		float operator[] (const size_t i) const
		{
			assert(i < 4);

			return *(&x+i);
		}

		float& operator[] (const size_t i)
		{
			assert(i < 4);

			return *(&x+i);
		}

		/**
		 * Initializes the quaternion from a 3x3 rotation matrix.
		 * 			
		 * @note	It's up to the caller to ensure the matrix is orthonormal.
		 */
		void fromRotationMatrix(const Matrix3& mat);

		/**
		 * Initializes the quaternion from an angle axis pair. Quaternion will represent a rotation of "angle" radians 
		 * around "axis".
		 */
        void fromAxisAngle(const Vector3& axis, const Radian& angle);

        /**
         * Initializes the quaternion from orthonormal set of axes. Quaternion will represent a rotation from base axes 
		 * to the specified set of axes.
         * 			
		 * @note	It's up to the caller to ensure the axes are orthonormal.
         */
        void fromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
        
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
        void fromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle);

        /**
         * Creates a quaternion from the provided Pitch/Yaw/Roll angles.
         *
		 * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
		 * @param[in]	order 	The order in which rotations will be extracted. Different values can be retrieved depending 
		 *						on the order.
         */
        void fromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order);

		/**
		 * Converts a quaternion to a rotation matrix.
		 */
		void toRotationMatrix(Matrix3& mat) const;

		/**
		 * Converts a quaternion to an angle axis pair.
		 *
		 * @param[out]	axis 	The axis around the which rotation takes place.
		 * @param[out]	angle	The angle in radians determining amount of rotation around the axis.
		 */
		void toAxisAngle(Vector3& axis, Radian& angle) const;

		/**
		 * Converts a quaternion to an orthonormal set of axes.
		 *
		 * @param[out]	xAxis	The X axis.
		 * @param[out]	yAxis	The Y axis.
		 * @param[out]	zAxis	The Z axis.
		 */
		void toAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;

		/**
         * Extracts Pitch/Yaw/Roll rotations from this quaternion.
         *
         * @param[out]	xAngle	Rotation about x axis. (AKA Pitch)
         * @param[out]	yAngle  Rotation about y axis. (AKA Yaw)
         * @param[out]	zAngle 	Rotation about z axis. (AKA Roll)
         *
         * @return	True if unique solution was found, false otherwise.
         */
        bool toEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const;

        /** Gets the positive x-axis of the coordinate system transformed by this quaternion. */
        Vector3 xAxis() const;

        /** Gets the positive y-axis of the coordinate system transformed by this quaternion. */
        Vector3 yAxis() const;

		/** Gets the positive z-axis of the coordinate system transformed by this quaternion. */
        Vector3 zAxis() const;

        Quaternion& operator= (const Quaternion& rhs)
		{
			w = rhs.w;
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;

			return *this;
		}

        Quaternion operator+ (const Quaternion& rhs) const 
		{
			return Quaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
		}

        Quaternion operator- (const Quaternion& rhs) const
		{
			return Quaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Quaternion operator* (const Quaternion& rhs) const
		{
			return Quaternion
			(
				w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
				w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
				w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
				w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x
			);
		}

		Quaternion operator* (float rhs) const
		{
			return Quaternion(rhs * w, rhs * x, rhs * y, rhs * z);
		}

        Quaternion operator- () const
		{
			return Quaternion(-w, -x, -y, -z);
		}

        bool operator== (const Quaternion& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) && (rhs.z == z) && (rhs.w == w);
		}

        bool operator!= (const Quaternion& rhs) const
		{
			return !operator==(rhs);
		}

		Quaternion& operator+= (const Quaternion& rhs)
		{
			w += rhs.w;
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}

		Quaternion& operator-= (const Quaternion& rhs)
		{
			w -= rhs.w;
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}

		Quaternion& operator*= (const Quaternion& rhs)
		{
			float newW = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
			float newX = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
			float newY = w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z;
			float newZ = w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x;

			w = newW;
			x = newX;
			y = newY;
			z = newZ;

			return *this;
		}

		friend Quaternion operator* (float lhs, const Quaternion& rhs)
		{
			return Quaternion(lhs * rhs.w, lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
		}

        /** Calculates the dot product of this quaternion and another. */
        float dot(const Quaternion& other) const
        {
			return w * other.w + x * other.x + y * other.y + z * other.z;
        }

        /** Normalizes this quaternion, and returns the previous length. */
        float normalize()
        {
			float len = w*w + x*x + y*y + z*z;
			float factor = 1.0f / Math::sqrt(len);
			*this = *this * factor;
			return len;
        }

        /**
         * Gets the inverse.
         *
         * @note	Quaternion must be non-zero.
         */
        Quaternion inverse() const; 

        /** Rotates the provided vector. */
        Vector3 rotate(const Vector3& vec) const;

        /**
         * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
         */
		void lookRotation(const Vector3& forwardDir);

        /**
         * Orients the quaternion so its negative z axis points to the provided direction.
		 *
		 * @param[in]	forwardDir	Direction to orient towards.
		 * @param[in]	upDir		Constrains y axis orientation to a plane this vector lies on. This rule might be broken
		 *							if forward and up direction are nearly parallel.
         */
		void lookRotation(const Vector3& forwardDir, const Vector3& upDir);

		/** Query if any of the components of the quaternion are not a number. */
		bool isNaN() const
		{
			return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z) || Math::isNaN(w);
		}

		/** Calculates the dot product between two quaternions. */
		static float dot(const Quaternion& lhs, const Quaternion& rhs)
		{
			return lhs.w * rhs.w + lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

		/** Normalizes the provided quaternion. */
		static Quaternion normalize(const Quaternion& q)
		{
			float len = dot(q, q);
			float factor = 1.0f / Math::sqrt(len);

			return q * factor;
		}

        /**
         * Performs spherical interpolation between two quaternions. Spherical interpolation neatly interpolates between 
		 * two rotations without modifying the size of the vector it is applied to (unlike linear interpolation).
         */
        static Quaternion slerp(float t, const Quaternion& p, const Quaternion& q, bool shortestPath = true);

		/** 
		 * Linearly interpolates between the two quaternions using @p t. t should be in [0, 1] range, where t = 0 
		 * corresponds to the left vector, while t = 1 corresponds to the right vector.
		 */
		static Quaternion lerp(float t, const Quaternion& a, const Quaternion& b)
		{
			float d = dot(a, b);
			float flip = d >= 0.0f ? 1.0f : -1.0f;
			
			Quaternion output = flip * (1.0f - t) * a + t * b;
			return normalize(output);
		}

        /** Gets the shortest arc quaternion to rotate this vector to the destination vector. */
        static Quaternion getRotationFromTo(const Vector3& from, const Vector3& dest, const Vector3& fallbackAxis = Vector3::ZERO);

        static const float EPSILON;

        static const Quaternion ZERO;
        static const Quaternion IDENTITY;

		float x, y, z, w; // Note: Order is relevant, don't break it

		private:
			static const EulerAngleOrderData EA_LOOKUP[6];
    };

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Quaternion);
	/** @endcond */
}