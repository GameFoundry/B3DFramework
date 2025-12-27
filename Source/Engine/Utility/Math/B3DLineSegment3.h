//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a line segment in three dimensional space defined by a start and an end point. */
	template<typename T>
	struct B3D_EXPORT TLineSegment3
	{
		TLineSegment3() = default;
		TLineSegment3(const TVector3<T>& start, const TVector3<T>& end);

		/**
		 * Find the nearest point on the line segment and the provided ray.
		 *
		 * @return	Set of nearest points and distance from the points. First nearest point is a point along the ray,
		 *			while the second is along the line segment.
		 *
		 * @note	If segment and ray are parallel the set of points at the segment origin are returned.
		 */
		std::pair<std::array<TVector3<T>, 2>, T> GetNearestPoint(const TRay<T>& ray) const;

		/** Returns the length of the line segment. */
		T GetLength() const { return Start.Distance(End); }

		/** Returns the center point along the line segment. */
		TVector3<T> GetCenter() const { return Start + (End - Start) * (T)0.5; }

		TVector3<T> Start = BsZero;
		TVector3<T> End = BsZero;
	};

	/** @} */
} // namespace b3d
