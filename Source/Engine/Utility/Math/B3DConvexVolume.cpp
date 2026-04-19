//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DConvexVolume.h"
#include "Math/B3DAABox.h"
#include "Math/B3DSphere.h"
#include "Math/B3DPlane.h"
#include "Math/B3DMath.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

template<typename T>
TConvexVolume<T>::TConvexVolume(const Vector<TPlane<T>>& planes)
	: mPlanes(planes)
{}

template<typename T>
TConvexVolume<T>::TConvexVolume(const TMatrix4<T>& projectionMatrix, bool useNearPlane)
{
	mPlanes.reserve(6);

	const TMatrix4<T>& proj = projectionMatrix;

	// Left
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] + proj[0][0];
		plane.Normal.Y = proj[3][1] + proj[0][1];
		plane.Normal.Z = proj[3][2] + proj[0][2];
		plane.D = proj[3][3] + proj[0][3];

		mPlanes.push_back(plane);
	}

	// Right
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] - proj[0][0];
		plane.Normal.Y = proj[3][1] - proj[0][1];
		plane.Normal.Z = proj[3][2] - proj[0][2];
		plane.D = proj[3][3] - proj[0][3];

		mPlanes.push_back(plane);
	}

	// Top
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] - proj[1][0];
		plane.Normal.Y = proj[3][1] - proj[1][1];
		plane.Normal.Z = proj[3][2] - proj[1][2];
		plane.D = proj[3][3] - proj[1][3];

		mPlanes.push_back(plane);
	}

	// Bottom
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] + proj[1][0];
		plane.Normal.Y = proj[3][1] + proj[1][1];
		plane.Normal.Z = proj[3][2] + proj[1][2];
		plane.D = proj[3][3] + proj[1][3];

		mPlanes.push_back(plane);
	}

	// Far
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] - proj[2][0];
		plane.Normal.Y = proj[3][1] - proj[2][1];
		plane.Normal.Z = proj[3][2] - proj[2][2];
		plane.D = proj[3][3] - proj[2][3];

		mPlanes.push_back(plane);
	}

	// Near
	if(useNearPlane)
	{
		TPlane<T> plane;
		plane.Normal.X = proj[3][0] + proj[2][0];
		plane.Normal.Y = proj[3][1] + proj[2][1];
		plane.Normal.Z = proj[3][2] + proj[2][2];
		plane.D = proj[3][3] + proj[2][3];

		mPlanes.push_back(plane);
	}

	for(u32 planeIndex = 0; planeIndex < (u32)mPlanes.size(); planeIndex++)
	{
		T length = mPlanes[planeIndex].Normal.Normalize();
		mPlanes[planeIndex].D /= -length;
	}
}

template<typename T>
bool TConvexVolume<T>::Intersects(const TAABox<T>& box) const
{
	TVector3<T> center = box.GetCenter();
	TVector3<T> extents = box.GetExtents();
	TVector3<T> absExtents(Math::Abs(extents.X), Math::Abs(extents.Y), Math::Abs(extents.Z));

	for(auto& plane : mPlanes)
	{
		T dist = center.Dot(plane.Normal) - plane.D;

		T effectiveRadius = absExtents.X * abs(plane.Normal.X);
		effectiveRadius += absExtents.Y * abs(plane.Normal.Y);
		effectiveRadius += absExtents.Z * abs(plane.Normal.Z);

		if(dist < -effectiveRadius)
			return false;
	}

	return true;
}

template<typename T>
bool TConvexVolume<T>::Intersects(const TSphere<T>& sphere) const
{
	TVector3<T> center = sphere.Center;
	T radius = sphere.Radius;

	for(auto& plane : mPlanes)
	{
		T dist = center.Dot(plane.Normal) - plane.D;

		if(dist < -radius)
			return false;
	}

	return true;
}

template<typename T>
bool TConvexVolume<T>::Contains(const TVector3<T>& p, T expand) const
{
	for(auto& plane : mPlanes)
	{
		if(plane.GetDistance(p) < -expand)
			return false;
	}

	return true;
}

template<typename T>
const TPlane<T>& TConvexVolume<T>::GetPlane(FrustumPlane whichPlane) const
{
	if(!B3D_ENSURE_LOG(whichPlane < mPlanes.size(), "Requested plane does not exist in this volume."))
	{
		static TPlane<T> kDefault;
		return kDefault;
	}

	return mPlanes[whichPlane];
}

template struct B3D_EXPORT TConvexVolume<float>;
template struct B3D_EXPORT TConvexVolume<double>;
