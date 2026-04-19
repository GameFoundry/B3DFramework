//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DRect3.h"
#include "Math/B3DRay.h"
#include "Math/B3DLineSegment3.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

std::pair<std::array<Vector3, 2>, float> Rect3::GetNearestPoint(const Ray& ray) const
{
	const Vector3& org = ray.Origin;
	const Vector3& dir = ray.Direction;

	bool foundNearest = false;
	float t = 0.0f;
	std::array<Vector3, 2> nearestPoints{ { Vector3::kZero, Vector3::kZero } };
	float distance = 0.0f;

	// Check if Ray intersects the rectangle
	auto intersectResult = Intersects(ray);
	if(intersectResult.first)
	{
		t = intersectResult.second;

		nearestPoints[0] = org + dir * t;
		nearestPoints[1] = nearestPoints[0]; // Just one point of intersection
		foundNearest = true;
	}

	// Ray is either passing next to the rectangle or parallel to it,
	// compare ray to 4 edges of the rectangle
	if(!foundNearest)
	{
		Vector3 scaledAxes[2];
		scaledAxes[0] = HorizontalExtent * HorizontalAxis;
		scaledAxes[1] = VerticalExtent * VerticalAxis;
		;

		distance = std::numeric_limits<float>::max();
		for(u32 axisIndex = 0; axisIndex < 2; axisIndex++)
		{
			for(u32 sideIndex = 0; sideIndex < 2; sideIndex++)
			{
				float sign = (float)(2 * sideIndex - 1);
				Vector3 segCenter = Center + sign * scaledAxes[axisIndex];
				Vector3 segStart = segCenter - scaledAxes[1 - axisIndex];
				Vector3 segEnd = segCenter + scaledAxes[1 - axisIndex];

				LineSegment3 segment(segStart, segEnd);
				auto segResult = segment.GetNearestPoint(ray);

				if(segResult.second < distance)
				{
					nearestPoints = segResult.first;
					distance = segResult.second;
				}
			}
		}
	}

	// Front of the ray is nearest, use found points
	if(t >= 0.0f)
	{
		// Do nothing, we already have the points
	}
	else // Rectangle is behind the ray origin, find nearest point to origin
	{
		auto nearestPointToOrg = GetNearestPoint(org);

		nearestPoints[0] = org;
		nearestPoints[1] = nearestPointToOrg.first;
		distance = nearestPointToOrg.second;
	}

	return std::make_pair(nearestPoints, distance);
}

std::pair<Vector3, float> Rect3::GetNearestPoint(const Vector3& point) const
{
	Vector3 diff = Center - point;
	float b0 = diff.Dot(HorizontalAxis);
	float b1 = diff.Dot(VerticalAxis);
	float s0 = -b0, s1 = -b1;
	float sqrDistance = diff.Dot(diff);

	if(s0 < -HorizontalExtent)
		s0 = -HorizontalExtent;
	else if(s0 > HorizontalExtent)
		s0 = HorizontalExtent;

	sqrDistance += s0 * (s0 + 2.0f * b0);

	if(s1 < -VerticalExtent)
		s1 = -VerticalExtent;
	else if(s1 > VerticalExtent)
		s1 = VerticalExtent;

	sqrDistance += s1 * (s1 + 2.0f * b1);

	if(sqrDistance < 0.0f)
		sqrDistance = 0.0f;

	float dist = std::sqrt(sqrDistance);
	Vector3 nearestPoint = Center + s0 * HorizontalAxis + s1 * VerticalAxis;

	return std::make_pair(nearestPoint, dist);
}

std::pair<bool, float> Rect3::Intersects(const Ray& ray) const
{
	const Vector3& org = ray.Origin;
	const Vector3& dir = ray.Direction;

	Vector3 normal = HorizontalAxis.Cross(VerticalAxis);
	float NdotD = normal.Dot(dir);
	if(fabs(NdotD) > 0.0f)
	{
		Vector3 diff = org - Center;
		Vector3 basis[3];

		basis[0] = dir;
		basis[0].OrthogonalComplement(basis[1], basis[2]);

		float UdD0 = basis[1].Dot(HorizontalAxis);
		float UdD1 = basis[1].Dot(VerticalAxis);
		float UdPmC = basis[1].Dot(diff);
		float VdD0 = basis[2].Dot(HorizontalAxis);
		float VdD1 = basis[2].Dot(VerticalAxis);
		float VdPmC = basis[2].Dot(diff);
		float invDet = 1.0f / (UdD0 * VdD1 - UdD1 * VdD0);

		float s0 = (VdD1 * UdPmC - UdD1 * VdPmC) * invDet;
		float s1 = (UdD0 * VdPmC - VdD0 * UdPmC) * invDet;

		if(fabs(s0) <= HorizontalExtent && fabs(s1) <= VerticalExtent)
		{
			float DdD0 = dir.Dot(HorizontalAxis);
			float DdD1 = dir.Dot(VerticalAxis);
			float DdDiff = dir.Dot(diff);

			float t = s0 * DdD0 + s1 * DdD1 - DdDiff;

			return std::make_pair(true, t);
		}
	}

	return std::make_pair(false, 0.0f);
}
