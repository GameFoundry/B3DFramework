//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DPlane.h"

namespace b3d
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
	template<typename T>
	struct B3D_EXPORT TConvexVolume
	{
	public:
		TConvexVolume() = default;
		TConvexVolume(const Vector<TPlane<T>>& planes);

		/** Creates frustum planes from the provided projection matrix. */
		TConvexVolume(const TMatrix4<T>& projectionMatrix, bool useNearPlane = true);

		/**
		 * Checks does the volume intersects the provided axis aligned box.
		 * This will return true if the box is fully inside the volume.
		 */
		bool Intersects(const TAABox<T>& box) const;

		/**
		 * Checks does the volume intersects the provided sphere.
		 * This will return true if the sphere is fully inside the volume.
		 */
		bool Intersects(const TSphere<T>& sphere) const;

		/**
		 * Checks if the convex volume contains the provided point.
		 *
		 * @param	p		Point to check.
		 * @param	expand	Optional value to expand the size of the convex volume by the specified value during the
		 *					check. Negative values shrink the volume.
		 */
		bool Contains(const TVector3<T>& p, T expand = (T)0.0) const;

		/** Returns the internal set of planes that represent the volume. */
		Vector<TPlane<T>> GetPlanes() const { return mPlanes; }

		/** Returns the specified plane that represents the volume. */
		const TPlane<T>& GetPlane(FrustumPlane whichPlane) const;

	private:
		Vector<TPlane<T>> mPlanes;
	};

	/** @} */
} // namespace b3d
