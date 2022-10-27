//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsQuaternion.h"

#include "Math/BsMath.h"
#include "Math/BsMatrix3.h"
#include "Math/BsVector3.h"

using namespace bs;

const Quaternion Quaternion::kZero{ BS_ZERO() };
const Quaternion Quaternion::kIdentity{ BS_IDENTITY() };

void Quaternion::FromRotationMatrix(const Matrix3& mat)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	float trace = mat[0][0] + mat[1][1] + mat[2][2];
	float root;

	if(trace > 0.0f)
	{
		// |w| > 1/2, may as well choose w > 1/2
		root = Math::Sqrt(trace + 1.0f); // 2w
		W = 0.5f * root;
		root = 0.5f / root; // 1/(4w)
		X = (mat[2][1] - mat[1][2]) * root;
		Y = (mat[0][2] - mat[2][0]) * root;
		Z = (mat[1][0] - mat[0][1]) * root;
	}
	else
	{
		// |w| <= 1/2
		static u32 nextLookup[3] = { 1, 2, 0 };
		u32 i = 0;

		if(mat[1][1] > mat[0][0])
			i = 1;

		if(mat[2][2] > mat[i][i])
			i = 2;

		u32 j = nextLookup[i];
		u32 k = nextLookup[j];

		root = Math::Sqrt(mat[i][i] - mat[j][j] - mat[k][k] + 1.0f);

		float* cmpntLookup[3] = { &X, &Y, &Z };
		*cmpntLookup[i] = 0.5f * root;
		root = 0.5f / root;

		W = (mat[k][j] - mat[j][k]) * root;
		*cmpntLookup[j] = (mat[j][i] + mat[i][j]) * root;
		*cmpntLookup[k] = (mat[k][i] + mat[i][k]) * root;
	}

	Normalize();
}

void Quaternion::FromAxisAngle(const Vector3& axis, const Radian& angle)
{
	Radian halfAngle(0.5f * angle);
	float sin = Math::Sin(halfAngle);

	W = Math::Cos(halfAngle);
	X = sin * axis.X;
	Y = sin * axis.Y;
	Z = sin * axis.Z;
}

void Quaternion::FromAxes(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
{
	Matrix3 kRot;

	kRot[0][0] = xaxis.X;
	kRot[1][0] = xaxis.Y;
	kRot[2][0] = xaxis.Z;

	kRot[0][1] = yaxis.X;
	kRot[1][1] = yaxis.Y;
	kRot[2][1] = yaxis.Z;

	kRot[0][2] = zaxis.X;
	kRot[1][2] = zaxis.Y;
	kRot[2][2] = zaxis.Z;

	FromRotationMatrix(kRot);
}

void Quaternion::FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
{
	Radian halfXAngle = xAngle * 0.5f;
	Radian halfYAngle = yAngle * 0.5f;
	Radian halfZAngle = zAngle * 0.5f;

	float cx = Math::Cos(halfXAngle);
	float sx = Math::Sin(halfXAngle);

	float cy = Math::Cos(halfYAngle);
	float sy = Math::Sin(halfYAngle);

	float cz = Math::Cos(halfZAngle);
	float sz = Math::Sin(halfZAngle);

	Quaternion quatX(cx, sx, 0.0f, 0.0f);
	Quaternion quatY(cy, 0.0f, sy, 0.0f);
	Quaternion quatZ(cz, 0.0f, 0.0f, sz);

	*this = quatZ * (quatX * quatY);
}

void Quaternion::FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order)
{
	static constexpr const EulerAngleOrderData kEaLookup[6] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 } };
	const EulerAngleOrderData& l = kEaLookup[(int)order];

	Radian halfXAngle = xAngle * 0.5f;
	Radian halfYAngle = yAngle * 0.5f;
	Radian halfZAngle = zAngle * 0.5f;

	float cx = Math::Cos(halfXAngle);
	float sx = Math::Sin(halfXAngle);

	float cy = Math::Cos(halfYAngle);
	float sy = Math::Sin(halfYAngle);

	float cz = Math::Cos(halfZAngle);
	float sz = Math::Sin(halfZAngle);

	Quaternion quats[3];
	quats[0] = Quaternion(cx, sx, 0.0f, 0.0f);
	quats[1] = Quaternion(cy, 0.0f, sy, 0.0f);
	quats[2] = Quaternion(cz, 0.0f, 0.0f, sz);

	*this = quats[l.C] * (quats[l.B] * quats[l.A]);
}

void Quaternion::ToRotationMatrix(Matrix3& mat) const
{
	float tx = X + X;
	float ty = Y + Y;
	float tz = Z + Z;
	float twx = tx * W;
	float twy = ty * W;
	float twz = tz * W;
	float txx = tx * X;
	float txy = ty * X;
	float txz = tz * X;
	float tyy = ty * Y;
	float tyz = tz * Y;
	float tzz = tz * Z;

	mat[0][0] = 1.0f - (tyy + tzz);
	mat[0][1] = txy - twz;
	mat[0][2] = txz + twy;
	mat[1][0] = txy + twz;
	mat[1][1] = 1.0f - (txx + tzz);
	mat[1][2] = tyz - twx;
	mat[2][0] = txz - twy;
	mat[2][1] = tyz + twx;
	mat[2][2] = 1.0f - (txx + tyy);
}

void Quaternion::ToAxisAngle(Vector3& axis, Radian& angle) const
{
	float sqrLength = X * X + Y * Y + Z * Z;
	if(sqrLength > 0.0)
	{
		angle = 2.0 * Math::Acos(W);
		float invLength = Math::InvSqrt(sqrLength);
		axis.X = X * invLength;
		axis.Y = Y * invLength;
		axis.Z = Z * invLength;
	}
	else
	{
		// Angle is 0 (mod 2*pi), so any axis will do
		angle = Radian(0.0);
		axis.X = 1.0;
		axis.Y = 0.0;
		axis.Z = 0.0;
	}
}

void Quaternion::ToAxes(Vector3& xaxis, Vector3& yaxis, Vector3& zaxis) const
{
	Matrix3 matRot;
	ToRotationMatrix(matRot);

	xaxis.X = matRot[0][0];
	xaxis.Y = matRot[1][0];
	xaxis.Z = matRot[2][0];

	yaxis.X = matRot[0][1];
	yaxis.Y = matRot[1][1];
	yaxis.Z = matRot[2][1];

	zaxis.X = matRot[0][2];
	zaxis.Y = matRot[1][2];
	zaxis.Z = matRot[2][2];
}

bool Quaternion::ToEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const
{
	Matrix3 matRot;
	ToRotationMatrix(matRot);
	return matRot.ToEulerAngles(xAngle, yAngle, zAngle);
}

Vector3 Quaternion::XAxis() const
{
	float fTy = 2.0f * Y;
	float fTz = 2.0f * Z;
	float fTwy = fTy * W;
	float fTwz = fTz * W;
	float fTxy = fTy * X;
	float fTxz = fTz * X;
	float fTyy = fTy * Y;
	float fTzz = fTz * Z;

	return Vector3(1.0f - (fTyy + fTzz), fTxy + fTwz, fTxz - fTwy);
}

Vector3 Quaternion::YAxis() const
{
	float fTx = 2.0f * X;
	float fTy = 2.0f * Y;
	float fTz = 2.0f * Z;
	float fTwx = fTx * W;
	float fTwz = fTz * W;
	float fTxx = fTx * X;
	float fTxy = fTy * X;
	float fTyz = fTz * Y;
	float fTzz = fTz * Z;

	return Vector3(fTxy - fTwz, 1.0f - (fTxx + fTzz), fTyz + fTwx);
}

Vector3 Quaternion::ZAxis() const
{
	float fTx = 2.0f * X;
	float fTy = 2.0f * Y;
	float fTz = 2.0f * Z;
	float fTwx = fTx * W;
	float fTwy = fTy * W;
	float fTxx = fTx * X;
	float fTxz = fTz * X;
	float fTyy = fTy * Y;
	float fTyz = fTz * Y;

	return Vector3(fTxz + fTwy, fTyz - fTwx, 1.0f - (fTxx + fTyy));
}

Quaternion Quaternion::Inverse() const
{
	float fNorm = W * W + X * X + Y * Y + Z * Z;
	if(fNorm > 0.0f)
	{
		float fInvNorm = 1.0f / fNorm;
		return Quaternion(W * fInvNorm, -X * fInvNorm, -Y * fInvNorm, -Z * fInvNorm);
	}
	else
	{
		// Return an invalid result to flag the error
		return kZero;
	}
}

Vector3 Quaternion::Rotate(const Vector3& v) const
{
	// Note: Does compiler generate fast code here? Perhaps its better to pull all code locally without constructing
	//       an intermediate matrix.
	Matrix3 rot;
	ToRotationMatrix(rot);
	return rot.Multiply(v);
}

void Quaternion::LookRotation(const Vector3& forwardDir)
{
	if(forwardDir == Vector3::kZero)
		return;

	Vector3 nrmForwardDir = Vector3::Normalize(forwardDir);
	Vector3 currentForwardDir = -ZAxis();

	if((nrmForwardDir + currentForwardDir).SquaredLength() < 0.00005f)
	{
		// Oops, a 180 degree turn (infinite possible rotation axes)
		// Default to yaw i.e. use current UP
		*this = Quaternion(-Y, -Z, W, X);
	}
	else
	{
		// Derive shortest arc to new direction
		Quaternion rotQuat = GetRotationFromTo(currentForwardDir, nrmForwardDir);
		*this = rotQuat * *this;
	}
}

void Quaternion::LookRotation(const Vector3& forwardDir, const Vector3& upDir)
{
	Vector3 forward = Vector3::Normalize(forwardDir);
	Vector3 up = Vector3::Normalize(upDir);

	if(Math::ApproxEquals(Vector3::Dot(forward, up), 1.0f))
	{
		LookRotation(forward);
		return;
	}

	Vector3 x = Vector3::Cross(forward, up);
	Vector3 y = Vector3::Cross(x, forward);

	x.Normalize();
	y.Normalize();

	*this = Quaternion(x, y, -forward);
}

Quaternion Quaternion::Slerp(float t, const Quaternion& p, const Quaternion& q, bool shortestPath)
{
	float cos = p.Dot(q);
	Quaternion quat;

	if(cos < 0.0f && shortestPath)
	{
		cos = -cos;
		quat = -q;
	}
	else
	{
		quat = q;
	}

	if(abs(cos) < 1 - kEpsilon)
	{
		// Standard case (slerp)
		float sin = Math::Sqrt(1 - Math::Sqr(cos));
		Radian angle = Math::Atan2(sin, cos);
		float invSin = 1.0f / sin;
		float coeff0 = Math::Sin((1.0f - t) * angle) * invSin;
		float coeff1 = Math::Sin(t * angle) * invSin;
		return coeff0 * p + coeff1 * quat;
	}
	else
	{
		// There are two situations:
		// 1. "p" and "q" are very close (fCos ~= +1), so we can do a linear
		//    interpolation safely.
		// 2. "p" and "q" are almost inverse of each other (fCos ~= -1), there
		//    are an infinite number of possibilities interpolation. but we haven't
		//    have method to fix this case, so just use linear interpolation here.
		Quaternion ret = (1.0f - t) * p + t * quat;

		// Taking the complement requires renormalization
		ret.Normalize();
		return ret;
	}
}

Quaternion Quaternion::GetRotationFromTo(const Vector3& from, const Vector3& dest, const Vector3& fallbackAxis)
{
	// Based on Stan Melax's article in Game Programming Gems
	Quaternion q;

	Vector3 v0 = from;
	Vector3 v1 = dest;
	v0.Normalize();
	v1.Normalize();

	float d = v0.Dot(v1);

	// If dot == 1, vectors are the same
	if(d >= 1.0f)
		return Quaternion::kIdentity;

	if(d < (1e-6f - 1.0f))
	{
		if(fallbackAxis != Vector3::kZero)
		{
			// Rotate 180 degrees about the fallback axis
			q.FromAxisAngle(fallbackAxis, Radian(Math::kPi));
		}
		else
		{
			// Generate an axis
			Vector3 axis = Vector3::kUnitX.Cross(from);
			if(axis.IsZeroLength()) // Pick another if colinear
				axis = Vector3::kUnitY.Cross(from);
			axis.Normalize();
			q.FromAxisAngle(axis, Radian(Math::kPi));
		}
	}
	else
	{
		float s = Math::Sqrt((1 + d) * 2);
		float invs = 1 / s;

		Vector3 c = v0.Cross(v1);

		q.X = c.X * invs;
		q.Y = c.Y * invs;
		q.Z = c.Z * invs;
		q.W = s * 0.5f;
		q.Normalize();
	}

	return q;
}
