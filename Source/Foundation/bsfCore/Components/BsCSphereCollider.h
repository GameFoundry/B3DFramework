//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsSphereCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphereCollider
	 *
	 * @note Wraps SphereCollider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:SphereCollider) CSphereCollider : public CCollider
	{
	public:
		CSphereCollider(const HSceneObject& parent, float radius = 1.0f);

		/** @copydoc SphereCollider::setRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void SetRadius(float radius);

		/** @copydoc SphereCollider::getRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float GetRadius() const { return mRadius; }

		/** Determines position of the sphere shape, relative to the component's scene object. */
		BS_SCRIPT_EXPORT(n:Center,pr:setter)
		void SetCenter(const Vector3& center);

		/** @copydoc setCenter() */
		BS_SCRIPT_EXPORT(n:Center,pr:getter)
		Vector3 GetCenter() const { return mLocalPosition; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the sphere collider that this component wraps. */
		SphereCollider* _getInternal() const { return static_cast<SphereCollider*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> CreateInternal() override;

	protected:
		float mRadius = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphereColliderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSphereCollider(); // Serialization only
	};

	 /** @} */
}
