//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Math/BsVector3.h"

namespace bs
{
	class PhysicsScene;

	/** @addtogroup Physics
	 *  @{
	 */

	/**
	 * Controls climbing behaviour for a capsule character controller. Normally the character controller will not
	 * automatically climb when heights are greater than the assigned step offset. However due to the shape of the capsule
	 * it might automatically climb over slightly larger heights than assigned step offsets.
	 */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterClimbingMode
	{
		Normal, /**< Normal behaviour. Capsule character controller will be able to auto-step even above the step offset. */
		Constrained /**< The system will attempt to limit auto-step to the provided step offset and no higher. */
	};

	/** Controls behaviour when a character controller reaches a slope thats larger than its slope offset. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterNonWalkableMode
	{
		Prevent, /**< Character will be prevented from going further, but will be allowed to move laterally. */
		PreventAndSlide /**< Character will be prevented from going further, but also slide down the slope. */
	};

	/** Reports in which directions is the character colliding with other objects. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) CharacterCollisionFlag
	{
		Sides = 0x1, /**< Character is colliding with its sides. */
		Up = 0x2, /**< Character is colliding with the ceiling. */
		Down = 0x4 /**< Character is colliding with the ground. */
	};

	/** @copydoc CharacterCollisionFlag */
	typedef Flags<CharacterCollisionFlag> CharacterCollisionFlags;
	BS_FLAGS_OPERATORS(CharacterCollisionFlag)

	struct CHAR_CONTROLLER_DESC;
	struct ControllerColliderCollision;
	struct ControllerControllerCollision;

	/**
	 * Special physics controller meant to be used for game characters. Uses the "slide-and-collide" physics instead of
	 * of the standard physics model to handle various issues with manually moving kinematic objects. Uses a capsule to
	 * represent the character's bounds.
	 */
	class BS_CORE_EXPORT CharacterController
	{
	public:
		CharacterController(const CHAR_CONTROLLER_DESC& desc) {}

		virtual ~CharacterController() {}

		/**
		 * Moves the controller in the specified direction by the specified amount, while interacting with surrounding
		 * geometry. Returns flags signaling where collision occurred after the movement.
		 *
		 * Does not account for gravity, you must apply it manually.
		 */
		virtual CharacterCollisionFlags Move(const Vector3& displacement) = 0;

		/** Returns position of the center of the controller. */
		virtual Vector3 GetPosition() const = 0;

		/**
		 * Sets position of the center of the controller. This will teleport the character to the location. Use move()
		 * for movement that includes physics.
		 */
		virtual void SetPosition(const Vector3& position) = 0;

		/** @copydoc setFootPosition() */
		virtual Vector3 GetFootPosition() const = 0;

		/**
		 * Determines the  position of the bottom of the controller. Position takes contact offset into account. Changing
		 * this will teleport the character to the location. Use move() for movement that includes physics.
		 */
		virtual void SetFootPosition(const Vector3& position) = 0;

		/** @copydoc setRadius() */
		virtual float GetRadius() const = 0;

		/** Determines the radius of the controller capsule. */
		virtual void SetRadius(float radius) = 0;

		/** @copydoc setHeight() */
		virtual float GetHeight() const = 0;

		/** Determines the height between the centers of the two spheres of the controller capsule. */
		virtual void SetHeight(float height) = 0;

		/** @copydoc setUp() */
		virtual Vector3 GetUp() const = 0;

		/** Determines the up direction of capsule. Determines capsule orientation. */
		virtual void SetUp(const Vector3& up) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::climbingMode */
		virtual CharacterClimbingMode GetClimbingMode() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::climbingMode */
		virtual void SetClimbingMode(CharacterClimbingMode mode) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::nonWalkableMode */
		virtual CharacterNonWalkableMode GetNonWalkableMode() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::nonWalkableMode */
		virtual void SetNonWalkableMode(CharacterNonWalkableMode mode) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::minMoveDistance */
		virtual float GetMinMoveDistance() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::minMoveDistance */
		virtual void SetMinMoveDistance(float value) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::contactOffset */
		virtual float GetContactOffset() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::contactOffset */
		virtual void SetContactOffset(float value) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::stepOffset */
		virtual float GetStepOffset() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::stepOffset */
		virtual void SetStepOffset(float value) = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::slopeLimit */
		virtual Radian GetSlopeLimit() const = 0;

		/** @copydoc CHAR_CONTROLLER_DESC::slopeLimit */
		virtual void SetSlopeLimit(Radian value) = 0;

		/** Determines the layer that controls what can the controller collide with. */
		virtual void SetLayer(u64 layer) { mLayer = layer; }

		/** @copydoc setLayer() */
		virtual u64 GetLayer() const { return mLayer; }

		/**
		 * Creates a new character controller.
		 *
		 * @param[in]	scene		Scene to add the controller to.
		 * @param[in]	desc		Describes controller geometry and movement.
		 */
		static SPtr<CharacterController> Create(PhysicsScene& scene, const CHAR_CONTROLLER_DESC& desc);

		/** Triggered when the controller hits a collider. */
		Event<void(const ControllerColliderCollision&)> OnColliderHit;

		/** Triggered when the controller hits another character controller. */
		Event<void(const ControllerControllerCollision&)> OnControllerHit;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void SetOwnerInternal(PhysicsOwnerType type, void* owner)
		{
			mOwner.Type = type;
			mOwner.OwnerData = owner;
		}

		/**
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* GetOwnerInternal(PhysicsOwnerType type) const { return mOwner.Type == type ? mOwner.OwnerData : nullptr; }

		/** @} */
	private:
		PhysicsObjectOwner mOwner;
		u64 mLayer = 1;
	};

	/** Contains all the information required for initializing a character controller. */
	struct CHAR_CONTROLLER_DESC
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
		 * height of the maximum obstacle that will be stepped over (with exceptions, see climbingMode).
		 */
		float StepOffset = 0.5f;

		/**
		 * Controls which slopes should the character consider too steep and won't be able to move over. See
		 * nonWalkableMode for more information.
		 */
		Radian SlopeLimit = Degree(45.0f);

		/**
		 * Represents minimum distance that the character will move during a call to move(). This is used to stop the
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
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerCollision
	{
		Vector3 Position; /**< Contact position. */
		Vector3 Normal; /**< Contact normal. */
		Vector3 MotionDir; /**< Direction of motion after the hit. */
		float MotionAmount; /**< Magnitude of motion after the hit. */
	};

	/** Contains data about a collision of a character controller and a collider. */
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerColliderCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which
		 * case check #colliderRaw.
		 */
		HCollider Collider;

		BS_SCRIPT_EXPORT(Exclude(true))
		bs::Collider* ColliderRaw; /**< Collider that was touched. */
		u32 TriangleIndex; /**< Touched triangle index for mesh colliders. */
	};

	/** Contains data about a collision between two character controllers. */
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ControllerControllerCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which
		 * case check #controllerRaw.
		 */
		HCharacterController Controller;

		BS_SCRIPT_EXPORT(Exclude(true))
		CharacterController* ControllerRaw; /**< Controller that was touched. */
	};

	/** @} */
} // namespace bs
