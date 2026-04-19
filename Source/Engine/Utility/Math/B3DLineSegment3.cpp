//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DLineSegment3.h"
#include "Math/B3DRay.h"

using namespace b3d;

template<typename T>
TLineSegment3<T>::TLineSegment3(const TVector3<T>& start, const TVector3<T>& end)
	: Start(start), End(end)
{
}

template<typename T>
std::pair<std::array<TVector3<T>, 2>, T> TLineSegment3<T>::GetNearestPoint(const TRay<T>& ray) const
{
	const TVector3<T>& org = ray.Origin;
	const TVector3<T>& dir = ray.Direction;

	TVector3<T> segDir = End - Start;
	T segExtent = segDir.Normalize() * (T)0.5;
	TVector3<T> segCenter = Start + segDir * segExtent;

	TVector3<T> diff = org - segCenter;
	T a01 = -dir.Dot(segDir);
	T b0 = diff.Dot(dir);
	T c = diff.Dot(diff);
	T det = fabs((T)1.0 - a01 * a01);

	T s0, s1;
	T sqrDistance;
	if(det > (T)0.0) // Not parallel
	{

		T b1 = -diff.Dot(segDir);
		s1 = a01 * b0 - b1;
		T extDet = segExtent * det;

		if(s1 >= -extDet)
		{
			if(s1 <= extDet) // Interior of the segment and interior of the ray are closest
			{
				T invDet = (T)1.0 / det;
				s0 = (a01 * b1 - b0) * invDet;
				s1 *= invDet;

				sqrDistance = s0 * (s0 + a01 * s1 + (T)2.0 * b0) +
					s1 * (a01 * s0 + s1 + (T)2.0 * b1) + c;
			}
			else // Segment end and interior of the ray are closest
			{
				s1 = segExtent;
				s0 = -(a01 * s1 + b0);
				sqrDistance = -s0 * s0 + s1 * (s1 + (T)(2.0) * b1) + c;
			}
		}
		else // Segment start and interior of the ray are closest
		{
			s1 = -segExtent;
			s0 = -(a01 * s1 + b0);
			sqrDistance = -s0 * s0 + s1 * (s1 + (T)(2.0) * b1) + c;
		}
	}
	else // Parallel
	{
		s1 = (T)0.0;
		s0 = -b0;
		sqrDistance = b0 * s0 + c;
	}

	if(sqrDistance < (T)0.0)
		sqrDistance = (T)0.0;

	T distance = std::sqrt(sqrDistance);

	std::array<TVector3<T>, 2> nearestPoints;
	nearestPoints[0] = org + s0 * dir;
	nearestPoints[1] = segCenter + s1 * segDir;

	return std::make_pair(nearestPoints, distance);
}

template struct B3D_EXPORT TLineSegment3<float>;
template struct B3D_EXPORT TLineSegment3<double>;
