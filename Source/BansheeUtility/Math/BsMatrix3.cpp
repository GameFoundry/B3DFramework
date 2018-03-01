//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Math/BsMatrix3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsMath.h"

namespace bs
{
	const float Matrix3::EPSILON = 1e-06f;
	const Matrix3 Matrix3::ZERO(0,0,0,0,0,0,0,0,0);
	const Matrix3 Matrix3::IDENTITY(1,0,0,0,1,0,0,0,1);
	const float Matrix3::SVD_EPSILON = 1e-04f;
	const unsigned int Matrix3::SVD_MAX_ITERS = 32;
	const Matrix3::EulerAngleOrderData Matrix3::EA_LOOKUP[6] = 
		{ { 0, 1, 2, 1.0f}, { 0, 2, 1, -1.0f}, { 1, 0, 2, -1.0f},
		  { 1, 2, 0, 1.0f}, { 2, 0, 1,  1.0f}, { 2, 1, 0, -1.0f} };;


	Vector3 Matrix3::getColumn(UINT32 col) const
	{
		assert(col < 3);
		
		return Vector3(m[0][col],m[1][col], m[2][col]);
	}

	void Matrix3::setColumn(UINT32 col, const Vector3& vec)
	{
		assert(col < 3);

		m[0][col] = vec.x;
		m[1][col] = vec.y;
		m[2][col] = vec.z;
	}

	void Matrix3::fromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
	{
		setColumn(0, xAxis);
		setColumn(1, yAxis);
		setColumn(2, zAxis);
	}

	bool Matrix3::operator== (const Matrix3& rhs) const
	{
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
			{
				if (m[row][col] != rhs.m[row][col])
					return false;
			}
		}

		return true;
	}

	bool Matrix3::operator!= (const Matrix3& rhs) const
	{
		return !operator==(rhs);
	}

	Matrix3 Matrix3::operator+ (const Matrix3& rhs) const
	{
		Matrix3 sum;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
			{
				sum.m[row][col] = m[row][col] + rhs.m[row][col];
			}
		}

		return sum;
	}

	Matrix3 Matrix3::operator- (const Matrix3& rhs) const
	{
		Matrix3 diff;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
			{
				diff.m[row][col] = m[row][col] -
					rhs.m[row][col];
			}
		}

		return diff;
	}

	Matrix3 Matrix3::operator* (const Matrix3& rhs) const
	{
		Matrix3 prod;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
			{
				prod.m[row][col] = m[row][0]*rhs.m[0][col] +
					m[row][1]*rhs.m[1][col] + m[row][2]*rhs.m[2][col];
			}
		}

		return prod;
	}

	Matrix3 Matrix3::operator- () const
	{
		Matrix3 neg;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
				neg[row][col] = -m[row][col];
		}

		return neg;
	}

	Matrix3 Matrix3::operator* (float rhs) const
	{
		Matrix3 prod;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
				prod[row][col] = rhs*m[row][col];
		}

		return prod;
	}

	Matrix3 operator* (float lhs, const Matrix3& rhs)
	{
		Matrix3 prod;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
				prod[row][col] = lhs*rhs.m[row][col];
		}

		return prod;
	}

	Vector3 Matrix3::multiply(const Vector3& vec) const
	{
		Vector3 prod;
		for (UINT32 row = 0; row < 3; row++)
		{
			prod[row] =
				m[row][0]*vec[0] +
				m[row][1]*vec[1] +
				m[row][2]*vec[2];
		}

		return prod;
	}

	Matrix3 Matrix3::transpose() const
	{
		Matrix3 matTranspose;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
				matTranspose[row][col] = m[col][row];
		}

		return matTranspose;
	}

	bool Matrix3::inverse(Matrix3& matInv, float tolerance) const
	{
		matInv[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
		matInv[0][1] = m[0][2]*m[2][1] - m[0][1]*m[2][2];
		matInv[0][2] = m[0][1]*m[1][2] - m[0][2]*m[1][1];
		matInv[1][0] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
		matInv[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
		matInv[1][2] = m[0][2]*m[1][0] - m[0][0]*m[1][2];
		matInv[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
		matInv[2][1] = m[0][1]*m[2][0] - m[0][0]*m[2][1];
		matInv[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];

		float det = m[0][0]*matInv[0][0] + m[0][1]*matInv[1][0] + m[0][2]*matInv[2][0];

		if (Math::abs(det) <= tolerance)
			return false;

		float invDet = 1.0f/det;
		for (UINT32 row = 0; row < 3; row++)
		{
			for (UINT32 col = 0; col < 3; col++)
				matInv[row][col] *= invDet;
		}

		return true;
	}

	Matrix3 Matrix3::inverse(float tolerance) const
	{
		Matrix3 matInv = Matrix3::ZERO;
		inverse(matInv, tolerance);
		return matInv;
	}

	float Matrix3::determinant() const
	{
		float cofactor00 = m[1][1]*m[2][2] - m[1][2]*m[2][1];
		float cofactor10 = m[1][2]*m[2][0] - m[1][0]*m[2][2];
		float cofactor20 = m[1][0]*m[2][1] - m[1][1]*m[2][0];

		float det = m[0][0]*cofactor00 + m[0][1]*cofactor10 + m[0][2]*cofactor20;

		return det;
	}

	void Matrix3::bidiagonalize (Matrix3& matA, Matrix3& matL, Matrix3& matR)
	{
		float v[3], w[3];
		float length, sign, t1, invT1, t2;
		bool bIdentity;

		// Map first column to (*,0,0)
		length = Math::sqrt(matA[0][0]*matA[0][0] + matA[1][0]*matA[1][0] + matA[2][0]*matA[2][0]);
		if (length > 0.0f)
		{
			sign = (matA[0][0] > 0.0f ? 1.0f : -1.0f);
			t1 = matA[0][0] + sign*length;
			invT1 = 1.0f/t1;
			v[1] = matA[1][0]*invT1;
			v[2] = matA[2][0]*invT1;

			t2 = -2.0f/(1.0f+v[1]*v[1]+v[2]*v[2]);
			w[0] = t2*(matA[0][0]+matA[1][0]*v[1]+matA[2][0]*v[2]);
			w[1] = t2*(matA[0][1]+matA[1][1]*v[1]+matA[2][1]*v[2]);
			w[2] = t2*(matA[0][2]+matA[1][2]*v[1]+matA[2][2]*v[2]);
			matA[0][0] += w[0];
			matA[0][1] += w[1];
			matA[0][2] += w[2];
			matA[1][1] += v[1]*w[1];
			matA[1][2] += v[1]*w[2];
			matA[2][1] += v[2]*w[1];
			matA[2][2] += v[2]*w[2];

			matL[0][0] = 1.0f+t2;
			matL[0][1] = matL[1][0] = t2*v[1];
			matL[0][2] = matL[2][0] = t2*v[2];
			matL[1][1] = 1.0f+t2*v[1]*v[1];
			matL[1][2] = matL[2][1] = t2*v[1]*v[2];
			matL[2][2] = 1.0f+t2*v[2]*v[2];
			bIdentity = false;
		}
		else
		{
			matL = Matrix3::IDENTITY;
			bIdentity = true;
		}

		// Map first row to (*,*,0)
		length = Math::sqrt(matA[0][1]*matA[0][1]+matA[0][2]*matA[0][2]);
		if ( length > 0.0 )
		{
			sign = (matA[0][1] > 0.0f ? 1.0f : -1.0f);
			t1 = matA[0][1] + sign*length;
			v[2] = matA[0][2]/t1;

			t2 = -2.0f/(1.0f+v[2]*v[2]);
			w[0] = t2*(matA[0][1]+matA[0][2]*v[2]);
			w[1] = t2*(matA[1][1]+matA[1][2]*v[2]);
			w[2] = t2*(matA[2][1]+matA[2][2]*v[2]);
			matA[0][1] += w[0];
			matA[1][1] += w[1];
			matA[1][2] += w[1]*v[2];
			matA[2][1] += w[2];
			matA[2][2] += w[2]*v[2];

			matR[0][0] = 1.0;
			matR[0][1] = matR[1][0] = 0.0;
			matR[0][2] = matR[2][0] = 0.0;
			matR[1][1] = 1.0f+t2;
			matR[1][2] = matR[2][1] = t2*v[2];
			matR[2][2] = 1.0f+t2*v[2]*v[2];
		}
		else
		{
			matR = Matrix3::IDENTITY;
		}

		// Map second column to (*,*,0)
		length = Math::sqrt(matA[1][1]*matA[1][1]+matA[2][1]*matA[2][1]);
		if ( length > 0.0 )
		{
			sign = (matA[1][1] > 0.0f ? 1.0f : -1.0f);
			t1 = matA[1][1] + sign*length;
			v[2] = matA[2][1]/t1;

			t2 = -2.0f/(1.0f+v[2]*v[2]);
			w[1] = t2*(matA[1][1]+matA[2][1]*v[2]);
			w[2] = t2*(matA[1][2]+matA[2][2]*v[2]);
			matA[1][1] += w[1];
			matA[1][2] += w[2];
			matA[2][2] += v[2]*w[2];

			float a = 1.0f+t2;
			float b = t2*v[2];
			float c = 1.0f+b*v[2];

			if (bIdentity)
			{
				matL[0][0] = 1.0;
				matL[0][1] = matL[1][0] = 0.0;
				matL[0][2] = matL[2][0] = 0.0;
				matL[1][1] = a;
				matL[1][2] = matL[2][1] = b;
				matL[2][2] = c;
			}
			else
			{
				for (int row = 0; row < 3; row++)
				{
					float tmp0 = matL[row][1];
					float tmp1 = matL[row][2];
					matL[row][1] = a*tmp0+b*tmp1;
					matL[row][2] = b*tmp0+c*tmp1;
				}
			}
		}
	}

	void Matrix3::golubKahanStep (Matrix3& matA, Matrix3& matL, Matrix3& matR)
	{
		float f11 = matA[0][1]*matA[0][1]+matA[1][1]*matA[1][1];
		float t22 = matA[1][2]*matA[1][2]+matA[2][2]*matA[2][2];
		float t12 = matA[1][1]*matA[1][2];
		float trace = f11+t22;
		float diff = f11-t22;
		float discr = Math::sqrt(diff*diff+4.0f*t12*t12);
		float root1 = 0.5f*(trace+discr);
		float root2 = 0.5f*(trace-discr);

		// Adjust right
		float y = matA[0][0] - (Math::abs(root1-t22) <= Math::abs(root2-t22) ? root1 : root2);
		float z = matA[0][1];
		float invLength = Math::invSqrt(y*y+z*z);
		float sin = z*invLength;
		float cos = -y*invLength;

		float tmp0 = matA[0][0];
		float tmp1 = matA[0][1];
		matA[0][0] = cos*tmp0-sin*tmp1;
		matA[0][1] = sin*tmp0+cos*tmp1;
		matA[1][0] = -sin*matA[1][1];
		matA[1][1] *= cos;

		UINT32 row;
		for (row = 0; row < 3; row++)
		{
			tmp0 = matR[0][row];
			tmp1 = matR[1][row];
			matR[0][row] = cos*tmp0-sin*tmp1;
			matR[1][row] = sin*tmp0+cos*tmp1;
		}

		// Adjust left
		y = matA[0][0];
		z = matA[1][0];
		invLength = Math::invSqrt(y*y+z*z);
		sin = z*invLength;
		cos = -y*invLength;

		matA[0][0] = cos*matA[0][0]-sin*matA[1][0];
		tmp0 = matA[0][1];
		tmp1 = matA[1][1];
		matA[0][1] = cos*tmp0-sin*tmp1;
		matA[1][1] = sin*tmp0+cos*tmp1;
		matA[0][2] = -sin*matA[1][2];
		matA[1][2] *= cos;

		UINT32 col;
		for (col = 0; col < 3; col++)
		{
			tmp0 = matL[col][0];
			tmp1 = matL[col][1];
			matL[col][0] = cos*tmp0-sin*tmp1;
			matL[col][1] = sin*tmp0+cos*tmp1;
		}

		// Adjust right
		y = matA[0][1];
		z = matA[0][2];
		invLength = Math::invSqrt(y*y+z*z);
		sin = z*invLength;
		cos = -y*invLength;

		matA[0][1] = cos*matA[0][1]-sin*matA[0][2];
		tmp0 = matA[1][1];
		tmp1 = matA[1][2];
		matA[1][1] = cos*tmp0-sin*tmp1;
		matA[1][2] = sin*tmp0+cos*tmp1;
		matA[2][1] = -sin*matA[2][2];
		matA[2][2] *= cos;

		for (row = 0; row < 3; row++)
		{
			tmp0 = matR[1][row];
			tmp1 = matR[2][row];
			matR[1][row] = cos*tmp0-sin*tmp1;
			matR[2][row] = sin*tmp0+cos*tmp1;
		}

		// Adjust left
		y = matA[1][1];
		z = matA[2][1];
		invLength = Math::invSqrt(y*y+z*z);
		sin = z*invLength;
		cos = -y*invLength;

		matA[1][1] = cos*matA[1][1]-sin*matA[2][1];
		tmp0 = matA[1][2];
		tmp1 = matA[2][2];
		matA[1][2] = cos*tmp0-sin*tmp1;
		matA[2][2] = sin*tmp0+cos*tmp1;

		for (col = 0; col < 3; col++)
		{
			tmp0 = matL[col][1];
			tmp1 = matL[col][2];
			matL[col][1] = cos*tmp0-sin*tmp1;
			matL[col][2] = sin*tmp0+cos*tmp1;
		}
	}

	void Matrix3::singularValueDecomposition(Matrix3& matL, Vector3& matS, Matrix3& matR) const
	{
		UINT32 row, col;

		Matrix3 mat = *this;
		bidiagonalize(mat, matL, matR);

		for (unsigned int i = 0; i < SVD_MAX_ITERS; i++)
		{
			float tmp, tmp0, tmp1;
			float sin0, cos0, tan0;
			float sin1, cos1, tan1;

			bool test1 = (Math::abs(mat[0][1]) <= SVD_EPSILON*(Math::abs(mat[0][0])+Math::abs(mat[1][1])));
			bool test2 = (Math::abs(mat[1][2]) <= SVD_EPSILON*(Math::abs(mat[1][1])+Math::abs(mat[2][2])));

			if (test1)
			{
				if (test2)
				{
					matS[0] = mat[0][0];
					matS[1] = mat[1][1];
					matS[2] = mat[2][2];
					break;
				}
				else
				{
					// 2x2 closed form factorization
					tmp = (mat[1][1]*mat[1][1] - mat[2][2]*mat[2][2] + mat[1][2]*mat[1][2])/(mat[1][2]*mat[2][2]);
					tan0 = 0.5f*(tmp+Math::sqrt(tmp*tmp + 4.0f));
					cos0 = Math::invSqrt(1.0f+tan0*tan0);
					sin0 = tan0*cos0;

					for (col = 0; col < 3; col++)
					{
						tmp0 = matL[col][1];
						tmp1 = matL[col][2];
						matL[col][1] = cos0*tmp0-sin0*tmp1;
						matL[col][2] = sin0*tmp0+cos0*tmp1;
					}

					tan1 = (mat[1][2]-mat[2][2]*tan0)/mat[1][1];
					cos1 = Math::invSqrt(1.0f+tan1*tan1);
					sin1 = -tan1*cos1;

					for (row = 0; row < 3; row++)
					{
						tmp0 = matR[1][row];
						tmp1 = matR[2][row];
						matR[1][row] = cos1*tmp0-sin1*tmp1;
						matR[2][row] = sin1*tmp0+cos1*tmp1;
					}

					matS[0] = mat[0][0];
					matS[1] = cos0*cos1*mat[1][1] - sin1*(cos0*mat[1][2]-sin0*mat[2][2]);
					matS[2] = sin0*sin1*mat[1][1] + cos1*(sin0*mat[1][2]+cos0*mat[2][2]);
					break;
				}
			}
			else
			{
				if (test2)
				{
					// 2x2 closed form factorization
					tmp = (mat[0][0]*mat[0][0] + mat[1][1]*mat[1][1] - mat[0][1]*mat[0][1])/(mat[0][1]*mat[1][1]);
					tan0 = 0.5f*(-tmp+Math::sqrt(tmp*tmp + 4.0f));
					cos0 = Math::invSqrt(1.0f+tan0*tan0);
					sin0 = tan0*cos0;

					for (col = 0; col < 3; col++)
					{
						tmp0 = matL[col][0];
						tmp1 = matL[col][1];
						matL[col][0] = cos0*tmp0-sin0*tmp1;
						matL[col][1] = sin0*tmp0+cos0*tmp1;
					}

					tan1 = (mat[0][1]-mat[1][1]*tan0)/mat[0][0];
					cos1 = Math::invSqrt(1.0f+tan1*tan1);
					sin1 = -tan1*cos1;

					for (row = 0; row < 3; row++)
					{
						tmp0 = matR[0][row];
						tmp1 = matR[1][row];
						matR[0][row] = cos1*tmp0-sin1*tmp1;
						matR[1][row] = sin1*tmp0+cos1*tmp1;
					}

					matS[0] = cos0*cos1*mat[0][0] - sin1*(cos0*mat[0][1]-sin0*mat[1][1]);
					matS[1] = sin0*sin1*mat[0][0] + cos1*(sin0*mat[0][1]+cos0*mat[1][1]);
					matS[2] = mat[2][2];
					break;
				}
				else
				{
					golubKahanStep(mat, matL, matR);
				}
			}
		}

		// Positize diagonal
		for (row = 0; row < 3; row++)
		{
			if ( matS[row] < 0.0 )
			{
				matS[row] = -matS[row];
				for (col = 0; col < 3; col++)
					matR[row][col] = -matR[row][col];
			}
		}
	}

	void Matrix3::orthonormalize()
	{
		// Compute q0
		float invLength = Math::invSqrt(m[0][0]*m[0][0]+ m[1][0]*m[1][0] + m[2][0]*m[2][0]);

		m[0][0] *= invLength;
		m[1][0] *= invLength;
		m[2][0] *= invLength;

		// Compute q1
		float dot0 = m[0][0]*m[0][1] + m[1][0]*m[1][1] + m[2][0]*m[2][1];

		m[0][1] -= dot0*m[0][0];
		m[1][1] -= dot0*m[1][0];
		m[2][1] -= dot0*m[2][0];

		invLength = Math::invSqrt(m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]);

		m[0][1] *= invLength;
		m[1][1] *= invLength;
		m[2][1] *= invLength;

		// Compute q2
		float dot1 = m[0][1]*m[0][2] + m[1][1]*m[1][2] + m[2][1]*m[2][2];
		dot0 = m[0][0]*m[0][2] + m[1][0]*m[1][2] + m[2][0]*m[2][2];

		m[0][2] -= dot0*m[0][0] + dot1*m[0][1];
		m[1][2] -= dot0*m[1][0] + dot1*m[1][1];
		m[2][2] -= dot0*m[2][0] + dot1*m[2][1];

		invLength = Math::invSqrt(m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]);

		m[0][2] *= invLength;
		m[1][2] *= invLength;
		m[2][2] *= invLength;
	}

	void Matrix3::decomposition(Quaternion& rotation, Vector3& scale) const
	{
		Matrix3 matQ;
		Vector3 vecU;
		QDUDecomposition(matQ, scale, vecU);

		rotation = Quaternion(matQ);
	}

	void Matrix3::QDUDecomposition(Matrix3& matQ, Vector3& vecD, Vector3& vecU) const
	{
		// Build orthogonal matrix Q
		float invLength = Math::invSqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]);
		matQ[0][0] = m[0][0]*invLength;
		matQ[1][0] = m[1][0]*invLength;
		matQ[2][0] = m[2][0]*invLength;

		float dot = matQ[0][0]*m[0][1] + matQ[1][0]*m[1][1] + matQ[2][0]*m[2][1];
		matQ[0][1] = m[0][1]-dot*matQ[0][0];
		matQ[1][1] = m[1][1]-dot*matQ[1][0];
		matQ[2][1] = m[2][1]-dot*matQ[2][0];

		invLength = Math::invSqrt(matQ[0][1]*matQ[0][1] + matQ[1][1]*matQ[1][1] + matQ[2][1]*matQ[2][1]);
		matQ[0][1] *= invLength;
		matQ[1][1] *= invLength;
		matQ[2][1] *= invLength;

		dot = matQ[0][0]*m[0][2] + matQ[1][0]*m[1][2] + matQ[2][0]*m[2][2];
		matQ[0][2] = m[0][2]-dot*matQ[0][0];
		matQ[1][2] = m[1][2]-dot*matQ[1][0];
		matQ[2][2] = m[2][2]-dot*matQ[2][0];

		dot = matQ[0][1]*m[0][2] + matQ[1][1]*m[1][2] + matQ[2][1]*m[2][2];
		matQ[0][2] -= dot*matQ[0][1];
		matQ[1][2] -= dot*matQ[1][1];
		matQ[2][2] -= dot*matQ[2][1];

		invLength = Math::invSqrt(matQ[0][2]*matQ[0][2] + matQ[1][2]*matQ[1][2] + matQ[2][2]*matQ[2][2]);
		matQ[0][2] *= invLength;
		matQ[1][2] *= invLength;
		matQ[2][2] *= invLength;

		// Guarantee that orthogonal matrix has determinant 1 (no reflections)
		float fDet = matQ[0][0]*matQ[1][1]*matQ[2][2] + matQ[0][1]*matQ[1][2]*matQ[2][0] +
			matQ[0][2]*matQ[1][0]*matQ[2][1] - matQ[0][2]*matQ[1][1]*matQ[2][0] -
			matQ[0][1]*matQ[1][0]*matQ[2][2] - matQ[0][0]*matQ[1][2]*matQ[2][1];

		if (fDet < 0.0f)
		{
			for (UINT32 row = 0; row < 3; row++)
				for (UINT32 col = 0; col < 3; col++)
					matQ[row][col] = -matQ[row][col];
		}

		// Build "right" matrix R
		Matrix3 matRight;
		matRight[0][0] = matQ[0][0]*m[0][0] + matQ[1][0]*m[1][0] +
			matQ[2][0]*m[2][0];
		matRight[0][1] = matQ[0][0]*m[0][1] + matQ[1][0]*m[1][1] +
			matQ[2][0]*m[2][1];
		matRight[1][1] = matQ[0][1]*m[0][1] + matQ[1][1]*m[1][1] +
			matQ[2][1]*m[2][1];
		matRight[0][2] = matQ[0][0]*m[0][2] + matQ[1][0]*m[1][2] +
			matQ[2][0]*m[2][2];
		matRight[1][2] = matQ[0][1]*m[0][2] + matQ[1][1]*m[1][2] +
			matQ[2][1]*m[2][2];
		matRight[2][2] = matQ[0][2]*m[0][2] + matQ[1][2]*m[1][2] +
			matQ[2][2]*m[2][2];

		// The scaling component
		vecD[0] = matRight[0][0];
		vecD[1] = matRight[1][1];
		vecD[2] = matRight[2][2];

		// The shear component
		float invD0 = 1.0f/vecD[0];
		vecU[0] = matRight[0][1]*invD0;
		vecU[1] = matRight[0][2]*invD0;
		vecU[2] = matRight[1][2]/vecD[1];
	}

	void Matrix3::toAxisAngle(Vector3& axis, Radian& radians) const
	{
		float trace = m[0][0] + m[1][1] + m[2][2];
		float cos = 0.5f*(trace-1.0f);
		radians = Math::acos(cos);  // In [0, PI]

		if (radians > Radian(0.0f))
		{
			if (radians < Radian(Math::PI))
			{
				axis.x = m[2][1]-m[1][2];
				axis.y = m[0][2]-m[2][0];
				axis.z = m[1][0]-m[0][1];
				axis.normalize();
			}
			else
			{
				// Angle is PI
				float fHalfInverse;
				if (m[0][0] >= m[1][1])
				{
					// r00 >= r11
					if (m[0][0] >= m[2][2])
					{
						// r00 is maximum diagonal term
						axis.x = 0.5f*Math::sqrt(m[0][0] - m[1][1] - m[2][2] + 1.0f);
						fHalfInverse = 0.5f/axis.x;
						axis.y = fHalfInverse*m[0][1];
						axis.z = fHalfInverse*m[0][2];
					}
					else
					{
						// r22 is maximum diagonal term
						axis.z = 0.5f*Math::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.0f);
						fHalfInverse = 0.5f/axis.z;
						axis.x = fHalfInverse*m[0][2];
						axis.y = fHalfInverse*m[1][2];
					}
				}
				else
				{
					// r11 > r00
					if ( m[1][1] >= m[2][2] )
					{
						// r11 is maximum diagonal term
						axis.y = 0.5f*Math::sqrt(m[1][1] - m[0][0] - m[2][2] + 1.0f);
						fHalfInverse  = 0.5f/axis.y;
						axis.x = fHalfInverse*m[0][1];
						axis.z = fHalfInverse*m[1][2];
					}
					else
					{
						// r22 is maximum diagonal term
						axis.z = 0.5f*Math::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.0f);
						fHalfInverse = 0.5f/axis.z;
						axis.x = fHalfInverse*m[0][2];
						axis.y = fHalfInverse*m[1][2];
					}
				}
			}
		}
		else
		{
			// The angle is 0 and the matrix is the identity.  Any axis will
			// work, so just use the x-axis.
			axis.x = 1.0f;
			axis.y = 0.0f;
			axis.z = 0.0f;
		}
	}

	void Matrix3::fromAxisAngle(const Vector3& axis, const Radian& angle)
	{
		float cos = Math::cos(angle);
		float sin = Math::sin(angle);
		float oneMinusCos = 1.0f-cos;
		float x2 = axis.x*axis.x;
		float y2 = axis.y*axis.y;
		float z2 = axis.z*axis.z;
		float xym = axis.x*axis.y*oneMinusCos;
		float xzm = axis.x*axis.z*oneMinusCos;
		float yzm = axis.y*axis.z*oneMinusCos;
		float xSin = axis.x*sin;
		float ySin = axis.y*sin;
		float zSin = axis.z*sin;

		m[0][0] = x2*oneMinusCos+cos;
		m[0][1] = xym-zSin;
		m[0][2] = xzm+ySin;
		m[1][0] = xym+zSin;
		m[1][1] = y2*oneMinusCos+cos;
		m[1][2] = yzm-xSin;
		m[2][0] = xzm-ySin;
		m[2][1] = yzm+xSin;
		m[2][2] = z2*oneMinusCos+cos;
	}

	void Matrix3::toQuaternion(Quaternion& quat) const
	{
		quat.fromRotationMatrix(*this);
	}

	void Matrix3::fromQuaternion(const Quaternion& quat)
	{
		quat.toRotationMatrix(*this);
	}

	bool Matrix3::toEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const
	{
		xAngle = -Radian(Math::asin(m[1][2]));
		if (xAngle < Radian(Math::HALF_PI))
		{
			if (xAngle > Radian(-Math::HALF_PI))
			{
				yAngle = Math::atan2(m[0][2], m[2][2]);
				zAngle = Math::atan2(m[1][0], m[1][1]);

				return true;
			}
			else
			{
				// Note: Not an unique solution.
				xAngle = Radian(-Math::HALF_PI);
				yAngle = Math::atan2(-m[0][1], m[0][0]);
				zAngle = Radian(0.0f);

				return false;
			}
		}
		else
		{
			// Note: Not an unique solution.
			xAngle = Radian(Math::HALF_PI);
			yAngle = Math::atan2(m[0][1], m[0][0]);
			zAngle = Radian(0.0f);
			
			return false;
		}
	}

	void Matrix3::fromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
	{
		float cx = Math::cos(xAngle);
		float sx = Math::sin(xAngle);

		float cy = Math::cos(yAngle);
		float sy = Math::sin(yAngle);

		float cz = Math::cos(zAngle);
		float sz = Math::sin(zAngle);

		m[0][0] = cy * cz + sx * sy * sz;
		m[0][1] = cz * sx * sy - cy * sz;
		m[0][2] = cx * sy;

		m[1][0] = cx * sz;
		m[1][1] = cx * cz;
		m[1][2] = -sx;

		m[2][0] = -cz * sy + cy * sx * sz;
		m[2][1] = cy * cz * sx + sy * sz;
		m[2][2] = cx * cy;
	}

	void Matrix3::fromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order)
	{
		const EulerAngleOrderData& l = EA_LOOKUP[(int)order];

		Matrix3 mats[3];
		float cx = Math::cos(xAngle);
		float sx = Math::sin(xAngle);
		mats[0] = Matrix3(
			1.0f, 0.0f, 0.0f,
			0.0f, cx, -sx,
			0.0f, sx, cx);

		float cy = Math::cos(yAngle);
		float sy = Math::sin(yAngle);
		mats[1] = Matrix3(
			cy, 0.0f, sy,
			0.0f, 1.0f, 0.0f,
			-sy, 0.0f, cy);

		float cz = Math::cos(zAngle);
		float sz = Math::sin(zAngle);
		mats[2] = Matrix3(
			cz, -sz, 0.0f,
			sz, cz, 0.0f,
			0.0f, 0.0f, 1.0f);
	
		*this = mats[l.c]*(mats[l.b]*mats[l.a]);
	}

	void Matrix3::tridiagonal(float diag[3], float subDiag[3])
	{
		// Householder reduction T = Q^t M Q
		//   Input:
		//     mat, symmetric 3x3 matrix M
		//   Output:
		//     mat, orthogonal matrix Q
		//     diag, diagonal entries of T
		//     subd, subdiagonal entries of T (T is symmetric)

		float fA = m[0][0];
		float fB = m[0][1];
		float fC = m[0][2];
		float fD = m[1][1];
		float fE = m[1][2];
		float fF = m[2][2];

		diag[0] = fA;
		subDiag[2] = 0.0;
		if (Math::abs(fC) >= EPSILON)
		{
			float length = Math::sqrt(fB*fB+fC*fC);
			float invLength = 1.0f/length;
			fB *= invLength;
			fC *= invLength;
			float fQ = 2.0f*fB*fE+fC*(fF-fD);
			diag[1] = fD+fC*fQ;
			diag[2] = fF-fC*fQ;
			subDiag[0] = length;
			subDiag[1] = fE-fB*fQ;
			m[0][0] = 1.0;
			m[0][1] = 0.0;
			m[0][2] = 0.0;
			m[1][0] = 0.0;
			m[1][1] = fB;
			m[1][2] = fC;
			m[2][0] = 0.0;
			m[2][1] = fC;
			m[2][2] = -fB;
		}
		else
		{
			diag[1] = fD;
			diag[2] = fF;
			subDiag[0] = fB;
			subDiag[1] = fE;
			m[0][0] = 1.0;
			m[0][1] = 0.0;
			m[0][2] = 0.0;
			m[1][0] = 0.0;
			m[1][1] = 1.0;
			m[1][2] = 0.0;
			m[2][0] = 0.0;
			m[2][1] = 0.0;
			m[2][2] = 1.0;
		}
	}

	bool Matrix3::QLAlgorithm(float diag[3], float subDiag[3])
	{
		// QL iteration with implicit shifting to reduce matrix from tridiagonal to diagonal

		for (int i = 0; i < 3; i++)
		{
			const unsigned int maxIter = 32;
			unsigned int iter;
			for (iter = 0; iter < maxIter; iter++)
			{
				int j;
				for (j = i; j <= 1; j++)
				{
					float sum = Math::abs(diag[j]) + Math::abs(diag[j+1]);

					if (Math::abs(subDiag[j]) + sum == sum)
						break;
				}

				if (j == i)
					break;

				float tmp0 = (diag[i+1]-diag[i])/(2.0f*subDiag[i]);
				float tmp1 = Math::sqrt(tmp0*tmp0+1.0f);

				if (tmp0 < 0.0f)
					tmp0 = diag[j]-diag[i]+subDiag[i]/(tmp0-tmp1);
				else
					tmp0 = diag[j]-diag[i]+subDiag[i]/(tmp0+tmp1);

				float sin = 1.0f;
				float cos = 1.0f;
				float tmp2 = 0.0f;
				for (int k = j-1; k >= i; k--)
				{
					float tmp3 = sin*subDiag[k];
					float tmp4 = cos*subDiag[k];

					if (Math::abs(tmp3) >= Math::abs(tmp0))
					{
						cos = tmp0/tmp3;
						tmp1 = Math::sqrt(cos*cos+1.0f);
						subDiag[k+1] = tmp3*tmp1;
						sin = 1.0f/tmp1;
						cos *= sin;
					}
					else
					{
						sin = tmp3/tmp0;
						tmp1 = Math::sqrt(sin*sin+1.0f);
						subDiag[k+1] = tmp0*tmp1;
						cos = 1.0f/tmp1;
						sin *= cos;
					}

					tmp0 = diag[k+1]-tmp2;
					tmp1 = (diag[k]-tmp0)*sin+2.0f*tmp4*cos;
					tmp2 = sin*tmp1;
					diag[k+1] = tmp0+tmp2;
					tmp0 = cos*tmp1-tmp4;

					for (int row = 0; row < 3; row++)
					{
						tmp3 = m[row][k+1];
						m[row][k+1] = sin*m[row][k] + cos*tmp3;
						m[row][k] = cos*m[row][k] - sin*tmp3;
					}
				}

				diag[i] -= tmp2;
				subDiag[i] = tmp0;
				subDiag[j] = 0.0;
			}

			if (iter == maxIter)
			{
				// Should not get here under normal circumstances
				return false;
			}
		}

		return true;
	}

	void Matrix3::eigenSolveSymmetric(float eigenValues[3], Vector3 eigenVectors[3]) const
	{
		Matrix3 mat = *this;
		float subDiag[3];
		mat.tridiagonal(eigenValues, subDiag);
		mat.QLAlgorithm(eigenValues, subDiag);

		for (UINT32 i = 0; i < 3; i++)
		{
			eigenVectors[i][0] = mat[0][i];
			eigenVectors[i][1] = mat[1][i];
			eigenVectors[i][2] = mat[2][i];
		}

		// Make eigenvectors form a right--handed system
		Vector3 cross = eigenVectors[1].cross(eigenVectors[2]);
		float det = eigenVectors[0].dot(cross);
		if (det < 0.0f)
		{
			eigenVectors[2][0] = -eigenVectors[2][0];
			eigenVectors[2][1] = -eigenVectors[2][1];
			eigenVectors[2][2] = -eigenVectors[2][2];
		}
	}
}
