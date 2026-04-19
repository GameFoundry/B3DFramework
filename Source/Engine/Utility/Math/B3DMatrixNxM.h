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
	 * Class representing a NxM matrix.
	 *
	 * @note	If you need to use matrices for more than just data storage then
	 *			it is suggested you use specialized Matrix3 or Matrix4 classes
	 *			as they provide a wide range of functionality.
	 */
	template <int N, int M>
	class MatrixNxM
	{
	public:
		MatrixNxM() = default;
		MatrixNxM(const MatrixNxM&) = default;
		MatrixNxM& operator=(const MatrixNxM&) = default;

		explicit MatrixNxM(float data[N * M])
		{
			memcpy(Data, data, N * M * sizeof(float));
		}

		/** Returns a transpose of the matrix (switched columns and rows). */
		MatrixNxM<M, N> Transpose() const
		{
			MatrixNxM<M, N> matTranspose;
			for(u32 row = 0; row < N; row++)
			{
				for(u32 col = 0; col < M; col++)
					matTranspose[col][row] = Data[row][col];
			}

			return matTranspose;
		}

		/** Returns a row of the matrix. */
		float* operator[](u32 row) const
		{
			B3D_ASSERT(row < N);

			return (float*)Data[row];
		}

		bool operator==(const MatrixNxM& rhs) const
		{
			for(u32 row = 0; row < N; row++)
			{
				for(u32 col = 0; col < M; col++)
				{
					if(Data[row][col] != rhs.Data[row][col])
						return false;
				}
			}

			return true;
		}

		bool operator!=(const MatrixNxM& rhs) const
		{
			return !operator==(rhs);
		}

		float Data[N][M];
	};

	typedef MatrixNxM<2, 2> Matrix2;
	typedef MatrixNxM<2, 3> Matrix2x3;
	typedef MatrixNxM<2, 4> Matrix2x4;
	typedef MatrixNxM<3, 2> Matrix3x2;
	typedef MatrixNxM<3, 4> Matrix3x4;
	typedef MatrixNxM<4, 2> Matrix4x2;
	typedef MatrixNxM<4, 3> Matrix4x3;

	/** @} */
} // namespace b3d
