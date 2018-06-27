//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsConvexVolume.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsPlane.h"
#include "Math/BsMath.h"
#include "Error/BsException.h"

namespace bs
{
	ConvexVolume::ConvexVolume(const Vector<Plane>& planes)
		:mPlanes(planes)
	{ }

	ConvexVolume::ConvexVolume(const Matrix4& projectionMatrix, bool useNearPlane)
	{
		mPlanes.reserve(6);

		const Matrix4& proj = projectionMatrix;

		// Left
		{
			Plane plane;
			plane.normal.x = proj[3][0] + proj[0][0];
			plane.normal.y = proj[3][1] + proj[0][1];
			plane.normal.z = proj[3][2] + proj[0][2];
			plane.d = proj[3][3] + proj[0][3];

			mPlanes.push_back(plane);
		}

		// Right
		{
			Plane plane;
			plane.normal.x = proj[3][0] - proj[0][0];
			plane.normal.y = proj[3][1] - proj[0][1];
			plane.normal.z = proj[3][2] - proj[0][2];
			plane.d = proj[3][3] - proj[0][3];

			mPlanes.push_back(plane);
		}

		// Top
		{
			Plane plane;
			plane.normal.x = proj[3][0] - proj[1][0];
			plane.normal.y = proj[3][1] - proj[1][1];
			plane.normal.z = proj[3][2] - proj[1][2];
			plane.d = proj[3][3] - proj[1][3];

			mPlanes.push_back(plane);
		}

		// Bottom
		{
			Plane plane;
			plane.normal.x = proj[3][0] + proj[1][0];
			plane.normal.y = proj[3][1] + proj[1][1];
			plane.normal.z = proj[3][2] + proj[1][2];
			plane.d = proj[3][3] + proj[1][3];

			mPlanes.push_back(plane);
		}

		// Far
		{
			Plane plane;
			plane.normal.x = proj[3][0] - proj[2][0];
			plane.normal.y = proj[3][1] - proj[2][1];
			plane.normal.z = proj[3][2] - proj[2][2];
			plane.d = proj[3][3] - proj[2][3];

			mPlanes.push_back(plane);
		}

		// Near
		if(useNearPlane)
		{
			Plane plane;
			plane.normal.x = proj[3][0] + proj[2][0];
			plane.normal.y = proj[3][1] + proj[2][1];
			plane.normal.z = proj[3][2] + proj[2][2];
			plane.d = proj[3][3] + proj[2][3];

			mPlanes.push_back(plane);
		}

		for (UINT32 i = 0; i < (UINT32)mPlanes.size(); i++)
		{
			float length = mPlanes[i].normal.normalize();
			mPlanes[i].d /= -length;
		}
	}

	bool ConvexVolume::intersects(const AABox& box) const
	{
		Vector3 center = box.getCenter();
		Vector3 extents = box.getHalfSize();
		Vector3 absExtents(Math::abs(extents.x), Math::abs(extents.y), Math::abs(extents.z));

		for (auto& plane : mPlanes)
		{
			float dist = center.dot(plane.normal) - plane.d;

			float effectiveRadius = absExtents.x * Math::abs(plane.normal.x);
			effectiveRadius += absExtents.y * Math::abs(plane.normal.y);
			effectiveRadius += absExtents.z * Math::abs(plane.normal.z);

			if (dist < -effectiveRadius)
				return false;
		}

		return true;
	}

	bool ConvexVolume::intersects(const Sphere& sphere) const
	{
		Vector3 center = sphere.getCenter();
		float radius = sphere.getRadius();

		for (auto& plane : mPlanes)
		{
			float dist = center.dot(plane.normal) - plane.d;

			if (dist < -radius)
				return false;
		}

		return true;
	}

	bool ConvexVolume::contains(const Vector3& p, float expand) const
	{
		for(auto& plane : mPlanes)
		{
			if (plane.getDistance(p) < -expand)
				return false;
		}

		return true;
	}

	const Plane& ConvexVolume::getPlane(FrustumPlane whichPlane) const
	{
		if(whichPlane >= mPlanes.size())
		{
			BS_EXCEPT(InvalidParametersException, "Requested plane does not exist in this volume.");
		}

		return mPlanes[whichPlane];
	}
}
