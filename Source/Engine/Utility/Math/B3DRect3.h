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

	/**
	 * Represents a rectangle in three dimensional space. It is represented by two axes that extend from the specified
	 * origin. Axes should be perpendicular to each other and they extend in both positive and negative directions from the
	 * origin by the amount specified by extents.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true)) Rect3
	{
	public:
		Rect3() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		Rect3(const Vector3& center, const std::array<Vector3, 2>& axes, const std::array<float, 2>& extents)
			: Center(center), HorizontalAxis(axes[0]), VerticalAxis(axes[1]), HorizontalExtent(extents[0]), VerticalExtent(extents[1])
		{}

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

		Vector3 Center = Vector3::kZero;
		Vector3 HorizontalAxis = Vector3::kZero;
		Vector3 VerticalAxis = Vector3::kZero;
		float HorizontalExtent = 0.0f;
		float VerticalExtent = 0.0f;
	};

	/** @} */
} // namespace b3d
