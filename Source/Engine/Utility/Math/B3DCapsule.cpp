//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DCapsule.h"
#include "Math/B3DRay.h"

using namespace b3d;

template<typename T>
TCapsule<T>::TCapsule(const TLineSegment3<T>& segment, T radius)
	: mSegment(segment), mRadius(radius)
{}

template<typename T>
std::pair<bool, T> TCapsule<T>::Intersects(const TRay<T>& ray) const
{
	const TVector3<T>& org = ray.Origin;
	const TVector3<T>& dir = ray.Direction;

	TVector3<T> segDir = mSegment.End - mSegment.Start;
	T segExtent = segDir.Normalize() * (T)0.5;
	TVector3<T> segCenter = mSegment.Start + segDir * segExtent;

	TVector3<T> basis[3];
	basis[0] = segDir;
	basis[0].OrthogonalComplement(basis[1], basis[2]);

	T rSqr = mRadius * mRadius;

	TVector3<T> diff = org - segCenter;
	TVector3<T> P(basis[1].Dot(diff), basis[2].Dot(diff), basis[0].Dot(diff));

	// Get the z-value, in capsule coordinates, of the incoming line's
	// unit-length direction.
	T dz = basis[0].Dot(dir);
	if(std::abs(dz) == (T)1.0)
	{
		// The line is parallel to the capsule axis.  Determine whether the
		// line intersects the capsule hemispheres.
		T radialSqrDist = rSqr - P[0] * P[0] - P[1] * P[1];
		if(radialSqrDist < (T)0.0)
		{
			// The line is outside the cylinder of the capsule, so there is no
			// intersection.
			return std::make_pair(false, (T)0.0);
		}

		// The line intersects the hemispherical caps.
		T zOffset = std::sqrt(radialSqrDist) + segExtent;
		if(dz > (T)0.0)
			return std::make_pair(true, -P[2] - zOffset);
		else
			return std::make_pair(true, P[2] - zOffset);
	}

	// Convert the incoming line unit-length direction to capsule coordinates.
	TVector3<T> D(basis[1].Dot(dir), basis[2].Dot(dir), dz);

	// Test intersection of line with infinite cylinder
	T a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
	T a1 = P[0] * D[0] + P[1] * D[1];
	T a2 = D[0] * D[0] + D[1] * D[1];
	T discr = a1 * a1 - a0 * a2;

	if(discr < (T)0.0)
	{
		// The line does not intersect the infinite cylinder.
		return std::make_pair(false, (T)0.0);
	}

	T root, inv, tValue, zValue;
	T nearestT = std::numeric_limits<T>::max();
	bool foundOneIntersection = false;

	if(discr > (T)0.0)
	{
		// The line intersects the infinite cylinder in two places.
		root = std::sqrt(discr);
		inv = (T)1.0 / a2;

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
	T PZpE = P[2] + segExtent;
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
	else if(discr == (T)0.0)
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
	a1 -= (T)2.0 * segExtent * D[2];
	a0 -= (T)4.0 * segExtent * P[2];
	discr = a1 * a1 - a0;
	if(discr > (T)0.0)
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
	else if(discr == (T)0.0)
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

	return std::make_pair(false, (T)0.0);
}

template struct B3D_EXPORT TCapsule<float>;
template struct B3D_EXPORT TCapsule<double>;
