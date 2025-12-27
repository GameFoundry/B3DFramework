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
	 * Represents a torus at the world center. Outer radius represents the distance from the center, and inner radius
	 * represents the radius of the tube. Inner radius must be less or equal than the outer radius.
	 */
	class B3D_EXPORT Torus
	{
	public:
		Torus() = default;

		Torus(const Vector3& normal, float outerRadius, float innerRadius)
			: Normal(normal), OuterRadius(outerRadius), InnerRadius(innerRadius)
		{}

		/** Ray/torus intersection, returns boolean result and distance to nearest intersection point. */
		std::pair<bool, float> Intersects(const Ray& ray) const;

		Vector3 Normal{ BsZero };
		float OuterRadius = 0.0f;
		float InnerRadius = 0.0f;
	};

	/** @} */
} // namespace b3d
