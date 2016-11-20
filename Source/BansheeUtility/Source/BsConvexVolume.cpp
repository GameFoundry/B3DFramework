//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsConvexVolume.h"
#include "BsAABox.h"
#include "BsSphere.h"
#include "BsPlane.h"
#include "BsMath.h"

namespace bs
{
	ConvexVolume::ConvexVolume(const Vector<Plane>& planes)
		:mPlanes(planes)
	{ }

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
}