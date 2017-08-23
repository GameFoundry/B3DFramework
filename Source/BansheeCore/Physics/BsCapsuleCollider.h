//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

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
		 * Determines the half height of the capsule, from the origin to one of the hemispherical centers, along the normal 
		 * vector. 
		 */
		virtual void setHalfHeight(float halfHeight) = 0;

		/** @copdyoc setHalfHeight() */
		virtual float getHalfHeight() const = 0;

		/** Determines the radius of the capsule. */
		virtual void setRadius(float radius) = 0;

		/** @copydoc setRadius() .*/
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