//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRect3.h"
#include "BsRay.h"
#include "BsLineSegment3.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	Rect3::Rect3()
		:mCenter(BsZero), mAxisHorz(BsZero), mAxisVert(BsZero), mExtentHorz(0.0f), mExtentVert(0.0f)
	{ }

	Rect3::Rect3(const Vector3& center, const std::array<Vector3, 2>& axes,
		const std::array<float, 2>& extents)
		:mCenter(center), mAxisHorz(axes[0]), mAxisVert(axes[1]),
		mExtentHorz(extents[0]), mExtentVert(extents[1])
	{

	}

	std::pair<std::array<Vector3, 2>, float> Rect3::getNearestPoint(const Ray& ray) const
	{
		const Vector3& org = ray.getOrigin();
		const Vector3& dir = ray.getDirection();

		bool foundNearest = false;
		float t = 0.0f;
		std::array<Vector3, 2> nearestPoints { Vector3::ZERO, Vector3::ZERO };
		float distance = 0.0f;

		// Check if Ray intersects the rectangle
		auto intersectResult = intersects(ray);
		if (intersectResult.first)
		{
			t = intersectResult.second;

			nearestPoints[0] = org + dir * t;
			nearestPoints[1] = nearestPoints[0]; // Just one point of intersection
			foundNearest = true;
		}

		// Ray is either passing next to the rectangle or parallel to it, 
		// compare ray to 4 edges of the rectangle
		if (!foundNearest)
		{
			Vector3 scaledAxes[2];
			scaledAxes[0] = mExtentHorz * mAxisHorz;
			scaledAxes[1] = mExtentVert * mAxisVert;;

			distance = std::numeric_limits<float>::max();
			for (UINT32 i = 0; i < 2; i++)
			{
				for (UINT32 j = 0; j < 2; j++)
				{
					float sign = (float)(2 * j - 1);
					Vector3 segCenter = mCenter + sign * scaledAxes[i];
					Vector3 segStart = segCenter - scaledAxes[1 - i];
					Vector3 segEnd = segCenter + scaledAxes[1 - i];

					LineSegment3 segment(segStart, segEnd);
					auto segResult = segment.getNearestPoint(ray);

					if (segResult.second < distance)
					{
						nearestPoints = segResult.first;
						distance = segResult.second;
					}
				}
			}
		}

		// Front of the ray is nearest, use found points
		if (t >= 0.0f)
		{
			// Do nothing, we already have the points
		}
		else // Rectangle is behind the ray origin, find nearest point to origin
		{
			auto nearestPointToOrg = getNearestPoint(org);

			nearestPoints[0] = org;
			nearestPoints[1] = nearestPointToOrg.first;
			distance = nearestPointToOrg.second;
		}

		return std::make_pair(nearestPoints, distance);
	}

	std::pair<Vector3, float> Rect3::getNearestPoint(const Vector3& point) const
	{
		Vector3 diff = mCenter - point;
		float b0 = diff.dot(mAxisHorz);
		float b1 = diff.dot(mAxisVert);
		float s0 = -b0, s1 = -b1;
		float sqrDistance = diff.dot(diff);

		if (s0 < -mExtentHorz)
			s0 = -mExtentHorz;
		else if (s0 > mExtentHorz)
			s0 = mExtentHorz;

		sqrDistance += s0*(s0 + 2.0f*b0);

		if (s1 < -mExtentVert)
			s1 = -mExtentVert;
		else if (s1 > mExtentVert)
			s1 = mExtentVert;

		sqrDistance += s1*(s1 + 2.0f*b1);

		if (sqrDistance < 0.0f)
			sqrDistance = 0.0f;

		float dist = std::sqrt(sqrDistance);
		Vector3 nearestPoint = mCenter + s0 * mAxisHorz + s1 * mAxisVert;

		return std::make_pair(nearestPoint, dist);
	}

	std::pair<bool, float> Rect3::intersects(const Ray& ray) const
	{
		const Vector3& org = ray.getOrigin();
		const Vector3& dir = ray.getDirection();

		Vector3 normal = mAxisHorz.cross(mAxisVert);
		float NdotD = normal.dot(dir);
		if (fabs(NdotD) > 0.0f)
		{
			Vector3 diff = org - mCenter;
			Vector3 basis[3];

			basis[0] = dir;
			basis[0].orthogonalComplement(basis[1], basis[2]);

			float UdD0 = basis[1].dot(mAxisHorz);
			float UdD1 = basis[1].dot(mAxisVert);
			float UdPmC = basis[1].dot(diff);
			float VdD0 = basis[2].dot(mAxisHorz);
			float VdD1 = basis[2].dot(mAxisVert);
			float VdPmC = basis[2].dot(diff);
			float invDet = 1.0f / (UdD0*VdD1 - UdD1*VdD0);

			float s0 = (VdD1*UdPmC - UdD1*VdPmC)*invDet;
			float s1 = (UdD0*VdPmC - VdD0*UdPmC)*invDet;

			if (fabs(s0) <= mExtentHorz && fabs(s1) <= mExtentVert)
			{
				float DdD0 = dir.dot(mAxisHorz);
				float DdD1 = dir.dot(mAxisVert);
				float DdDiff = dir.dot(diff);

				float t = s0 * DdD0 + s1 * DdD1 - DdDiff;

				return std::make_pair(true, t);
			}
		}

		return std::make_pair(false, 0.0f);
	}
}