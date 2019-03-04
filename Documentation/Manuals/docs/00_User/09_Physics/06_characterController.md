Character controller 						{#characterController}
===============
[TOC]

Character controller is a special type of a dynamic physics objects that is intended to be used primarily for the movement of the user-controller character in your game. Although you could simulate your character using a rigidbody, it is usually preferable to use the character controller due to various specific functionality it offers.

It is represented using the @ref bs::CCharacterController "CharacterController" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject controllerSO = SceneObject::create("Rigidbody");
HCharacterController charController = controllerSO->addComponent<CCharacterController>();
~~~~~~~~~~~~~

# Shape {#characterController_a}
Similar to a rigidbody, the controller requires a shape to represent its surface. This shape is always a capsule, and you can control the size and orientation of it using the following methods:
 - @ref bs::CCharacterController::setHeight "CCharacterController::setHeight()" - Sets the length of the line going along the center of the capsule.
 - @ref bs::CCharacterController::setRadius "CCharacterController::setRadius()" - Sets the distance that determines how far away is the capsule's surface from its center line.
 - @ref bs::CCharacterController::setUp "CCharacterController::setUp()" - Sets the direction of the center line. 
 
When setting the capsule shape, make sure it covers your character model. But without making it too large so there isn't a lot of empty space in order to prevent unrealistic collisions in which the capsule shape collides with an object, but the visible model doesn't.
 
~~~~~~~~~~~~~{.cpp}
// Make a capsule pointing straight up, with a total height of 1.8 units (e.g. meters) and width/depth of 0.8 units
charController->setHeight(1.0f);
charController->setRadius(0.4f);
charController->setUp(Vector3::UNIT_Y);
~~~~~~~~~~~~~

# Movement {#characterController_b}
Similar to a rigidbody, when needing to move the controller you should use a specialized @ref bs::CCharacterController::move "CCharacterController::move()" method instead of the movement methods available on the scene object. Unlike with **Rigidbody**, this *move()* method takes as a parameter a displacement determining how much to move from the current position, unlike **Rigidbody** whose *move()* method takes absolute coordinates.

~~~~~~~~~~~~~{.cpp}
// Move 1 unit forward for every frame while W key is pressed
if(gInput().isButtonHeld(BC_W))
	charController->move(Vector3(0, 0, 1.0f));
~~~~~~~~~~~~~

**CCharacterController::move()** will perform the movement immediately, unlike most other physics objects which only move every physics "tick". This also means any collisions can be reported immediately, which is done by the return value of **CCharacterController::move()**. The return value is of type @ref bs::CharacterCollisionFlags "CharacterCollisionFlags" and can be used to tell if the controller collided with anything, and if it has, whether it was on its sides, its top or its bottom (or a combination of these). 

This is similar to collision events reported by colliders and rigidbodies, but as the controller move is executely immediately its collision is also reported right away, meaning you can respond to the event with no delay so there is less input latency (which is important for objects controlled directly by the player).

Unlike with **Rigidbody** rotation must be handled through the scene object. Such rotation will not properly physically interact with the environment.

# Slopes {#characterController_c}
Character controller has the ability to limit what is the maximum slope it can be moved on. This is useful to prevent the game character from climbing unrealistically steep slopes. Use @ref bs::CCharacterController::setSlopeLimit "CCharacterController::setSlopeLimit()" to set the maximum slope the controller is allowed to climb, in angles.

~~~~~~~~~~~~~{.cpp}
// Maximum slope of 45 degrees
charController->setSlopeLimit(Degree(45));
~~~~~~~~~~~~~

Once the controller reaches the maximum slope the wanted behaviour can be:
 - to stop the controller from moving any further
 - to stop the controller and have it slide down

This behaviour can be controlled by calling @ref bs::CCharacterController::setNonWalkableMode "CCharacterController::setNonWalkableMode()", which accepts a parameter of type @ref bs::CharacterNonWalkableMode "CharacterNonWalkableMode".

~~~~~~~~~~~~~{.cpp}
charController->setNonWalkableMode(CharacterNonWalkableMode::PreventAndSlide);
~~~~~~~~~~~~~

# Steps {#characterController_d}
Controller can be also made to automatically climb on small steep obstacles, like stairs. Maximum height that the controller can step over is controlled by @ref bs::CCharacterController::setStepOffset "CCharacterController::setStepOffset()".

~~~~~~~~~~~~~{.cpp}
// Step over anything 0.15 units (e.g. 15cm) in height
charController->setStepOffset(0.15f);
~~~~~~~~~~~~~

# Events {#characterController_e}
Similar to colliders, character controllers also report events when they collide with other objects. Use @ref bs::CCharacterController::onColliderHit "CCharacterController::onColliderHit" to be notified when the controller hits a collider.

The value reported is @ref bs::ControllerColliderCollision "ControllerColliderCollision" which contains the position, normal and motion of a single contact point, as well as the reference to the collider that was hit.

~~~~~~~~~~~~~{.cpp}
auto colliderHit = [](const ControllerColliderCollision& data)
{
	HCollider hitCollider = data.collider;
	String hitSceneObjectName = hitCollider->SO()->getName();
	
	Vector3 contactPoint = data.position;
	gDebug().logDebug("Hit " + hitSceneObjectName + " at point " + toString(contactPoint));
};

charController->onColliderHit.connect(colliderHit);
~~~~~~~~~~~~~

You can also receive events when the controller hits another controller by subscribing to @ref bs::CCharacterController::onControllerHit "CCharacterController::onControllerHit". 

The value reported is @ref bs::ControllerControllerCollision "ControllerControllerCollision" which contains the position, normal and motion of a single contact point, as well as the reference to the other controller that was hit.

~~~~~~~~~~~~~{.cpp}
auto controllerHit = [](const ControllerControllerCollision& data)
{
	HCharacterController otherController = data.controller;
	String otherSceneObjectName = otherController->SO()->getName();
	
	Vector3 contactPoint = data.position;
	gDebug().logDebug("Hit " + otherSceneObjectName + " at point " + toString(contactPoint));
};

collider->onControllerHit.connect(controllerHit);
~~~~~~~~~~~~~
