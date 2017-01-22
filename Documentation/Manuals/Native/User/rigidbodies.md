Dynamic objects 						{#rigidbodies}
===============

So far we have only talked about static physical objects (colliders). Now it's time to discuss objects that can be moved in a physically realistic way, while interacting with other physical objects (whether static or dynamic). Such dynamic objects are known as rigid bodies and are represented with the @ref bs::CRigidbody "Rigidbody" component. 

~~~~~~~~~~~~~{.cpp}
HSceneObject rigidbodySO = SceneObject::create("Rigidbody");
HRigidbody rigidbody = rigidbodySO->addComponent<CRigidbody>();
~~~~~~~~~~~~~

> We call them "rigid" because their shape cannot be changed as a result of physics (unlike objects in the real world).

![Rigidbody under the influence of gravity](rigidbodyGravity.gif)

# Shape
A rigidbody represents a physical object, and as such must have a certain shape. To define a rigidbody shape we use the **Collider** components we talked about in previous chapter. Any collider component that is added to the same scene object as a rigidbody, or to a child of such scene object, will define a shape of the rigidbody. A rigidbody can have one or multiple child colliders. 

Such collider components no longer represent static geometry and will be moved under the influence of the rigidbody.

~~~~~~~~~~~~~{.cpp}
// Add a box shape to a rigidbody
HBoxCollider boxCollider = rigidbodySO->addComponent<CBoxCollider>();

// Box 1x1x1 in size
boxCollider->setExtents(Vector3(0.5f, 0.5f, 0.5f));

// Add a sphere shape, as a child scene object
HSceneObject sphereSO = SceneObject::create("Sphere shape");
sphereSO->setParent(rigidbodySO);

HSphereCollider sphereCollider = sphereSO->addComponent<CSphereCollider>();

// A unit sphere
sphereCollider->setRadius(1.0f);
~~~~~~~~~~~~~

# Mass
Aside from a shape, rigidbody also requires mass. Mass will determine how easy is the object to move and how easily will the object move others as it collides with them. Objects with zero mass are considered immovable (similar to static colliders). To change object's mass you can call @ref bs::CRigidbody::setMass "CRigidbody::setMass()".

~~~~~~~~~~~~~{.cpp}
rigidbody->setMass(10.0f); // 10 units (e.g. kilograms)
~~~~~~~~~~~~~

This mass is considered to be uniformly distributed over all of rigidbody's shapes. If your rigidbody contains multiple shapes each with different density, you can instead set mass on a per-collider basis by calling @ref bs::CCollider::setMass "CCollider::setMass()". 

~~~~~~~~~~~~~{.cpp}
boxCollider->setMass(8.0); // Box shape is much heavier than the sphere shape
sphereCollider->setMass(2.0f);
~~~~~~~~~~~~~

To make sure the rigidbody uses the mass from its child colliders, you must call @ref bs::CRigidbody::setFlags "CRigidbody::setFlags()" with both **Rigidbody::Flag::AutoTensors** and @ref bs::Rigidbody::Flag::AutoMass "Rigidbody::Flag::AutoMass" flags set. **Rigidbody::Flag::AutoMass** enables mass calculation based on child shapes, and we'll talk about **Rigidbody::Flag::AutoTensors** later - for now it's enough to know that automatic mass calculation doesn't work without it.

~~~~~~~~~~~~~{.cpp}
rigidbody->setFlags(Rigidbody::Flag::AutoTensors);
rigidbody->setFlags(Rigidbody::Flag::AutoMass);
~~~~~~~~~~~~~

By properly distributing mass and density over child shapes you can achieve much more realistic simulation for complex objects (e.g. a car). For simple objects (e.g. a barrel, tree trunk) it's best to keep uniform mass density.

# Forces
A rigidbody will not move until we apply some forces to it. Forces can be applied directly (as shown here), and indirectly by being hit by another rigidbody.

## Gravity
The most basic force you can apply to a rigidbody is that of gravity.

You can enable gravity on a rigidbody by calling @ref bs::CRigidbody::setUseGravity() "CRigidbody::setUseGravity()". This is enabled by default, but it can be disabled if gravity is not required.

~~~~~~~~~~~~~{.cpp}
// Disable gravity for this object
rigidbody->setUseGravity(false);
~~~~~~~~~~~~~

You can also change the strength of gravity by changing its acceleration factor. This can be done globally through the @ref bs::Physics "Physics" system accessible through @ref bs::gPhysics() "gPhysics()". Call @ref bs::Physics::setGravity "Physics::setGravity()" to change gravity.

~~~~~~~~~~~~~{.cpp}
// Set gravity to the value on the Moon
gPhysics().setGravity(1.622f); // in m/s^2
~~~~~~~~~~~~~

## Manual forces
You can also apply forces manually. There are two types of forces:
 - Force which produces linear momentum (moves the object)
 - Torque which produces angular momentum (rotates the object)

Force can be applied by calling @ref bs::CRigidbody::addForce() "CRigidbody::addForce()". This force is always applied to the object's center of mass (talked about later), which means it will never cause angular momentum (i.e. object will just move, and not rotate).
 
~~~~~~~~~~~~~{.cpp}
// Add 10 units of force in the up direction (e.g. make the object jump)
rigidbody->addForce(Vector3(0.0f, 10.0f, 0.0f));
~~~~~~~~~~~~~ 

Torque can be applied by calling @ref bs::CRigidbody::addTorque() "CRigidbody::addTorque()". Torque will cause the object to rotate (but not move).

~~~~~~~~~~~~~{.cpp}
// Add 10 units of torque in the right direction (e.g. make a wheel spin)
rigidbody->addTorque(Vector3(10.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~ 
 
You can also choose to apply force to an arbitrary point on an object. This will generally cause the object to both move and rotate. Apply force to a specific point by calling @ref bs::CRigidbody::addForceAtPoint "CRigidbody::addForceAtPoint()".

~~~~~~~~~~~~~{.cpp}
// Apply force to center of an edge of a box (assuming we're using the box/sphere shape defined above). This will cause the object both to move, and rotate around the Y axis
rigidbody->addForceAtPoint(Vector3(10.0f, 0.0f, 0.0f), Vector3(0.5f, 0.0f, 0.5f));
~~~~~~~~~~~~~ 

### Force modes
When applying forces using **CRigidbody::addForce()** or **CRigidbody::addTorque()** you can specify an additional @ref bs::ForceMode "ForceMode" parameter. This parameter determines how is your "force" value interpreted. The supported values are:
 - **ForceMode::Force** - The default value. Your value is interpreted as normal force which will be applied during current frame and then cause the object's position/velocity/acceleratation to change during the next few frames.
 - **ForceMode::Impulse** - The value is interpreted as an impulse and the force is calculated from it. The impulse is an instantenous change to the object's linear or angular momentum. This is the change in momentum that would happen during a single frame of physics calculation.
 - **ForceMode::Velocity** - The value will be applied as a change in velocity, and force derived from that. This is similar to impulse but it also ignores the object's mass.
 - **ForceMode::Acceleration** - The value will be applied as a change in acceleration, and force derived from that. This is similar to applying force normally, but it ignores the object's mass.
 
When calling **CRigidbody::addForceAtPoint()** you can also specify force modes using the @ref bs::PointForceMode "PointForceMode" enum, which supports just **Force** and **Impulse** modes, which have the same meaning as described above.

## Setting velocity
Finally, you can directly change object's linear or angular velocity by calling @ref bs::CRigidbody::setVelocity "CRigidbody::setVelocity()" and @ref bs::CRigidbody::setAngularVelocity "CRigidbody::setAngularVelocity()", respectively. You can use this if you need exact control over velocity, instead of controlling it through forces.

~~~~~~~~~~~~~{.cpp}
// Make the object move 50 units per second (e.g. 50km/h) in the forward direction
rigidbody->setVelocity(Vector3(0.0f, 0.0f, -50.0f));
~~~~~~~~~~~~~ 

# Drag
Each object has two types of drag properties:
 - Linear drag
 - Angular drag
 
When drag is set to a higher value the object will be harder (require more force) to move (linear drag) or rotate (angular drag).

Use @ref bs::CRigidbody::setDrag() "CRigidbody::setDrag()" to change the linear drag, and @ref bs::CRigidbody::setAngularDrag() "CRigidbody::setAngularDrag()" to change the angular drag.

~~~~~~~~~~~~~{.cpp}
// Make the object harder to move/rotate (e.g. as if it was in a denser fluid)
rigidbody->setDrag(3.0f);
rigidbody->setAngularDrag(3.0f);
~~~~~~~~~~~~~

Note that you generally don't want to increase these values in order to simulate friction when colliding with other physical objects. That is better done using physical materials, which we'll talk about later. Properties above are more useful when simulating interaction with fluids like air or water (which aren't represented as physical objects).

# Kinematic rigidbodies
Kinematic mode can be enabled on a rigidbody by calling @ref bs::CRigidbody::setIsKinematic() "CRigidbody::setIsKinematic()". A rigidbody in kinematic mode has two major differences compared to normal rigidbodies:
 - Such rigidbody cannot be moved by other physical objects
 - Its position and orientation can be changed directly instead of through forces
 
This is very similar to a collider, which also cannot be moved by other physical objects. However the major difference is that changing kinematic rigidbody's position or orientation will result in a correct physical interaction with the environment. For example, moving a kinematic object forward would push any normal rigidbodies out of its way, and it would stop when it reached a physical object it can't move through.

Such rigidbodies are especially useful for simulating user or AI controlled characters, as their movement is generally difficult to handle using forces.

Once kinematic mode is enabled you can move a rigidbody using @ref bs::CRigidbody::move "CRigidbody::move()", or rotate it using @ref bs::CRigidbody::rotate "CRigidbody::rotate()".

~~~~~~~~~~~~~{.cpp}
// Move 10 units forward and rotate 90 degrees around Y axis
rigidbody->move(Vector3(0.0f, 0.0f, 10.0f));
rigidbody->rotate(Quaternion(Vector3::UNIT_Y, Degree(90)));
~~~~~~~~~~~~~

Note that you should not move/rotate such rigidbody by using its scene object (e.g. by calling **SceneObject::setPosition()** or similar). Although this will move the object to the wanted position, the object will not correctly interact with the environment.

## Continous collision detection
When moving a rigid object you should be careful not to move it too much in a single frame. If you move too much the object might move beyond an obstacle it shouldn't have been able to move through. Generally you want to call **CRigibody::move()** and **CRigidbody::rotate()** every frame, in small increments.

In case you are making a fast-paced game, where movement in a single-frame is very high (e.g. a racing game), and want to prevent rigidbodies "tunneling" through obstacles, you can enable continous collision detection by calling @ref bs::CRigidbody::setFlags "CRigidbody::setFlags()" with @ref bs::Rigidbody::Flag::CCD "Rigidbody::Flag::CCD" flag.

You must also enable CCD globally through @ref bs::Physics::setFlag "Physics::setFlag()", by enabling the @ref bs::PhysicsFlag::CCD_Enable "PhysicsFlag::CCD_Enable" flag.

~~~~~~~~~~~~~{.cpp}
gPhysics().setFlag(PhysicsFlag::CCD_Enable, true);
rigidbody->setFlags(Rigidbody::Flag::CCD);
~~~~~~~~~~~~~

This form of collision detection will prevent such tunneling, at the cost of lower performance.

# Tensors
Tensors allow you to fine tune how does force and torque affect your rigidbody. The tensors are determined by shapes (colliders) of the rigidbody and their mass, or can be set manually. There are two types of tensors:
 - Center of mass - Determines the point the object rotates around. Also determines how much rotation vs. movement is applied to an object resulting from a force applied to a specific point.
 - Inertia tensor - Determines how the object rotates. Objects with different shapes and densities will rotate more easily in certain directions than others.
 
In most cases you want both of these properties to be calculated automatically. In which case you should provide the @ref bs::Rigidbody::Flag::AutoTensors "Rigidbody::Flag::AutoTensors" flag to **CRigidbody::setFlags**. This will ensure these values are calculated from child collider shapes and mass.

~~~~~~~~~~~~~{.cpp}
rigidbody->setFlags(Rigidbody::Flag::AutoTensors);
~~~~~~~~~~~~~

If you wish to set them manually, you can instead call @ref bs::CRigidbody::setCenterOfMass "CRigidbody::setCenterOfMass()" and @ref bs::CRigidbody::setInertiaTensor "CRigidbody::setInertiaTensor()".

# Sleep
For performance reasons, objects that are not moving or are barely moving will be put to sleep. This allows the physics system to avoid those objects in its calculations. Such objects will be automatically woken up when other objects interact with them, or you move them from code.

In general this process is automatic and isn't something you need to worry about, but in some cases it can be useful to perform it manually. For this reason rigidbodies contain a set of methods for manipulating and checking the sleep state:
 - @ref bs::CRigidbody::isSleeping "CRigidbody::isSleeping()" - Checks is rigidbody currently sleeping.
 - @ref bs::CRigidbody::sleep "CRigidbody::sleep()" - Forces the object to sleep.
 - @ref bs::CRigidbody::wakeUp "CRigidbody::wakeUp()" - Forces the object to wake up.
 - @ref bs::CRigidbody::setSleepThreshold "CRigidbody::setSleepThreshold()" - Changes the required amount of kinetic energy the object needs to be affected by in order to stay awake. Once kinetic energy drops below this limit, object will be put to sleep.