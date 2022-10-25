//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsConvexVolume.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsPlane.h"
#include "Math/BsMath.h"
#include "Error/BsException.h"

using namespace bs;

ConvexVolume::ConvexVolume(const Vector<Plane>& planes)
	: mPlanes(planes)
{}

ConvexVolume::ConvexVolume(const Matrix4& projectionMatrix, bool useNearPlane)
{
	mPlanes.reserve(6);

	const Matrix4& proj = projectionMatrix;

	// Left
	{
		Plane plane;
		plane.Normal.X = proj[3][0] + proj[0][0];
		plane.Normal.Y = proj[3][1] + proj[0][1];
		plane.Normal.Z = proj[3][2] + proj[0][2];
		plane.D = proj[3][3] + proj[0][3];

		mPlanes.push_back(plane);
	}

	// Right
	{
		Plane plane;
		plane.Normal.X = proj[3][0] - proj[0][0];
		plane.Normal.Y = proj[3][1] - proj[0][1];
		plane.Normal.Z = proj[3][2] - proj[0][2];
		plane.D = proj[3][3] - proj[0][3];

		mPlanes.push_back(plane);
	}

	// Top
	{
		Plane plane;
		plane.Normal.X = proj[3][0] - proj[1][0];
		plane.Normal.Y = proj[3][1] - proj[1][1];
		plane.Normal.Z = proj[3][2] - proj[1][2];
		plane.D = proj[3][3] - proj[1][3];

		mPlanes.push_back(plane);
	}

	// Bottom
	{
		Plane plane;
		plane.Normal.X = proj[3][0] + proj[1][0];
		plane.Normal.Y = proj[3][1] + proj[1][1];
		plane.Normal.Z = proj[3][2] + proj[1][2];
		plane.D = proj[3][3] + proj[1][3];

		mPlanes.push_back(plane);
	}

	// Far
	{
		Plane plane;
		plane.Normal.X = proj[3][0] - proj[2][0];
		plane.Normal.Y = proj[3][1] - proj[2][1];
		plane.Normal.Z = proj[3][2] - proj[2][2];
		plane.D = proj[3][3] - proj[2][3];

		mPlanes.push_back(plane);
	}

	// Near
	if(useNearPlane)
	{
		Plane plane;
		plane.Normal.X = proj[3][0] + proj[2][0];
		plane.Normal.Y = proj[3][1] + proj[2][1];
		plane.Normal.Z = proj[3][2] + proj[2][2];
		plane.D = proj[3][3] + proj[2][3];

		mPlanes.push_back(plane);
	}

	for(u32 i = 0; i < (u32)mPlanes.size(); i++)
	{
		float length = mPlanes[i].Normal.Normalize();
		mPlanes[i].D /= -length;
	}
}

bool ConvexVolume::Intersects(const AABox& box) const
{
	Vector3 center = box.GetCenter();
	Vector3 extents = box.GetHalfSize();
	Vector3 absExtents(Math::Abs(extents.X), Math::Abs(extents.Y), Math::Abs(extents.Z));

	for(auto& plane : mPlanes)
	{
		float dist = center.Dot(plane.Normal) - plane.D;

		float effectiveRadius = absExtents.X * abs(plane.Normal.X);
		effectiveRadius += absExtents.Y * abs(plane.Normal.Y);
		effectiveRadius += absExtents.Z * abs(plane.Normal.Z);

		if(dist < -effectiveRadius)
			return false;
	}

	return true;
}

bool ConvexVolume::Intersects(const Sphere& sphere) const
{
	Vector3 center = sphere.GetCenter();
	float radius = sphere.GetRadius();

	for(auto& plane : mPlanes)
	{
		float dist = center.Dot(plane.Normal) - plane.D;

		if(dist < -radius)
			return false;
	}

	return true;
}

bool ConvexVolume::Contains(const Vector3& p, float expand) const
{
	for(auto& plane : mPlanes)
	{
		if(plane.GetDistance(p) < -expand)
			return false;
	}

	return true;
}

const Plane& ConvexVolume::GetPlane(FrustumPlane whichPlane) const
{
	if(whichPlane >= mPlanes.size())
	{
		BS_EXCEPT(InvalidParametersException, "Requested plane does not exist in this volume.");
	}

	return mPlanes[whichPlane];
}
