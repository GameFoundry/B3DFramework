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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(CharacterController)) CCharacterController : public Component
	{
	public:
		CCharacterController(const HSceneObject& parent);

		/** @copydoc CharacterController::Move */
		BS_SCRIPT_EXPORT(ExportName(Move))
		CharacterCollisionFlags Move(const Vector3& displacement);

		/** @copydoc CharacterController::GetFootPosition */
		BS_SCRIPT_EXPORT(ExportName(FootPosition), Property(Getter), UI(Hide))
		Vector3 GetFootPosition() const;

		/** @copydoc CharacterController::SetFootPosition */
		BS_SCRIPT_EXPORT(ExportName(FootPosition), Property(Setter), UI(Hide))
		void SetFootPosition(const Vector3& position);

		/** @copydoc CharacterController::GetRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mDesc.Radius; }

		/** @copydoc CharacterController::SetRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc CharacterController::GetHeight */
		BS_SCRIPT_EXPORT(ExportName(Height), Property(Getter))

		float GetHeight() const { return mDesc.Height; }

		/** @copydoc CharacterController::SetHeight */
		BS_SCRIPT_EXPORT(ExportName(Height), Property(Setter))
		void SetHeight(float height);

		/** @copydoc CharacterController::GetUp */
		BS_SCRIPT_EXPORT(ExportName(Up), Property(Getter))

		Vector3 GetUp() const { return mDesc.Up; }

		/** @copydoc CharacterController::SetUp */
		BS_SCRIPT_EXPORT(ExportName(Up), Property(Setter))
		void SetUp(const Vector3& up);

		/** @copydoc CharacterController::GetClimbingMode */
		BS_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Getter))

		CharacterClimbingMode GetClimbingMode() const { return mDesc.ClimbingMode; }

		/** @copydoc CharacterController::SetClimbingMode */
		BS_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Setter))
		void SetClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::GetNonWalkableMode */
		BS_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Getter))

		CharacterNonWalkableMode GetNonWalkableMode() const { return mDesc.NonWalkableMode; }

		/** @copydoc CharacterController::SetNonWalkableMode */
		BS_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Setter))
		void SetNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::GetMinMoveDistance */
		BS_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Getter))

		float GetMinMoveDistance() const { return mDesc.MinMoveDistance; }

		/** @copydoc CharacterController::SetMinMoveDistance */
		BS_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Setter))
		void SetMinMoveDistance(float value);

		/** @copydoc CharacterController::GetContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Getter))

		float GetContactOffset() const { return mDesc.ContactOffset; }

		/** @copydoc CharacterController::SetContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc CharacterController::GetStepOffset */
		BS_SCRIPT_EXPORT(ExportName(StepOffset), Property(Getter))

		float GetStepOffset() const { return mDesc.StepOffset; }

		/** @copydoc CharacterController::SetStepOffset */
		BS_SCRIPT_EXPORT(ExportName(StepOffset), Property(Setter))
		void SetStepOffset(float value);

		/** @copydoc CharacterController::GetSlopeLimit */
		BS_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Getter), UIValueRange([ 0, 180 ]), UI(AsSlider))

		Radian GetSlopeLimit() const { return mDesc.SlopeLimit; }

		/** @copydoc CharacterController::SetSlopeLimit */
		BS_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Setter), UIValueRange([ 0, 180 ]), UI(AsSlider))
		void SetSlopeLimit(Radian value);

		/** @copydoc CharacterController::GetLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Getter), UI(AsLayerMask))

		u64 GetLayer() const { return mLayer; }

		/** @copydoc CharacterController::SetLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Setter), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc CharacterController::OnColliderHit */
		BS_SCRIPT_EXPORT(ExportName(OnColliderHit))
		Event<void(const ControllerColliderCollision&)> OnColliderHit;

		/** @copydoc CharacterController::OnControllerHit */
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

		void OnInitialized() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
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
} // namespace bs
