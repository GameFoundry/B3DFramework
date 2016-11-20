//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsSphereCollider.h"
#include "BsCCollider.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphereCollider
	 *
	 * Wraps SphereCollider as a Component.
	 */
    class BS_CORE_EXPORT CSphereCollider : public CCollider
    {
    public:
		CSphereCollider(const HSceneObject& parent, float radius = 1.0f);

		/** @copydoc SphereCollider::setRadius */
		void setRadius(float radius);

		/** @copydoc SphereCollider::getRadius */
		float getRadius() const { return mRadius; }

		/** Sets the position of the sphere shape, relative to the component's scene object. */
		void setCenter(const Vector3& center);

		/** Gets the position of the sphere shape, relative to the component's scene object. */
		Vector3 getCenter() const { return mLocalPosition; }

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the sphere collider that this component wraps. */
		SphereCollider* _getInternal() const { return static_cast<SphereCollider*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> createInternal() override;

    protected:
		float mRadius;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphereColliderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSphereCollider() {} // Serialization only
     };

	 /** @} */
}