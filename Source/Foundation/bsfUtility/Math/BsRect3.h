//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Represents a rectangle in three dimensional space. It is represented by two axes that extend from the specified
	 * origin. Axes should be perpendicular to each other and they extend in both positive and negative directions from the
	 * origin by the amount specified by extents.
	 */
	class BS_UTILITY_EXPORT Rect3
	{
	public:
		Rect3() = default;

		Rect3(const Vector3& center, const std::array<Vector3, 2>& axes,
			const std::array<float, 2>& extents)
		:mCenter(center), mAxisHorz(axes[0]), mAxisVert(axes[1]),
		mExtentHorz(extents[0]), mExtentVert(extents[1])
		{ }

		/**
		 * Find the nearest points of the provided ray and the rectangle.
		 *
		 * @return	A set of nearest points and nearest distance. First value in the set corresponds to nearest point on
		 *			the ray, and the second to the nearest point on the rectangle. They are same in the case of intersection.
		 *			When ray is parallel to the rectangle there are two sets of nearest points but only one the set nearest
		 *			to the ray origin is returned.
		 */
		std::pair<std::array<Vector3, 2>, float> GetNearestPoint(const Ray& ray) const;

		/**
		 * Find the nearest point on the rectangle to the provided point.
		 *
		 * @return	Nearest point and distance to nearest point.
		 */
		std::pair<Vector3, float> GetNearestPoint(const Vector3& point) const;

		/**
		 * Ray/rectangle intersection.
		 *
		 * @return	Boolean result and distance to intersection point.
		 */
		std::pair<bool, float> Intersects(const Ray& ray) const;

		/** Gets the origin of the rectangle. */
		const Vector3& GetCenter() const { return mCenter; }

		/** Returns the rectangle's horizontal axis. */
		const Vector3& GetHorizontalAxis() const { return mAxisHorz; }

		/** Returns the rectangle's vertical axis. */
		const Vector3& GetVerticalAxis() const { return mAxisVert; }

		/** Gets the extent of the rectangle along its horizontal axis. */
		const float& GetHorizontalExtent() const { return mExtentHorz; }

		/** Gets the extent of the rectangle along its vertical axis. */
		const float& GetVerticalExtent() const { return mExtentVert; }

	private:
		Vector3 mCenter{BsZero};
		Vector3 mAxisHorz{BsZero};
		Vector3 mAxisVert{BsZero};
		float mExtentHorz = 0.0f;
		float mExtentVert = 0.0f;
	};

	/** @} */
}
