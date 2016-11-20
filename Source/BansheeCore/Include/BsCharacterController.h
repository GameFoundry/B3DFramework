//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPhysicsCommon.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** 
	 * Controls climbing behaviour for a capsule character controller. Normally the character controller will not
	 * automatically climb when heights are greater than the assigned step offset. However due to the shape of the capsule
	 * it might automatically climb over slightly larger heights than assigned step offsets.
	 */
	enum class CharacterClimbingMode
	{
		Normal, /**< Normal behaviour. Capsule character controller will be able to auto-step even above the step offset. */
		Constrained /**< The system will attempt to limit auto-step to the provided step offset and no higher. */
	};

	/** Controls behaviour when a character controller reaches a slope thats larger than its slope offset. */
	enum class CharacterNonWalkableMode
	{
		Prevent, /**< Character will be prevented from going further, but will be allowed to move laterally. */
		PreventAndSlide /**< Character will be prevented from going further, but also slide down the slope. */
	};

	/** Reports in which directions is the character colliding with other objects. */
	enum class CharacterCollisionFlag
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
		CharacterController(const CHAR_CONTROLLER_DESC& desc) { }
		virtual ~CharacterController() { }

		/**
		 * Moves the controller in the specified direction by the specified amount, while interacting with surrounding
		 * geometry. Returns flags signaling where collision occurred after the movement.
		 *
		 * Does not account for gravity, you must apply it manually.
		 */
		virtual CharacterCollisionFlags move(const Vector3& displacement) = 0;

		/** Returns position of the center of the controller. */
		virtual Vector3 getPosition() const = 0;

		/** 
		 * Sets position of the center of the controller. This will teleport the character to the location. Use move()
		 * for movement that includes physics.
		 */
		virtual void setPosition(const Vector3& position) = 0;

		/** Returns position of the bottom of the controller. Position takes contact offset into account. */
		virtual Vector3 getFootPosition() const = 0;

		/** 
		 * Sets position of the bottom of the controller. Position takes contact offset into account. This will teleport the
		 * character to the location. Use move() for movement that includes physics.
		 */
		virtual void setFootPosition(const Vector3& position) = 0;

		/** Returns the radius of the controller capsule. */
		virtual float getRadius() const = 0;

		/** Sets the radius of the controller capsule. */
		virtual void setRadius(float radius) = 0;

		/** Returns the height between the centers of the two spheres of the controller capsule. */
		virtual float getHeight() const = 0;

		/** Sets the height between the centers of the two spheres of the controller capsule. */
		virtual void setHeight(float height) = 0;

		/** Returns the up direction of capsule. Determines capsule orientation. */
		virtual Vector3 getUp() const = 0;

		/** Sets the up direction of capsule. Determines capsule orientation. */
		virtual void setUp(const Vector3& up) = 0;

		/** 
		 * Returns climbing mode.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::climbingMode
		 */
		virtual CharacterClimbingMode getClimbingMode() const = 0;

		/** 
		 * Sets climbing mode.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::climbingMode
		 */
		virtual void setClimbingMode(CharacterClimbingMode mode) = 0;

		/** 
		 * Returns non-walkable mode.
		 *
		 * @copydoc CHAR_CONTROLLER_DESC::nonWalkableMode
		 */
		virtual CharacterNonWalkableMode getNonWalkableMode() const = 0;

		/** 
		 * Sets non-walkable mode.
		 *
		 * @copydoc CHAR_CONTROLLER_DESC::nonWalkableMode
		 */
		virtual void setNonWalkableMode(CharacterNonWalkableMode mode) = 0;

		/** 
		 * Returns minimum move distance.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::minMoveDistance
		 */
		virtual float getMinMoveDistance() const = 0;

		/** 
		 * Sets minimum move distance.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::minMoveDistance
		 */
		virtual void setMinMoveDistance(float value) = 0;

		/** 
		 * Returns the contact offset.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::contactOffset
		 */
		virtual float getContactOffset() const = 0;

		/** 
		 * Sets the contact offset.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::contactOffset
		 */
		virtual void setContactOffset(float value) = 0;

		/** 
		 * Returns the step offset.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::stepOffset
		 */
		virtual float getStepOffset() const = 0;

		/** 
		 * Sets the step offset.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::stepOffset
		 */
		virtual void setStepOffset(float value) = 0;

		/**
		 * Returns the slope angle.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::slopeLimit
		 */
		virtual Radian getSlopeLimit() const = 0;

		/**
		 * Sets the slope angle.
		 *
		 * @copydoc	CHAR_CONTROLLER_DESC::slopeLimit
		 */
		virtual void setSlopeLimit(Radian value) = 0;

		/** Sets the layer that controls what can the controller collide with. */
		virtual void setLayer(UINT64 layer) { mLayer = layer; }

		/** Gets the layer that controls what can the controller collide with. */
		virtual UINT64 getLayer() const { return mLayer; }

		/** Creates a new character controller. */
		static SPtr<CharacterController> create(const CHAR_CONTROLLER_DESC& desc);

		/** Triggered when the controller hits a collider. */
		Event<void(const ControllerColliderCollision&)> onColliderHit;

		/** Triggered when the controller hits another character controller. */
		Event<void(const ControllerControllerCollision&)> onControllerHit;

		/** @name Internal
		 *  @{
		 */

		/** 
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void _setOwner(PhysicsOwnerType type, void* owner) { mOwner.type = type; mOwner.ownerData = owner; }

		/** 
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* _getOwner(PhysicsOwnerType type) const { return mOwner.type == type ? mOwner.ownerData : nullptr; }

		/** @} */
	private:
		PhysicsObjectOwner mOwner;
		UINT64 mLayer = 1;
	};

	/** Contains all the information required for initializing a character controller. */
	struct CHAR_CONTROLLER_DESC
	{
		/** Center of the controller capsule */
		Vector3 position = Vector3::ZERO;

		/**
		 * Contact offset specifies a skin around the object within which contacts will be generated. It should be a small
		 * positive non-zero value.
		 */
		float contactOffset = 0.1f;

		/**
		 * Controls which obstacles will the character be able to automatically step over without being stopped. This is the
		 * height of the maximum obstacle that will be stepped over (with exceptions, see climbingMode).
		 */
		float stepOffset = 0.5f;

		/**
		 * Controls which slopes should the character consider too steep and won't be able to move over. See
		 * nonWalkableMode for more information.
		 */
		Radian slopeLimit = Degree(45.0f);

		/** 
		 * Represents minimum distance that the character will move during a call to move(). This is used to stop the
		 * recursive motion algorithm when the remaining distance is too small.
		 */
		float minMoveDistance = 0.0f;

		/** Height between the centers of the two spheres of the controller capsule. */
		float height = 0.0f; 

		/** Radius of the controller capsule. */
		float radius = 1.0f;
		
		/** Up direction of controller capsule. Determines capsule orientation. */
		Vector3 up = Vector3::UNIT_Y;

		/** 
		 * Controls what happens when character encounters a height higher than its step offset. 
		 *
		 * @see	CharacterClimbingMode
		 */
		CharacterClimbingMode climbingMode = CharacterClimbingMode::Normal;

		/** 
		 * Controls what happens when character encounters a slope higher than its slope offset. 
		 *
		 * @see	CharacterNonWalkableMode
		 */
		CharacterNonWalkableMode nonWalkableMode = CharacterNonWalkableMode::Prevent;
	};

	/** Contains data about a collision of a character controller and another object. */
	struct ControllerCollision
	{
		Vector3 position; /**< Contact position. */
		Vector3 normal; /**< Contact normal. */
		Vector3 motionDir; /**< Direction of motion after the hit. */
		float motionAmount; /**< Magnitude of motion after the hit. */
	};

	/** Contains data about a collision of a character controller and a collider. */
	struct ControllerColliderCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which 
		 * case check #colliderRaw. 
		 */
		HCollider collider;
		Collider* colliderRaw; /**< Collider that was touched. */
		UINT32 triangleIndex; /**< Touched triangle index for mesh colliders. */
	};

	/** Contains data about a collision between two character controllers. */
	struct ControllerControllerCollision : ControllerCollision
	{
		/**
		 * Component of the controller that was touched. Can be null if the controller has no component parent, in which 
		 * case check #controllerRaw. 
		 */
		HCharacterController controller; 
		CharacterController* controllerRaw; /**< Controller that was touched. */
	};

	/** @} */
}