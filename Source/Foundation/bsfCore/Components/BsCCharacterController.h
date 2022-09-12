//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		CharacterCollisionFlags Move(const Vector3& displacement);

		/** @copydoc CharacterController::getFootPosition */
		BS_SCRIPT_EXPORT(n:FootPosition,pr:getter,hide)
		Vector3 GetFootPosition() const;

		/** @copydoc CharacterController::setFootPosition */
		BS_SCRIPT_EXPORT(n:FootPosition,pr:setter,hide)
		void SetFootPosition(const Vector3& position);

		/** @copydoc CharacterController::getRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float GetRadius() const { return mDesc.radius; }

		/** @copydoc CharacterController::setRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void SetRadius(float radius);

		/** @copydoc CharacterController::getHeight */
		BS_SCRIPT_EXPORT(n:Height,pr:getter)
		float GetHeight() const { return mDesc.height; }

		/** @copydoc CharacterController::setHeight */
		BS_SCRIPT_EXPORT(n:Height,pr:setter)
		void SetHeight(float height);

		/** @copydoc CharacterController::getUp */
		BS_SCRIPT_EXPORT(n:Up,pr:getter)
		Vector3 GetUp() const { return mDesc.up; }

		/** @copydoc CharacterController::setUp */
		BS_SCRIPT_EXPORT(n:Up,pr:setter)
		void SetUp(const Vector3& up);

		/** @copydoc CharacterController::getClimbingMode */
		BS_SCRIPT_EXPORT(n:ClimbingMode,pr:getter)
		CharacterClimbingMode GetClimbingMode() const { return mDesc.climbingMode; }

		/** @copydoc CharacterController::setClimbingMode */
		BS_SCRIPT_EXPORT(n:ClimbingMode,pr:setter)
		void SetClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::getNonWalkableMode */
		BS_SCRIPT_EXPORT(n:NonWalkableMode,pr:getter)
		CharacterNonWalkableMode GetNonWalkableMode() const { return mDesc.nonWalkableMode; }

		/** @copydoc CharacterController::setNonWalkableMode */
		BS_SCRIPT_EXPORT(n:NonWalkableMode,pr:setter)
		void SetNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::getMinMoveDistance */
		BS_SCRIPT_EXPORT(n:MinMoveDistance,pr:getter)
		float GetMinMoveDistance() const { return mDesc.minMoveDistance; }

		/** @copydoc CharacterController::setMinMoveDistance */
		BS_SCRIPT_EXPORT(n:MinMoveDistance,pr:setter)
		void SetMinMoveDistance(float value);

		/** @copydoc CharacterController::getContactOffset */
		BS_SCRIPT_EXPORT(n:ContactOffset,pr:getter)
		float GetContactOffset() const { return mDesc.contactOffset; }

		/** @copydoc CharacterController::setContactOffset */
		BS_SCRIPT_EXPORT(n:ContactOffset,pr:setter)
		void SetContactOffset(float value);

		/** @copydoc CharacterController::getStepOffset */
		BS_SCRIPT_EXPORT(n:StepOffset,pr:getter)
		float GetStepOffset() const { return mDesc.stepOffset; }

		/** @copydoc CharacterController::setStepOffset */
		BS_SCRIPT_EXPORT(n:StepOffset,pr:setter)
		void SetStepOffset(float value);

		/** @copydoc CharacterController::getSlopeLimit */
		BS_SCRIPT_EXPORT(n:SlopeLimit,pr:getter,range:[0,180],slider)
		Radian GetSlopeLimit() const { return mDesc.slopeLimit; }

		/** @copydoc CharacterController::setSlopeLimit */
		BS_SCRIPT_EXPORT(n:SlopeLimit,pr:setter,range:[0,180],slider)
		void SetSlopeLimit(Radian value);

		/** @copydoc CharacterController::getLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:getter,layerMask)
		UINT64 GetLayer() const { return mLayer; }

		/** @copydoc CharacterController::setLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:setter,layerMask)
		void SetLayer(UINT64 layer);

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
		CharacterController* _getInternal() const { return static_cast<CharacterController*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		using Component::destroyInternal;

		/** @copydoc Component::onInitialized() */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void OnTransformChanged(TransformChangedFlags flags) override;

		/** Updates the position by copying it from the controller to the component's scene object. */
		void UpdatePositionFromController();

		/** Updates the dimensions of the controller by taking account scale of the parent scene object. */
		void UpdateDimensions();

		/** Destroys the internal character controller representation. */
		void DestroyInternal();

		/** Triggered when the internal controller hits a collider. */
		void TriggerOnColliderHit(const ControllerColliderCollision& value);

		/** Triggered when the internal controller hits another controller. */
		void TriggerOnControllerHit(const ControllerControllerCollision& value);

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
		CCharacterController(); // Serialization only
	 };

	 /** @} */
}
