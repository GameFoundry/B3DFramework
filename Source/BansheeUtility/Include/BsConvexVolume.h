//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsPlane.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a convex volume defined by planes representing the volume border. */
	class BS_UTILITY_EXPORT ConvexVolume
	{
	public:
		ConvexVolume() {}
		ConvexVolume(const Vector<Plane>& planes);

		/**
		 * Checks does the volume intersects the provided axis aligned box.
		 * This will return true if the box is fully inside the volume.
		 */
		bool intersects(const AABox& box) const;

		/**
		 * Checks does the volume intersects the provided sphere.
		 * This will return true if the sphere is fully inside the volume.
		 */
		bool intersects(const Sphere& sphere) const;

		/** Returns the internal set of planes that represent the volume. */
		Vector<Plane> getPlanes() const { return mPlanes; }

	private:
		Vector<Plane> mPlanes;
	};

	/** @} */
}