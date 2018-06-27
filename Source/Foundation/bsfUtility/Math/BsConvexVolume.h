//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsPlane.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/**	Clip planes that form the camera frustum (visible area). */
	enum FrustumPlane
	{
		FRUSTUM_PLANE_LEFT = 0,
		FRUSTUM_PLANE_RIGHT = 1,
		FRUSTUM_PLANE_TOP = 2,
		FRUSTUM_PLANE_BOTTOM = 3,
		FRUSTUM_PLANE_FAR = 4,
		FRUSTUM_PLANE_NEAR = 5
	};

	/** Represents a convex volume defined by planes representing the volume border. */
	class BS_UTILITY_EXPORT ConvexVolume
	{
	public:
		ConvexVolume() = default;
		ConvexVolume(const Vector<Plane>& planes);

		/** Creates frustum planes from the provided projection matrix. */
		ConvexVolume(const Matrix4& projectionMatrix, bool useNearPlane = true);

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

		/**
		 * Checks if the convex volume contains the provided point.
		 * 
		 * @param[in]	p		Point to check.
		 * @param[in]	expand	Optional value to expand the size of the convex volume by the specified value during the
		 *						check. Negative values shrink the volume.
		 */
		bool contains(const Vector3& p, float expand = 0.0f) const;

		/** Returns the internal set of planes that represent the volume. */
		Vector<Plane> getPlanes() const { return mPlanes; }

		/** Returns the specified plane that represents the volume. */
		const Plane& getPlane(FrustumPlane whichPlane) const;

	private:
		Vector<Plane> mPlanes;
	};

	/** @} */
}
