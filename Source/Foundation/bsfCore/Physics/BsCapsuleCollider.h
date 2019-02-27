//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	class PhysicsScene;

	/** @addtogroup Physics
	 *  @{
	 */

	/** Collider with a capsule geometry. */
	class BS_CORE_EXPORT CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider() = default;

		/** 
		 * Determines the half height of the capsule, from the origin to one of the hemispherical centers, along the normal 
		 * vector. 
		 */
		virtual void setHalfHeight(float halfHeight) = 0;

		/** @copydoc setHalfHeight() */
		virtual float getHalfHeight() const = 0;

		/** Determines the radius of the capsule. */
		virtual void setRadius(float radius) = 0;

		/** @copydoc setRadius() */
		virtual float getRadius() const = 0;

		/** 
		 * Creates a new capsule collider. 
		 *
		 * @param[in]	scene		Scene into which to add the collider to.
		 * @param[in]	radius		Radius of the capsule.
		 * @param[in]	halfHeight	Half height of the capsule, from the origin to one of the hemispherical centers, along
		 *							the normal vector.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		static SPtr<CapsuleCollider> create(PhysicsScene& scene, float radius = 0.0f, float halfHeight = 0.0f, 
			const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}