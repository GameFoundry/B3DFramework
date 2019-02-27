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

	/** Collider with box geometry. */
	class BS_CORE_EXPORT BoxCollider : public Collider
	{
	public:
		BoxCollider() = default;

		/** Determines the extents (half size) of the geometry of the box. */
		virtual void setExtents(const Vector3& extents) = 0;

		/** @copydoc setExtents() */
		virtual Vector3 getExtents() const = 0;

		/** 
		 * Creates a new box collider. 
		 *
		 * @param[in]	scene		Scene into which to add the collider to.
		 * @param[in]	extents		Extents (half size) of the box.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		static SPtr<BoxCollider> create(PhysicsScene& scene, const Vector3& extents = Vector3::ZERO, 
			const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}