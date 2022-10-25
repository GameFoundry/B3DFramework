//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsCapsule.h"
#include "Math/BsRay.h"

using namespace bs;

Capsule::Capsule(const LineSegment3& segment, float radius)
	: mSegment(segment), mRadius(radius)
{}

std::pair<bool, float> Capsule::Intersects(const Ray& ray) const
{
	const Vector3& org = ray.GetOrigin();
	const Vector3& dir = ray.GetDirection();

	Vector3 segDir = mSegment.End - mSegment.Start;
	float segExtent = segDir.Normalize() * 0.5f;
	Vector3 segCenter = mSegment.Start + segDir * segExtent;

	Vector3 basis[3];
	basis[0] = segDir;
	basis[0].OrthogonalComplement(basis[1], basis[2]);

	float rSqr = mRadius * mRadius;

	Vector3 diff = org - segCenter;
	Vector3 P(basis[1].Dot(diff), basis[2].Dot(diff), basis[0].Dot(diff));

	// Get the z-value, in capsule coordinates, of the incoming line's
	// unit-length direction.
	float dz = basis[0].Dot(dir);
	if(std::abs(dz) == 1.0f)
	{
		// The line is parallel to the capsule axis.  Determine whether the
		// line intersects the capsule hemispheres.
		float radialSqrDist = rSqr - P[0] * P[0] - P[1] * P[1];
		if(radialSqrDist < 0.0f)
		{
			// The line is outside the cylinder of the capsule, so there is no
			// intersection.
			return std::make_pair(false, 0.0f);
		}

		// The line intersects the hemispherical caps.
		float zOffset = std::sqrt(radialSqrDist) + segExtent;
		if(dz > 0.0f)
			return std::make_pair(true, -P[2] - zOffset);
		else
			return std::make_pair(true, P[2] - zOffset);
	}

	// Convert the incoming line unit-length direction to capsule coordinates.
	Vector3 D(basis[1].Dot(dir), basis[2].Dot(dir), dz);

	// Test intersection of line with infinite cylinder
	float a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
	float a1 = P[0] * D[0] + P[1] * D[1];
	float a2 = D[0] * D[0] + D[1] * D[1];
	float discr = a1 * a1 - a0 * a2;

	if(discr < 0.0f)
	{
		// The line does not intersect the infinite cylinder.
		return std::make_pair(false, 0.0f);
	}

	float root, inv, tValue, zValue;
	float nearestT = std::numeric_limits<float>::max();
	bool foundOneIntersection = false;

	if(discr > 0.0f)
	{
		// The line intersects the infinite cylinder in two places.
		root = std::sqrt(discr);
		inv = 1.0f / a2;

		tValue = (-a1 - root) * inv;
		zValue = P[2] + tValue * D[2];
		if(std::abs(zValue) <= segExtent)
		{
			nearestT = tValue;
			foundOneIntersection = true;
		}

		tValue = (-a1 + root) * inv;
		zValue = P[2] + tValue * D[2];
		if(std::abs(zValue) <= segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}
	}
	else
	{
		// The line is tangent to the infinite cylinder but intersects the
		// cylinder in a single point.
		tValue = -a1 / a2;
		zValue = P[2] + tValue * D[2];
		if(std::abs(zValue) <= segExtent)
			return std::make_pair(true, tValue);
	}

	// Test intersection with bottom hemisphere.
	float PZpE = P[2] + segExtent;
	a1 += PZpE * D[2];
	a0 += PZpE * PZpE;
	discr = a1 * a1 - a0;
	if(discr > 0)
	{
		root = sqrt(discr);
		tValue = -a1 - root;
		zValue = P[2] + tValue * D[2];
		if(zValue <= -segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}

		tValue = -a1 + root;
		zValue = P[2] + tValue * D[2];
		if(zValue <= -segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}
	}
	else if(discr == 0.0f)
	{
		tValue = -a1;
		zValue = P[2] + tValue * D[2];
		if(zValue <= -segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}
	}

	// Test intersection with top hemisphere
	a1 -= 2.0f * segExtent * D[2];
	a0 -= 4.0f * segExtent * P[2];
	discr = a1 * a1 - a0;
	if(discr > 0.0f)
	{
		root = sqrt(discr);
		tValue = -a1 - root;
		zValue = P[2] + tValue * D[2];
		if(zValue >= segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}

		tValue = -a1 + root;
		zValue = P[2] + tValue * D[2];
		if(zValue >= segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}
	}
	else if(discr == 0.0f)
	{
		tValue = -a1;
		zValue = P[2] + tValue * D[2];
		if(zValue >= segExtent)
		{
			if(foundOneIntersection)
				return std::make_pair(true, nearestT < tValue ? nearestT : tValue);
			else
			{
				nearestT = tValue;
				foundOneIntersection = true;
			}
		}
	}

	if(foundOneIntersection)
		return std::make_pair(true, nearestT);

	return std::make_pair(false, 0.0f);
}
