//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSphere.h"
#include "BsRay.h"
#include "BsPlane.h"
#include "BsAABox.h"
#include "BsMath.h"

namespace bs
{
	void Sphere::merge(const Sphere& rhs)
	{
		Vector3 newCenter = (mCenter + rhs.mCenter) * 0.5f;

		float newRadiusA = newCenter.distance(mCenter) + getRadius();
		float newRadiusB = newCenter.distance(rhs.mCenter) + rhs.getRadius();
		
		mCenter = newCenter;
		mRadius = std::max(newRadiusA, newRadiusB);
	}

	void Sphere::merge(const Vector3& point)
	{
		float dist = point.distance(mCenter);
		mRadius = std::max(mRadius, dist);
	}

	void Sphere::transform(const Matrix4& matrix)
	{
		Vector3 edge = mCenter + Vector3::UNIT_X * mRadius;
		mCenter = matrix.multiplyAffine(mCenter);
		edge = matrix.multiplyAffine(edge);

		mRadius = mCenter.distance(edge);
	}

	bool Sphere::contains(const Vector3& v) const
	{
		return ((v - mCenter).squaredLength() <= Math::sqr(mRadius));
	}

	bool Sphere::intersects(const Sphere& s) const
	{
		return (s.mCenter - mCenter).squaredLength() <=
			Math::sqr(s.mRadius + mRadius);
	}

	std::pair<bool, float> Sphere::intersects(const Ray& ray, bool discardInside) const
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

	bool Sphere::intersects(const Plane& plane) const
	{
		return (Math::abs(plane.getDistance(getCenter())) <= getRadius());
	}

	bool Sphere::intersects(const AABox& box) const
	{
		return box.intersects(*this);
	}
}