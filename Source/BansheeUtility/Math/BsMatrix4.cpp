//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Math/BsMatrix4.h"

#include "Math/BsVector3.h"
#include "Math/BsMatrix3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	const Matrix4 Matrix4::ZERO(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f);

	const Matrix4 Matrix4::IDENTITY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	static float MINOR(const Matrix4& m, const UINT32 r0, const UINT32 r1, const UINT32 r2, 
								const UINT32 c0, const UINT32 c1, const UINT32 c2)
	{
		return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
			m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
			m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
	}

	Matrix4 Matrix4::adjoint() const
	{
		return Matrix4( MINOR(*this, 1, 2, 3, 1, 2, 3),
			-MINOR(*this, 0, 2, 3, 1, 2, 3),
			MINOR(*this, 0, 1, 3, 1, 2, 3),
			-MINOR(*this, 0, 1, 2, 1, 2, 3),

			-MINOR(*this, 1, 2, 3, 0, 2, 3),
			MINOR(*this, 0, 2, 3, 0, 2, 3),
			-MINOR(*this, 0, 1, 3, 0, 2, 3),
			MINOR(*this, 0, 1, 2, 0, 2, 3),

			MINOR(*this, 1, 2, 3, 0, 1, 3),
			-MINOR(*this, 0, 2, 3, 0, 1, 3),
			MINOR(*this, 0, 1, 3, 0, 1, 3),
			-MINOR(*this, 0, 1, 2, 0, 1, 3),

			-MINOR(*this, 1, 2, 3, 0, 1, 2),
			MINOR(*this, 0, 2, 3, 0, 1, 2),
			-MINOR(*this, 0, 1, 3, 0, 1, 2),
			MINOR(*this, 0, 1, 2, 0, 1, 2));
	}

	float Matrix4::determinant() const
	{
		return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
			m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
			m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
			m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
	}

	float Matrix4::determinant3x3() const
	{
		float cofactor00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
		float cofactor10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
		float cofactor20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

		float det = m[0][0] * cofactor00 + m[0][1] * cofactor10 + m[0][2] * cofactor20;

		return det;
	}

	Matrix4 Matrix4::inverse() const
	{
		float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
		float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
		float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
		float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

		float v0 = m20 * m31 - m21 * m30;
		float v1 = m20 * m32 - m22 * m30;
		float v2 = m20 * m33 - m23 * m30;
		float v3 = m21 * m32 - m22 * m31;
		float v4 = m21 * m33 - m23 * m31;
		float v5 = m22 * m33 - m23 * m32;

		float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
		float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
		float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
		float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

		float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

		float d00 = t00 * invDet;
		float d10 = t10 * invDet;
		float d20 = t20 * invDet;
		float d30 = t30 * invDet;

		float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m10 * m31 - m11 * m30;
		v1 = m10 * m32 - m12 * m30;
		v2 = m10 * m33 - m13 * m30;
		v3 = m11 * m32 - m12 * m31;
		v4 = m11 * m33 - m13 * m31;
		v5 = m12 * m33 - m13 * m32;

		float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m21 * m10 - m20 * m11;
		v1 = m22 * m10 - m20 * m12;
		v2 = m23 * m10 - m20 * m13;
		v3 = m22 * m11 - m21 * m12;
		v4 = m23 * m11 - m21 * m13;
		v5 = m23 * m12 - m22 * m13;

		float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;
		
		return Matrix4(
			d00, d01, d02, d03,
			d10, d11, d12, d13,
			d20, d21, d22, d23,
			d30, d31, d32, d33);
	}

	Matrix4 Matrix4::inverseAffine() const
	{
		float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
		float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

		float t00 = m22 * m11 - m21 * m12;
		float t10 = m20 * m12 - m22 * m10;
		float t20 = m21 * m10 - m20 * m11;

		float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

		float invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

		t00 *= invDet; t10 *= invDet; t20 *= invDet;

		m00 *= invDet; m01 *= invDet; m02 *= invDet;

		float r00 = t00;
		float r01 = m02 * m21 - m01 * m22;
		float r02 = m01 * m12 - m02 * m11;

		float r10 = t10;
		float r11 = m00 * m22 - m02 * m20;
		float r12 = m02 * m10 - m00 * m12;

		float r20 = t20;
		float r21 = m01 * m20 - m00 * m21;
		float r22 = m00 * m11 - m01 * m10;

		float m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

		float r03 = - (r00 * m03 + r01 * m13 + r02 * m23);
		float r13 = - (r10 * m03 + r11 * m13 + r12 * m23);
		float r23 = - (r20 * m03 + r21 * m13 + r22 * m23);

		return Matrix4(
			r00, r01, r02, r03,
			r10, r11, r12, r13,
			r20, r21, r22, r23,
			  0,   0,   0,   1);
	}

	void Matrix4::setTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
	{
		Matrix3 rot3x3;
		rotation.toRotationMatrix(rot3x3);

		m[0][0] = scale.x * rot3x3[0][0]; m[0][1] = scale.y * rot3x3[0][1]; m[0][2] = scale.z * rot3x3[0][2]; m[0][3] = translation.x;
		m[1][0] = scale.x * rot3x3[1][0]; m[1][1] = scale.y * rot3x3[1][1]; m[1][2] = scale.z * rot3x3[1][2]; m[1][3] = translation.y;
		m[2][0] = scale.x * rot3x3[2][0]; m[2][1] = scale.y * rot3x3[2][1]; m[2][2] = scale.z * rot3x3[2][2]; m[2][3] = translation.z;

		// No projection term
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
	}

	void Matrix4::setInverseTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
	{
		// Invert the parameters
		Vector3 invTranslate = -translation;
		Vector3 invScale(1 / scale.x, 1 / scale.y, 1 / scale.z);
		Quaternion invRot = rotation.inverse();

		// Because we're inverting, order is translation, rotation, scale
		// So make translation relative to scale & rotation
		invTranslate = invRot.rotate(invTranslate);
		invTranslate *= invScale;

		// Next, make a 3x3 rotation matrix
		Matrix3 rot3x3;
		invRot.toRotationMatrix(rot3x3);

		// Set up final matrix with scale, rotation and translation
		m[0][0] = invScale.x * rot3x3[0][0]; m[0][1] = invScale.x * rot3x3[0][1]; m[0][2] = invScale.x * rot3x3[0][2]; m[0][3] = invTranslate.x;
		m[1][0] = invScale.y * rot3x3[1][0]; m[1][1] = invScale.y * rot3x3[1][1]; m[1][2] = invScale.y * rot3x3[1][2]; m[1][3] = invTranslate.y;
		m[2][0] = invScale.z * rot3x3[2][0]; m[2][1] = invScale.z * rot3x3[2][1]; m[2][2] = invScale.z * rot3x3[2][2]; m[2][3] = invTranslate.z;		

		// No projection term
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
	}

	void Matrix4::decomposition(Vector3& position, Quaternion& rotation, Vector3& scale) const
	{
		Matrix3 m3x3;
		extract3x3Matrix(m3x3);

		Matrix3 matQ;
		Vector3 vecU;
		m3x3.QDUDecomposition(matQ, scale, vecU); 

		rotation = Quaternion(matQ);
		position = Vector3(m[0][3], m[1][3], m[2][3]);
	}

	void Matrix4::makeView(const Vector3& position, const Quaternion& orientation)
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
		Matrix3 rot;
		orientation.toRotationMatrix(rot);

		// Make the translation relative to new axes
		Matrix3 rotT = rot.transpose();
		Vector3 trans = (-rotT).multiply(position);

		// Make final matrix
		*this = Matrix4(rotT);
		m[0][3] = trans.x;
		m[1][3] = trans.y;
		m[2][3] = trans.z;
	}

	void Matrix4::makeProjectionOrtho(float left, float right, float top,
		float bottom, float near, float far)
	{
		// Create a matrix that transforms coordinate to normalized device coordinate in range:
		// Left -1 - Right 1
		// Bottom -1 - Top 1
		// Near -1 - Far 1

		float deltaX = right - left;
		float deltaY = bottom - top;
		float deltaZ = far - near;

		m[0][0] = 2.0F / deltaX;
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;
		m[0][3] = -(right + left) / deltaX;

		m[1][0] = 0.0f;
		m[1][1] = -2.0F / deltaY;
		m[1][2] = 0.0f;
		m[1][3] = (top + bottom) / deltaY;

		m[2][0] = 0.0f;
		m[2][1] = 0.0f;

		if (far == 0.0f)
		{
			m[2][2] = 1.0f;
			m[2][3] = 0.0f;
		}
		else
		{
			m[2][2] = -2.0F / deltaZ;
			m[2][3] = -(far + near) / deltaZ;
		}

		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;
		m[3][3] = 1.0f;
	}

	Matrix4 Matrix4::translation(const Vector3& translation)
	{
		Matrix4 mat;

		mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = translation.x;
		mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = translation.y;
		mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = translation.z;
		mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f;

		return mat;
	}

	Matrix4 Matrix4::scaling(const Vector3& scale)
	{
		Matrix4 mat;

		mat[0][0] = scale.x; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
		mat[1][0] = 0.0f; mat[1][1] = scale.y; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
		mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = scale.z; mat[2][3] = 0.0f;
		mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f;

		return mat;
	}

	Matrix4 Matrix4::scaling(float scale)
	{
		Matrix4 mat;

		mat[0][0] = scale; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
		mat[1][0] = 0.0f; mat[1][1] = scale; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
		mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = scale; mat[2][3] = 0.0f;
		mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f;

		return mat;
	}

	Matrix4 Matrix4::rotation(const Quaternion& rotation)
	{
		Matrix3 mat;
		rotation.toRotationMatrix(mat);

		return Matrix4(mat);
	}

	Matrix4 Matrix4::projectionPerspective(const Degree& horzFOV, float aspect, float near, float far, bool positiveZ)
	{
		// Note: Duplicate code in Camera, bring it all here eventually
		static constexpr float INFINITE_FAR_PLANE_ADJUST = 0.00001f;

		Radian thetaX(horzFOV * 0.5f);
		float tanThetaX = Math::tan(thetaX);
		float tanThetaY = tanThetaX / aspect;

		float half_w = tanThetaX * near;
		float half_h = tanThetaY * near;

		float left = -half_w;
		float right = half_w;
		float bottom = -half_h;
		float top = half_h;

		float inv_w = 1 / (right - left);
		float inv_h = 1 / (top - bottom);
		float inv_d = 1 / (far - near);

		float A = 2 * near * inv_w;
		float B = 2 * near * inv_h;
		float C = (right + left) * inv_w;
		float D = (top + bottom) * inv_h;
		float q, qn;
		float sign = positiveZ ? 1.0f : -1.0f;

		if (far == 0)
		{
			// Infinite far plane
			q = INFINITE_FAR_PLANE_ADJUST - 1;
			qn = near * (INFINITE_FAR_PLANE_ADJUST - 2);
		}
		else
		{
			q = sign * (far + near) * inv_d;
			qn = -2.0f * (far * near) * inv_d;
		}

		Matrix4 mat;
		mat[0][0] = A;		mat[0][1] = 0.0f;	mat[0][2] = C;		mat[0][3] = 0.0f;
		mat[1][0] = 0.0f;	mat[1][1] = B;		mat[1][2] = D;		mat[1][3] = 0.0f;
		mat[2][0] = 0.0f;	mat[2][1] = 0.0f;	mat[2][2] = q;		mat[2][3] = qn;
		mat[3][0] = 0.0f;	mat[3][1] = 0.0f;	mat[3][2] = sign;	mat[3][3] = 0.0f;

		return mat;
	}

	Matrix4 Matrix4::projectionOrthographic(float left, float right, float top, float bottom, float near, float far)
	{
		Matrix4 output;
		output.makeProjectionOrtho(left, right, top, bottom, near, far);

		return output;
	}

	Matrix4 Matrix4::view(const Vector3& position, const Quaternion& orientation)
	{
		Matrix4 mat;
		mat.makeView(position, orientation);

		return mat;
	}

	Matrix4 Matrix4::TRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
	{
		Matrix4 mat;
		mat.setTRS(translation, rotation, scale);

		return mat;
	}

	Matrix4 Matrix4::inverseTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
	{
		Matrix4 mat;
		mat.setInverseTRS(translation, rotation, scale);

		return mat;
	}
}
