//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCapsuleCollider.h"
#include "BsCCollider.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	CapsuleCollider
	 *
	 * Wraps CapsuleCollider as a Component.
	 */
    class BS_CORE_EXPORT CCapsuleCollider : public CCollider
    {
    public:
		CCapsuleCollider(const HSceneObject& parent, float radius = 1.0f, float halfHeight = 0.5f);

		/** Sets the normal vector of the capsule. It determines how is the capsule oriented. */
		void setNormal(const Vector3& normal);

		/** Gets the normal vector of the capsule. It determines how is the capsule oriented. */
		Vector3 getNormal() const { return mNormal; }

		/** Sets the position of the capsule shape, relative to the component's scene object. */
		void setCenter(const Vector3& center);

		/** Gets the position of the capsule shape, relative to the component's scene object. */
		Vector3 getCenter() const { return mLocalPosition; }

		/** @copydoc CapsuleCollider::setHalfHeight() */
		void setHalfHeight(float halfHeight);

		/** @copydoc CapsuleCollider::getHalfHeight() */
		float getHalfHeight() const { return mHalfHeight; }

		/** @copydoc CapsuleCollider::setRadius() */
		void setRadius(float radius);

		/** @copydoc CapsuleCollider::getRadius() */
		float getRadius() const { return mRadius; }

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the capsule collider that this component wraps. */
		CapsuleCollider* _getInternal() const { return static_cast<CapsuleCollider*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> createInternal() override;

    protected:
		Vector3 mNormal = Vector3::UNIT_Y;
		float mRadius = 1.0f;
		float mHalfHeight = 0.5f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCapsuleColliderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CCapsuleCollider() {} // Serialization only
     };

	 /** @} */
}