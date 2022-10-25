//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsPlane.h"
#include "Math/BsMatrix3.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsRay.h"
#include "Math/BsMath.h"

using namespace bs;

Plane::Plane(const Vector3& normal, float d)
	: Normal(normal), D(d)
{}

Plane::Plane(float a, float b, float c, float _d)
	: Normal(a, b, c), D(_d)
{}

Plane::Plane(const Vector3& normal, const Vector3& point)
	: Normal(normal), D(normal.Dot(point))
{}

Plane::Plane(const Vector3& point0, const Vector3& point1, const Vector3& point2)
{
	Vector3 kEdge1 = point1 - point0;
	Vector3 kEdge2 = point2 - point0;
	Normal = kEdge1.Cross(kEdge2);
	Normal.Normalize();
	D = Normal.Dot(point0);
}

float Plane::GetDistance(const Vector3& point) const
{
	return Normal.Dot(point) - D;
}

Plane::Side Plane::GetSide(const Vector3& point, float epsilon) const
{
	float dist = GetDistance(point);

	if(dist > epsilon)
		return Plane::POSITIVE_SIDE;

	if(dist < -epsilon)
		return Plane::NEGATIVE_SIDE;

	return Plane::NO_SIDE;
}

Plane::Side Plane::GetSide(const AABox& box) const
{
	// Calculate the distance between box centre and the plane
	float dist = GetDistance(box.GetCenter());

	// Calculate the maximize allows absolute distance for
	// the distance between box centre and plane
	Vector3 halfSize = box.GetHalfSize();
	float maxAbsDist = abs(Normal.X * halfSize.X) + abs(Normal.Y * halfSize.Y) + abs(Normal.Z * halfSize.Z);

	if(dist < -maxAbsDist)
		return Plane::NEGATIVE_SIDE;

	if(dist > +maxAbsDist)
		return Plane::POSITIVE_SIDE;

	return Plane::BOTH_SIDE;
}

Plane::Side Plane::GetSide(const Sphere& sphere) const
{
	// Calculate the distance between box centre and the plane
	float dist = GetDistance(sphere.GetCenter());
	float radius = sphere.GetRadius();

	if(dist < -radius)
		return Plane::NEGATIVE_SIDE;

	if(dist > +radius)
		return Plane::POSITIVE_SIDE;

	return Plane::BOTH_SIDE;
}

Vector3 Plane::ProjectVector(const Vector3& point) const
{
	// We know plane normal is unit length, so use simple method
	Matrix3 xform;
	xform[0][0] = 1.0f - Normal.X * Normal.X;
	xform[0][1] = -Normal.X * Normal.Y;
	xform[0][2] = -Normal.X * Normal.Z;
	xform[1][0] = -Normal.Y * Normal.X;
	xform[1][1] = 1.0f - Normal.Y * Normal.Y;
	xform[1][2] = -Normal.Y * Normal.Z;
	xform[2][0] = -Normal.Z * Normal.X;
	xform[2][1] = -Normal.Z * Normal.Y;
	xform[2][2] = 1.0f - Normal.Z * Normal.Z;
	return xform.Multiply(point);
}

float Plane::Normalize()
{
	float fLength = Normal.Length();

	// Will also work for zero-sized vectors, but will change nothing
	if(fLength > 1e-08f)
	{
		float fInvLength = 1.0f / fLength;
		Normal *= fInvLength;
		D *= fInvLength;
	}

	return fLength;
}

bool Plane::Intersects(const AABox& box) const
{
	return box.Intersects(*this);
}

bool Plane::Intersects(const Sphere& sphere) const
{
	return sphere.Intersects(*this);
}

std::pair<bool, float> Plane::Intersects(const Ray& ray) const
{
	float denom = Normal.Dot(ray.GetDirection());
	if(abs(denom) < std::numeric_limits<float>::epsilon())
	{
		// Parallel
		return std::pair<bool, float>(false, 0.0f);
	}
	else
	{
		float nom = Normal.Dot(ray.GetOrigin()) - D;
		float t = -(nom / denom);
		return std::pair<bool, float>(t >= 0.0f, t);
	}
}
