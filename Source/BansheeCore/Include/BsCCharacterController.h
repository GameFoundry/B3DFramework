//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCharacterController.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	CharacterController
	 *
	 * Wraps CharacterController as a Component.
	 */
    class BS_CORE_EXPORT CCharacterController : public Component
    {
    public:
		CCharacterController(const HSceneObject& parent);

		/** @copydoc CharacterController::move */
		inline CharacterCollisionFlags move(const Vector3& displacement);

		/** @copydoc CharacterController::getFootPosition */
		inline Vector3 getFootPosition() const;

		/** @copydoc CharacterController::setFootPosition */
		inline void setFootPosition(const Vector3& position);

		/** @copydoc CharacterController::getRadius */
		float getRadius() const { return mDesc.radius; }

		/** @copydoc CharacterController::setRadius */
		inline void setRadius(float radius);

		/** @copydoc CharacterController::getHeight */
		float getHeight() const { return mDesc.height; }

		/** @copydoc CharacterController::setHeight */
		inline void setHeight(float height);

		/** @copydoc CharacterController::getUp */
		Vector3 getUp() const { return mDesc.up; }

		/** @copydoc CharacterController::setUp */
		inline void setUp(const Vector3& up);

		/** @copydoc CharacterController::getClimbingMode */
		CharacterClimbingMode getClimbingMode() const { return mDesc.climbingMode; }

		/** @copydoc CharacterController::setClimbingMode */
		inline void setClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::getNonWalkableMode */
		CharacterNonWalkableMode getNonWalkableMode() const { return mDesc.nonWalkableMode; }

		/** @copydoc CharacterController::setNonWalkableMode */
		inline void setNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::getMinMoveDistance */
		float getMinMoveDistance() const { return mDesc.minMoveDistance; }

		/** @copydoc CharacterController::setMinMoveDistance */
		inline void setMinMoveDistance(float value);

		/** @copydoc CharacterController::getContactOffset */
		float getContactOffset() const { return mDesc.contactOffset; }

		/** @copydoc CharacterController::setContactOffset */
		inline void setContactOffset(float value);

		/** @copydoc CharacterController::getStepOffset */
		inline float getStepOffset() const { return mDesc.stepOffset; }

		/** @copydoc CharacterController::setStepOffset */
		inline void setStepOffset(float value);

		/** @copydoc CharacterController::getSlopeLimit */
		Radian getSlopeLimit() const { return mDesc.slopeLimit; }

		/** @copydoc CharacterController::setSlopeLimit */
		inline void setSlopeLimit(Radian value);

		/** @copydoc CharacterController::getLayer */
		UINT64 getLayer() const { return mLayer; }

		/** @copydoc CharacterController::setLayer */
		inline void setLayer(UINT64 layer);

		/** @copydoc CharacterController::onColliderHit */
		Event<void(const ControllerColliderCollision&)> onColliderHit;

		/** @copydoc CharacterController::onControllerHit */
		Event<void(const ControllerControllerCollision&)> onControllerHit;

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the character controller that this component wraps. */
		CharacterController* _getInternal() const { return static_cast<CharacterController*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		using Component::destroyInternal;

		/** @copydoc Component::onInitialized() */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;

		/** Updates the position by copying it from the controller to the component's scene object. */
		void updatePositionFromController();

		/** Updates the dimensions of the controller by taking account scale of the parent scene object. */
		void updateDimensions();

		/** Destroys the internal character controller representation. */
		void destroyInternal();

		/** Triggered when the internal controller hits a collider. */
		void triggerOnColliderHit(const ControllerColliderCollision& value);

		/** Triggered when the internal controller hits another controller. */
		void triggerOnControllerHit(const ControllerControllerCollision& value);

		SPtr<CharacterController> mInternal;
		CHAR_CONTROLLER_DESC mDesc;
		UINT64 mLayer = 1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCharacterControllerRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CCharacterController() {} // Serialization only
     };

	 /** @} */
}