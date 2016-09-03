//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlane.h"
#include "BsMatrix3.h"
#include "BsAABox.h"
#include "BsSphere.h"
#include "BsRay.h"
#include "BsMath.h"

namespace BansheeEngine 
{
	Plane::Plane()
		:normal(BsZero), d(0.0f)
	{ }

	Plane::Plane(const Plane& copy)
		:normal(copy.normal), d(copy.d)
	{ }

	Plane::Plane(const Vector3& normal, float d)
		:normal(normal), d(d)
	{ }

	Plane::Plane(float a, float b, float c, float _d)
		:normal(a, b, c), d(_d)
	{ }

	Plane::Plane(const Vector3& normal, const Vector3& point)
		:normal(normal), d(normal.dot(point))
	{ }

	Plane::Plane(const Vector3& point0, const Vector3& point1, const Vector3& point2)
	{
		Vector3 kEdge1 = point1 - point0;
		Vector3 kEdge2 = point2 - point0;
		normal = kEdge1.cross(kEdge2);
		normal.normalize();
		d = normal.dot(point0);
	}

	float Plane::getDistance(const Vector3& point) const
	{
		return normal.dot(point) - d;
	}

	Plane::Side Plane::getSide(const Vector3& point) const
	{
		float dist = getDistance(point);

		if (dist < 0.0f)
			return Plane::NEGATIVE_SIDE;

		if (dist > 0.0f)
			return Plane::POSITIVE_SIDE;

		return Plane::NO_SIDE;
	}

	Plane::Side Plane::getSide(const AABox& box) const
	{
		// Calculate the distance between box centre and the plane
		float dist = getDistance(box.getCenter());

		// Calculate the maximize allows absolute distance for
		// the distance between box centre and plane
		Vector3 halfSize = box.getHalfSize();
		float maxAbsDist = Math::abs(normal.x * halfSize.x) + Math::abs(normal.y * halfSize.y) + Math::abs(normal.z * halfSize.z);

		if (dist < -maxAbsDist)
			return Plane::NEGATIVE_SIDE;

		if (dist > +maxAbsDist)
			return Plane::POSITIVE_SIDE;

		return Plane::BOTH_SIDE;
	}

	Plane::Side Plane::getSide(const Sphere& sphere) const
	{
		// Calculate the distance between box centre and the plane
		float dist = getDistance(sphere.getCenter());
		float radius = sphere.getRadius();

		if (dist < -radius)
			return Plane::NEGATIVE_SIDE;

		if (dist > +radius)
			return Plane::POSITIVE_SIDE;

		return Plane::BOTH_SIDE;
	}

	Vector3 Plane::projectVector(const Vector3& point) const
	{
		// We know plane normal is unit length, so use simple method
		Matrix3 xform;
		xform[0][0] = 1.0f - normal.x * normal.x;
		xform[0][1] = -normal.x * normal.y;
		xform[0][2] = -normal.x * normal.z;
		xform[1][0] = -normal.y * normal.x;
		xform[1][1] = 1.0f - normal.y * normal.y;
		xform[1][2] = -normal.y * normal.z;
		xform[2][0] = -normal.z * normal.x;
		xform[2][1] = -normal.z * normal.y;
		xform[2][2] = 1.0f - normal.z * normal.z;
		return xform.transform(point);

	}

    float Plane::normalize()
    {
        float fLength = normal.length();

        // Will also work for zero-sized vectors, but will change nothing
        if (fLength > 1e-08f)
        {
            float fInvLength = 1.0f / fLength;
            normal *= fInvLength;
            d *= fInvLength;
        }

        return fLength;
    }

	bool Plane::intersects(const AABox& box) const
	{
		return box.intersects(*this);
	}

	bool Plane::intersects(const Sphere& sphere) const
	{
		return sphere.intersects(*this);
	}

	std::pair<bool, float> Plane::intersects(const Ray& ray) const
	{
		float denom = normal.dot(ray.getDirection());
		if (Math::abs(denom) < std::numeric_limits<float>::epsilon())
		{
			// Parallel
			return std::pair<bool, float>(false, 0.0f);
		}
		else
		{
			float nom = normal.dot(ray.getOrigin()) - d;
			float t = -(nom/denom);
			return std::pair<bool, float>(t >= 0.0f, t);
		}
	}
}
