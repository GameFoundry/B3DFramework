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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics),ExportName(CharacterController)) CCharacterController : public Component
	{
	public:
		CCharacterController(const HSceneObject& parent);

		/** @copydoc CharacterController::move */
		BS_SCRIPT_EXPORT(ExportName(Move))
		CharacterCollisionFlags Move(const Vector3& displacement);

		/** @copydoc CharacterController::getFootPosition */
		BS_SCRIPT_EXPORT(ExportName(FootPosition),Property(Getter),UI(Hide))
		Vector3 GetFootPosition() const;

		/** @copydoc CharacterController::setFootPosition */
		BS_SCRIPT_EXPORT(ExportName(FootPosition),Property(Setter),UI(Hide))
		void SetFootPosition(const Vector3& position);

		/** @copydoc CharacterController::getRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius),Property(Getter))
		float GetRadius() const { return mDesc.Radius; }

		/** @copydoc CharacterController::setRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius),Property(Setter))
		void SetRadius(float radius);

		/** @copydoc CharacterController::getHeight */
		BS_SCRIPT_EXPORT(ExportName(Height),Property(Getter))
		float GetHeight() const { return mDesc.Height; }

		/** @copydoc CharacterController::setHeight */
		BS_SCRIPT_EXPORT(ExportName(Height),Property(Setter))
		void SetHeight(float height);

		/** @copydoc CharacterController::getUp */
		BS_SCRIPT_EXPORT(ExportName(Up),Property(Getter))
		Vector3 GetUp() const { return mDesc.Up; }

		/** @copydoc CharacterController::setUp */
		BS_SCRIPT_EXPORT(ExportName(Up),Property(Setter))
		void SetUp(const Vector3& up);

		/** @copydoc CharacterController::getClimbingMode */
		BS_SCRIPT_EXPORT(ExportName(ClimbingMode),Property(Getter))
		CharacterClimbingMode GetClimbingMode() const { return mDesc.ClimbingMode; }

		/** @copydoc CharacterController::setClimbingMode */
		BS_SCRIPT_EXPORT(ExportName(ClimbingMode),Property(Setter))
		void SetClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::getNonWalkableMode */
		BS_SCRIPT_EXPORT(ExportName(NonWalkableMode),Property(Getter))
		CharacterNonWalkableMode GetNonWalkableMode() const { return mDesc.NonWalkableMode; }

		/** @copydoc CharacterController::setNonWalkableMode */
		BS_SCRIPT_EXPORT(ExportName(NonWalkableMode),Property(Setter))
		void SetNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::getMinMoveDistance */
		BS_SCRIPT_EXPORT(ExportName(MinMoveDistance),Property(Getter))
		float GetMinMoveDistance() const { return mDesc.MinMoveDistance; }

		/** @copydoc CharacterController::setMinMoveDistance */
		BS_SCRIPT_EXPORT(ExportName(MinMoveDistance),Property(Setter))
		void SetMinMoveDistance(float value);

		/** @copydoc CharacterController::getContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset),Property(Getter))
		float GetContactOffset() const { return mDesc.ContactOffset; }

		/** @copydoc CharacterController::setContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset),Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc CharacterController::getStepOffset */
		BS_SCRIPT_EXPORT(ExportName(StepOffset),Property(Getter))
		float GetStepOffset() const { return mDesc.StepOffset; }

		/** @copydoc CharacterController::setStepOffset */
		BS_SCRIPT_EXPORT(ExportName(StepOffset),Property(Setter))
		void SetStepOffset(float value);

		/** @copydoc CharacterController::getSlopeLimit */
		BS_SCRIPT_EXPORT(ExportName(SlopeLimit),Property(Getter),Range([0,180]),UI(AsSlider))
		Radian GetSlopeLimit() const { return mDesc.SlopeLimit; }

		/** @copydoc CharacterController::setSlopeLimit */
		BS_SCRIPT_EXPORT(ExportName(SlopeLimit),Property(Setter),Range([0,180]),UI(AsSlider))
		void SetSlopeLimit(Radian value);

		/** @copydoc CharacterController::getLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer),Property(Getter),UI(AsLayerMask))
		u64 GetLayer() const { return mLayer; }

		/** @copydoc CharacterController::setLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer),Property(Setter),UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc CharacterController::onColliderHit */
		BS_SCRIPT_EXPORT(ExportName(OnColliderHit))
		Event<void(const ControllerColliderCollision&)> OnColliderHit;

		/** @copydoc CharacterController::onControllerHit */
		BS_SCRIPT_EXPORT(ExportName(OnControllerHit))
		Event<void(const ControllerControllerCollision&)> OnControllerHit;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the character controller that this component wraps. */
		CharacterController* GetInternalInternal() const { return static_cast<CharacterController*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		using Component::DestroyInternal;

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
		u64 mLayer = 1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCharacterControllerRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CCharacterController(); // Serialization only
	 };

	 /** @} */
}
