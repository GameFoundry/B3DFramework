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

	/** A collider with plane geometry. Plane colliders cannot be a part of non-kinematic rigidbodies. */
	class BS_CORE_EXPORT PlaneCollider : public Collider
	{
	public:
		PlaneCollider();

		/** 
		 * Creates a new plane collider. 
		 *
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		static SPtr<PlaneCollider> create(const Vector3& position = Vector3::ZERO, 
			const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}