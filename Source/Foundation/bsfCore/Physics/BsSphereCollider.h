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

	/** A collider with sphere geometry. */
	class BS_CORE_EXPORT SphereCollider : public Collider
	{
	public:
		SphereCollider() = default;

		/** Determines the radius of the sphere geometry. */
		virtual void setRadius(float radius) = 0;

		/** @copydoc setRadius */
		virtual float getRadius() const = 0;

		/**
		 * Creates a new sphere collider.
		 * 
		 * @param[in]	scene		Scene into which to add the collider to.
		 * @param[in]	radius		Radius of the sphere geometry.
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		static SPtr<SphereCollider> create(PhysicsScene& scene, float radius = 0.0f, 
			const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}