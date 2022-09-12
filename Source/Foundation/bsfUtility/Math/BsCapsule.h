//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector3.h"
#include "Math/BsLineSegment3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a capsule with a line segment and a radius. */
	class BS_UTILITY_EXPORT Capsule
	{
	public:
		Capsule() = default;
		Capsule(const LineSegment3& segment, float radius);

		/**
		 * Ray/capsule intersection.
		 *
		 * @return	Boolean result and distance to the nearest intersection point.
		 */
		std::pair<bool, float> Intersects(const Ray& ray) const;

		/**
		 * Returns the line segment along which the capsule lies. All capsule points are at equal distance from this
		 * segment.
		 */
		const LineSegment3& GetSegment() const { return mSegment; }

		/** Returns the radius of the capsule. It defines the distance of the capsule from its line segment. */
		float GetRadius() const { return mRadius; }

		/**
		 * Returns the height of the capsule. The height is the distance between centers of the hemispheres that form the
		 * capsule's ends.
		 */
		float GetHeight() const { return mSegment.GetLength(); }

		/** Returns the center point of the capsule. */
		Vector3 GetCenter() const { return mSegment.GetCenter(); }

	private:
		LineSegment3 mSegment;
		float mRadius = 0.0f;
	};

	/** @} */
}
