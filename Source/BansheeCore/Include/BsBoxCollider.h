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

	/** Collider with box geometry. */
	class BS_CORE_EXPORT BoxCollider : public Collider
	{
	public:
		BoxCollider();

		/** Sets the extents (half size) of the geometry of the box. */
		virtual void setExtents(const Vector3& extents) = 0;

		/** Gets the extents (half size) of the geometry of the box. */
		virtual Vector3 getExtents() const = 0;

		/** 
		 * Creates a new box collider. 
		 *
		 * @param[in]	extents		Extents (half size) of the box.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		static SPtr<BoxCollider> create(const Vector3& extents = Vector3::ZERO, 
			const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
	};

	/** @} */
}