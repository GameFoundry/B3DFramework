//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCharacterController.h"
#include "Scene/BsComponent.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	CharacterController
	 *
	 * @note	Wraps CharacterController as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:CharacterController) CCharacterController : public Component
	{
	public:
		CCharacterController(const HSceneObject& parent);

		/** @copydoc CharacterController::move */
		BS_SCRIPT_EXPORT(n:Move)
		CharacterCollisionFlags move(const Vector3& displacement);

		/** @copydoc CharacterController::getFootPosition */
		BS_SCRIPT_EXPORT(n:FootPosition,pr:getter)
		Vector3 getFootPosition() const;

		/** @copydoc CharacterController::setFootPosition */
		BS_SCRIPT_EXPORT(n:FootPosition,pr:setter)
		void setFootPosition(const Vector3& position);

		/** @copydoc CharacterController::getRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float getRadius() const { return mDesc.radius; }

		/** @copydoc CharacterController::setRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void setRadius(float radius);

		/** @copydoc CharacterController::getHeight */
		BS_SCRIPT_EXPORT(n:Height,pr:getter)
		float getHeight() const { return mDesc.height; }

		/** @copydoc CharacterController::setHeight */
		BS_SCRIPT_EXPORT(n:Height,pr:setter)
		void setHeight(float height);

		/** @copydoc CharacterController::getUp */
		BS_SCRIPT_EXPORT(n:Up,pr:getter)
		Vector3 getUp() const { return mDesc.up; }

		/** @copydoc CharacterController::setUp */
		BS_SCRIPT_EXPORT(n:Up,pr:setter)
		void setUp(const Vector3& up);

		/** @copydoc CharacterController::getClimbingMode */
		BS_SCRIPT_EXPORT(n:ClimbingMode,pr:getter)
		CharacterClimbingMode getClimbingMode() const { return mDesc.climbingMode; }

		/** @copydoc CharacterController::setClimbingMode */
		BS_SCRIPT_EXPORT(n:ClimbingMode,pr:setter)
		void setClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::getNonWalkableMode */
		BS_SCRIPT_EXPORT(n:NonWalkableMode,pr:getter)
		CharacterNonWalkableMode getNonWalkableMode() const { return mDesc.nonWalkableMode; }

		/** @copydoc CharacterController::setNonWalkableMode */
		BS_SCRIPT_EXPORT(n:NonWalkableMode,pr:setter)
		void setNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::getMinMoveDistance */
		BS_SCRIPT_EXPORT(n:MinMoveDistance,pr:getter)
		float getMinMoveDistance() const { return mDesc.minMoveDistance; }

		/** @copydoc CharacterController::setMinMoveDistance */
		BS_SCRIPT_EXPORT(n:MinMoveDistance,pr:setter)
		void setMinMoveDistance(float value);

		/** @copydoc CharacterController::getContactOffset */
		BS_SCRIPT_EXPORT(n:ContactOffset,pr:getter)
		float getContactOffset() const { return mDesc.contactOffset; }

		/** @copydoc CharacterController::setContactOffset */
		BS_SCRIPT_EXPORT(n:ContactOffset,pr:setter)
		void setContactOffset(float value);

		/** @copydoc CharacterController::getStepOffset */
		BS_SCRIPT_EXPORT(n:StepOffset,pr:getter)
		float getStepOffset() const { return mDesc.stepOffset; }

		/** @copydoc CharacterController::setStepOffset */
		BS_SCRIPT_EXPORT(n:StepOffset,pr:setter)
		void setStepOffset(float value);

		/** @copydoc CharacterController::getSlopeLimit */
		BS_SCRIPT_EXPORT(n:SlopeLimit,pr:getter)
		Radian getSlopeLimit() const { return mDesc.slopeLimit; }

		/** @copydoc CharacterController::setSlopeLimit */
		BS_SCRIPT_EXPORT(n:SlopeLimit,pr:setter)
		void setSlopeLimit(Radian value);

		/** @copydoc CharacterController::getLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:getter)
		UINT64 getLayer() const { return mLayer; }

		/** @copydoc CharacterController::setLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:setter)
		void setLayer(UINT64 layer);

		/** @copydoc CharacterController::onColliderHit */
		BS_SCRIPT_EXPORT(n:OnColliderHit)
		Event<void(const ControllerColliderCollision&)> onColliderHit;

		/** @copydoc CharacterController::onControllerHit */
		BS_SCRIPT_EXPORT(n:OnControllerHit)
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