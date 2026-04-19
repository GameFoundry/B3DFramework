//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DQuaternion.h"

#include "Math/B3DMath.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DVector3.h"

using namespace b3d;

template<typename T>
void TQuaternion<T>::FromRotationMatrix(const TMatrix3<T>& mat)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	T trace = mat[0][0] + mat[1][1] + mat[2][2];
	T root;

	if(trace > 0.0f)
	{
		// |w| > 1/2, may as well choose w > 1/2
		root = Math::SquareRoot(trace + (T)1.0); // 2w
		W = (T)0.5 * root;
		root = (T)0.5 / root; // 1/(4w)
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

		root = Math::SquareRoot(mat[i][i] - mat[j][j] - mat[k][k] + (T)1.0);

		T* cmpntLookup[3] = { &X, &Y, &Z };
		*cmpntLookup[i] = (T)0.5 * root;
		root = (T)0.5 / root;

		W = (mat[k][j] - mat[j][k]) * root;
		*cmpntLookup[j] = (mat[j][i] + mat[i][j]) * root;
		*cmpntLookup[k] = (mat[k][i] + mat[i][k]) * root;
	}

	Normalize();
}

template<typename T>
void TQuaternion<T>::FromAxisAngle(const TVector3<T>& axis, const TRadian<T>& angle)
{
	TRadian halfAngle((T)0.5 * angle);
	T sin = Math::Sin(halfAngle);

	W = Math::Cos(halfAngle);
	X = sin * axis.X;
	Y = sin * axis.Y;
	Z = sin * axis.Z;
}

template<typename T>
void TQuaternion<T>::FromAxes(const TVector3<T>& xaxis, const TVector3<T>& yaxis, const TVector3<T>& zaxis)
{
	TMatrix3<T> kRot;

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

template<typename T>
void TQuaternion<T>::FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle)
{
	TRadian<T> halfXAngle = xAngle * (T)0.5;
	TRadian<T> halfYAngle = yAngle * (T)0.5;
	TRadian<T> halfZAngle = zAngle * (T)0.5;

	T cx = Math::Cos(halfXAngle);
	T sx = Math::Sin(halfXAngle);

	T cy = Math::Cos(halfYAngle);
	T sy = Math::Sin(halfYAngle);

	T cz = Math::Cos(halfZAngle);
	T sz = Math::Sin(halfZAngle);

	TQuaternion quatX(cx, sx, (T)0.0, (T)0.0);
	TQuaternion quatY(cy, (T)0.0, sy, (T)0.0);
	TQuaternion quatZ(cz, (T)0.0, (T)0.0, sz);

	*this = quatZ * (quatX * quatY);
}

template<typename T>
void TQuaternion<T>::FromEulerAngles(const TRadian<T>& xAngle, const TRadian<T>& yAngle, const TRadian<T>& zAngle, EulerAngleOrder order)
{
	static constexpr const EulerAngleOrderData kEaLookup[6] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 } };
	const EulerAngleOrderData& l = kEaLookup[(int)order];

	TRadian<T> halfXAngle = xAngle * (T)0.5;
	TRadian<T> halfYAngle = yAngle * (T)0.5;
	TRadian<T> halfZAngle = zAngle * (T)0.5;

	T cx = Math::Cos(halfXAngle);
	T sx = Math::Sin(halfXAngle);

	T cy = Math::Cos(halfYAngle);
	T sy = Math::Sin(halfYAngle);

	T cz = Math::Cos(halfZAngle);
	T sz = Math::Sin(halfZAngle);

	TQuaternion quats[3];
	quats[0] = TQuaternion(cx, sx, (T)0.0, (T)0.0);
	quats[1] = TQuaternion(cy, (T)0.0, sy, (T)0.0);
	quats[2] = TQuaternion(cz, (T)0.0, (T)0.0, sz);

	*this = quats[l.C] * (quats[l.B] * quats[l.A]);
}

template<typename T>
void TQuaternion<T>::ToRotationMatrix(TMatrix3<T>& mat) const
{
	T tx = X + X;
	T ty = Y + Y;
	T tz = Z + Z;
	T twx = tx * W;
	T twy = ty * W;
	T twz = tz * W;
	T txx = tx * X;
	T txy = ty * X;
	T txz = tz * X;
	T tyy = ty * Y;
	T tyz = tz * Y;
	T tzz = tz * Z;

	mat[0][0] = (T)1.0 - (tyy + tzz);
	mat[0][1] = txy - twz;
	mat[0][2] = txz + twy;
	mat[1][0] = txy + twz;
	mat[1][1] = (T)1.0 - (txx + tzz);
	mat[1][2] = tyz - twx;
	mat[2][0] = txz - twy;
	mat[2][1] = tyz + twx;
	mat[2][2] = (T)1.0 - (txx + tyy);
}

template<typename T>
void TQuaternion<T>::ToAxisAngle(TVector3<T>& axis, TRadian<T>& angle) const
{
	T sqrLength = X * X + Y * Y + Z * Z;
	if(sqrLength > (T)0.0)
	{
		angle = (T)2.0 * Math::Acos(W);
		T invLength = Math::InverseSquareRoot(sqrLength);
		axis.X = X * invLength;
		axis.Y = Y * invLength;
		axis.Z = Z * invLength;
	}
	else
	{
		// Angle is 0 (mod 2*pi), so any axis will do
		angle = TRadian((T)0.0);
		axis.X = (T)1.0;
		axis.Y = (T)0.0;
		axis.Z = (T)0.0;
	}
}

template<typename T>
void TQuaternion<T>::ToAxes(TVector3<T>& xaxis, TVector3<T>& yaxis, TVector3<T>& zaxis) const
{
	TMatrix3<T> matRot;
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

template<typename T>
bool TQuaternion<T>::ToEulerAngles(TRadian<T>& xAngle, TRadian<T>& yAngle, TRadian<T>& zAngle) const
{
	TMatrix3<T> matRot;
	ToRotationMatrix(matRot);
	return matRot.ToEulerAngles(xAngle, yAngle, zAngle);
}

template<typename T>
TVector3<T> TQuaternion<T>::XAxis() const
{
	T fTy = (T)2.0 * Y;
	T fTz = (T)2.0 * Z;
	T fTwy = fTy * W;
	T fTwz = fTz * W;
	T fTxy = fTy * X;
	T fTxz = fTz * X;
	T fTyy = fTy * Y;
	T fTzz = fTz * Z;

	return TVector3<T>((T)1.0 - (fTyy + fTzz), fTxy + fTwz, fTxz - fTwy);
}

template<typename T>
TVector3<T> TQuaternion<T>::YAxis() const
{
	T fTx = (T)2.0 * X;
	T fTy = (T)2.0 * Y;
	T fTz = (T)2.0 * Z;
	T fTwx = fTx * W;
	T fTwz = fTz * W;
	T fTxx = fTx * X;
	T fTxy = fTy * X;
	T fTyz = fTz * Y;
	T fTzz = fTz * Z;

	return TVector3<T>(fTxy - fTwz, 1.0f - (fTxx + fTzz), fTyz + fTwx);
}

template<typename T>
TVector3<T> TQuaternion<T>::ZAxis() const
{
	T fTx = (T)2.0 * X;
	T fTy = (T)2.0 * Y;
	T fTz = (T)2.0 * Z;
	T fTwx = fTx * W;
	T fTwy = fTy * W;
	T fTxx = fTx * X;
	T fTxz = fTz * X;
	T fTyy = fTy * Y;
	T fTyz = fTz * Y;

	return TVector3<T>(fTxz + fTwy, fTyz - fTwx, 1.0f - (fTxx + fTyy));
}

template<typename T>
TQuaternion<T> TQuaternion<T>::Inverse() const
{
	T fNorm = W * W + X * X + Y * Y + Z * Z;
	if(fNorm > (T)0.0)
	{
		T fInvNorm = (T)1.0 / fNorm;
		return TQuaternion(W * fInvNorm, -X * fInvNorm, -Y * fInvNorm, -Z * fInvNorm);
	}
	else
	{
		// Return an invalid result to flag the error
		return kZero;
	}
}

template<typename T>
TVector3<T> TQuaternion<T>::Rotate(const TVector3<T>& v) const
{
	// Note: Does compiler generate fast code here? Perhaps its better to pull all code locally without constructing
	//       an intermediate matrix.
	TMatrix3<T> rot;
	ToRotationMatrix(rot);
	return rot.Multiply(v);
}

template<typename T>
void TQuaternion<T>::LookRotation(const TVector3<T>& forwardDir)
{
	if(forwardDir == TVector3<T>::kZero)
		return;

	TVector3<T> nrmForwardDir = TVector3<T>::Normalize(forwardDir);
	TVector3<T> currentForwardDir = -ZAxis();

	if((nrmForwardDir + currentForwardDir).SquaredLength() < (T)0.00005)
	{
		// Oops, a 180 degree turn (infinite possible rotation axes)
		// Default to yaw i.e. use current UP
		*this = TQuaternion(-Y, -Z, W, X);
	}
	else
	{
		// Derive shortest arc to new direction
		TQuaternion rotQuat = GetRotationFromTo(currentForwardDir, nrmForwardDir);
		*this = rotQuat * *this;
	}
}

template<typename T>
void TQuaternion<T>::LookRotation(const TVector3<T>& forwardDir, const TVector3<T>& upDir)
{
	TVector3<T> forward = TVector3<T>::Normalize(forwardDir);
	TVector3<T> up = TVector3<T>::Normalize(upDir);

	if(Math::ApproxEquals(Math::Abs(TVector3<T>::Dot(forward, up)), (T)1.0))
	{
		LookRotation(forward);
		return;
	}

	TVector3<T> x = TVector3<T>::Cross(forward, up);
	TVector3<T> y = TVector3<T>::Cross(x, forward);

	x.Normalize();
	y.Normalize();

	*this = TQuaternion(x, y, -forward);
}

template<typename T>
TQuaternion<T> TQuaternion<T>::Slerp(T t, const TQuaternion<T>& p, const TQuaternion<T>& q, bool shortestPath)
{
	T cos = p.Dot(q);
	TQuaternion<T> quat;

	if(cos < (T)0.0 && shortestPath)
	{
		cos = -cos;
		quat = -q;
	}
	else
	{
		quat = q;
	}

	if(abs(cos) < (T)1 - kEpsilon)
	{
		// Standard case (slerp)
		T sin = Math::SquareRoot(1 - Math::Square(cos));
		TRadian<T> angle = Math::Atan2(sin, cos);
		T invSin = (T)1.0 / sin;
		T coeff0 = Math::Sin(((T)1.0 - t) * angle) * invSin;
		T coeff1 = Math::Sin(t * angle) * invSin;
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
		TQuaternion<T> ret = ((T)1.0 - t) * p + t * quat;

		// Taking the complement requires renormalization
		ret.Normalize();
		return ret;
	}
}

template<typename T>
TQuaternion<T> TQuaternion<T>::GetRotationFromTo(const TVector3<T>& from, const TVector3<T>& dest, const TVector3<T>& fallbackAxis)
{
	// Based on Stan Melax's article in Game Programming Gems
	TQuaternion<T> q;

	TVector3<T> v0 = from;
	TVector3<T> v1 = dest;
	v0.Normalize();
	v1.Normalize();

	T d = v0.Dot(v1);

	// If dot == 1, vectors are the same
	if(d >= (T)1.0)
		return TQuaternion::kIdentity;

	if(d < (T)(1e-6 - 1.0))
	{
		if(fallbackAxis != TVector3<T>::kZero)
		{
			// Rotate 180 degrees about the fallback axis
			q.FromAxisAngle(fallbackAxis, TRadian<T>(Math::kPi));
		}
		else
		{
			// Generate an axis
			TVector3<T> axis = TVector3<T>::kUnitX.Cross(from);
			if(axis.IsZeroLength()) // Pick another if colinear
				axis = TVector3<T>::kUnitY.Cross(from);
			axis.Normalize();
			q.FromAxisAngle(axis, TRadian<T>(Math::kPi));
		}
	}
	else
	{
		T s = Math::SquareRoot(((T)1 + d) * (T)2);
		T invs = (T)1 / s;

		TVector3<T> c = v0.Cross(v1);

		q.X = c.X * invs;
		q.Y = c.Y * invs;
		q.Z = c.Z * invs;
		q.W = s * (T)0.5;
		q.Normalize();
	}

	return q;
}

template struct B3D_EXPORT TQuaternion<float>;
template struct B3D_EXPORT TQuaternion<double>;
