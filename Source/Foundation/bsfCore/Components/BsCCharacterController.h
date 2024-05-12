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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(CharacterController)) CCharacterController : public Component
	{
	public:
		CCharacterController(const HSceneObject& parent);

		/** @copydoc CharacterController::Move */
		B3D_SCRIPT_EXPORT(ExportName(Move))
		CharacterCollisionFlags Move(const Vector3& displacement);

		/** @copydoc CharacterController::GetFootPosition */
		B3D_SCRIPT_EXPORT(ExportName(FootPosition), Property(Getter), UI(Hide))
		Vector3 GetFootPosition() const;

		/** @copydoc CharacterController::SetFootPosition */
		B3D_SCRIPT_EXPORT(ExportName(FootPosition), Property(Setter), UI(Hide))
		void SetFootPosition(const Vector3& position);

		/** @copydoc CharacterController::GetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mDesc.Radius; }

		/** @copydoc CharacterController::SetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc CharacterController::GetHeight */
		B3D_SCRIPT_EXPORT(ExportName(Height), Property(Getter))

		float GetHeight() const { return mDesc.Height; }

		/** @copydoc CharacterController::SetHeight */
		B3D_SCRIPT_EXPORT(ExportName(Height), Property(Setter))
		void SetHeight(float height);

		/** @copydoc CharacterController::GetUp */
		B3D_SCRIPT_EXPORT(ExportName(Up), Property(Getter))

		Vector3 GetUp() const { return mDesc.Up; }

		/** @copydoc CharacterController::SetUp */
		B3D_SCRIPT_EXPORT(ExportName(Up), Property(Setter))
		void SetUp(const Vector3& up);

		/** @copydoc CharacterController::GetClimbingMode */
		B3D_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Getter))

		CharacterClimbingMode GetClimbingMode() const { return mDesc.ClimbingMode; }

		/** @copydoc CharacterController::SetClimbingMode */
		B3D_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Setter))
		void SetClimbingMode(CharacterClimbingMode mode);

		/** @copydoc CharacterController::GetNonWalkableMode */
		B3D_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Getter))

		CharacterNonWalkableMode GetNonWalkableMode() const { return mDesc.NonWalkableMode; }

		/** @copydoc CharacterController::SetNonWalkableMode */
		B3D_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Setter))
		void SetNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc CharacterController::GetMinMoveDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Getter))

		float GetMinMoveDistance() const { return mDesc.MinMoveDistance; }

		/** @copydoc CharacterController::SetMinMoveDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Setter))
		void SetMinMoveDistance(float value);

		/** @copydoc CharacterController::GetContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Getter))

		float GetContactOffset() const { return mDesc.ContactOffset; }

		/** @copydoc CharacterController::SetContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc CharacterController::GetStepOffset */
		B3D_SCRIPT_EXPORT(ExportName(StepOffset), Property(Getter))

		float GetStepOffset() const { return mDesc.StepOffset; }

		/** @copydoc CharacterController::SetStepOffset */
		B3D_SCRIPT_EXPORT(ExportName(StepOffset), Property(Setter))
		void SetStepOffset(float value);

		/** @copydoc CharacterController::GetSlopeLimit */
		B3D_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Getter), UIValueRange([ 0, 180 ]), UI(AsSlider))

		Radian GetSlopeLimit() const { return mDesc.SlopeLimit; }

		/** @copydoc CharacterController::SetSlopeLimit */
		B3D_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Setter), UIValueRange([ 0, 180 ]), UI(AsSlider))
		void SetSlopeLimit(Radian value);

		/** @copydoc CharacterController::GetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Getter), UI(AsLayerMask))

		u64 GetLayer() const { return mLayer; }

		/** @copydoc CharacterController::SetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Setter), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc CharacterController::OnColliderHit */
		B3D_SCRIPT_EXPORT(ExportName(OnColliderHit))
		Event<void(const ControllerColliderCollision&)> OnColliderHit;

		/** @copydoc CharacterController::OnControllerHit */
		B3D_SCRIPT_EXPORT(ExportName(OnControllerHit))
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

		void OnBeginPlay() override;
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
