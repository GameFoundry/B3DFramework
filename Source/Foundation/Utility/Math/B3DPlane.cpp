//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DPlane.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DAABox.h"
#include "Math/B3DSphere.h"
#include "Math/B3DRay.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<typename T>
TPlane<T>::TPlane(const TVector3<T>& point0, const TVector3<T>& point1, const TVector3<T>& point2)
{
	TVector3<T> kEdge1 = point1 - point0;
	TVector3<T> kEdge2 = point2 - point0;
	Normal = kEdge1.Cross(kEdge2);
	Normal.Normalize();
	D = Normal.Dot(point0);
}

template<typename T>
T TPlane<T>::GetDistance(const TVector3<T>& point) const
{
	return Normal.Dot(point) - D;
}

template<typename T>
PlaneSide TPlane<T>::GetSide(const TVector3<T>& point, T epsilon) const
{
	T dist = GetDistance(point);

	if(dist > epsilon)
		return PlaneSide::Positive;

	if(dist < -epsilon)
		return PlaneSide::Negative;

	return PlaneSide::None;
}

template<typename T>
PlaneSide TPlane<T>::GetSide(const TAABox<T>& box) const
{
	// Calculate the distance between box centre and the plane
	T dist = GetDistance(box.GetCenter());

	// Calculate the maximize allows absolute distance for
	// the distance between box centre and plane
	TVector3<T> halfSize = box.GetExtents();
	T maxAbsDist = abs(Normal.X * halfSize.X) + abs(Normal.Y * halfSize.Y) + abs(Normal.Z * halfSize.Z);

	if(dist < -maxAbsDist)
		return PlaneSide::Negative;

	if(dist > +maxAbsDist)
		return PlaneSide::Positive;

	return PlaneSide::Both;
}

template<typename T>
PlaneSide TPlane<T>::GetSide(const TSphere<T>& sphere) const
{
	// Calculate the distance between box centre and the plane
	T dist = GetDistance(sphere.Center);
	T radius = sphere.Radius;

	if(dist < -radius)
		return PlaneSide::Negative;

	if(dist > +radius)
		return PlaneSide::Positive;

	return PlaneSide::Both;
}

template<typename T>
TVector3<T> TPlane<T>::ProjectVector(const TVector3<T>& point) const
{
	// We know plane normal is unit length, so use simple method
	TMatrix3<T> xform;
	xform[0][0] = (T)1.0 - Normal.X * Normal.X;
	xform[0][1] = -Normal.X * Normal.Y;
	xform[0][2] = -Normal.X * Normal.Z;
	xform[1][0] = -Normal.Y * Normal.X;
	xform[1][1] = (T)1.0 - Normal.Y * Normal.Y;
	xform[1][2] = -Normal.Y * Normal.Z;
	xform[2][0] = -Normal.Z * Normal.X;
	xform[2][1] = -Normal.Z * Normal.Y;
	xform[2][2] = (T)1.0 - Normal.Z * Normal.Z;
	return xform.Multiply(point);
}

template<typename T>
T TPlane<T>::Normalize()
{
	T fLength = Normal.Length();

	// Will also work for zero-sized vectors, but will change nothing
	if(fLength > (T)1e-08)
	{
		T fInvLength = (T)1.0 / fLength;
		Normal *= fInvLength;
		D *= fInvLength;
	}

	return fLength;
}

template<typename T>
bool TPlane<T>::Intersects(const TAABox<T>& box) const
{
	return box.Intersects(*this);
}

template<typename T>
bool TPlane<T>::Intersects(const TSphere<T>& sphere) const
{
	return sphere.Intersects(*this);
}

template<typename T>
std::pair<bool, T> TPlane<T>::Intersects(const TRay<T>& ray) const
{
	T denom = Normal.Dot(ray.Direction);
	if(abs(denom) < std::numeric_limits<T>::epsilon())
	{
		// Parallel
		return std::pair<bool, T>(false, (T)0.0);
	}
	else
	{
		T nom = Normal.Dot(ray.Origin) - D;
		T t = -(nom / denom);
		return std::pair<bool, T>(t >= (T)0.0, t);
	}
}

template struct B3D_EXPORT TPlane<float>;
template struct B3D_EXPORT TPlane<double>;
