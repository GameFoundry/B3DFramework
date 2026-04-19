//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DSphere.h"
#include "Math/B3DRay.h"
#include "Math/B3DPlane.h"
#include "Math/B3DAABox.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<typename T>
void TSphere<T>::Merge(const TSphere<T>& rhs)
{
	const TVector3<T> delta = Center - rhs.Center;
	const T centerDistance = delta.Length();

	if(Radius >= centerDistance + rhs.Radius)
		return; // This sphere encompasses the other sphere
	else if(rhs.Radius >= centerDistance + Radius)
	{
		*this = rhs;
		return; // Other sphere encompasses this sphere
	}

	const T newRadius = (centerDistance + Radius + rhs.Radius) * (T)0.5;
	Center = Center + (delta / centerDistance) * (newRadius - Radius);
	Radius = newRadius;
}

template<typename T>
void TSphere<T>::Merge(const TVector3<T>& point)
{
	T dist = point.Distance(Center);
	Radius = std::max(Radius, dist);
}

template<typename T>
void TSphere<T>::Transform(const TMatrix4<T>& matrix)
{
	T lengthSquared[3];
	for(u32 columnIndex = 0; columnIndex < 3; columnIndex++)
	{
		TVector3<T> column = matrix.GetColumn(columnIndex);
		lengthSquared[columnIndex] = column.Dot(column);
	}

	T maxLengthSquared = std::max(lengthSquared[0], std::max(lengthSquared[1], lengthSquared[2]));

	Center = matrix.MultiplyAffine(Center);
	Radius *= sqrt(maxLengthSquared);
}

template<typename T>
bool TSphere<T>::Contains(const TVector3<T>& v) const
{
	return ((v - Center).SquaredLength() <= Math::Square(Radius));
}

template<typename T>
bool TSphere<T>::Intersects(const TSphere<T>& s) const
{
	return (s.Center - Center).SquaredLength() <= Math::Square(s.Radius + Radius);
}

template<typename T>
std::pair<bool, T> TSphere<T>::Intersects(const TRay<T>& ray, bool discardInside) const
{
	const TVector3<T>& raydir = ray.Direction;
	const TVector3<T>& rayorig = ray.Origin - Center;
	T radius = Radius;

	// Check origin inside first
	if(rayorig.SquaredLength() <= radius * radius && discardInside)
	{
		return std::pair<bool, T>(true, (T)0.0);
	}

	// t = (-b +/- sqrt(b*b + 4ac)) / 2a
	T a = raydir.Dot(raydir);
	T b = 2 * rayorig.Dot(raydir);
	T c = rayorig.Dot(rayorig) - radius * radius;

	// Determinant
	T d = (b * b) - (4 * a * c);
	if(d < 0)
	{
		// No intersection
		return std::pair<bool, T>(false, (T)0.0);
	}
	else
	{
		// If d == 0 there is one intersection, if d > 0 there are 2.
		// We only return the first one.

		T t = (-b - Math::SquareRoot(d)) / ((T)2.0 * a);
		if(t < (T)0.0)
			t = (-b + Math::SquareRoot(d)) / ((T)2.0 * a);

		return std::pair<bool, T>(true, t);
	}
}

template<typename T>
bool TSphere<T>::Intersects(const TPlane<T>& plane) const
{
	return (Math::Abs(plane.GetDistance(Center)) <= Radius);
}

template<typename T>
bool TSphere<T>::Intersects(const TAABox<T>& box) const
{
	return box.Intersects(*this);
}

template struct B3D_EXPORT TSphere<float>;
template struct B3D_EXPORT TSphere<double>;
