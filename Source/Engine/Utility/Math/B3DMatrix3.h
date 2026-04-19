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

	/**
	 * A 3x3 matrix. Can be used for non-homogenous transformations of three dimensional vectors and points. In row major
	 * format.
	 */
	template<typename T>
	struct B3D_EXPORT TMatrix3
	{
	private:
		struct EulerAngleOrderData
		{
			int A, B, C;
			T Sign;
		};

	public:
		TMatrix3() = default;
		constexpr TMatrix3(const TMatrix3&) = default;
		constexpr TMatrix3& operator=(const TMatrix3&) = default;

		constexpr TMatrix3(ZeroTag)
			: m{ { (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)0.0 } }
		{}

		constexpr TMatrix3(IdentityTag)
			: m{ { (T)1.0, (T)0.0, (T)0.0 },
				 { (T)0.0, (T)1.0, (T)0.0 },
				 { (T)0.0, (T)0.0, (T)1.0 } }
		{}

		constexpr TMatrix3(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22)
			: m{ { m00, m01, m02 }, { m10, m11, m12 }, { m20, m21, m22 } }
		{}

		/** Construct a matrix from a TQuaternion<T>. */
		explicit TMatrix3(const TQuaternion<T>& rotation)
		{
			FromQuaternion(rotation);
		}

		/** Construct a matrix that performs rotation and scale. */
		explicit TMatrix3(const TQuaternion<T>& rotation, const TVector3<T>& scale)
		{
			FromQuaternion(rotation);

			for(int row = 0; row < 3; row++)
			{
				for(int col = 0; col < 3; col++)
					m[row][col] = scale[row] * m[row][col];
			}
		}

		/** Construct a matrix from an angle/axis pair. */
		explicit TMatrix3(const TVector3<T>& axis, const TRadian<T>& angle)
		{
			FromAxisAngle(axis, angle);
		}

		/** Construct a matrix from 3 orthonormal local axes. */
		explicit TMatrix3(const TVector3<T>& xaxis, const TVector3<T>& yaxis, const TVector3<T>& zaxis)
		{
			FromAxes(xaxis, yaxis, zaxis);
		}

		/**
		 * Construct a matrix from euler angles, YXZ ordering.
		 *
		 * @see		TMatrix3::FromEulerAngles
		 */
		explicit TMatrix3(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle)
		{
			FromEulerAngles(xAngle, yAngle, zAngle);
		}

		/**
		 * Construct a matrix from euler angles, custom ordering.
		 *
		 * @see		TMatrix3::FromEulerAngles
		 */
		explicit TMatrix3(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order)
		{
			FromEulerAngles(xAngle, yAngle, zAngle, order);
		}

		/** Swaps the contents of this matrix with another. */
		void Swap(TMatrix3& other)
		{
			std::swap(m[0][0], other.m[0][0]);
			std::swap(m[0][1], other.m[0][1]);
			std::swap(m[0][2], other.m[0][2]);
			std::swap(m[1][0], other.m[1][0]);
			std::swap(m[1][1], other.m[1][1]);
			std::swap(m[1][2], other.m[1][2]);
			std::swap(m[2][0], other.m[2][0]);
			std::swap(m[2][1], other.m[2][1]);
			std::swap(m[2][2], other.m[2][2]);
		}

		/** Returns a row of the matrix. */
		T* operator[](u32 row) const
		{
			B3D_ASSERT(row < 3);

			return (T*)m[row];
		}

		TVector3<T> GetColumn(u32 col) const;
		void SetColumn(u32 col, const TVector3<T>& vec);

		bool operator==(const TMatrix3& rhs) const;
		bool operator!=(const TMatrix3& rhs) const;

		TMatrix3 operator+(const TMatrix3& rhs) const;
		TMatrix3 operator-(const TMatrix3& rhs) const;
		TMatrix3 operator*(const TMatrix3& rhs) const;
		TMatrix3 operator-() const;
		TMatrix3 operator*(T rhs) const;

		friend TMatrix3 operator*(T lhs, const TMatrix3& rhs);

		/** Transforms the given vector by this matrix and returns the newly transformed vector. */
		TVector3<T> Multiply(const TVector3<T>& vec) const;

		/** Returns a transpose of the matrix (switched columns and rows). */
		TMatrix3 Transpose() const;

		/**
		 * Calculates an inverse of the matrix if it exists.
		 *
		 * @param	mat			Resulting matrix inverse.
		 * @param	fTolerance 	(optional) Tolerance to use when checking if determinant is zero (or near zero in this case).
		 * 							Zero determinant means inverse doesn't exist.
		 * @return					True if inverse exists, false otherwise.
		 */
		bool Inverse(TMatrix3& mat, T fTolerance = (T)1e-06) const;

		/**
		 * Calculates an inverse of the matrix if it exists.
		 *
		 * @param	fTolerance 	(optional) Tolerance to use when checking if determinant is zero (or near zero in this case).
		 * 							Zero determinant means inverse doesn't exist.
		 *
		 * @return					Resulting matrix inverse if it exists, otherwise a zero matrix.
		 */
		TMatrix3 Inverse(T fTolerance = (T)1e-06) const;

		/** Calculates the matrix determinant. */
		T Determinant() const;

		/**
		 * Decompose a TMatrix3 to rotation and scale.
		 *
		 * @note
		 * Matrix must consist only of rotation and uniform scale transformations, otherwise accurate results are not
		 * guaranteed. Applying non-uniform scale guarantees rotation portion will not be accurate.
		 */
		void Decomposition(TQuaternion<T>& rotation, TVector3<T>& scale) const;

		/**
		 * Decomposes the matrix into various useful values.
		 *
		 * @param	matL	Unitary matrix. Columns form orthonormal bases. If your matrix is affine and
		 * 						doesn't use non-uniform scaling this matrix will be a conjugate transpose of the rotation part of the matrix.
		 * @param	matS	Singular values of the matrix. If your matrix is affine these will be scaling factors of the matrix.
		 * @param	matR	Unitary matrix. Columns form orthonormal bases. If your matrix is affine and
		 * 						doesn't use non-uniform scaling this matrix will be the rotation part of the matrix.
		 */
		void SingularValueDecomposition(TMatrix3& matL, TVector3<T>& matS, TMatrix3& matR) const;

		/**
		 * Decomposes the matrix into a set of values.
		 *
		 * @param	matQ	Columns form orthonormal bases. If your matrix is affine and
		 * 						doesn't use non-uniform scaling this matrix will be the rotation part of the matrix.
		 * @param	vecD	If the matrix is affine these will be scaling factors of the matrix.
		 * @param	vecU	If the matrix is affine these will be shear factors of the matrix.
		 */
		void QDUDecomposition(TMatrix3& matQ, TVector3<T>& vecD, TVector3<T>& vecU) const;

		/** Gram-Schmidt orthonormalization (applied to columns of rotation matrix) */
		void Orthonormalize();

		/**
		 * Converts an orthonormal matrix to axis angle representation.
		 *
		 * @note	Matrix must be orthonormal.
		 */
		void ToAxisAngle(TVector3<T>& axis, TRadian<T>& angle) const;

		/** Creates a rotation matrix from an axis angle representation. */
		void FromAxisAngle(const TVector3<T>& axis, const TRadian<T>& angle);

		/**
		 * Converts an orthonormal matrix to TQuaternion<T> representation.
		 *
		 * @note	Matrix must be orthonormal.
		 */
		void ToQuaternion(TQuaternion<T>& quat) const;

		/** Creates a rotation matrix from a TQuaternion<T> representation. */
		void FromQuaternion(const TQuaternion<T>& quat);

		/** Creates a matrix from a three axes. */
		void FromAxes(const TVector3<T>& xAxis, const TVector3<T>& yAxis, const TVector3<T>& zAxis);

		/**
		 * Converts an orthonormal matrix to euler angle (pitch/yaw/roll) representation.
		 *
		 * @param	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param	yAngle  Rotation about y axis. (AKA Yaw)
		 * @param	zAngle 	Rotation about z axis. (AKA Roll)
		 * @return					True if unique solution was found, false otherwise.
		 *
		 * @note	Matrix must be orthonormal.
		 */
		bool ToEulerAngles(TRadian<T>& xAngle, TRadian<T>& yAngle, TRadian<T>& zAngle) const;

		/**
		 * Creates a rotation matrix from the provided Pitch/Yaw/Roll angles.
		 *
		 * @param	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param	zAngle	Rotation about z axis. (AKA Roll)
		 *
		 * @note	Matrix must be orthonormal.
		 * 			Since different values will be produced depending in which order are the rotations applied, this method assumes
		 * 			they are applied in YXZ order. If you need a specific order, use the overloaded "fromEulerAngles" method instead.
		 */
		void FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle);

		/**
		 * Creates a rotation matrix from the provided Pitch/Yaw/Roll angles.
		 *
		 * @param	xAngle	Rotation about x axis. (AKA Pitch)
		 * @param	yAngle	Rotation about y axis. (AKA Yaw)
		 * @param	zAngle	Rotation about z axis. (AKA Roll)
		 * @param	order 	The order in which rotations will be applied.
		 *						Different rotations can be created depending on the order.
		 *
		 * @note	Matrix must be orthonormal.
		 */
		void FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order);

		/**
		 * Eigensolver, matrix must be symmetric.
		 *
		 * @note
		 * Eigenvectors are vectors which when transformed by the matrix, only change in magnitude, but not in direction.
		 * Eigenvalue is that magnitude. In other words you will get the same result whether you multiply the vector by the
		 * matrix or by its eigenvalue.
		 */
		void EigenSolveSymmetric(T eigenValues[3], TVector3<T> eigenVectors[3]) const;

		static constexpr const T kEpsilon = (T)1e-06;
		static const TMatrix3 kZero;
		static const TMatrix3 kIdentity;

	protected:
		template<typename T2>
		friend struct TMatrix4;

		// Support for eigensolver
		void Tridiagonal(T diag[3], T subDiag[3]);
		bool QLAlgorithm(T diag[3], T subDiag[3]);

		// Support for singular value decomposition
		static constexpr const T kSvdEpsilon = (T)1e-04;
		static constexpr const unsigned int kSvdMaxIters = 32;

		static void Bidiagonalize(TMatrix3& matA, TMatrix3& matL, TMatrix3& matR);
		static void GolubKahanStep(TMatrix3& matA, TMatrix3& matL, TMatrix3& matR);

		T m[3][3];
	};

	template<> const TMatrix3<float> TMatrix3<float>::kZero{kZeroTag};
	template<> const TMatrix3<double> TMatrix3<double>::kZero{kZeroTag};
	template<> const TMatrix3<float> TMatrix3<float>::kIdentity{kIdentityTag};
	template<> const TMatrix3<double> TMatrix3<double>::kIdentity{kIdentityTag};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TMatrix3. */
template<typename T>
struct hash<b3d::TMatrix3<T>>
{
	size_t operator()(const b3d::TMatrix3<T>& value) const
	{
		using namespace b3d;

		size_t hash = 0;

		for(u32 rowIndex = 0; rowIndex < 3; ++rowIndex)
		{
			for(u32 columnIndex = 0; columnIndex < 3; ++columnIndex)
				B3DCombineHash(hash, value[rowIndex][columnIndex]);
		}

		return hash;
	}
};
} // namespace std

/** @endcond */
