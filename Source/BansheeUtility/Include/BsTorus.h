//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Represents a torus at the world center. Outer radius represents the distance from the center, and inner radius 
	 * represents the radius of the tube. Inner radius must be less or equal than the outer radius.
	 */
	class BS_UTILITY_EXPORT Torus
	{
	public:
		Torus();
		Torus(const Vector3& normal, float outerRadius, float innerRadius);

		/** Ray/torus intersection, returns boolean result and distance to nearest intersection point. */
		std::pair<bool, float> intersects(const Ray& ray) const;

		Vector3 normal;
		float outerRadius;
		float innerRadius;
	};

	/** @} */
}