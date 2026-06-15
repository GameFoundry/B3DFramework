//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DMatrix4.h"

#include "Math/B3DVector3.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DQuaternion.h"

using namespace b3d;

template<typename T>
static T MINOR(const TMatrix4<T>& m, const u32 r0, const u32 r1, const u32 r2, const u32 c0, const u32 c1, const u32 c2)
{
	return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
		m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
		m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Adjoint() const
{
	return TMatrix4(MINOR(*this, 1, 2, 3, 1, 2, 3), -MINOR(*this, 0, 2, 3, 1, 2, 3), MINOR(*this, 0, 1, 3, 1, 2, 3), -MINOR(*this, 0, 1, 2, 1, 2, 3),
				   -MINOR(*this, 1, 2, 3, 0, 2, 3), MINOR(*this, 0, 2, 3, 0, 2, 3), -MINOR(*this, 0, 1, 3, 0, 2, 3), MINOR(*this, 0, 1, 2, 0, 2, 3),
				   MINOR(*this, 1, 2, 3, 0, 1, 3), -MINOR(*this, 0, 2, 3, 0, 1, 3), MINOR(*this, 0, 1, 3, 0, 1, 3), -MINOR(*this, 0, 1, 2, 0, 1, 3),
				   -MINOR(*this, 1, 2, 3, 0, 1, 2), MINOR(*this, 0, 2, 3, 0, 1, 2), -MINOR(*this, 0, 1, 3, 0, 1, 2), MINOR(*this, 0, 1, 2, 0, 1, 2));
}

template<typename T>
T TMatrix4<T>::Determinant() const
{
	return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
		m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
		m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
		m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
}

template<typename T>
T TMatrix4<T>::Determinant3x3() const
{
	T cofactor00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	T cofactor10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
	T cofactor20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

	T det = m[0][0] * cofactor00 + m[0][1] * cofactor10 + m[0][2] * cofactor20;

	return det;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Inverse() const
{
	T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	T m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	T v0 = m20 * m31 - m21 * m30;
	T v1 = m20 * m32 - m22 * m30;
	T v2 = m20 * m33 - m23 * m30;
	T v3 = m21 * m32 - m22 * m31;
	T v4 = m21 * m33 - m23 * m31;
	T v5 = m22 * m33 - m23 * m32;

	T t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
	T t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
	T t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
	T t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

	T invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	T d00 = t00 * invDet;
	T d10 = t10 * invDet;
	T d20 = t20 * invDet;
	T d30 = t30 * invDet;

	T d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	T d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	T d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	return TMatrix4(
		d00, d01, d02, d03,
		d10, d11, d12, d13,
		d20, d21, d22, d23,
		d30, d31, d32, d33);
}

template<typename T>
TMatrix4<T> TMatrix4<T>::InverseAffine() const
{
	T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
	T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

	T t00 = m22 * m11 - m21 * m12;
	T t10 = m20 * m12 - m22 * m10;
	T t20 = m21 * m10 - m20 * m11;

	T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

	T invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

	t00 *= invDet;
	t10 *= invDet;
	t20 *= invDet;

	m00 *= invDet;
	m01 *= invDet;
	m02 *= invDet;

	T r00 = t00;
	T r01 = m02 * m21 - m01 * m22;
	T r02 = m01 * m12 - m02 * m11;

	T r10 = t10;
	T r11 = m00 * m22 - m02 * m20;
	T r12 = m02 * m10 - m00 * m12;

	T r20 = t20;
	T r21 = m01 * m20 - m00 * m21;
	T r22 = m00 * m11 - m01 * m10;

	T m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

	T r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
	T r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
	T r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

	return TMatrix4(
		r00, r01, r02, r03,
		r10, r11, r12, r13,
		r20, r21, r22, r23,
		0, 0, 0, 1);
}

template<typename T>
void TMatrix4<T>::SetTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale)
{
	TMatrix3<T> rot3x3;
	rotation.ToRotationMatrix(rot3x3);

	m[0][0] = scale.X * rot3x3[0][0];
	m[0][1] = scale.Y * rot3x3[0][1];
	m[0][2] = scale.Z * rot3x3[0][2];
	m[0][3] = translation.X;
	m[1][0] = scale.X * rot3x3[1][0];
	m[1][1] = scale.Y * rot3x3[1][1];
	m[1][2] = scale.Z * rot3x3[1][2];
	m[1][3] = translation.Y;
	m[2][0] = scale.X * rot3x3[2][0];
	m[2][1] = scale.Y * rot3x3[2][1];
	m[2][2] = scale.Z * rot3x3[2][2];
	m[2][3] = translation.Z;

	// No projection term
	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
}

template<typename T>
void TMatrix4<T>::SetInverseTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale)
{
	// Invert the parameters
	TVector3<T> invTranslate = -translation;
	TVector3<T> invScale((T)1.0 / scale.X, (T)1.0 / scale.Y, (T)1.0 / scale.Z);
	TQuaternion<T> invRot = rotation.Inverse();

	// Because we're inverting, order is translation, rotation, scale
	// So make translation relative to scale & rotation
	invTranslate = invRot.Rotate(invTranslate);
	invTranslate *= invScale;

	// Next, make a 3x3 rotation matrix
	TMatrix3<T> rot3x3;
	invRot.ToRotationMatrix(rot3x3);

	// Set up final matrix with scale, rotation and translation
	m[0][0] = invScale.X * rot3x3[0][0];
	m[0][1] = invScale.X * rot3x3[0][1];
	m[0][2] = invScale.X * rot3x3[0][2];
	m[0][3] = invTranslate.X;
	m[1][0] = invScale.Y * rot3x3[1][0];
	m[1][1] = invScale.Y * rot3x3[1][1];
	m[1][2] = invScale.Y * rot3x3[1][2];
	m[1][3] = invTranslate.Y;
	m[2][0] = invScale.Z * rot3x3[2][0];
	m[2][1] = invScale.Z * rot3x3[2][1];
	m[2][2] = invScale.Z * rot3x3[2][2];
	m[2][3] = invTranslate.Z;

	// No projection term
	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
}

template<typename T>
void TMatrix4<T>::Decomposition(TVector3<T>& position, TQuaternion<T>& rotation, TVector3<T>& scale) const
{
	TMatrix3<T> m3x3 = Get3x3();

	TMatrix3<T> matQ;
	TVector3<T> vecU;
	m3x3.QDUDecomposition(matQ, scale, vecU);

	rotation = TQuaternion<T>(matQ);
	position = TVector3<T>(m[0][3], m[1][3], m[2][3]);
}

template<typename T>
void TMatrix4<T>::MakeView(const TVector3<T>& position, const TQuaternion<T>& orientation)
{
	// View matrix is:
	//
	//  [ Lx  Uy  Dz  Tx  ]
	//  [ Lx  Uy  Dz  Ty  ]
	//  [ Lx  Uy  Dz  Tz  ]
	//  [ 0   0   0   1   ]
	//
	// Where T = -(Transposed(Rot) * Pos)

	// This is most efficiently done using 3x3 Matrices
	TMatrix3<T> rot;
	orientation.ToRotationMatrix(rot);

	// Make the translation relative to new axes
	TMatrix3<T> rotT = rot.Transpose();
	TVector3<T> trans = (-rotT).Multiply(position);

	// Make final matrix
	*this = TMatrix4(rotT);
	m[0][3] = trans.X;
	m[1][3] = trans.Y;
	m[2][3] = trans.Z;
}

template<typename T>
void TMatrix4<T>::MakeProjectionOrtho(T left, T right, T top, T bottom, T near, T far)
{
	// Create a matrix that transforms coordinate to normalized device coordinate in range:
	// Left -1 - Right 1
	// Bottom -1 - Top 1
	// Near -1 - Far 1

	T deltaX = right - left;
	T deltaY = bottom - top;
	T deltaZ = far - near;

	m[0][0] = (T)2.0 / deltaX;
	m[0][1] = (T)0.0;
	m[0][2] = (T)0.0;
	m[0][3] = -(right + left) / deltaX;

	m[1][0] = (T)0.0;
	m[1][1] = (T)-2.0 / deltaY;
	m[1][2] = (T)0.0;
	m[1][3] = (top + bottom) / deltaY;

	m[2][0] = (T)0.0;
	m[2][1] = (T)0.0;

	if(far == (T)0.0)
	{
		m[2][2] = (T)1.0;
		m[2][3] = (T)0.0;
	}
	else
	{
		m[2][2] = (T)-2.0 / deltaZ;
		m[2][3] = -(far + near) / deltaZ;
	}

	m[3][0] = (T)0.0;
	m[3][1] = (T)0.0;
	m[3][2] = (T)0.0;
	m[3][3] = (T)1.0;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Translation(const TVector3<T>& translation)
{
	TMatrix4 mat;

	mat[0][0] = (T)1.0;
	mat[0][1] = (T)0.0;
	mat[0][2] = (T)0.0;
	mat[0][3] = translation.X;
	mat[1][0] = (T)0.0;
	mat[1][1] = (T)1.0;
	mat[1][2] = (T)0.0;
	mat[1][3] = translation.Y;
	mat[2][0] = (T)0.0;
	mat[2][1] = (T)0.0;
	mat[2][2] = (T)1.0;
	mat[2][3] = translation.Z;
	mat[3][0] = (T)0.0;
	mat[3][1] = (T)0.0;
	mat[3][2] = (T)0.0;
	mat[3][3] = (T)1.0;

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Scaling(const TVector3<T>& scale)
{
	TMatrix4 mat;

	mat[0][0] = scale.X;
	mat[0][1] = (T)0.0;
	mat[0][2] = (T)0.0;
	mat[0][3] = (T)0.0;
	mat[1][0] = (T)0.0;
	mat[1][1] = scale.Y;
	mat[1][2] = (T)0.0;
	mat[1][3] = (T)0.0;
	mat[2][0] = (T)0.0;
	mat[2][1] = (T)0.0;
	mat[2][2] = scale.Z;
	mat[2][3] = (T)0.0;
	mat[3][0] = (T)0.0;
	mat[3][1] = (T)0.0;
	mat[3][2] = (T)0.0;
	mat[3][3] = (T)1.0;

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Scaling(T scale)
{
	TMatrix4 mat;

	mat[0][0] = scale;
	mat[0][1] = (T)0.0;
	mat[0][2] = (T)0.0;
	mat[0][3] = (T)0.0;
	mat[1][0] = (T)0.0;
	mat[1][1] = scale;
	mat[1][2] = (T)0.0;
	mat[1][3] = (T)0.0;
	mat[2][0] = (T)0.0;
	mat[2][1] = (T)0.0;
	mat[2][2] = scale;
	mat[2][3] = (T)0.0;
	mat[3][0] = (T)0.0;
	mat[3][1] = (T)0.0;
	mat[3][2] = (T)0.0;
	mat[3][3] = (T)1.0;

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::Rotation(const TQuaternion<T>& rotation)
{
	TMatrix3<T> mat;
	rotation.ToRotationMatrix(mat);

	return TMatrix4(mat);
}

template<typename T>
TMatrix4<T> TMatrix4<T>::ProjectionPerspective(const TDegree<T>& horzFOV, T aspect, T near, T far, bool positiveZ)
{
	// Note: Duplicate code in Camera, bring it all here eventually
	static constexpr T kInfiniteFarPlaneAdjust = (T)0.00001;

	TRadian<T> thetaX(horzFOV * (T)0.5);
	T tanThetaX = Math::Tan(thetaX);
	T tanThetaY = tanThetaX / aspect;

	T half_w = tanThetaX * near;
	T half_h = tanThetaY * near;

	T left = -half_w;
	T right = half_w;
	T bottom = -half_h;
	T top = half_h;

	T inv_w = 1 / (right - left);
	T inv_h = 1 / (top - bottom);
	T inv_d = 1 / (far - near);

	T A = 2 * near * inv_w;
	T B = 2 * near * inv_h;
	T C = (right + left) * inv_w;
	T D = (top + bottom) * inv_h;
	T q, qn;
	T sign = positiveZ ? (T)1.0 : -(T)1.0;

	if(far == 0)
	{
		// Infinite far plane
		q = kInfiniteFarPlaneAdjust - 1;
		qn = near * (kInfiniteFarPlaneAdjust - 2);
	}
	else
	{
		q = sign * (far + near) * inv_d;
		qn = -2.0f * (far * near) * inv_d;
	}

	TMatrix4 mat;
	mat[0][0] = A;
	mat[0][1] = (T)0.0;
	mat[0][2] = C;
	mat[0][3] = (T)0.0;
	mat[1][0] = (T)0.0;
	mat[1][1] = B;
	mat[1][2] = D;
	mat[1][3] = (T)0.0;
	mat[2][0] = (T)0.0;
	mat[2][1] = (T)0.0;
	mat[2][2] = q;
	mat[2][3] = qn;
	mat[3][0] = (T)0.0;
	mat[3][1] = (T)0.0;
	mat[3][2] = sign;
	mat[3][3] = (T)0.0;

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::ProjectionOrthographic(T left, T right, T top, T bottom, T near, T far)
{
	TMatrix4 output;
	output.MakeProjectionOrtho(left, right, top, bottom, near, far);

	return output;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::View(const TVector3<T>& position, const TQuaternion<T>& orientation)
{
	TMatrix4 mat;
	mat.MakeView(position, orientation);

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::TRS(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale)
{
	TMatrix4 mat;
	mat.SetTrs(translation, rotation, scale);

	return mat;
}

template<typename T>
TMatrix4<T> TMatrix4<T>::InverseTrs(const TVector3<T>& translation, const TQuaternion<T>& rotation, const TVector3<T>& scale)
{
	TMatrix4 mat;
	mat.SetInverseTrs(translation, rotation, scale);

	return mat;
}

namespace b3d
{
	template struct B3D_EXPORT TMatrix4<float>;
	template struct B3D_EXPORT TMatrix4<double>;
} // namespace b3d
