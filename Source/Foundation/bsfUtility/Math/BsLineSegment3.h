//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a line segment in three dimensional space defined by a start and an end point. */
	class B3D_UTILITY_EXPORT LineSegment3
	{
	public:
		LineSegment3() = default;
		LineSegment3(const Vector3& start, const Vector3& end);

		/**
		 * Find the nearest point on the line segment and the provided ray.
		 *
		 * @return	Set of nearest points and distance from the points. First nearest point is a point along the ray,
		 *			while the second is along the line segment.
		 *
		 * @note	If segment and ray are parallel the set of points at the segment origin are returned.
		 */
		std::pair<std::array<Vector3, 2>, float> GetNearestPoint(const Ray& ray) const;

		/** Returns the length of the line segment. */
		float GetLength() const { return Start.Distance(End); }

		/** Returns the center point along the line segment. */
		Vector3 GetCenter() const { return Start + (End - Start) * 0.5f; }

		Vector3 Start = BsZero;
		Vector3 End = BsZero;
	};

	/** @} */
} // namespace bs
