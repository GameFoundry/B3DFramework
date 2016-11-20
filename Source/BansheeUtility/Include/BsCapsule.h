//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector3.h"
#include "BsLineSegment3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a capsule with a line segment and a radius. */
	class BS_UTILITY_EXPORT Capsule
	{
	public:
		Capsule();
		Capsule(const LineSegment3& segment, float radius);

		/**
		 * Ray/capsule intersection.
		 *
		 * @return	Boolean result and distance to the nearest intersection point.
		 */
		std::pair<bool, float> intersects(const Ray& ray) const;

		/**
		 * Returns the line segment along which the capsule lies. All capsule points are at equal distance from this 
		 * segment.
		 */
		const LineSegment3& getSegment() const { return mSegment; }

		/** Returns the radius of the capsule. It defines the distance of the capsule from its line segment. */
		float getRadius() const { return mRadius; }

		/** 
		 * Returns the height of the capsule. The height is the distance between centers of the hemispheres that form the
		 * capsule's ends.
		 */
		float getHeight() const { return mSegment.getLength(); }

		/** Returns the center point of the capsule. */
		Vector3 getCenter() const { return mSegment.getCenter(); }

	private:
		LineSegment3 mSegment;
		float mRadius;
	};

	/** @} */
}