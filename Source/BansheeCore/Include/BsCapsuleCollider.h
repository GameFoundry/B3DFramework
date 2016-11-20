//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCollider.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	 /** Collider with a capsule geometry. */
	class BS_CORE_EXPORT CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider();

		/** 
		 * Sets the half height of the capsule, from the origin to one of the hemispherical centers, along the normal 
		 * vector. 
		 */
		virtual void setHalfHeight(float halfHeight) = 0;

		/** 
		 * Gets the half height of the capsule, from the origin to one of the hemispherical centers, along the normal
		 * vector. 
		 */
		virtual float getHalfHeight() const = 0;

		/** Sets the radius of the capsule. */
		virtual void setRadius(float radius) = 0;

		/** Gets the radius of the capsule. */
		virtual float getRadius() const = 0;

		/** 
		 * Creates a new capsule collider. 
		 *
		 * @param[in]	radius		Radius of the capsule.
		 * @param[in]	halfHeight	Half height of the capsule, from the origin to one of the hemispherical centers, along
		 *							the normal vector.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		static SPtr<CapsuleCollider> create(float radius = 0.0f, float halfHeight = 0.0f, 
			const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}