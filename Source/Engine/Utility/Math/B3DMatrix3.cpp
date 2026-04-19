//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DMatrix3.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<typename T>
TVector3<T> TMatrix3<T>::GetColumn(u32 col) const
{
	B3D_ASSERT(col < 3);

	return TVector3<T>(m[0][col], m[1][col], m[2][col]);
}

template<typename T>
void TMatrix3<T>::SetColumn(u32 col, const TVector3<T>& vec)
{
	B3D_ASSERT(col < 3);

	m[0][col] = vec.X;
	m[1][col] = vec.Y;
	m[2][col] = vec.Z;
}

template<typename T>
void TMatrix3<T>::FromAxes(const TVector3<T>& xAxis, const TVector3<T>& yAxis, const TVector3<T>& zAxis)
{
	SetColumn(0, xAxis);
	SetColumn(1, yAxis);
	SetColumn(2, zAxis);
}

template<typename T>
bool TMatrix3<T>::operator==(const TMatrix3& rhs) const
{
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
		{
			if(m[row][col] != rhs.m[row][col])
				return false;
		}
	}

	return true;
}

template<typename T>
bool TMatrix3<T>::operator!=(const TMatrix3& rhs) const
{
	return !operator==(rhs);
}

template<typename T>
TMatrix3<T> TMatrix3<T>::operator+(const TMatrix3& rhs) const
{
	TMatrix3 sum;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
		{
			sum.m[row][col] = m[row][col] + rhs.m[row][col];
		}
	}

	return sum;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::operator-(const TMatrix3& rhs) const
{
	TMatrix3 diff;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
		{
			diff.m[row][col] = m[row][col] -
				rhs.m[row][col];
		}
	}

	return diff;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::operator*(const TMatrix3& rhs) const
{
	TMatrix3 prod;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
		{
			prod.m[row][col] = m[row][0] * rhs.m[0][col] +
				m[row][1] * rhs.m[1][col] + m[row][2] * rhs.m[2][col];
		}
	}

	return prod;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::operator-() const
{
	TMatrix3 neg;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
			neg[row][col] = -m[row][col];
	}

	return neg;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::operator*(T rhs) const
{
	TMatrix3 prod;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
			prod[row][col] = rhs * m[row][col];
	}

	return prod;
}

template<typename T>
TVector3<T> TMatrix3<T>::Multiply(const TVector3<T>& vec) const
{
	TVector3<T> prod;
	for(u32 row = 0; row < 3; row++)
	{
		prod[row] =
			m[row][0] * vec[0] +
			m[row][1] * vec[1] +
			m[row][2] * vec[2];
	}

	return prod;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::Transpose() const
{
	TMatrix3 matTranspose;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
			matTranspose[row][col] = m[col][row];
	}

	return matTranspose;
}

template<typename T>
bool TMatrix3<T>::Inverse(TMatrix3& matInv, T tolerance) const
{
	matInv[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	matInv[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
	matInv[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];
	matInv[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
	matInv[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
	matInv[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];
	matInv[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
	matInv[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
	matInv[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

	T det = m[0][0] * matInv[0][0] + m[0][1] * matInv[1][0] + m[0][2] * matInv[2][0];

	if(abs(det) <= tolerance)
		return false;

	T invDet = (T)1.0 / det;
	for(u32 row = 0; row < 3; row++)
	{
		for(u32 col = 0; col < 3; col++)
			matInv[row][col] *= invDet;
	}

	return true;
}

template<typename T>
TMatrix3<T> TMatrix3<T>::Inverse(T tolerance) const
{
	TMatrix3 matInv = TMatrix3<T>::kZero;
	Inverse(matInv, tolerance);
	return matInv;
}

template<typename T>
T TMatrix3<T>::Determinant() const
{
	T cofactor00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	T cofactor10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
	T cofactor20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

	T det = m[0][0] * cofactor00 + m[0][1] * cofactor10 + m[0][2] * cofactor20;

	return det;
}

template<typename T>
void TMatrix3<T>::Bidiagonalize(TMatrix3& matA, TMatrix3& matL, TMatrix3& matR)
{
	T v[3], w[3];
	T length, sign, t1, invT1, t2;
	bool bIdentity;

	// Map first column to (*,0,0)
	length = Math::SquareRoot(matA[0][0] * matA[0][0] + matA[1][0] * matA[1][0] + matA[2][0] * matA[2][0]);
	if(length > (T)0.0)
	{
		sign = (matA[0][0] > (T)0.0 ? (T)1.0 : -(T)1.0);
		t1 = matA[0][0] + sign * length;
		invT1 = (T)1.0 / t1;
		v[1] = matA[1][0] * invT1;
		v[2] = matA[2][0] * invT1;

		t2 = (T)-2.0 / ((T)1.0 + v[1] * v[1] + v[2] * v[2]);
		w[0] = t2 * (matA[0][0] + matA[1][0] * v[1] + matA[2][0] * v[2]);
		w[1] = t2 * (matA[0][1] + matA[1][1] * v[1] + matA[2][1] * v[2]);
		w[2] = t2 * (matA[0][2] + matA[1][2] * v[1] + matA[2][2] * v[2]);
		matA[0][0] += w[0];
		matA[0][1] += w[1];
		matA[0][2] += w[2];
		matA[1][1] += v[1] * w[1];
		matA[1][2] += v[1] * w[2];
		matA[2][1] += v[2] * w[1];
		matA[2][2] += v[2] * w[2];

		matL[0][0] = (T)1.0 + t2;
		matL[0][1] = matL[1][0] = t2 * v[1];
		matL[0][2] = matL[2][0] = t2 * v[2];
		matL[1][1] = (T)1.0 + t2 * v[1] * v[1];
		matL[1][2] = matL[2][1] = t2 * v[1] * v[2];
		matL[2][2] = (T)1.0 + t2 * v[2] * v[2];
		bIdentity = false;
	}
	else
	{
		matL = TMatrix3<T>::kIdentity;
		bIdentity = true;
	}

	// Map first row to (*,*,0)
	length = Math::SquareRoot(matA[0][1] * matA[0][1] + matA[0][2] * matA[0][2]);
	if(length > (T)0.0)
	{
		sign = (matA[0][1] > (T)0.0 ? (T)1.0 : -(T)1.0);
		t1 = matA[0][1] + sign * length;
		v[2] = matA[0][2] / t1;

		t2 = (T)-2.0f / ((T)1.0 + v[2] * v[2]);
		w[0] = t2 * (matA[0][1] + matA[0][2] * v[2]);
		w[1] = t2 * (matA[1][1] + matA[1][2] * v[2]);
		w[2] = t2 * (matA[2][1] + matA[2][2] * v[2]);
		matA[0][1] += w[0];
		matA[1][1] += w[1];
		matA[1][2] += w[1] * v[2];
		matA[2][1] += w[2];
		matA[2][2] += w[2] * v[2];

		matR[0][0] = (T)1.0;
		matR[0][1] = matR[1][0] = (T)0.0;
		matR[0][2] = matR[2][0] = (T)0.0;
		matR[1][1] = (T)1.0 + t2;
		matR[1][2] = matR[2][1] = t2 * v[2];
		matR[2][2] = (T)1.0 + t2 * v[2] * v[2];
	}
	else
	{
		matR = TMatrix3<T>::kIdentity;
	}

	// Map second column to (*,*,0)
	length = Math::SquareRoot(matA[1][1] * matA[1][1] + matA[2][1] * matA[2][1]);
	if(length > (T)0.0)
	{
		sign = (matA[1][1] > (T)0.0 ? (T)1.0 : -(T)1.0);
		t1 = matA[1][1] + sign * length;
		v[2] = matA[2][1] / t1;

		t2 = (T)-2.0f / ((T)1.0 + v[2] * v[2]);
		w[1] = t2 * (matA[1][1] + matA[2][1] * v[2]);
		w[2] = t2 * (matA[1][2] + matA[2][2] * v[2]);
		matA[1][1] += w[1];
		matA[1][2] += w[2];
		matA[2][2] += v[2] * w[2];

		T a = (T)1.0 + t2;
		T b = t2 * v[2];
		T c = (T)1.0 + b * v[2];

		if(bIdentity)
		{
			matL[0][0] = (T)1.0;
			matL[0][1] = matL[1][0] = (T)0.0;
			matL[0][2] = matL[2][0] = (T)0.0;
			matL[1][1] = a;
			matL[1][2] = matL[2][1] = b;
			matL[2][2] = c;
		}
		else
		{
			for(int row = 0; row < 3; row++)
			{
				T tmp0 = matL[row][1];
				T tmp1 = matL[row][2];
				matL[row][1] = a * tmp0 + b * tmp1;
				matL[row][2] = b * tmp0 + c * tmp1;
			}
		}
	}
}

template<typename T>
void TMatrix3<T>::GolubKahanStep(TMatrix3& matA, TMatrix3& matL, TMatrix3& matR)
{
	T f11 = matA[0][1] * matA[0][1] + matA[1][1] * matA[1][1];
	T t22 = matA[1][2] * matA[1][2] + matA[2][2] * matA[2][2];
	T t12 = matA[1][1] * matA[1][2];
	T trace = f11 + t22;
	T diff = f11 - t22;
	T discr = Math::SquareRoot(diff * diff + (T)4.0 * t12 * t12);
	T root1 = (T)0.5 * (trace + discr);
	T root2 = (T)0.5 * (trace - discr);

	// Adjust right
	T y = matA[0][0] - (abs(root1 - t22) <= abs(root2 - t22) ? root1 : root2);
	T z = matA[0][1];
	T invLength = Math::InverseSquareRoot(y * y + z * z);
	T sin = z * invLength;
	T cos = -y * invLength;

	T tmp0 = matA[0][0];
	T tmp1 = matA[0][1];
	matA[0][0] = cos * tmp0 - sin * tmp1;
	matA[0][1] = sin * tmp0 + cos * tmp1;
	matA[1][0] = -sin * matA[1][1];
	matA[1][1] *= cos;

	u32 row;
	for(row = 0; row < 3; row++)
	{
		tmp0 = matR[0][row];
		tmp1 = matR[1][row];
		matR[0][row] = cos * tmp0 - sin * tmp1;
		matR[1][row] = sin * tmp0 + cos * tmp1;
	}

	// Adjust left
	y = matA[0][0];
	z = matA[1][0];
	invLength = Math::InverseSquareRoot(y * y + z * z);
	sin = z * invLength;
	cos = -y * invLength;

	matA[0][0] = cos * matA[0][0] - sin * matA[1][0];
	tmp0 = matA[0][1];
	tmp1 = matA[1][1];
	matA[0][1] = cos * tmp0 - sin * tmp1;
	matA[1][1] = sin * tmp0 + cos * tmp1;
	matA[0][2] = -sin * matA[1][2];
	matA[1][2] *= cos;

	u32 col;
	for(col = 0; col < 3; col++)
	{
		tmp0 = matL[col][0];
		tmp1 = matL[col][1];
		matL[col][0] = cos * tmp0 - sin * tmp1;
		matL[col][1] = sin * tmp0 + cos * tmp1;
	}

	// Adjust right
	y = matA[0][1];
	z = matA[0][2];
	invLength = Math::InverseSquareRoot(y * y + z * z);
	sin = z * invLength;
	cos = -y * invLength;

	matA[0][1] = cos * matA[0][1] - sin * matA[0][2];
	tmp0 = matA[1][1];
	tmp1 = matA[1][2];
	matA[1][1] = cos * tmp0 - sin * tmp1;
	matA[1][2] = sin * tmp0 + cos * tmp1;
	matA[2][1] = -sin * matA[2][2];
	matA[2][2] *= cos;

	for(row = 0; row < 3; row++)
	{
		tmp0 = matR[1][row];
		tmp1 = matR[2][row];
		matR[1][row] = cos * tmp0 - sin * tmp1;
		matR[2][row] = sin * tmp0 + cos * tmp1;
	}

	// Adjust left
	y = matA[1][1];
	z = matA[2][1];
	invLength = Math::InverseSquareRoot(y * y + z * z);
	sin = z * invLength;
	cos = -y * invLength;

	matA[1][1] = cos * matA[1][1] - sin * matA[2][1];
	tmp0 = matA[1][2];
	tmp1 = matA[2][2];
	matA[1][2] = cos * tmp0 - sin * tmp1;
	matA[2][2] = sin * tmp0 + cos * tmp1;

	for(col = 0; col < 3; col++)
	{
		tmp0 = matL[col][1];
		tmp1 = matL[col][2];
		matL[col][1] = cos * tmp0 - sin * tmp1;
		matL[col][2] = sin * tmp0 + cos * tmp1;
	}
}

template<typename T>
void TMatrix3<T>::SingularValueDecomposition(TMatrix3& matL, TVector3<T>& matS, TMatrix3& matR) const
{
	u32 row, col;

	TMatrix3 mat = *this;
	Bidiagonalize(mat, matL, matR);

	for(unsigned int iterationIndex = 0; iterationIndex < kSvdMaxIters; iterationIndex++)
	{
		T tmp, tmp0, tmp1;
		T sin0, cos0, tan0;
		T sin1, cos1, tan1;

		bool test1 = (abs(mat[0][1]) <= kSvdEpsilon * (abs(mat[0][0]) + abs(mat[1][1])));
		bool test2 = (abs(mat[1][2]) <= kSvdEpsilon * (abs(mat[1][1]) + abs(mat[2][2])));

		if(test1)
		{
			if(test2)
			{
				matS[0] = mat[0][0];
				matS[1] = mat[1][1];
				matS[2] = mat[2][2];
				break;
			}
			else
			{
				// 2x2 closed form factorization
				tmp = (mat[1][1] * mat[1][1] - mat[2][2] * mat[2][2] + mat[1][2] * mat[1][2]) / (mat[1][2] * mat[2][2]);
				tan0 = (T)0.5 * (tmp + Math::SquareRoot(tmp * tmp + (T)4.0));
				cos0 = Math::InverseSquareRoot((T)1.0 + tan0 * tan0);
				sin0 = tan0 * cos0;

				for(col = 0; col < 3; col++)
				{
					tmp0 = matL[col][1];
					tmp1 = matL[col][2];
					matL[col][1] = cos0 * tmp0 - sin0 * tmp1;
					matL[col][2] = sin0 * tmp0 + cos0 * tmp1;
				}

				tan1 = (mat[1][2] - mat[2][2] * tan0) / mat[1][1];
				cos1 = Math::InverseSquareRoot((T)1.0 + tan1 * tan1);
				sin1 = -tan1 * cos1;

				for(row = 0; row < 3; row++)
				{
					tmp0 = matR[1][row];
					tmp1 = matR[2][row];
					matR[1][row] = cos1 * tmp0 - sin1 * tmp1;
					matR[2][row] = sin1 * tmp0 + cos1 * tmp1;
				}

				matS[0] = mat[0][0];
				matS[1] = cos0 * cos1 * mat[1][1] - sin1 * (cos0 * mat[1][2] - sin0 * mat[2][2]);
				matS[2] = sin0 * sin1 * mat[1][1] + cos1 * (sin0 * mat[1][2] + cos0 * mat[2][2]);
				break;
			}
		}
		else
		{
			if(test2)
			{
				// 2x2 closed form factorization
				tmp = (mat[0][0] * mat[0][0] + mat[1][1] * mat[1][1] - mat[0][1] * mat[0][1]) / (mat[0][1] * mat[1][1]);
				tan0 = (T)0.5 * (-tmp + Math::SquareRoot(tmp * tmp + (T)4.0));
				cos0 = Math::InverseSquareRoot((T)1.0 + tan0 * tan0);
				sin0 = tan0 * cos0;

				for(col = 0; col < 3; col++)
				{
					tmp0 = matL[col][0];
					tmp1 = matL[col][1];
					matL[col][0] = cos0 * tmp0 - sin0 * tmp1;
					matL[col][1] = sin0 * tmp0 + cos0 * tmp1;
				}

				tan1 = (mat[0][1] - mat[1][1] * tan0) / mat[0][0];
				cos1 = Math::InverseSquareRoot((T)1.0 + tan1 * tan1);
				sin1 = -tan1 * cos1;

				for(row = 0; row < 3; row++)
				{
					tmp0 = matR[0][row];
					tmp1 = matR[1][row];
					matR[0][row] = cos1 * tmp0 - sin1 * tmp1;
					matR[1][row] = sin1 * tmp0 + cos1 * tmp1;
				}

				matS[0] = cos0 * cos1 * mat[0][0] - sin1 * (cos0 * mat[0][1] - sin0 * mat[1][1]);
				matS[1] = sin0 * sin1 * mat[0][0] + cos1 * (sin0 * mat[0][1] + cos0 * mat[1][1]);
				matS[2] = mat[2][2];
				break;
			}
			else
			{
				GolubKahanStep(mat, matL, matR);
			}
		}
	}

	// Positize diagonal
	for(row = 0; row < 3; row++)
	{
		if(matS[row] < 0.0)
		{
			matS[row] = -matS[row];
			for(col = 0; col < 3; col++)
				matR[row][col] = -matR[row][col];
		}
	}
}

template<typename T>
void TMatrix3<T>::Orthonormalize()
{
	// Compute q0
	T invLength = Math::InverseSquareRoot(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);

	m[0][0] *= invLength;
	m[1][0] *= invLength;
	m[2][0] *= invLength;

	// Compute q1
	T dot0 = m[0][0] * m[0][1] + m[1][0] * m[1][1] + m[2][0] * m[2][1];

	m[0][1] -= dot0 * m[0][0];
	m[1][1] -= dot0 * m[1][0];
	m[2][1] -= dot0 * m[2][0];

	invLength = Math::InverseSquareRoot(m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1]);

	m[0][1] *= invLength;
	m[1][1] *= invLength;
	m[2][1] *= invLength;

	// Compute q2
	T dot1 = m[0][1] * m[0][2] + m[1][1] * m[1][2] + m[2][1] * m[2][2];
	dot0 = m[0][0] * m[0][2] + m[1][0] * m[1][2] + m[2][0] * m[2][2];

	m[0][2] -= dot0 * m[0][0] + dot1 * m[0][1];
	m[1][2] -= dot0 * m[1][0] + dot1 * m[1][1];
	m[2][2] -= dot0 * m[2][0] + dot1 * m[2][1];

	invLength = Math::InverseSquareRoot(m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2]);

	m[0][2] *= invLength;
	m[1][2] *= invLength;
	m[2][2] *= invLength;
}

template<typename T>
void TMatrix3<T>::Decomposition(TQuaternion<T>& rotation, TVector3<T>& scale) const
{
	TMatrix3 matQ;
	TVector3<T> vecU;
	QDUDecomposition(matQ, scale, vecU);

	rotation = TQuaternion<T>(matQ);
}

template<typename T>
void TMatrix3<T>::QDUDecomposition(TMatrix3& matQ, TVector3<T>& vecD, TVector3<T>& vecU) const
{
	// Build orthogonal matrix Q
	T invLength = Math::InverseSquareRoot(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);
	matQ[0][0] = m[0][0] * invLength;
	matQ[1][0] = m[1][0] * invLength;
	matQ[2][0] = m[2][0] * invLength;

	T dot = matQ[0][0] * m[0][1] + matQ[1][0] * m[1][1] + matQ[2][0] * m[2][1];
	matQ[0][1] = m[0][1] - dot * matQ[0][0];
	matQ[1][1] = m[1][1] - dot * matQ[1][0];
	matQ[2][1] = m[2][1] - dot * matQ[2][0];

	invLength = Math::InverseSquareRoot(matQ[0][1] * matQ[0][1] + matQ[1][1] * matQ[1][1] + matQ[2][1] * matQ[2][1]);
	matQ[0][1] *= invLength;
	matQ[1][1] *= invLength;
	matQ[2][1] *= invLength;

	dot = matQ[0][0] * m[0][2] + matQ[1][0] * m[1][2] + matQ[2][0] * m[2][2];
	matQ[0][2] = m[0][2] - dot * matQ[0][0];
	matQ[1][2] = m[1][2] - dot * matQ[1][0];
	matQ[2][2] = m[2][2] - dot * matQ[2][0];

	dot = matQ[0][1] * m[0][2] + matQ[1][1] * m[1][2] + matQ[2][1] * m[2][2];
	matQ[0][2] -= dot * matQ[0][1];
	matQ[1][2] -= dot * matQ[1][1];
	matQ[2][2] -= dot * matQ[2][1];

	invLength = Math::InverseSquareRoot(matQ[0][2] * matQ[0][2] + matQ[1][2] * matQ[1][2] + matQ[2][2] * matQ[2][2]);
	matQ[0][2] *= invLength;
	matQ[1][2] *= invLength;
	matQ[2][2] *= invLength;

	// Guarantee that orthogonal matrix has determinant 1 (no reflections)
	T fDet = matQ[0][0] * matQ[1][1] * matQ[2][2] + matQ[0][1] * matQ[1][2] * matQ[2][0] +
		matQ[0][2] * matQ[1][0] * matQ[2][1] - matQ[0][2] * matQ[1][1] * matQ[2][0] -
		matQ[0][1] * matQ[1][0] * matQ[2][2] - matQ[0][0] * matQ[1][2] * matQ[2][1];

	if(fDet < (T)0.0)
	{
		for(u32 row = 0; row < 3; row++)
			for(u32 col = 0; col < 3; col++)
				matQ[row][col] = -matQ[row][col];
	}

	// Build "right" matrix R
	TMatrix3 matRight;
	matRight[0][0] = matQ[0][0] * m[0][0] + matQ[1][0] * m[1][0] +
		matQ[2][0] * m[2][0];
	matRight[0][1] = matQ[0][0] * m[0][1] + matQ[1][0] * m[1][1] +
		matQ[2][0] * m[2][1];
	matRight[1][1] = matQ[0][1] * m[0][1] + matQ[1][1] * m[1][1] +
		matQ[2][1] * m[2][1];
	matRight[0][2] = matQ[0][0] * m[0][2] + matQ[1][0] * m[1][2] +
		matQ[2][0] * m[2][2];
	matRight[1][2] = matQ[0][1] * m[0][2] + matQ[1][1] * m[1][2] +
		matQ[2][1] * m[2][2];
	matRight[2][2] = matQ[0][2] * m[0][2] + matQ[1][2] * m[1][2] +
		matQ[2][2] * m[2][2];

	// The scaling component
	vecD[0] = matRight[0][0];
	vecD[1] = matRight[1][1];
	vecD[2] = matRight[2][2];

	// The shear component
	T invD0 = (T)1.0 / vecD[0];
	vecU[0] = matRight[0][1] * invD0;
	vecU[1] = matRight[0][2] * invD0;
	vecU[2] = matRight[1][2] / vecD[1];
}

template<typename T>
void TMatrix3<T>::ToAxisAngle(TVector3<T>& axis, TRadian<T>& radians) const
{
	T trace = m[0][0] + m[1][1] + m[2][2];
	T cos = (T)0.5 * (trace - (T)1.0);
	radians = Math::Acos(cos); // In [0, PI]

	if(radians > TRadian<T>((T)0.0))
	{
		if(radians < TRadian<T>(Math::kPi))
		{
			axis.X = m[2][1] - m[1][2];
			axis.Y = m[0][2] - m[2][0];
			axis.Z = m[1][0] - m[0][1];
			axis.Normalize();
		}
		else
		{
			// Angle is PI
			T fHalfInverse;
			if(m[0][0] >= m[1][1])
			{
				// r00 >= r11
				if(m[0][0] >= m[2][2])
				{
					// r00 is maximum diagonal term
					axis.X = (T)0.5 * Math::SquareRoot(m[0][0] - m[1][1] - m[2][2] + (T)1.0);
					fHalfInverse = (T)0.5 / axis.X;
					axis.Y = fHalfInverse * m[0][1];
					axis.Z = fHalfInverse * m[0][2];
				}
				else
				{
					// r22 is maximum diagonal term
					axis.Z = (T)0.5 * Math::SquareRoot(m[2][2] - m[0][0] - m[1][1] + (T)1.0);
					fHalfInverse = (T)0.5 / axis.Z;
					axis.X = fHalfInverse * m[0][2];
					axis.Y = fHalfInverse * m[1][2];
				}
			}
			else
			{
				// r11 > r00
				if(m[1][1] >= m[2][2])
				{
					// r11 is maximum diagonal term
					axis.Y = (T)0.5 * Math::SquareRoot(m[1][1] - m[0][0] - m[2][2] + (T)1.0);
					fHalfInverse = (T)0.5 / axis.Y;
					axis.X = fHalfInverse * m[0][1];
					axis.Z = fHalfInverse * m[1][2];
				}
				else
				{
					// r22 is maximum diagonal term
					axis.Z = (T)0.5 * Math::SquareRoot(m[2][2] - m[0][0] - m[1][1] + (T)1.0);
					fHalfInverse = (T)0.5 / axis.Z;
					axis.X = fHalfInverse * m[0][2];
					axis.Y = fHalfInverse * m[1][2];
				}
			}
		}
	}
	else
	{
		// The angle is 0 and the matrix is the identity.  Any axis will
		// work, so just use the x-axis.
		axis.X = (T)1.0;
		axis.Y = (T)0.0;
		axis.Z = (T)0.0;
	}
}

template<typename T>
void TMatrix3<T>::FromAxisAngle(const TVector3<T>& axis, const TRadian<T>& angle)
{
	T cos = Math::Cos(angle);
	T sin = Math::Sin(angle);
	T oneMinusCos = (T)1.0 - cos;
	T x2 = axis.X * axis.X;
	T y2 = axis.Y * axis.Y;
	T z2 = axis.Z * axis.Z;
	T xym = axis.X * axis.Y * oneMinusCos;
	T xzm = axis.X * axis.Z * oneMinusCos;
	T yzm = axis.Y * axis.Z * oneMinusCos;
	T xSin = axis.X * sin;
	T ySin = axis.Y * sin;
	T zSin = axis.Z * sin;

	m[0][0] = x2 * oneMinusCos + cos;
	m[0][1] = xym - zSin;
	m[0][2] = xzm + ySin;
	m[1][0] = xym + zSin;
	m[1][1] = y2 * oneMinusCos + cos;
	m[1][2] = yzm - xSin;
	m[2][0] = xzm - ySin;
	m[2][1] = yzm + xSin;
	m[2][2] = z2 * oneMinusCos + cos;
}

template<typename T>
void TMatrix3<T>::ToQuaternion(TQuaternion<T>& quat) const
{
	quat.FromRotationMatrix(*this);
}

template<typename T>
void TMatrix3<T>::FromQuaternion(const TQuaternion<T>& quat)
{
	quat.ToRotationMatrix(*this);
}

template<typename T>
bool TMatrix3<T>::ToEulerAngles(TRadian<T>& xAngle, TRadian<T>& yAngle, TRadian<T>& zAngle) const
{
	T m21 = m[2][1];
	if(m21 < 1)
	{
		if(m21 > -1)
		{
			xAngle = TRadian<T>(Math::Asin(m21));
			yAngle = atan2(-m[2][0], m[2][2]);
			zAngle = atan2(-m[0][1], m[1][1]);

			return true;
		}
		else
		{
			// Note: Not an unique solution.
			xAngle = TRadian<T>(-Math::kHalfPi);
			yAngle = TRadian<T>((T)0.0);
			zAngle = -atan2(m[0][2], m[0][0]);

			return false;
		}
	}
	else
	{
		// Note: Not an unique solution.
		xAngle = TRadian<T>(Math::kHalfPi);
		yAngle = TRadian<T>((T)0.0);
		zAngle = atan2(m[0][2], m[0][0]);

		return false;
	}
}

template<typename T>
void TMatrix3<T>::FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle)
{
	T cx = Math::Cos(xAngle);
	T sx = Math::Sin(xAngle);

	T cy = Math::Cos(yAngle);
	T sy = Math::Sin(yAngle);

	T cz = Math::Cos(zAngle);
	T sz = Math::Sin(zAngle);

	m[0][0] = cy * cz - sx * sy * sz;
	m[0][1] = -cx * sz;
	m[0][2] = cz * sy + cy * sx * sz;

	m[1][0] = cz * sx * sy + cy * sz;
	m[1][1] = cx * cz;
	m[1][2] = -cy * cz * sx + sy * sz;

	m[2][0] = -cx * sy;
	m[2][1] = sx;
	m[2][2] = cx * cy;
}

template<typename T>
void TMatrix3<T>::FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order)
{
	// Euler angle conversions
	static constexpr const EulerAngleOrderData kEaLookup[6] = { { 0, 1, 2, (T)1.0 }, { 0, 2, 1, -(T)1.0 }, { 1, 0, 2, -(T)1.0 }, { 1, 2, 0, (T)1.0 }, { 2, 0, 1, (T)1.0 }, { 2, 1, 0, -(T)1.0 } };

	const EulerAngleOrderData& l = kEaLookup[(int)order];

	TMatrix3 mats[3];
	T cx = Math::Cos(xAngle);
	T sx = Math::Sin(xAngle);
	mats[0] = TMatrix3(
		(T)1.0, (T)0.0, (T)0.0,
		(T)0.0, cx, -sx,
		(T)0.0, sx, cx);

	T cy = Math::Cos(yAngle);
	T sy = Math::Sin(yAngle);
	mats[1] = TMatrix3(
		cy, (T)0.0, sy,
		(T)0.0, (T)1.0, (T)0.0,
		-sy, (T)0.0, cy);

	T cz = Math::Cos(zAngle);
	T sz = Math::Sin(zAngle);
	mats[2] = TMatrix3(
		cz, -sz, (T)0.0,
		sz, cz, (T)0.0,
		(T)0.0, (T)0.0, (T)1.0);

	*this = mats[l.C] * (mats[l.B] * mats[l.A]);
}

template<typename T>
void TMatrix3<T>::Tridiagonal(T diag[3], T subDiag[3])
{
	// Householder reduction T = Q^t M Q
	//   Input:
	//     mat, symmetric 3x3 matrix M
	//   Output:
	//     mat, orthogonal matrix Q
	//     diag, diagonal entries of T
	//     subd, subdiagonal entries of T (T is symmetric)

	T fA = m[0][0];
	T fB = m[0][1];
	T fC = m[0][2];
	T fD = m[1][1];
	T fE = m[1][2];
	T fF = m[2][2];

	diag[0] = fA;
	subDiag[2] = (T)0.0;
	if(abs(fC) >= kEpsilon)
	{
		T length = Math::SquareRoot(fB * fB + fC * fC);
		T invLength = (T)1.0 / length;
		fB *= invLength;
		fC *= invLength;
		T fQ = (T)2.0 * fB * fE + fC * (fF - fD);
		diag[1] = fD + fC * fQ;
		diag[2] = fF - fC * fQ;
		subDiag[0] = length;
		subDiag[1] = fE - fB * fQ;
		m[0][0] = (T)1.0;
		m[0][1] = (T)0.0;
		m[0][2] = (T)0.0;
		m[1][0] = (T)0.0;
		m[1][1] = fB;
		m[1][2] = fC;
		m[2][0] = (T)0.0;
		m[2][1] = fC;
		m[2][2] = -fB;
	}
	else
	{
		diag[1] = fD;
		diag[2] = fF;
		subDiag[0] = fB;
		subDiag[1] = fE;
		m[0][0] = (T)1.0;
		m[0][1] = (T)0.0;
		m[0][2] = (T)0.0;
		m[1][0] = (T)0.0;
		m[1][1] = (T)1.0;
		m[1][2] = (T)0.0;
		m[2][0] = (T)0.0;
		m[2][1] = (T)0.0;
		m[2][2] = (T)1.0;
	}
}

template<typename T>
bool TMatrix3<T>::QLAlgorithm(T diag[3], T subDiag[3])
{
	// QL iteration with implicit shifting to reduce matrix from tridiagonal to diagonal

	for(int elementIndex = 0; elementIndex < 3; elementIndex++)
	{
		const unsigned int maxIter = 32;
		unsigned int iter;
		for(iter = 0; iter < maxIter; iter++)
		{
			int j;
			for(j = elementIndex; j <= 1; j++)
			{
				T sum = abs(diag[j]) + abs(diag[j + 1]);

				if(abs(subDiag[j]) + sum == sum)
					break;
			}

			if(j == elementIndex)
				break;

			T tmp0 = (diag[elementIndex + 1] - diag[elementIndex]) / ((T)2.0 * subDiag[elementIndex]);
			T tmp1 = Math::SquareRoot(tmp0 * tmp0 + (T)1.0);

			if(tmp0 < (T)0.0)
				tmp0 = diag[j] - diag[elementIndex] + subDiag[elementIndex] / (tmp0 - tmp1);
			else
				tmp0 = diag[j] - diag[elementIndex] + subDiag[elementIndex] / (tmp0 + tmp1);

			T sin = (T)1.0;
			T cos = (T)1.0;
			T tmp2 = (T)0.0;
			for(int k = j - 1; k >= elementIndex; k--)
			{
				T tmp3 = sin * subDiag[k];
				T tmp4 = cos * subDiag[k];

				if(abs(tmp3) >= abs(tmp0))
				{
					cos = tmp0 / tmp3;
					tmp1 = Math::SquareRoot(cos * cos + (T)1.0);
					subDiag[k + 1] = tmp3 * tmp1;
					sin = (T)1.0 / tmp1;
					cos *= sin;
				}
				else
				{
					sin = tmp3 / tmp0;
					tmp1 = Math::SquareRoot(sin * sin + (T)1.0);
					subDiag[k + 1] = tmp0 * tmp1;
					cos = (T)1.0 / tmp1;
					sin *= cos;
				}

				tmp0 = diag[k + 1] - tmp2;
				tmp1 = (diag[k] - tmp0) * sin + 2.0f * tmp4 * cos;
				tmp2 = sin * tmp1;
				diag[k + 1] = tmp0 + tmp2;
				tmp0 = cos * tmp1 - tmp4;

				for(int row = 0; row < 3; row++)
				{
					tmp3 = m[row][k + 1];
					m[row][k + 1] = sin * m[row][k] + cos * tmp3;
					m[row][k] = cos * m[row][k] - sin * tmp3;
				}
			}

			diag[elementIndex] -= tmp2;
			subDiag[elementIndex] = tmp0;
			subDiag[j] = 0.0;
		}

		if(iter == maxIter)
		{
			// Should not get here under normal circumstances
			return false;
		}
	}

	return true;
}

template<typename T>
void TMatrix3<T>::EigenSolveSymmetric(T eigenValues[3], TVector3<T> eigenVectors[3]) const
{
	TMatrix3 mat = *this;
	T subDiag[3];
	mat.Tridiagonal(eigenValues, subDiag);
	mat.QLAlgorithm(eigenValues, subDiag);

	for(u32 vectorIndex = 0; vectorIndex < 3; vectorIndex++)
	{
		eigenVectors[vectorIndex][0] = mat[0][vectorIndex];
		eigenVectors[vectorIndex][1] = mat[1][vectorIndex];
		eigenVectors[vectorIndex][2] = mat[2][vectorIndex];
	}

	// Make eigenvectors form a right--handed system
	TVector3<T> cross = eigenVectors[1].Cross(eigenVectors[2]);
	T det = eigenVectors[0].Dot(cross);
	if(det < (T)0.0)
	{
		eigenVectors[2][0] = -eigenVectors[2][0];
		eigenVectors[2][1] = -eigenVectors[2][1];
		eigenVectors[2][2] = -eigenVectors[2][2];
	}
}

template struct B3D_EXPORT TMatrix3<float>;
template struct B3D_EXPORT TMatrix3<double>;
