---
title: Character controller
---

Character controller is a special type of dynamic physics object that is intended to be used primarily for the movement of the user-controlled character in your game. Although you could simulate your character using a rigidbody, it is usually preferable to use the character controller due to various specific functionality it offers.

It is represented using the @b3d::CharacterController component.

~~~~~~~~~~~~~{.cpp}
HSceneObject controllerSceneObject = SceneObject::Create("CharacterController");
HCharacterController characterController = controllerSceneObject->AddComponent<CharacterController>();
~~~~~~~~~~~~~

# Shape
Similar to a rigidbody, the controller requires a shape to represent its surface. This shape is always a capsule, and you can control the size and orientation of it using the following methods:
 - @b3d::CharacterController::SetHeight - Sets the length of the line going along the center of the capsule.
 - @b3d::CharacterController::SetRadius - Sets the distance that determines how far away the capsule's surface is from its center line.
 - @b3d::CharacterController::SetUp - Sets the direction of the center line.

When setting the capsule shape, make sure it covers your character model, but without making it too large so there isn't a lot of empty space. This prevents unrealistic collisions in which the capsule shape collides with an object but the visible model doesn't.

~~~~~~~~~~~~~{.cpp}
// Make a capsule pointing straight up, with a total height of 1.8 units (e.g. meters) and width/depth of 0.8 units
characterController->SetHeight(1.0f);
characterController->SetRadius(0.4f);
characterController->SetUp(Vector3::kUnitY);
~~~~~~~~~~~~~

Query shape properties:

~~~~~~~~~~~~~{.cpp}
float height = characterController->GetHeight();
float radius = characterController->GetRadius();
Vector3 up = characterController->GetUp();
~~~~~~~~~~~~~

# Movement
Similar to a rigidbody, when needing to move the controller you should use a specialized @b3d::CharacterController::Move method instead of the movement methods available on the scene object. Unlike with **Rigidbody**, this **Move()** method takes as a parameter a displacement determining how much to move from the current position, unlike **Rigidbody** whose **Move()** method takes absolute coordinates.

~~~~~~~~~~~~~{.cpp}
// Move 1 unit forward for every frame while W key is pressed
if (GetInput().IsButtonHeld(BC_W))
	characterController->Move(Vector3(0.0f, 0.0f, 1.0f));
~~~~~~~~~~~~~

**CharacterController::Move()** will perform the movement immediately, unlike most other physics objects which only move every physics "tick". This also means any collisions can be reported immediately, which is done by the return value of **CharacterController::Move()**. The return value is of type @b3d::CharacterCollisionFlags and can be used to tell if the controller collided with anything, and if it has, whether it was on its sides, its top, or its bottom (or a combination of these).

~~~~~~~~~~~~~{.cpp}
CharacterCollisionFlags collisionFlags = characterController->Move(Vector3(0.0f, 0.0f, 1.0f));

if (collisionFlags & CharacterCollisionFlag::Sides)
	B3D_LOG(Info, LogPhysics, "Collided with sides");
if (collisionFlags & CharacterCollisionFlag::Up)
	B3D_LOG(Info, LogPhysics, "Collided with ceiling");
if (collisionFlags & CharacterCollisionFlag::Down)
	B3D_LOG(Info, LogPhysics, "Collided with ground");
~~~~~~~~~~~~~

This is similar to collision events reported by colliders and rigidbodies, but as the controller move is executed immediately, its collision is also reported right away, meaning you can respond to the event with no delay so there is less input latency (which is important for objects controlled directly by the player).

Unlike with **Rigidbody**, rotation must be handled through the scene object. Such rotation will not properly physically interact with the environment.

# Slopes
Character controller has the ability to limit what is the maximum slope it can be moved on. This is useful to prevent the game character from climbing unrealistically steep slopes. Use @b3d::CharacterController::SetSlopeLimit to set the maximum slope the controller is allowed to climb, in angles.

~~~~~~~~~~~~~{.cpp}
// Maximum slope of 45 degrees
characterController->SetSlopeLimit(Degree(45));
~~~~~~~~~~~~~

Query the slope limit:

~~~~~~~~~~~~~{.cpp}
Radian slopeLimit = characterController->GetSlopeLimit();
~~~~~~~~~~~~~

Once the controller reaches the maximum slope the wanted behaviour can be:
 - to stop the controller from moving any further
 - to stop the controller and have it slide down

This behaviour can be controlled by calling @b3d::CharacterController::SetNonWalkableMode, which accepts a parameter of type @b3d::CharacterNonWalkableMode.

~~~~~~~~~~~~~{.cpp}
characterController->SetNonWalkableMode(CharacterNonWalkableMode::PreventAndSlide);
~~~~~~~~~~~~~

Query the non-walkable mode:

~~~~~~~~~~~~~{.cpp}
CharacterNonWalkableMode mode = characterController->GetNonWalkableMode();
~~~~~~~~~~~~~

# Steps
Controller can be also made to automatically climb on small steep obstacles, like stairs. Maximum height that the controller can step over is controlled by @b3d::CharacterController::SetStepOffset.

~~~~~~~~~~~~~{.cpp}
// Step over anything 0.15 units (e.g. 15cm) in height
characterController->SetStepOffset(0.15f);
~~~~~~~~~~~~~

Query the step offset:

~~~~~~~~~~~~~{.cpp}
float stepOffset = characterController->GetStepOffset();
~~~~~~~~~~~~~

# Events
Similar to colliders, character controllers also report events when they collide with other objects. Use @b3d::CharacterController::OnColliderHit to be notified when the controller hits a collider.

The value reported is @b3d::ControllerColliderCollision which contains the position, normal and motion of a single contact point, as well as the reference to the collider that was hit.

~~~~~~~~~~~~~{.cpp}
auto colliderHit = [](const ControllerColliderCollision& data)
{
	HCollider hitCollider = data.Collider;
	String hitSceneObjectName = hitCollider->SO()->GetName();

	Vector3 contactPoint = data.Position;
	B3D_LOG(Info, LogPhysics, "Hit {0} at point {1}", hitSceneObjectName, contactPoint);
};

characterController->OnColliderHit.Connect(colliderHit);
~~~~~~~~~~~~~

You can also receive events when the controller hits another controller by subscribing to @b3d::CharacterController::OnControllerHit.

The value reported is @b3d::ControllerControllerCollision which contains the position, normal and motion of a single contact point, as well as the reference to the other controller that was hit.

~~~~~~~~~~~~~{.cpp}
auto controllerHit = [](const ControllerControllerCollision& data)
{
	HCharacterController otherController = data.Controller;
	String otherSceneObjectName = otherController->SO()->GetName();

	Vector3 contactPoint = data.Position;
	B3D_LOG(Info, LogPhysics, "Hit {0} at point {1}", otherSceneObjectName, contactPoint);
};

characterController->OnControllerHit.Connect(controllerHit);
~~~~~~~~~~~~~
