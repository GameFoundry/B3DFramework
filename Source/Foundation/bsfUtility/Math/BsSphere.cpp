//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsSphere.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsAABox.h"
#include "Math/BsMath.h"

using namespace bs;

void Sphere::Merge(const Sphere& rhs)
{
	Vector3 newCenter = (mCenter + rhs.mCenter) * 0.5f;

	float newRadiusA = newCenter.Distance(mCenter) + GetRadius();
	float newRadiusB = newCenter.Distance(rhs.mCenter) + rhs.GetRadius();

	mCenter = newCenter;
	mRadius = std::max(newRadiusA, newRadiusB);
}

void Sphere::Merge(const Vector3& point)
{
	float dist = point.Distance(mCenter);
	mRadius = std::max(mRadius, dist);
}

void Sphere::Transform(const Matrix4& matrix)
{
	float lengthSqrd[3];
	for(u32 i = 0; i < 3; i++)
	{
		Vector3 column = matrix.GetColumn(i);
		lengthSqrd[i] = column.Dot(column);
	}

	float maxLengthSqrd = std::max(lengthSqrd[0], std::max(lengthSqrd[1], lengthSqrd[2]));

	mCenter = matrix.MultiplyAffine(mCenter);
	mRadius *= sqrt(maxLengthSqrd);
}

bool Sphere::Contains(const Vector3& v) const
{
	return ((v - mCenter).SquaredLength() <= Math::Sqr(mRadius));
}

bool Sphere::Intersects(const Sphere& s) const
{
	return (s.mCenter - mCenter).SquaredLength() <=
		Math::Sqr(s.mRadius + mRadius);
}

std::pair<bool, float> Sphere::Intersects(const Ray& ray, bool discardInside) const
{
	const Vector3& raydir = ray.GetDirection();
	const Vector3& rayorig = ray.GetOrigin() - GetCenter();
	float radius = GetRadius();

	// Check origin inside first
	if(rayorig.SquaredLength() <= radius * radius && discardInside)
	{
		return std::pair<bool, float>(true, 0.0f);
	}

	// t = (-b +/- sqrt(b*b + 4ac)) / 2a
	float a = raydir.Dot(raydir);
	float b = 2 * rayorig.Dot(raydir);
	float c = rayorig.Dot(rayorig) - radius * radius;

	// Determinant
	float d = (b * b) - (4 * a * c);
	if(d < 0)
	{
		// No intersection
		return std::pair<bool, float>(false, 0.0f);
	}
	else
	{
		// If d == 0 there is one intersection, if d > 0 there are 2.
		// We only return the first one.

		float t = (-b - Math::Sqrt(d)) / (2 * a);
		if(t < 0)
			t = (-b + Math::Sqrt(d)) / (2 * a);

		return std::pair<bool, float>(true, t);
	}
}

bool Sphere::Intersects(const Plane& plane) const
{
	return (Math::Abs(plane.GetDistance(GetCenter())) <= GetRadius());
}

bool Sphere::Intersects(const AABox& box) const
{
	return box.Intersects(*this);
}
