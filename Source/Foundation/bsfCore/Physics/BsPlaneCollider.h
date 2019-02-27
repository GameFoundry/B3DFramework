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

	/** A collider with plane geometry. Plane colliders cannot be a part of non-kinematic rigidbodies. */
	class BS_CORE_EXPORT PlaneCollider : public Collider
	{
	public:
		PlaneCollider() = default;

		/** 
		 * Creates a new plane collider. 
		 *
		 * @param[in]	scene		Scene into which to add the collider to.
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		static SPtr<PlaneCollider> create(PhysicsScene& scene, const Vector3& position = Vector3::ZERO, 
			const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}