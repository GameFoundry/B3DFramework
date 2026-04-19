//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

#include "Math/B3DVector3.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DVector4.h"
#include "Math/B3DPlane.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Class representing a 4x4 matrix, in row major format. */
	template<typename T>
	struct B3D_EXPORT TMatrix4
	{
	public:
		TMatrix4() = default;
		constexpr TMatrix4(const TMatrix4&) = default;
		constexpr TMatrix4& operator=(const TMatrix4&) = default;

		constexpr TMatrix4(ZeroTag)
			: m{ { (T)0.0, (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0, (T)0.0 } }
		{}

		constexpr TMatrix4(IdentityTag)
			: m{ { (T)1.0, (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)1.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)1.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0, (T)1.0 } }
		{}

		constexpr TMatrix4(
			T m00, T m01, T m02, T m03,
			T m10, T m11, T m12, T m13,
			T m20, T m21, T m22, T m23,
			T m30, T m31, T m32, T m33)
			: m{ { m00, m01, m02, m03 },
				 { m10, m11, m12, m13 },
				 { m20, m21, m22, m23 },
				 { m30, m31, m32, m33 } }
		{}

		/** Creates a 4x4 transformation matrix with a zero translation part from a rotation/scaling 3x3 matrix. */
		constexpr explicit TMatrix4(const TMatrix3<T>& mat3)
			: m{ { mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], (T)0.0 },
				 { mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], (T)0.0 },
				 { mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0, (T)1.0 } }
		{}

		/** Swaps the contents of this matrix with another. */
		void Swap(TMatrix4& other)
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
		TVector4<T>& operator[](u32 row)
		{
			B3D_ASSERT(row < 4);

			return *(TVector4<T>*)m[row];
		}

		/** Returns a row of the matrix. */
		const TVector4<T>& operator[](u32 row) const
		{
			B3D_ASSERT(row < 4);

			return *(TVector4<T>*)m[row];
		}

		TMatrix4 operator*(const TMatrix4& rhs) const
		{
			TMatrix4 r;

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

		TMatrix4 operator+(const TMatrix4& rhs) const
		{
			TMatrix4 r;

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

		TMatrix4 operator-(const TMatrix4& rhs) const
		{
			TMatrix4 r;
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

		inline bool operator==(const TMatrix4& rhs) const
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

		inline bool operator!=(const TMatrix4& rhs) const
		{
			return !operator==(rhs);
		}

		TMatrix4 operator*(T rhs) const
		{
			return TMatrix4(rhs * m[0][0], rhs * m[0][1], rhs * m[0][2], rhs * m[0][3], rhs * m[1][0], rhs * m[1][1], rhs * m[1][2], rhs * m[1][3], rhs * m[2][0], rhs * m[2][1], rhs * m[2][2], rhs * m[2][3], rhs * m[3][0], rhs * m[3][1], rhs * m[3][2], rhs * m[3][3]);
		}

		/** Returns the specified column of the matrix, ignoring the last row. */
		TVector3<T> GetColumn(u32 col) const
		{
			B3D_ASSERT(col < 4);

			return TVector3(m[0][col], m[1][col], m[2][col]);
		}

		/** Returns the specified column of the matrix. */
		TVector4<T> GetColumn4D(u32 col) const
		{
			B3D_ASSERT(col < 4);

			return TVector4(m[0][col], m[1][col], m[2][col], m[3][col]);
		}

		/** Returns a transpose of the matrix (switched columns and rows). */
		TMatrix4 Transpose() const
		{
			return TMatrix4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
		}

		/** Assigns the vector to a column of the matrix. */
		void SetColumn(u32 idx, const TVector4<T>& column)
		{
			m[0][idx] = column.X;
			m[1][idx] = column.Y;
			m[2][idx] = column.Z;
			m[3][idx] = column.W;
		}

		/** Assigns the vector to a row of the matrix. */
		void SetRow(u32 idx, const TVector4<T>& column)
		{
			m[idx][0] = column.X;
			m[idx][1] = column.Y;
			m[idx][2] = column.Z;
			m[idx][3] = column.W;
		}

		/** Returns the rotation/scaling part of the matrix as a 3x3 matrix. */
		TMatrix3<T> Get3x3() const
		{
			TMatrix3<T> m3x3;
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
		TMatrix4 Adjoint() const;

		/** Calculates the determinant of the matrix. */
		T Determinant() const;

		/** Calculates the determinant of the 3x3 sub-matrix. */
		T Determinant3x3() const;

		/** Calculates the inverse of the matrix. */
		TMatrix4 Inverse() const;

		/**
		 * Creates a matrix from translation, rotation and scale.
		 *
		 * @note	The transformation are applied in scale->rotation->translation order.
		 */
		void SetTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale);

		/**
		 * Creates a matrix from inverse translation, rotation and scale.
		 *
		 * @note	This is cheaper than setTRS() and then performing inverse().
		 */
		void SetInverseTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale);

		/**
		 * Decompose a Matrix4 to translation, rotation and scale.
		 *
		 * @note
		 * This method is unable to decompose all types of matrices, in particular these are the limitations:
		 *  - Only translation, rotation and scale transforms are supported
		 *  - Plain TRS matrices (that aren't composed with other matrices) can always be decomposed
		 *  - Composed TRS matrices can be decomposed ONLY if the scaling factor is uniform
		 */
		void Decomposition(TVector3<T>& position, TQuaternion<T>& rotation, TVector3<T>& scale) const;

		/** Extracts the translation (position) part of the matrix. */
		TVector3<T> GetTranslation() const { return TVector3<T>(m[0][3], m[1][3], m[2][3]); }

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
		TMatrix4 InverseAffine() const;

		/**
		 * Concatenate two affine matrices.
		 *
		 * @note	Both matrices must be affine.
		 */
		TMatrix4 ConcatenateAffine(const TMatrix4& other) const
		{
			return TMatrix4(
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

				(T)0.0, (T)0.0, (T)0.0, (T)1.0);
		}

		/**
		 * Transform a plane by this matrix.
		 *
		 * @note	Matrix must be affine.
		 */
		TPlane<T> MultiplyAffine(const TPlane<T>& p) const
		{
			TVector4<T> localNormal(p.Normal.X, p.Normal.Y, p.Normal.Z, (T)0.0);
			TVector4<T> localPoint = localNormal * p.D;
			localPoint.W = (T)1.0;

			TMatrix4 itMat = Inverse().Transpose();
			TVector4<T> worldNormal = itMat.MultiplyAffine(localNormal);
			TVector4<T> worldPoint = MultiplyAffine(localPoint);

			T d = worldNormal.Dot(worldPoint);

			return TPlane<T>(worldNormal.X, worldNormal.Y, worldNormal.Z, d);
		}

		/**
		 * Transform a 3D point by this matrix.
		 *
		 * @note	Matrix must be affine, if it is not use multiply() method.
		 */
		TVector3<T> MultiplyAffine(const TVector3<T>& v) const
		{
			return TVector3<T>(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3],
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3],
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3]);
		}

		/**
		 * Transform a 4D vector by this matrix.
		 *
		 * @note	Matrix must be affine, if it is not use multiply() method.
		 */
		TVector4<T> MultiplyAffine(const TVector4<T>& v) const
		{
			return TVector4<T>(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W,
				v.W);
		}

		/** Transform a 3D direction by this matrix. */
		TVector3<T> MultiplyDirection(const TVector3<T>& v) const
		{
			return TVector3<T>(
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
		TVector3<T> Multiply(const TVector3<T>& v) const
		{
			TVector3<T> r(kZeroTag);

			T fInvW = (T)1.0 / (m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3]);

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
		TVector4<T> Multiply(const TVector4<T>& v) const
		{
			return TVector4<T>(
				m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
				m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
				m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W,
				m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3] * v.W);
		}

		/** Creates a view matrix and applies optional reflection. */
		void MakeView(const TVector3<T>& position, const TQuaternion<T>& orientation);

		/**
		 * Creates an ortographic projection matrix that scales the part of the view bounded by @p left, @p right,
		 * @p top and @p bottom into [-1, 1] range. If @p far is non-zero the matrix will also transform the depth into
		 * [-1, 1] range, otherwise it will leave it as-is.
		 */
		void MakeProjectionOrtho(T left, T right, T top, T bottom, T near, T far);

		/** Creates a 4x4 transformation matrix that performs translation. */
		static TMatrix4 Translation(const TVector3<T>& translation);

		/** Creates a 4x4 transformation matrix that performs scaling. */
		static TMatrix4 Scaling(const TVector3<T>& scale);

		/** Creates a 4x4 transformation matrix that performs uniform scaling. */
		static TMatrix4 Scaling(T scale);

		/** Creates a 4x4 transformation matrix that performs rotation. */
		static TMatrix4 Rotation(const TQuaternion<T>& rotation);

		/**
		 * Creates a 4x4 perspective projection matrix.
		 *
		 * @param	horzFOV		Horizontal field of view.
		 * @param	aspect		Aspect ratio. Determines the vertical field of view.
		 * @param	near		Distance to the near plane.
		 * @param	far			Distance to the far plane.
		 * @param	positiveZ	If true the matrix will project geometry as if its looking along the positive Z axis.
		 *						Otherwise it projects along the negative Z axis (default).
		 */
		static TMatrix4 ProjectionPerspective(const TDegree<T>& horzFOV, T aspect, T near, T far, bool positiveZ = false);

		/** @copydoc MakeProjectionOrtho() */
		static TMatrix4 ProjectionOrthographic(T left, T right, T top, T bottom, T near, T far);

		/** Creates a view matrix. */
		static TMatrix4 View(const TVector3<T>& position, const TQuaternion<T>& orientation);

		/**
		 * Creates a matrix from translation, rotation and scale.
		 *
		 * @note	The transformation are applied in scale->rotation->translation order.
		 */
		static TMatrix4 TRS(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale);

		/**
		 * Creates a matrix from inverse translation, rotation and scale.
		 *
		 * @note	This is cheaper than setTRS() and then performing inverse().
		 */
		static TMatrix4 InverseTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale);

		static const TMatrix4 kZero;
		static const TMatrix4 kIdentity;

	private:
		T m[4][4];
	};

	template<> const TMatrix4<float> TMatrix4<float>::kZero{kZeroTag};
	template<> const TMatrix4<double> TMatrix4<double>::kZero{kZeroTag};
	template<> const TMatrix4<float> TMatrix4<float>::kIdentity{kIdentityTag};
	template<> const TMatrix4<double> TMatrix4<double>::kIdentity{kIdentityTag};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TMatrix4<T>. */
template<typename T>
struct hash<b3d::TMatrix4<T>>
{
	size_t operator()(const b3d::TMatrix4<T>& value) const
	{
		using namespace b3d;

		size_t hash = 0;

		for(u32 rowIndex = 0; rowIndex < 4; ++rowIndex)
		{
			for(u32 columnIndex = 0; columnIndex < 4; ++columnIndex)
				B3DCombineHash(hash, value[rowIndex][columnIndex]);
		}

		return hash;
	}
};
} // namespace std

/** @endcond */
