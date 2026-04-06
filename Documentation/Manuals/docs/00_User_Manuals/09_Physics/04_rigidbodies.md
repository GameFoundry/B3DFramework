---
title: Dynamic objects
---

So far we have only talked about static physical objects (colliders). Now it's time to discuss objects that can be moved in a physically realistic way while interacting with other physical objects (whether static or dynamic). Such dynamic objects are known as rigid bodies and are represented with the @b3d::Rigidbody component.

~~~~~~~~~~~~~{.cpp}
HSceneObject rigidbodySceneObject = SceneObject::Create("Rigidbody");
HRigidbody rigidbody = rigidbodySceneObject->AddComponent<Rigidbody>();
~~~~~~~~~~~~~

> We call them "rigid" because their shape cannot be changed as a result of physics (unlike objects in the real world).

![Rigidbody under the influence of gravity](../../Images/rigidbodyGravity.gif)

# Shape
A rigidbody represents a physical object, and as such must have a certain shape. To define a rigidbody shape, we use the **Collider** components we talked about in the previous chapter. Any collider component that is added to the same scene object as a rigidbody, or to a child of such a scene object, will define a shape of the rigidbody. A rigidbody can have one or multiple child colliders.

Such collider components no longer represent static geometry and will be moved under the influence of the rigidbody.

~~~~~~~~~~~~~{.cpp}
// Add a box shape to a rigidbody
HBoxCollider boxCollider = rigidbodySceneObject->AddComponent<BoxCollider>();

// Box 1x1x1 in size
boxCollider->SetExtents(Vector3(0.5f, 0.5f, 0.5f));

// Add a sphere shape as a child scene object
HSceneObject sphereSceneObject = SceneObject::Create("Sphere shape");
sphereSceneObject->SetParent(rigidbodySceneObject);

HSphereCollider sphereCollider = sphereSceneObject->AddComponent<SphereCollider>();

// A unit sphere
sphereCollider->SetRadius(1.0f);
~~~~~~~~~~~~~

# Mass
Aside from a shape, a rigidbody also requires mass. Mass will determine how easy the object is to move and how easily the object will move others as it collides with them. Objects with zero mass are considered immovable (similar to static colliders). To change the object's mass, you can call @b3d::Rigidbody::SetMass.

~~~~~~~~~~~~~{.cpp}
rigidbody->SetMass(10.0f); // 10 units (e.g. kilograms)
~~~~~~~~~~~~~

Query the current mass:

~~~~~~~~~~~~~{.cpp}
float mass = rigidbody->GetMass();
~~~~~~~~~~~~~

This mass is considered to be uniformly distributed over all of the rigidbody's shapes. If your rigidbody contains multiple shapes, each with different density, you can instead set mass on a per-collider basis by calling @b3d::Collider::SetMass.

~~~~~~~~~~~~~{.cpp}
boxCollider->SetMass(8.0f); // Box shape is much heavier than the sphere shape
sphereCollider->SetMass(2.0f);
~~~~~~~~~~~~~

To make sure the rigidbody uses the mass from its child colliders, you must call @b3d::Rigidbody::SetFlags with both **RigidbodyFlag::AutoTensors** and **RigidbodyFlag::AutoMass** flags set. **RigidbodyFlag::AutoMass** enables mass calculation based on child shapes, and we'll talk about **RigidbodyFlag::AutoTensors** later. For now, it's enough to know that automatic mass calculation doesn't work without it.

~~~~~~~~~~~~~{.cpp}
rigidbody->SetFlags(RigidbodyFlag::AutoTensors | RigidbodyFlag::AutoMass);
~~~~~~~~~~~~~

By properly distributing mass and density over child shapes, you can achieve much more realistic simulation for complex objects (e.g. a car). For simple objects (e.g. a barrel, a rock), it's best to keep uniform mass density.

# Forces
A rigidbody will not move until we apply some forces to it. Forces can be applied directly (as shown here), and indirectly by being hit by another rigidbody.

## Gravity
The most basic force you can apply to a rigidbody is that of gravity.

You can enable gravity on a rigidbody by calling @b3d::Rigidbody::SetUseGravity. This is enabled by default, but it can be disabled if gravity is not required.

~~~~~~~~~~~~~{.cpp}
// Disable gravity for this object
rigidbody->SetUseGravity(false);
~~~~~~~~~~~~~

Check if gravity is enabled:

~~~~~~~~~~~~~{.cpp}
bool usesGravity = rigidbody->GetUseGravity();
~~~~~~~~~~~~~

You can also change the strength of gravity by changing its acceleration factor. This can be done through the **PhysicsScene**. Call @b3d::PhysicsScene::SetGravity to change gravity.

~~~~~~~~~~~~~{.cpp}
// Set gravity of the main scene to the value on the Moon
const SPtr<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
physicsScene->SetGravity(Vector3(0.0f, -1.622f, 0.0f)); // in m/s^2
~~~~~~~~~~~~~

Query current gravity:

~~~~~~~~~~~~~{.cpp}
Vector3 gravity = physicsScene->GetGravity();
~~~~~~~~~~~~~

## Manual forces
You can also apply forces manually. There are two types of forces:
 - **Force** - Produces linear momentum (moves the object)
 - **Torque** - Produces angular momentum (rotates the object)

Force can be applied by calling @b3d::Rigidbody::AddForce. This force is always applied to the object's center of mass (talked about later), which means it will never cause angular momentum (i.e. the object will just move, and not rotate).

~~~~~~~~~~~~~{.cpp}
// Add 10 units of force in the up direction (e.g. make the object jump)
rigidbody->AddForce(Vector3(0.0f, 10.0f, 0.0f));
~~~~~~~~~~~~~

Torque can be applied by calling @b3d::Rigidbody::AddTorque. Torque will cause the object to rotate (but not move).

~~~~~~~~~~~~~{.cpp}
// Add 10 units of torque in the right direction (e.g. make a wheel spin)
rigidbody->AddTorque(Vector3(10.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~

You can also choose to apply force to an arbitrary point on an object. This will generally cause the object to both move and rotate. Apply force to a specific point by calling @b3d::Rigidbody::AddForceAtPoint.

~~~~~~~~~~~~~{.cpp}
// Apply force to center of an edge of a box (assuming we're using the box/sphere shape defined above)
// This will cause the object both to move and rotate around the Y axis
rigidbody->AddForceAtPoint(Vector3(10.0f, 0.0f, 0.0f), Vector3(0.5f, 0.0f, 0.5f));
~~~~~~~~~~~~~ 

### Force modes
When applying forces using **Rigidbody::AddForce()** or **Rigidbody::AddTorque()**, you can specify an additional @b3d::ForceMode parameter. This parameter determines how your "force" value is interpreted. The supported values are:
 - **ForceMode::Force** - The default value. Your value is interpreted as normal force which will be applied during the current frame and then cause the object's position, velocity, and acceleration to change during the next few frames.
 - **ForceMode::Impulse** - The value is interpreted as an impulse and the force is calculated from it. The impulse is an instantaneous change to the object's linear or angular momentum. This is the change in momentum that would happen during a single frame of physics calculation.
 - **ForceMode::Velocity** - The value will be applied as a change in velocity, and force derived from that. This is similar to impulse but it also ignores the object's mass.
 - **ForceMode::Acceleration** - The value will be applied as a change in acceleration, and force derived from that. This is similar to applying force normally, but it ignores the object's mass.

When calling **Rigidbody::AddForceAtPoint()**, you can also specify force modes using the @b3d::PointForceMode enum, which supports just **Force** and **Impulse** modes, which have the same meaning as described above.

## Setting velocity
Finally, you can directly change the object's linear or angular velocity by calling @b3d::Rigidbody::SetVelocity and @b3d::Rigidbody::SetAngularVelocity, respectively. You can use this if you need exact control over velocity instead of controlling it through forces.

~~~~~~~~~~~~~{.cpp}
// Make the object move 50 units per second (e.g. 50km/h) in the forward direction
rigidbody->SetVelocity(Vector3(0.0f, 0.0f, -50.0f));
~~~~~~~~~~~~~

Query current velocities:

~~~~~~~~~~~~~{.cpp}
Vector3 velocity = rigidbody->GetVelocity();
Vector3 angularVelocity = rigidbody->GetAngularVelocity();
~~~~~~~~~~~~~ 

# Drag
Each object has two types of drag properties:
 - **Linear drag** - Resistance to linear motion
 - **Angular drag** - Resistance to rotational motion

When drag is set to a higher value, the object will be harder (require more force) to move (linear drag) or rotate (angular drag).

Use @b3d::Rigidbody::SetDrag to change the linear drag, and @b3d::Rigidbody::SetAngularDrag to change the angular drag.

~~~~~~~~~~~~~{.cpp}
// Make the object harder to move/rotate (e.g. as if it was in a denser fluid)
rigidbody->SetDrag(3.0f);
rigidbody->SetAngularDrag(3.0f);
~~~~~~~~~~~~~

Query drag values:

~~~~~~~~~~~~~{.cpp}
float linearDrag = rigidbody->GetDrag();
float angularDrag = rigidbody->GetAngularDrag();
~~~~~~~~~~~~~

Note that you generally don't want to increase these values in order to simulate friction when colliding with other physical objects. That is better done using physical materials, which we'll talk about later. These properties are more useful when simulating interaction with fluids like air or water (which aren't represented as physical objects).

# Kinematic rigidbodies
Kinematic mode can be enabled on a rigidbody by calling @b3d::Rigidbody::SetIsKinematic. A rigidbody in kinematic mode has two major differences compared to normal rigidbodies:
 - Such a rigidbody cannot be moved by other physical objects
 - Its position and orientation can be changed directly instead of through forces

This is very similar to a collider, which also cannot be moved by other physical objects. However, the major difference is that changing a kinematic rigidbody's position or orientation will result in correct physical interaction with the environment. For example, moving a kinematic object forward would push any normal rigidbodies out of its way, and it would stop when it reached a physical object it cannot move through.

Such rigidbodies are especially useful for simulating user or AI-controlled characters, as their movement is generally difficult to handle using forces.

Once kinematic mode is enabled, you can move a rigidbody using @b3d::Rigidbody::Move, or rotate it using @b3d::Rigidbody::Rotate.

~~~~~~~~~~~~~{.cpp}
// Enable kinematic mode
rigidbody->SetIsKinematic(true);

// Move 10 units forward and rotate 90 degrees around Y axis
rigidbody->Move(Vector3(0.0f, 0.0f, 10.0f));
rigidbody->Rotate(Quaternion(Vector3::kUnitY, Degree(90.0f)));
~~~~~~~~~~~~~

Check if a rigidbody is kinematic:

~~~~~~~~~~~~~{.cpp}
bool isKinematic = rigidbody->GetIsKinematic();
~~~~~~~~~~~~~

Note that you should not move or rotate such a rigidbody by using its scene object (e.g. by calling **SceneObject::SetPosition()** or similar). Although this will move the object to the wanted position, the object will not correctly interact with the environment.

## Continuous collision detection
When moving a rigid object, you should be careful not to move it too much in a single frame. If you move too much, the object might move beyond an obstacle it shouldn't have been able to move through. Generally, you want to call **Rigidbody::Move()** and **Rigidbody::Rotate()** every frame in small increments.

In case you are making a fast-paced game where movement in a single frame is very high (e.g. a racing game), and want to prevent rigidbodies from "tunneling" through obstacles, you can enable continuous collision detection by calling @b3d::Rigidbody::SetFlags with the @b3d::RigidbodyFlag::CCD flag.

~~~~~~~~~~~~~{.cpp}
rigidbody->SetFlags(RigidbodyFlag::CCD);
~~~~~~~~~~~~~

This form of collision detection will prevent such tunneling at the cost of lower performance.

# Tensors
Tensors allow you to fine-tune how force and torque affect your rigidbody. The tensors are determined by the shapes (colliders) of the rigidbody and their mass, or can be set manually. There are two types of tensors:
 - **Center of mass** - Determines the point the object rotates around. Also determines how much rotation vs. movement is applied to an object resulting from a force applied to a specific point.
 - **Inertia tensor** - Determines how the object rotates. Objects with different shapes and densities will rotate more easily in certain directions than others.

In most cases, you want both of these properties to be calculated automatically. In which case, you should provide the @b3d::RigidbodyFlag::AutoTensors flag to **Rigidbody::SetFlags**. This will ensure these values are calculated from child collider shapes and mass.

~~~~~~~~~~~~~{.cpp}
rigidbody->SetFlags(RigidbodyFlag::AutoTensors);
~~~~~~~~~~~~~

If you wish to set them manually, you can instead call @b3d::Rigidbody::SetCenterOfMassPosition and @b3d::Rigidbody::SetInertiaTensor.

~~~~~~~~~~~~~{.cpp}
// Manually set center of mass
rigidbody->SetCenterOfMassPosition(Vector3(0.0f, 1.0f, 0.0f));

// Manually set inertia tensor
rigidbody->SetInertiaTensor(Vector3(1.0f, 1.0f, 1.0f));
~~~~~~~~~~~~~

Query tensor values:

~~~~~~~~~~~~~{.cpp}
Vector3 centerOfMass = rigidbody->GetCenterOfMassPosition();
Vector3 inertiaTensor = rigidbody->GetInertiaTensor();
~~~~~~~~~~~~~

# Sleep
For performance reasons, objects that are not moving or are barely moving will be put to sleep. This allows the physics system to avoid those objects in its calculations. Such objects will be automatically woken up when other objects interact with them, or you move them from code.

In general, this process is automatic and isn't something you need to worry about, but in some cases it can be useful to perform it manually. For this reason, rigidbodies contain a set of methods for manipulating and checking the sleep state:
 - @b3d::Rigidbody::IsSleeping - Checks if the rigidbody is currently sleeping.
 - @b3d::Rigidbody::Sleep - Forces the object to sleep.
 - @b3d::Rigidbody::WakeUp - Forces the object to wake up.
 - @b3d::Rigidbody::SetSleepThreshold - Changes the required amount of kinetic energy the object needs to be affected by in order to stay awake. Once kinetic energy drops below this limit, the object will be put to sleep.

~~~~~~~~~~~~~{.cpp}
// Check if rigidbody is sleeping
if (rigidbody->IsSleeping())
	B3D_LOG(Info, LogPhysics, "Rigidbody is sleeping");

// Force the rigidbody to sleep
rigidbody->Sleep();

// Wake up the rigidbody
rigidbody->WakeUp();

// Set custom sleep threshold
rigidbody->SetSleepThreshold(0.1f);

// Query sleep threshold
float sleepThreshold = rigidbody->GetSleepThreshold();
~~~~~~~~~~~~~
