//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"

namespace b3d
{
	class ColliderShape;
	class ICharacterControllerImplementation;

	/** @addtogroup Physics
	 *  @{
	 */

	/**
	 * Controls climbing behaviour for a capsule character controller. Normally the character controller will not
	 * automatically climb when heights are greater than the assigned step offset. However due to the shape of the capsule
	 * it might automatically climb over slightly larger heights than assigned step offsets.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterClimbingMode
	{
		Normal, /**< Normal behaviour. Capsule character controller will be able to auto-step even above the step offset. */
		Constrained /**< The system will attempt to limit auto-step to the provided step offset and no higher. */
	};

	/** Controls behaviour when a character controller reaches a slope thats larger than its slope offset. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterNonWalkableMode
	{
		Prevent, /**< Character will be prevented from going further, but will be allowed to move laterally. */
		PreventAndSlide /**< Character will be prevented from going further, but also slide down the slope. */
	};

	/** Reports in which directions is the character colliding with other objects. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterCollisionFlag
	{
		Sides = 0x1, /**< Character is colliding with its sides. */
		Up = 0x2, /**< Character is colliding with the ceiling. */
		Down = 0x4 /**< Character is colliding with the ground. */
	};

	/** @copydoc CharacterCollisionFlag */
	typedef Flags<CharacterCollisionFlag> CharacterCollisionFlags;
	B3D_FLAGS_OPERATORS(CharacterCollisionFlag)

	/** Contains all the information required for initializing a character controller. */
	struct CharacterControllerCreateInformation
	{
		/** Center of the controller capsule */
		Vector3 Position = Vector3::kZero;

		/**
		 * Contact offset specifies a skin around the object within which contacts will be generated. It should be a small
		 * positive non-zero value.
		 */
		float ContactOffset = 0.1f;

		/**
		 * Controls which obstacles will the character be able to automatically step over without being stopped. This is the
		 * height of the maximum obstacle that will be stepped over (with exceptions, see ClimbingMode).
		 */
		float StepOffset = 0.5f;

		/**
		 * Controls which slopes should the character consider too steep and won't be able to move over. See
		 * NonWalkableMode for more information.
		 */
		Radian SlopeLimit = Degree(45.0f);

		/**
		 * Represents minimum distance that the character will move during a call to Move(). This is used to stop the
		 * recursive motion algorithm when the remaining distance is too small.
		 */
		float MinMoveDistance = 0.0f;

		/** Height between the centers of the two spheres of the controller capsule. */
		float Height = 1.0f;

		/** Radius of the controller capsule. */
		float Radius = 1.0f;

		/** Up direction of controller capsule. Determines capsule orientation. */
		Vector3 Up = Vector3::kUnitY;

		/**
		 * Controls what happens when character encounters a height higher than its step offset.
		 *
		 * @see	CharacterClimbingMode
		 */
		CharacterClimbingMode ClimbingMode = CharacterClimbingMode::Normal;

		/**
		 * Controls what happens when character encounters a slope higher than its slope offset.
		 *
		 * @see	CharacterNonWalkableMode
		 */
		CharacterNonWalkableMode NonWalkableMode = CharacterNonWalkableMode::Prevent;
	};

	/** Contains data about a collision of a character controller and another object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerCollision
	{
		Vector3 Position; /**< Contact position. */
		Vector3 Normal; /**< Contact normal. */
		Vector3 MotionDir; /**< Direction of motion after the hit. */
		float MotionAmount; /**< Magnitude of motion after the hit. */
	};

	/** Contains data about a collision of a character controller and a collider. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerColliderCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which
		 * case check #colliderRaw.
		 */
		HCollider Collider;

		B3D_SCRIPT_EXPORT(Exclude(true))
		ColliderShape* ColliderShape; /**< Collider that was touched. */
		u32 TriangleIndex; /**< Touched triangle index for mesh colliders. */
	};

	/** Contains data about a collision between two character controllers. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerControllerCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which
		 * case check #controllerRaw.
		 */
		HCharacterController Controller;

		B3D_SCRIPT_EXPORT(Exclude(true))
		CharacterController* ControllerRaw; /**< Controller that was touched. */
	};

	/**
	 * Special physics controller meant to be used for game characters. Uses the "slide-and-collide" physics instead of
	 * of the standard physics model to handle various issues with manually moving kinematic objects. Uses a capsule to
	 * represent the character's bounds.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterController : public Component
	{
	public:
		CharacterController(const HSceneObject& parent);

		/**
		 * Moves the controller in the specified direction by the specified amount, while interacting with surrounding
		 * geometry. Returns flags signaling where collision occurred after the movement.
		 *
		 * Does not account for gravity, you must apply it manually.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Move))
		CharacterCollisionFlags Move(const Vector3& displacement);

		/**
		 * Determines the  position of the bottom of the controller. Position takes contact offset into account. Changing
		 * this will teleport the character to the location. Use Move() for movement that includes physics.
		 */
		B3D_SCRIPT_EXPORT(ExportName(FootPosition), Property(Setter), UI(Hide))
		void SetFootPosition(const Vector3& position);

		/** @copydoc GetFootPosition */
		B3D_SCRIPT_EXPORT(ExportName(FootPosition), Property(Getter), UI(Hide))
		Vector3 GetFootPosition() const;

		/** Determines the radius of the controller capsule. */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc SetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mInformation.Radius; }

		/** Determines the height between the centers of the two spheres of the controller capsule. */
		B3D_SCRIPT_EXPORT(ExportName(Height), Property(Setter))
		void SetHeight(float height);

		/** @copydoc SetHeight */
		B3D_SCRIPT_EXPORT(ExportName(Height), Property(Getter))
		float GetHeight() const { return mInformation.Height; }

		/** Determines the up direction of capsule. Determines capsule orientation. */
		B3D_SCRIPT_EXPORT(ExportName(Up), Property(Setter))
		void SetUp(const Vector3& up);

		/** @copydoc SetUp */
		B3D_SCRIPT_EXPORT(ExportName(Up), Property(Getter))
		Vector3 GetUp() const { return mInformation.Up; }

		/** @copydoc CharacterControllerCreateInformation::ClimbingMode */
		B3D_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Setter))
		void SetClimbingMode(CharacterClimbingMode mode);

		/** @copydoc SetClimbingMode */
		B3D_SCRIPT_EXPORT(ExportName(ClimbingMode), Property(Getter))
		CharacterClimbingMode GetClimbingMode() const { return mInformation.ClimbingMode; }

		/** @copydoc CharacterControllerCreateInformation::NonWalkableMode */
		B3D_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Setter))
		void SetNonWalkableMode(CharacterNonWalkableMode mode);

		/** @copydoc SetNonWalkableMode */
		B3D_SCRIPT_EXPORT(ExportName(NonWalkableMode), Property(Getter))
		CharacterNonWalkableMode GetNonWalkableMode() const { return mInformation.NonWalkableMode; }

		/** @copydoc CharacterControllerCreateInformation::MinMoveDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Setter))
		void SetMinMoveDistance(float value);

		/** @copydoc SetMinMoveDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinMoveDistance), Property(Getter))
		float GetMinMoveDistance() const { return mInformation.MinMoveDistance; }

		/** @copydoc CharacterControllerCreateInformation::ContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc SetContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Getter))
		float GetContactOffset() const { return mInformation.ContactOffset; }

		/** @copydoc CharacterControllerCreateInformation::StepOffset */
		B3D_SCRIPT_EXPORT(ExportName(StepOffset), Property(Setter))
		void SetStepOffset(float value);

		/** @copydoc SetStepOffset */
		B3D_SCRIPT_EXPORT(ExportName(StepOffset), Property(Getter))
		float GetStepOffset() const { return mInformation.StepOffset; }

		/** @copydoc CharacterControllerCreateInformation::SlopeLimit */
		B3D_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Setter), UIValueRange([ 0, 180 ]), UI(AsSlider))
		void SetSlopeLimit(Radian value);

		/** @copydoc SetSlopeLimit */
		B3D_SCRIPT_EXPORT(ExportName(SlopeLimit), Property(Getter), UIValueRange([ 0, 180 ]), UI(AsSlider))
		Radian GetSlopeLimit() const { return mInformation.SlopeLimit; }

		/** Determines the layer that controls what can the controller collide with. */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Setter), UI(AsLayerMask))
		void SetLayer(u64 layer) { mLayer = layer; }

		/** @copydoc SetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Getter), UI(AsLayerMask))
		u64 GetLayer() const { return mLayer; }

		/** Triggered when the controller hits a collider. */
		B3D_SCRIPT_EXPORT(ExportName(OnColliderHit))
		Event<void(const ControllerColliderCollision&)> OnColliderHit;

		/** Triggered when the controller hits another character controller. */
		B3D_SCRIPT_EXPORT(ExportName(OnControllerHit))
		Event<void(const ControllerControllerCollision&)> OnControllerHit;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		/** Updates the position by copying it from the controller to the component's scene object. */
		void UpdateSceneObjectPositionFromController();

		/** Updates the dimensions of the controller by taking account scale of the parent scene object. */
		void UpdateDimensions();

		/** Destroys the internal character controller representation. */
		void DestroyInternal();

		/** Triggered when the internal controller hits a collider. */
		void TriggerOnColliderHit(const ControllerColliderCollision& value);

		/** Triggered when the internal controller hits another controller. */
		void TriggerOnControllerHit(const ControllerControllerCollision& value);

		UPtr<ICharacterControllerImplementation> mImplementation;
		CharacterControllerCreateInformation mInformation;
		u64 mLayer = 1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CharacterControllerRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		CharacterController(); // Serialization only
	};

	/** @} */

	/**
	 * @addtogroup Physics-Internal
	 * @{
	 */

	/** Low-level interface for a character controller. Should be implemented by the physics plugin to provide functionality. */
	class B3D_EXPORT ICharacterControllerImplementation
	{
	public:
		virtual ~ICharacterControllerImplementation() = default;

		/** @copydoc CharacterController::Move */
		virtual CharacterCollisionFlags Move(const Vector3& displacement) = 0;

		/**
		 * Determines position of the center of the controller. This will teleport the character to the location. Use Move()
		 * for movement that includes physics.
		 */
		virtual void SetPosition(const Vector3& position) = 0;

		/** @copydoc SetPosition */
		virtual Vector3 GetPosition() const = 0;

		/** @copydoc CharacterController::SetFootPosition */
		virtual void SetFootPosition(const Vector3& position) = 0;

		/** @copydoc SetFootPosition */
		virtual Vector3 GetFootPosition() const = 0;

		/** @copydoc CharacterController::SetRadius */
		virtual void SetRadius(float radius) = 0;

		/** @copydoc SetRadius() */
		virtual float GetRadius() const = 0;

		/** @copydoc CharacterController::SetHeight */
		virtual void SetHeight(float height) = 0;

		/** @copydoc SetHeight() */
		virtual float GetHeight() const = 0;

		/** @copydoc CharacterController::SetUp */
		virtual void SetUp(const Vector3& up) = 0;

		/** @copydoc SetUp() */
		virtual Vector3 GetUp() const = 0;

		/** @copydoc CharacterController::SetClimbingMode */
		virtual void SetClimbingMode(CharacterClimbingMode mode) = 0;

		/** @copydoc SetClimbingMode */
		virtual CharacterClimbingMode GetClimbingMode() const = 0;

		/** @copydoc CharacterController::SetNonWalkableMode */
		virtual void SetNonWalkableMode(CharacterNonWalkableMode mode) = 0;

		/** @copydoc SetNonWalkableMode */
		virtual CharacterNonWalkableMode GetNonWalkableMode() const = 0;

		/** @copydoc CharacterController::SetMinMoveDistance */
		virtual void SetMinMoveDistance(float value) = 0;

		/** @copydoc SetMinMoveDistance */
		virtual float GetMinMoveDistance() const = 0;

		/** @copydoc CharacterController::SetContactOffset */
		virtual void SetContactOffset(float value) = 0;

		/** @copydoc SetContactOffset */
		virtual float GetContactOffset() const = 0;

		/** @copydoc CharacterController::SetStepOffset */
		virtual void SetStepOffset(float value) = 0;

		/** @copydoc SetStepOffset */
		virtual float GetStepOffset() const = 0;

		/** @copydoc CharacterController::SetSlopeLimit */
		virtual void SetSlopeLimit(Radian value) = 0;

		/** @copydoc SetSlopeLimit */
		virtual Radian GetSlopeLimit() const = 0;
	};

	/** @} */
} // namespace b3d
