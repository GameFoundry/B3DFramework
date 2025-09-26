//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"
#include "Math/B3DLineSegment3.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a capsule with a line segment and a radius. */
	template<typename T>
	struct B3D_EXPORT TCapsule
	{
		TCapsule() = default;
		TCapsule(const TLineSegment3<T>& segment, T radius);

		/**
		 * Ray/capsule intersection.
		 *
		 * @return	Boolean result and distance to the nearest intersection point.
		 */
		std::pair<bool, T> Intersects(const TRay<T>& ray) const;

		/**
		 * Returns the line segment along which the capsule lies. All capsule points are at equal distance from this
		 * segment.
		 */
		const TLineSegment3<T>& GetSegment() const { return mSegment; }

		/** Returns the radius of the capsule. It defines the distance of the capsule from its line segment. */
		T GetRadius() const { return mRadius; }

		/**
		 * Returns the height of the capsule. The height is the distance between centers of the hemispheres that form the
		 * capsule's ends.
		 */
		T GetHeight() const { return mSegment.GetLength(); }

		/** Returns the center point of the capsule. */
		TVector3<T> GetCenter() const { return mSegment.GetCenter(); }

	private:
		TLineSegment3<T> mSegment;
		T mRadius = (T)0.0;
	};

	/** @} */
} // namespace b3d
