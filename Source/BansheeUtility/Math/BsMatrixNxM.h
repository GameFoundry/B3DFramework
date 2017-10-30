//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Class representing a NxM matrix.
	 *
	 * @note	If you need to use matrices for more than just data storage then
	 *			it is suggested you use specialized Matrix3 or Matrix4 classes
	 *			as they provide a wide range of functionality.
	 */
	template<int N, int M>
	class MatrixNxM
	{
	public:
		MatrixNxM() {}

		MatrixNxM(float data[N*M])
		{
			memcpy(m, data, N*M * sizeof(float));
		}

		/** Returns a transpose of the matrix (switched columns and rows). */
		MatrixNxM<M, N> transpose() const
		{
			MatrixNxM<M, N> matTranspose;
			for (UINT32 row = 0; row < N; row++)
			{
				for (UINT32 col = 0; col < M; col++)
					matTranspose[col][row] = m[row][col];
			}

			return matTranspose;
		}

		/** Returns a row of the matrix. */
        float* operator[] (UINT32 row) const
		{
			assert(row < N);

			return (float*)m[row];
		}

		MatrixNxM<N, M>& operator= (const MatrixNxM<N, M>& rhs)
		{
			memcpy(m, rhs.m, (N*M)*sizeof(float));
			return *this;
		}

		bool operator== (const MatrixNxM<N, M>& rhs) const
		{
			for (UINT32 row = 0; row < N; row++)
			{
				for (UINT32 col = 0; col < M; col++)
				{
					if (m[row][col] != rhs.m[row][col])
						return false;
				}
			}

			return true;
		}

		bool operator!= (const MatrixNxM<N, M>& rhs) const
		{
			return !operator==(rhs);
		}

		float m[N][M];
	};

	typedef MatrixNxM<2, 2> Matrix2;
	typedef MatrixNxM<2, 3> Matrix2x3;
	typedef MatrixNxM<2, 4> Matrix2x4;
	typedef MatrixNxM<3, 2> Matrix3x2;
	typedef MatrixNxM<3, 4> Matrix3x4;
	typedef MatrixNxM<4, 2> Matrix4x2;
	typedef MatrixNxM<4, 3> Matrix4x3;

	/** @} */
}