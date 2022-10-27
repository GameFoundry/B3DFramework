//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

#include "Math/BsVector3.h"
#include "Math/BsMatrix3.h"
#include "Math/BsVector4.h"
#include "Math/BsPlane.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Class representing a 4x4 matrix, in row major format. */
	class BS_UTILITY_EXPORT Matrix4
	{
	public:
		Matrix4() = default;
		constexpr Matrix4(const Matrix4&) = default;
		constexpr Matrix4& operator=(const Matrix4&) = default;

		constexpr Matrix4(BS_ZERO)
			: m{ { 0.0f, 0.0f, 0.0f, 0.0f },
				 { 0.0f, 0.0f, 0.0f, 0.0f },
				 { 0.0f, 0.0f, 0.0f, 0.0f },
				 { 0.0f, 0.0f, 0.0f, 0.0f } }
		{}

		constexpr Matrix4(BS_IDENTITY)
			: m{ { 1.0f, 0.0f, 0.0f, 0.0f },
				 { 0.0f, 1.0f, 0.0f, 0.0f },
				 { 0.0f, 0.0f, 1.0f, 0.0f },
				 { 0.0f, 0.0f, 0.0f, 1.0f } }
		{}

		constexpr Matrix4(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33)
			: m{ { m00, m01, m02, m03 },
				 { m10, m11, m12, m13 },
				 { m20, m21, m22, m23 },
				 { m30, m31, m32, m33 } }
		{}

		/** Creates a 4x4 transformation matrix with a zero translation part from a rotation/scaling 3x3 matrix. */
		constexpr explicit Matrix4(const Matrix3& mat3)
			: m{ { mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], 0.0f },
				 { mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], 0.0f },
				 { mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], 0.0f },
				 { 0.0f, 0.0f, 0.0f, 1.0f } }
		{}

		/** Swaps the contents of this matrix with another. */
		void Swap(Matrix4& other)
		{
			std::swap(m[0][0], other.m[0][0]);
			std::swap(m[0][1], other.m[0][1]);
			std::swap(m[0][2], other.m[0][2]);
			std::swap(m[0][3], other.m[0][3]);
			std::swap(m[1][0], other.m[1][0]);
			std::swap(m[1][1], other.m[1][1]);
			std::swap(m[1][2], other.m[1][2]);
			std::swap(m[1][3], other.m[1][3]);
			std::swap(m[2][0], other.m[2][0]);
			std::swap(m[2][1], other.m[2][1]);
			std::swap(m[2][2], other.m[2][2]);
			std::swap(m[2][3], other.m[2][3]);
			std::swap(m[3][0], other.m[3][0]);
			std::swap(m[3][1], other.m[3][1]);
			std::swap(m[3][2], other.m[3][2]);
			std::swap(m[3][3], other.m[3][3]);
		}

		/** Returns a row of the matrix. */
		Vector4& operator[](u32 row)
		{
			assert(row < 4);

			return *(Vector4*)m[row];
		}

		/** Returns a row of the matrix. */
		const Vector4& operator[](u32 row) const
		{
			assert(row < 4);

			return *(Vector4*)m[row];
		}

		Matrix4 operator*(const Matrix4& rhs) const
		{
			Matrix4 r;

			r.m[0][0] = m[0][0] * rhs.m[0][0] + m[0][1] * rhs.m[1][0] + m[0][2] * rhs.m[2][0] + m[0][3] * rhs.m[3][0];
			r.m[0][1] = m[0][0] * rhs.m[0][1] + m[0][1] * rhs.m[1][1] + m[0][2] * rhs.m[2][1] + m[0][3] * rhs.m[3][1];
			r.m[0][2] = m[0][0] * rhs.m[0][2] + m[0][1] * rhs.m[1][2] + m[0][2] * rhs.m[2][2] + m[0][3] * rhs.m[3][2];
			r.m[0][3] = m[0][0] * rhs.m[0][3] + m[0][1] * rhs.m[1][3] + m[0][2] * rhs.m[2][3] + m[0][3] * rhs.m[3][3];

			r.m[1][0] = m[1][0] * rhs.m[0][0] + m[1][1] * rhs.m[1][0] + m[1][2] * rhs.m[2][0] + m[1][3] * rhs.m[3][0];
			r.m[1][1] = m[1][0] * rhs.m[0][1] + m[1][1] * rhs.m[1][1] + m[1][2] * rhs.m[2][1] + m[1][3] * rhs.m[3][1];
			r.m[1][2] = m[1][0] * rhs.m[0][2] + m[1][1] * rhs.m[1][2] + m[1][2] * rhs.m[2][2] + m[1][3] * rhs.m[3][2];
			r.m[1][3] = m[1][0] * rhs.m[0][3] + m[1][1] * rhs.m[1][3] + m[1][2] * rhs.m[2][3] + m[1][3] * rhs.m[3][3];

			r.m[2][0] = m[2][0] * rhs.m[0][0] + m[2][1] * rhs.m[1][0] + m[2][2] * rhs.m[2][0] + m[2][3] * rhs.m[3][0];
			r.m[2][1] = m[2][0] * rhs.m[0][1] + m[2][1] * rhs.m[1][1] + m[2][2] * rhs.m[2][1] + m[2][3] * rhs.m[3][1];
			r.m[2][2] = m[2][0] * rhs.m[0][2] + m[2][1] * rhs.m[1][2] + m[2][2] * rhs.m[2][2] + m[2][3] * rhs.m[3][2];
			r.m[2][3] = m[2][0] * rhs.m[0][3] + m[2][1] * rhs.m[1][3] + m[2][2] * rhs.m[2][3] + m[2][3] * rhs.m[3][3];

			r.m[3][0] = m[3][0] * rhs.m[0][0] + m[3][1] * rhs.m[1][0] + m[3][2] * rhs.m[2][0] + m[3][3] * rhs.m[3][0];
			r.m[3][1] = m[3][0] * rhs.m[0][1] + m[3][1] * rhs.m[1][1] + m[3][2] * rhs.m[2][1] + m[3][3] * rhs.m[3][1];
			r.m[3][2] = m[3][0] * rhs.m[0][2] + m[3][1] * rhs.m[1][2] + m[3][2] * rhs.m[2][2] + m[3][3] * rhs.m[3][2];
			r.m[3][3] = m[3][0] * rhs.m[0][3] + m[3][1] * rhs.m[1][3] + m[3][2] * rhs.m[2][3] + m[3][3] * rhs.m[3][3];

			return r;
		}

		Matrix4 operator+(const Matrix4& rhs) const
		{
			Matrix4 r;

			r.m[0][0] = m[0][0] + rhs.m[0][0];
			r.m[0][1] = m[0][1] + rhs.m[0][1];
			r.m[0][2] = m[0][2] + rhs.m[0][2];
			r.m[0][3] = m[0][3] + rhs.m[0][3];

			r.m[1][0] = m[1][0] + rhs.m[1][0];
			r.m[1][1] = m[1][1] + rhs.m[1][1];
			r.m[1][2] = m[1][2] + rhs.m[1][2];
			r.m[1][3] = m[1][3] + rhs.m[1][3];

			r.m[2][0] = m[2][0] + rhs.m[2][0];
			r.m[2][1] = m[2][1] + rhs.m[2][1];
			r.m[2][2] = m[2][2] + rhs.m[2][2];
			r.m[2][3] = m[2][3] + rhs.m[2][3];

			r.m[3][0] = m[3][0] + rhs.m[3][0];
			r.m[3][1] = m[3][1] + rhs.m[3][1];
			r.m[3][2] = m[3][2] + rhs.m[3][2];
			r.m[3][3] = m[3][3] + rhs.m[3][3];

			return r;
		}

		Matrix4 operator-(const Matrix4& rhs) const
		{
			Matrix4 r;
			r.m[0][0] = m[0][0] - rhs.m[0][0];
			r.m[0][1] = m[0][1] - rhs.m[0][1];
			r.m[0][2] = m[0][2] - rhs.m[0][2];
			r.m[0][3] = m[0][3] - rhs.m[0][3];

			r.m[1][0] = m[1][0] - rhs.m[1][0];
			r.m[1][1] = m[1][1] - rhs.m[1][1];
			r.m[1][2] = m[1][2] - rhs.m[1][2];
			r.m[1][3] = m[1][3] - rhs.m[1][3];

			r.m[2][0] = m[2][0] - rhs.m[2][0];
			r.m[2][1] = m[2][1] - rhs.m[2][1];
			r.m[2][2] = m[2][2] - rhs.m[2][2];
			r.m[2][3] = m[2][3] - rhs.m[2][3];

			r.m[3][0] = m[3][0] - rhs.m[3][0];
			r.m[3][1] = m[3][1] - rhs.m[3][1];
			r.m[3][2] = m[3][2] - rhs.m[3][2];
			r.m[3][3] = m[3][3] - rhs.m[3][3];

			return r;
		}

		inline bool operator==(const Matrix4& rhs) const
		{
			if(m[0][0] != rhs.m[0][0] || m[0][1] != rhs.m[0][1] || m[0][2] != rhs.m[0][2] || m[0][3] != rhs.m[0][3] ||
			   m[1][0] != rhs.m[1][0] || m[1][1] != rhs.m[1][1] || m[1][2] != rhs.m[1][2] || m[1][3] != rhs.m[1][3] ||
			   m[2][0] != rhs.m[2][0] || m[2][1] != rhs.m[2][1] || m[2][2] != rhs.m[2][2] || m[2][3] != rhs.m[2][3] ||
			   m[3][0] != rhs.m[3][0] || m[3][1] != rhs.m[3][1] || m[3][2] != rhs.m[3][2] || m[3][3] != rhs.m[3][3])
			{
				return false;
			}

			return true;
		}

		inline bool operator!=(const Matrix4& rhs) const
		{
			return !operator==(rhs);
		}

		Matrix4 operator*(float rhs) const
		{
			return Matrix4(rhs * m[0][0], rhs * m[0][1], rhs * m[0][2], rhs * m[0][3], rhs * m[1][0], rhs * m[1][1], rhs * m[1][2], rhs * m[1][3], rhs * m[2][0], rhs * m[2][1], rhs * m[2][2], rhs * m[2][3], rhs * m[3][0], rhs * m[3][1], rhs * m[3][2], rhs * m[3][3]);
		}

		/** Returns the specified column of the matrix, ignoring the last row. */
		Vector3 GetColumn(u32 col) const
		{
			assert(col < 4);

			return Vector3(m[0][col], m[1][col], m[2][col]);
		}

		/** Returns the specified column of the matrix. */
		Vector4 GetColumn4D(u32 col) const
		{
			assert(col < 4);

			return Vector4(m[0][col], m[1][col], m[2][col], m[3][col]);
		}

		/** Returns a transpose of the matrix (switched columns and rows). */
		Matrix4 Transpose() const
		{
			return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
		}

		/** Assigns the vector to a column of the matrix. */
		void SetColumn(u32 idx, const Vector4& column)
		{
			m[0][idx] = column.X;
			m[1][idx] = column.Y;
			m[2][idx] = column.Z;
			m[3][idx] = column.W;
		}

		/** Assigns the vector to a row of the matrix. */
		void SetRow(u32 idx, const Vector4& column)
		{
			m[idx][0] = column.X;
			m[idx][1] = column.Y;
			m[idx][2] = column.Z;
			m[idx][3] = column.W;
		}

		/** Returns the rotation/scaling part of the matrix as a 3x3 matrix. */
		Matrix3 Get3x3() const
		{
			Matrix3 m3x3;
			m3x3.m[0][0] = m[0][0];
			m3x3.m[0][1] = m[0][1];
			m3x3.m[0][2] = m[0][2];
			m3x3.m[1][0] = m[1][0];
			m3x3.m[1][1] = m[1][1];
			m3x3.m[1][2] = m[1][2];
			m3x3.m[2][0] = m[2][0];
			m3x3.m[2][1] = m[2][1];
			m3x3.m[2][2] = m[2][2];

			return m3x3;
		}

		/** Calculates the adjoint of the matrix. */
		Matrix4 Adjoint() const;

		/** Calculates the determinant of the matrix. */
		float Determinant() const;

		/** Calculates the determinant of the 3x3 sub-matrix. */
		float Determinant3x3() const;

		/** Calculates the inverse of the matrix. */
		Matrix4 Inverse() const;

		/**
		 * Creates a matrix from translation, rotation and scale.
		 *
		 * @note	The transformation are applied in scale->rotation->translation order.
		 */
		void SetTrs(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

		/**
		 * Creates a matrix from inverse translation, rotation and scale.
		 *
		 * @note	This is cheaper than setTRS() and then performing inverse().
		 */
		void SetInverseTrs(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

		/**
		 * Decompose a Matrix4 to translation, rotation and scale.
		 *
		 * @note
		 * This method is unable to decompose all types of matrices, in particular these are the limitations:
		 *  - Only translation, rotation and scale transforms are supported
		 *  - Plain TRS matrices (that aren't composed with other matrices) can always be decomposed
		 *  - Composed TRS matrices can be decomposed ONLY if the scaling factor is uniform
		 */
		void Decomposition(Vector3& position, Quaternion& rotation, Vector3& scale) const;

		/** Extracts the translation (position) part of the matrix. */
		Vector3 GetTranslation() const { return Vector3(m[0][3], m[1][3], m[2][3]); }

		/**
		 * Check whether or not the matrix is affine matrix.
		 *
		 * @note	An affine matrix is a 4x4 matrix with row 3 equal to (0, 0, 0, 1), meaning no projective coefficients.
		 */
		bool IsAffine() const
		{
			return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
		}

		/**
		 * Returns the inverse of the affine matrix.
		 *
		 * @note	Matrix must be affine.
		 */
		Matrix4 InverseAffine() const;

		/**
		 * Concatenate two affine matrices.
		 *
		 * @note	Both matrices must be affine.
		 */
		Matrix4 ConcatenateAffine(const Matrix4& other) const
		{
			return Matrix4(
				m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0],
				m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1],
				m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2],
				m[0][0] * other.m[0][3] + m[0][1] * other.m[1][3] + m[0][2] * other.m[2][3] + m[0][3],

				m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0],
				m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1],
				m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2],
				m[1][0] * other.m[0][3] + m[1][1] * other.m[1][3] + m[1][2] * other.m[2][3] + m[1][3],

				m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0],
				m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1],
				m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2],
				m[2][0] * other.m[0][3] + m[2][1] * other.m[1][3] + m[2][2] * other.m[2][3] + m[2][3],

				0, 0, 0, 1);
		}

		/**
		 * Transform a plane by this matrix.
		 *
		 * @note	Matrix must be affine.
		 */
		Plane MultiplyAffine(const Plane& p) const
		{
			Vector4 localNormal(p.Normal.X, p.Normal.Y, p.Normal.Z, 0.0f);
			Vector4 localPoint = localNormal * p.D;
			localPoint.W = 1.0f;

			Matrix4 itMat = Inverse().Transpose();
			Vector4 worldNormal = itMat.MultiplyAffine(localNormal);
			Vector4 worldPoint = MultiplyAffine(localPoint);

			float d = worldNormal.Dot(worldPoint);

			return Plane(worldNormal.X, worldNormal.Y, worldNormal.Z, d);
		}

		/**
		 * Transform a 3D point by this matrix.
		 *
		 * @note	Matrix must be affine, if it is not use multiply() method.
		 */
		Vector3 MultiplyAffine(const Vector3& v) const
		{
			return Vector3(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3],
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3],
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3]);
		}

		/**
		 * Transform a 4D vector by this matrix.
		 *
		 * @note	Matrix must be affine, if it is not use multiply() method.
		 */
		Vector4 MultiplyAffine(const Vector4& v) const
		{
			return Vector4(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W,
				v.W);
		}

		/** Transform a 3D direction by this matrix. */
		Vector3 MultiplyDirection(const Vector3& v) const
		{
			return Vector3(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z,
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z,
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z);
		}

		/**
		 * Transform a 3D point by this matrix.
		 *
		 * @note
		 * w component of the vector is assumed to be 1. After transformation all components
		 * are projected back so that w remains 1.
		 * @note
		 * If your matrix doesn't contain projection components use MultiplyAffine() method as it is faster.
		 */
		Vector3 Multiply(const Vector3& v) const
		{
			Vector3 r(BsZero);

			float fInvW = 1.0f / (m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3]);

			r.X = (m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3]) * fInvW;
			r.Y = (m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3]) * fInvW;
			r.Z = (m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3]) * fInvW;

			return r;
		}

		/**
		 * Transform a 4D vector by this matrix.
		 *
		 * @note	If your matrix doesn't contain projection components use MultiplyAffine() method as it is faster.
		 */
		Vector4 Multiply(const Vector4& v) const
		{
			return Vector4(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W,
				m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3] * v.W);
		}

		/** Creates a view matrix and applies optional reflection. */
		void MakeView(const Vector3& position, const Quaternion& orientation);

		/**
		 * Creates an ortographic projection matrix that scales the part of the view bounded by @p left, @p right,
		 * @p top and @p bottom into [-1, 1] range. If @p far is non-zero the matrix will also transform the depth into
		 * [-1, 1] range, otherwise it will leave it as-is.
		 */
		void MakeProjectionOrtho(float left, float right, float top, float bottom, float near, float far);

		/** Creates a 4x4 transformation matrix that performs translation. */
		static Matrix4 Translation(const Vector3& translation);

		/** Creates a 4x4 transformation matrix that performs scaling. */
		static Matrix4 Scaling(const Vector3& scale);

		/** Creates a 4x4 transformation matrix that performs uniform scaling. */
		static Matrix4 Scaling(float scale);

		/** Creates a 4x4 transformation matrix that performs rotation. */
		static Matrix4 Rotation(const Quaternion& rotation);

		/**
		 * Creates a 4x4 perspective projection matrix.
		 *
		 * @param[in]	horzFOV		Horizontal field of view.
		 * @param[in]	aspect		Aspect ratio. Determines the vertical field of view.
		 * @param[in]	near		Distance to the near plane.
		 * @param[in]	far			Distance to the far plane.
		 * @param[in]	positiveZ	If true the matrix will project geometry as if its looking along the positive Z axis.
		 *							Otherwise it projects along the negative Z axis (default).
		 */
		static Matrix4 ProjectionPerspective(const Degree& horzFOV, float aspect, float near, float far, bool positiveZ = false);

		/** @copydoc makeProjectionOrtho() */
		static Matrix4 ProjectionOrthographic(float left, float right, float top, float bottom, float near, float far);

		/** Creates a view matrix. */
		static Matrix4 View(const Vector3& position, const Quaternion& orientation);

		/**
		 * Creates a matrix from translation, rotation and scale.
		 *
		 * @note	The transformation are applied in scale->rotation->translation order.
		 */
		static Matrix4 TRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

		/**
		 * Creates a matrix from inverse translation, rotation and scale.
		 *
		 * @note	This is cheaper than setTRS() and then performing inverse().
		 */
		static Matrix4 InverseTrs(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

		static const Matrix4 kZero;
		static const Matrix4 kIdentity;

	private:
		float m[4][4];
	};

	/** @} */
} // namespace bs
