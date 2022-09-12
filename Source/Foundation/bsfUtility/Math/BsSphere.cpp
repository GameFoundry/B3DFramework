//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsSphere.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsAABox.h"
#include "Math/BsMath.h"

namespace bs
{
	void Sphere::Merge(const Sphere& rhs)
	{
		Vector3 newCenter = (mCenter + rhs.mCenter) * 0.5f;

		float newRadiusA = newCenter.distance(mCenter) + getRadius();
		float newRadiusB = newCenter.distance(rhs.mCenter) + rhs.getRadius();
		
		mCenter = newCenter;
		mRadius = std::max(newRadiusA, newRadiusB);
	}

	void Sphere::Merge(const Vector3& point)
	{
		float dist = point.distance(mCenter);
		mRadius = std::max(mRadius, dist);
	}

	void Sphere::Transform(const Matrix4& matrix)
	{
		float lengthSqrd[3];
		for(UINT32 i = 0; i < 3; i++)
		{
			Vector3 column = matrix.getColumn(i);
			lengthSqrd[i] = column.dot(column);
		}

		float maxLengthSqrd = std::max(lengthSqrd[0], std::max(lengthSqrd[1], lengthSqrd[2]));

		mCenter = matrix.multiplyAffine(mCenter);
		mRadius *= sqrt(maxLengthSqrd);
	}

	bool Sphere::Contains(const Vector3& v) const
	{
		return ((v - mCenter).squaredLength() <= Math::sqr(mRadius));
	}

	bool Sphere::Intersects(const Sphere& s) const
	{
		return (s.mCenter - mCenter).squaredLength() <=
			Math::sqr(s.mRadius + mRadius);
	}

	std::pair<bool, float> Sphere::Intersects(const Ray& ray, bool discardInside) const
	{
		const Vector3& raydir = ray.getDirection();
		const Vector3& rayorig = ray.getOrigin() - getCenter();
		float radius = getRadius();

		// Check origin inside first
		if (rayorig.squaredLength() <= radius*radius && discardInside)
		{
			return std::pair<bool, float>(true, 0.0f);
		}

		// t = (-b +/- sqrt(b*b + 4ac)) / 2a
		float a = raydir.dot(raydir);
		float b = 2 * rayorig.dot(raydir);
		float c = rayorig.dot(rayorig) - radius*radius;

		// Determinant
		float d = (b*b) - (4 * a * c);
		if (d < 0)
		{
			// No intersection
			return std::pair<bool, float>(false, 0.0f);
		}
		else
		{
			// If d == 0 there is one intersection, if d > 0 there are 2.
			// We only return the first one.
			
			float t = ( -b - Math::sqrt(d) ) / (2 * a);
			if (t < 0)
				t = ( -b + Math::sqrt(d) ) / (2 * a);

			return std::pair<bool, float>(true, t);
		}
	}

	bool Sphere::Intersects(const Plane& plane) const
	{
		return (Math::abs(plane.getDistance(getCenter())) <= getRadius());
	}

	bool Sphere::Intersects(const AABox& box) const
	{
		return box.intersects(*this);
	}
}
