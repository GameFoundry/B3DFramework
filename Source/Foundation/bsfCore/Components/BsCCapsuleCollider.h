//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCapsuleCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	CapsuleCollider
	 *
	 * @note Wraps CapsuleCollider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:CapsuleCollider) CCapsuleCollider : public CCollider
	{
	public:
		CCapsuleCollider(const HSceneObject& parent, float radius = 1.0f, float halfHeight = 0.5f);

		/** Normal vector that determines how is the capsule oriented. */
		BS_SCRIPT_EXPORT(n:Normal,pr:setter)
		void SetNormal(const Vector3& normal);

		/** @copydoc setNormal() */
		BS_SCRIPT_EXPORT(n:Normal,pr:getter)
		Vector3 GetNormal() const { return mNormal; }

		/** Determines the position of the capsule shape, relative to the component's scene object. */
		BS_SCRIPT_EXPORT(n:Center,pr:setter)
		void SetCenter(const Vector3& center);

		/** @copydoc setCenter() */
		BS_SCRIPT_EXPORT(n:Center,pr:getter)
		Vector3 GetCenter() const { return mLocalPosition; }

		/** @copydoc CapsuleCollider::setHalfHeight() */
		BS_SCRIPT_EXPORT(n:HalfHeight,pr:setter)
		void SetHalfHeight(float halfHeight);

		/** @copydoc CapsuleCollider::getHalfHeight() */
		BS_SCRIPT_EXPORT(n:HalfHeight,pr:getter)
		float GetHalfHeight() const { return mHalfHeight; }

		/** @copydoc CapsuleCollider::setRadius() */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void SetRadius(float radius);

		/** @copydoc CapsuleCollider::getRadius() */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float GetRadius() const { return mRadius; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the capsule collider that this component wraps. */
		CapsuleCollider* _getInternal() const { return static_cast<CapsuleCollider*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> CreateInternal() override;

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
		CCapsuleCollider(); // Serialization only
	};

	 /** @} */
}
