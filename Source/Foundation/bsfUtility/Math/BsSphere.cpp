//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsSphere.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsAABox.h"
#include "Math/BsMath.h"

using namespace bs;

template<typename T>
void TSphere<T>::Merge(const TSphere<T>& rhs)
{
	TVector3<T> newCenter = (Center + rhs.Center) * (T)0.5;

	T newRadiusA = newCenter.Distance(Center) + Radius;
	T newRadiusB = newCenter.Distance(rhs.Center) + rhs.Radius;

	Center = newCenter;
	Radius = std::max(newRadiusA, newRadiusB);
}

template<typename T>
void TSphere<T>::Merge(const TVector3<T>& point)
{
	T dist = point.Distance(Center);
	Radius = std::max(Radius, dist);
}

template<>
template<>
void TSphere<float>::Transform<float, 0>(const Matrix4& matrix)
{
	using T = float;

	T lengthSqrd[3];
	for(u32 i = 0; i < 3; i++)
	{
		TVector3<T> column = matrix.GetColumn(i);
		lengthSqrd[i] = column.Dot(column);
	}

	T maxLengthSqrd = std::max(lengthSqrd[0], std::max(lengthSqrd[1], lengthSqrd[2]));

	Center = matrix.MultiplyAffine(Center);
	Radius *= sqrt(maxLengthSqrd);
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

template struct B3D_UTILITY_EXPORT TSphere<float>;
template struct B3D_UTILITY_EXPORT TSphere<double>;
