---
title: Joints
---

Joints allow you to constrain movement of two rigidbodies in some way. A typical example would be a door hinge. 

Framework supports six different joint types:
 - Fixed - Locks origins and orientations together
 - Distance - Keeps origins within a certain distance range
 - Spherical - Keeps origins together but allows rotation with no restrictions (also known as ball-and-socket joint)
 - Hinge - Keeps origins together but allows rotation around X axis
 - Slider - Keeps orientations together but allows movement on the X axis
 - D6 - Fully customizable joint type that can be used for implementing all previously mentioned joints, as well as configure new types
 
All joints are components derived from @b3d::Joint. Before we talk about individual joint types, lets cover the functionality shared by all joints.

# Basics
All joints require two bodies to influence. You can also choose to specify *null* for one of the bodies, in which case the other body is assumed to move relative to an immovable reference frame. Note that at least one body attached to a joint must be movable.

To assign bodies to a joint call @b3d::Joint::SetBody. The method accepts a @b3d::JointBody parameter specifying which of the two bodies to assign the value to, and a **Rigidbody** component representing the body (or null).

~~~~~~~~~~~~~{.cpp}
// Create a rigidbody to manipulate with the joint
HSceneObject rigidbodySceneObject = SceneObject::Create("Rigidbody");
HRigidbody rigidbody = rigidbodySceneObject->AddComponent<Rigidbody>();

HBoxCollider boxCollider = rigidbodySceneObject->AddComponent<BoxCollider>();
boxCollider->SetExtents(Vector3(0.5f, 0.5f, 0.5f));

// Add a fixed joint (covered later)
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HJoint joint = jointSceneObject->AddComponent<FixedJoint>();

// Move the body so its not at the same position as the joint
rigidbodySceneObject->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

// Add a body to the joint. Since we didn't add a second body this one will be anchored to an immovable reference frame
joint->SetBody(JointBody::Target, rigidbody);

// This joint will now ensure the origins and orientations of the rigidbody and joint remain constant. In this particular case since we attached the body to an immovable reference frame using a fixed joint, we have made the body immovable.
~~~~~~~~~~~~~

Query the bodies attached to a joint:

~~~~~~~~~~~~~{.cpp}
HRigidbody anchorBody = joint->GetBody(JointBody::Anchor);
HRigidbody targetBody = joint->GetBody(JointBody::Target);
~~~~~~~~~~~~~

# Joint types

## Fixed joint
Fixed joints are the simplest joint types. As the name implies they fix the origins and orientations of two bodies together. For example imagine two wheels welded together with a metal bar - when one rotates so must the other, and when one moves so must the other.

[TODO_IMAGE]()

They are represented with the @b3d::FixedJoint component. They don't allow any additional properties to be set, aside from the bodies to influence.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HFixedJoint joint = jointSceneObject->AddComponent<FixedJoint>();
~~~~~~~~~~~~~

## Distance joint
Distance joint simply keeps two bodies together at the specified distance range. You can specify a minimum and a maximum distance, as well as a spring property that makes the objects spring back or forth as they reach the range limits. For example you might use this joint type to model a rope.

[TODO_IMAGE]()

Distance joints are represented using the @b3d::DistanceJoint component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HDistanceJoint joint = jointSceneObject->AddComponent<DistanceJoint>();
~~~~~~~~~~~~~

### Limits
To specify the distance range call @b3d::DistanceJoint::SetMinDistance and @b3d::DistanceJoint::SetMaxDistance.

You must also explicitly enable the distance limits by calling @b3d::DistanceJoint::SetFlag with @b3d::DistanceJointFlag::MinDistance and @b3d::DistanceJointFlag::MaxDistance flags. This allows you to only enable one or another, in case no restrictions are needed in one extreme.

~~~~~~~~~~~~~{.cpp}
// Keep the bodies in between 5 and 20 units between each other
joint->SetMinDistance(5.0f);
joint->SetMaxDistance(20.0f);

// Enable the limits
joint->SetFlag(DistanceJointFlag::MinDistance, true);
joint->SetFlag(DistanceJointFlag::MaxDistance, true);
~~~~~~~~~~~~~

Query the distance limits:

~~~~~~~~~~~~~{.cpp}
float minDistance = joint->GetMinDistance();
float maxDistance = joint->GetMaxDistance();
~~~~~~~~~~~~~

### Spring
By default when the objects reach the specified limits they will come to a dead stop. Generally it's more realistic to have them be pushed back towards valid range slowly, in which case you can apply a spring parameter by calling @b3d::DistanceJoint::SetSpring.

It accepts an object of type @b3d::Spring which has two properties:
 - @b3d::Spring::Stiffness - Determines the strength of the spring when greater than zero.
 - @b3d::Spring::Damping - Determines the damping of the spring when greater than zero.

You must also explicitly enable springs by calling @b3d::DistanceJoint::SetFlag with the @b3d::DistanceJointFlag::Spring parameter.

~~~~~~~~~~~~~{.cpp}
Spring spring(1.0f, 1.0f);

joint->SetSpring(spring);
joint->SetFlag(DistanceJointFlag::Spring, true);
~~~~~~~~~~~~~

Query the spring:

~~~~~~~~~~~~~{.cpp}
Spring spring = joint->GetSpring();
~~~~~~~~~~~~~

### Tolerance
To ensure springs have enough time to activate, so they can begin pushing back the object before it actually breaches the valid range, you can specify an additional tolerance parameter. This parameter adds a "buffer" on the inner edges of the valid range, at which the spring will activate.

Use @b3d::DistanceJoint::SetTolerance to set the tolerance. This property is only relevant if using the spring.

~~~~~~~~~~~~~{.cpp}
joint->SetTolerance(0.5f);
~~~~~~~~~~~~~

Query the tolerance:

~~~~~~~~~~~~~{.cpp}
float tolerance = joint->GetTolerance();
~~~~~~~~~~~~~

### Current value
Sometimes it is useful to find out how much is the joint currently "stretched". Call @b3d::DistanceJoint::GetDistance to get the current distance between its two bodies.

~~~~~~~~~~~~~{.cpp}
float currentDistance = joint->GetDistance();
~~~~~~~~~~~~~

## Spherical joint
Spherical joint allows for a full range of rotations, while keeping the origins of the two bodies together. This allows you to create a ball-and-socket functionality, for example.

[TODO_IMAGE]()

Spherical joint is represented with the @b3d::SphericalJoint component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HSphericalJoint joint = jointSceneObject->AddComponent<SphericalJoint>();
~~~~~~~~~~~~~

### Limits
You can limit the rotation along Y and Z axes by specifying the limit angles in @b3d::LimitConeRange. As the name implies the angles represent the cone in which rotation is valid.

You apply the limits by calling @b3d::SphericalJoint::SetLimit, and enabling the limit by calling @b3d::SphericalJoint::SetFlag with @b3d::SphericalJointFlag::Limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone
LimitConeRange limit;
limit.YLimitAngle = Degree(90.0f);
limit.ZLimitAngle = Degree(90.0f);

joint->SetLimit(limit);
joint->SetFlag(SphericalJointFlag::Limit, true);
~~~~~~~~~~~~~

Query the limit:

~~~~~~~~~~~~~{.cpp}
LimitConeRange limit = joint->GetLimit();
~~~~~~~~~~~~~

Rotation around the X axis cannot be limited using this joint type and is always free.

### Spring
**LimitConeRange** also contains a @b3d::LimitConeRange::Restitution parameter, which has the same meaning as in **PhysicsMaterial**. If restitution is zero the body will just stop when it reaches the limit. If it is non-zero the object will bounce instead.

You can also specify a **Spring** parameter as @b3d::LimitConeRange::Spring, which controls how the body is pulled back after the limit has been breached. Its values have the same meaning as we described earlier.

Finally, you can specify a contact distance parameter as @b3d::LimitConeRange::ContactDist, which ensures springs have enough time to activate before the body goes past the valid range. This is similar to the tolerance value we talked about regarding distance joints.

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone, with a soft limit
LimitConeRange limit;
limit.YLimitAngle = Degree(90.0f);
limit.ZLimitAngle = Degree(90.0f);

limit.Restitution = 0.8f;
limit.ContactDist = -1.0f;
limit.Spring.Stiffness = 1.0f;
limit.Spring.Damping = 1.0f;

joint->SetLimit(limit);
joint->SetFlag(SphericalJointFlag::Limit, true);
~~~~~~~~~~~~~

Note that all the joint types that are about to follow have these four properties in their *Limit* structures, so we won't talk about them any further.

## Slider joint
As the name implies slider joint constrains the body movement to slide along a single axis (X axis in particular). No movement along other axes, or rotation is allowed. For example this could be used for modeling some kind of a piston moving up and down.

[TODO_IMAGE]()

It is represented using the @b3d::SliderJoint component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HSliderJoint joint = jointSceneObject->AddComponent<SliderJoint>();
~~~~~~~~~~~~~

### Limits
@b3d::LimitLinearRange can be used to specify minimum and maximum allowed distance between the two bodies (similar to the distance joint).

You apply the limit by calling @b3d::SliderJoint::SetLimit, and enable the limit by calling @b3d::SliderJoint::SetFlag with @b3d::SliderJointFlag::Limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a distance range [5, 20] units
LimitLinearRange limit;
limit.Lower = 5.0f;
limit.Upper = 20.0f;

joint->SetLimit(limit);
joint->SetFlag(SliderJointFlag::Limit, true);
~~~~~~~~~~~~~

Query the limit:

~~~~~~~~~~~~~{.cpp}
LimitLinearRange limit = joint->GetLimit();
~~~~~~~~~~~~~

### Current value
You can find out the current position of the slider by calling @b3d::SliderJoint::GetPosition, and the current speed of the slider by calling @b3d::SliderJoint::GetSpeed.

~~~~~~~~~~~~~{.cpp}
float position = joint->GetPosition();
float speed = joint->GetSpeed();
~~~~~~~~~~~~~

## Hinge joint
Hinge joint constrains rotation around a single axis (X axis specifically). Rotation around other axes, or translation is not allowed. This can be used for modeling door hinges or similar behaviour.

[TODO_IMAGE]()

It is represented using the @b3d::HingeJoint component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HHingeJoint joint = jointSceneObject->AddComponent<HingeJoint>();
~~~~~~~~~~~~~

### Limits
@b3d::LimitAngularRange can be used to specify minimum and maximum allowed angle between the two bodies.

You apply the limit by calling @b3d::HingeJoint::SetLimit, and enable the limit by calling @b3d::HingeJoint::SetFlag with @b3d::HingeJointFlag::Limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a hinge that can swing a maximum of 90 degrees
LimitAngularRange limit;
limit.Lower = Degree(0.0f);
limit.Upper = Degree(90.0f);

joint->SetLimit(limit);
joint->SetFlag(HingeJointFlag::Limit, true);
~~~~~~~~~~~~~

Query the limit:

~~~~~~~~~~~~~{.cpp}
LimitAngularRange limit = joint->GetLimit();
~~~~~~~~~~~~~

### Drive
Drive is a special object of type @b3d::HingeJointDrive that can be assigned to a hinge joint, to make the joint move without external forces. For example if you wanted to make a propeller, you could set up a drive on the hinge joint so it keeps on spinning.

Drive can be assigned through @b3d::HingeJoint::SetDrive, and must be explicitly enabled by calling **HingeJoint::SetFlag()** with @b3d::HingeJointFlag::Drive.

**HingeJointDrive** object has a few properties:
 - @b3d::HingeJointDrive::Speed - Speed at which the drive will try to spin at
 - @b3d::HingeJointDrive::ForceLimit - Optional maximum force the drive is allowed to apply
 - @b3d::HingeJointDrive::FreeSpin - If the joint ends up moving faster than the drive (due to an external force), the drive will try to brake in order to bring it down to drive's speed. If you don't want that to happen enable free spin.
 - @b3d::HingeJointDrive::GearRatio - Scales the velocity of the target body by this value.

~~~~~~~~~~~~~{.cpp}
HingeJointDrive drive;
drive.Speed = 10.0f;
drive.ForceLimit = 100.0f;
drive.FreeSpin = false;
drive.GearRatio = 1.0f;

joint->SetDrive(drive);
joint->SetFlag(HingeJointFlag::Drive, true);
~~~~~~~~~~~~~

Query the drive:

~~~~~~~~~~~~~{.cpp}
HingeJointDrive drive = joint->GetDrive();
~~~~~~~~~~~~~

### Current value
You can find out the current angle of the hinge by calling @b3d::HingeJoint::GetAngle, and the current angular speed of the joint by calling @b3d::HingeJoint::GetSpeed.

~~~~~~~~~~~~~{.cpp}
Radian angle = joint->GetAngle();
float speed = joint->GetSpeed();
~~~~~~~~~~~~~

## D6 joint
D6 is the most complex, and the most customizable type of joint. It can be used to create all joint types we have talked about so far, and to configure new custom joint types.

It is represented using the @b3d::D6Joint component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSceneObject = SceneObject::Create("Joint");
HD6Joint joint = jointSceneObject->AddComponent<D6Joint>();
~~~~~~~~~~~~~

### Degrees of freedom
The joint has six degrees of freedom as represented by the @b3d::D6JointAxis enum:
 - X - Movement on the X axis
 - Y - Movement on the Y axis
 - Z - Movement on the Z axis
 - Twist - Rotation around the X axis
 - SwingY - Rotation around the Y axis
 - SwingZ - Rotation around the Z axis

Each of those degrees can be in one of three states as represented by @b3d::D6JointMotion enum:
 - Free - Axis is not constrained at all
 - Locked - Axis is immovable
 - Limited - Axis is constrained according to the provided *Limit* object

Rotational degrees of freedom are partitioned as *twist* and *swing*. Different effects can be achieved by unlocking different combinations of them:
	- If a single degree of angular freedom is unlocked it should be the twist degree as it has extra options for that case (for example a hinge joint).
	- If both swing degrees are unlocked but twist is locked the result is a zero-twist joint.
	- If one swing and one twist degree of freedom are unlocked the result is a zero-swing joint (for example an arm attached at the elbow)
	- If all angular degrees of freedom are unlocked the result is the same as the spherical joint.

You can lock/unlock different axes by calling @b3d::D6Joint::SetMotion.

~~~~~~~~~~~~~{.cpp}
// Create an equivalent of a hinge joint
joint->SetMotion(D6JointAxis::X, D6JointMotion::Locked);
joint->SetMotion(D6JointAxis::Y, D6JointMotion::Locked);
joint->SetMotion(D6JointAxis::Z, D6JointMotion::Locked);
joint->SetMotion(D6JointAxis::Twist, D6JointMotion::Free);
joint->SetMotion(D6JointAxis::SwingY, D6JointMotion::Locked);
joint->SetMotion(D6JointAxis::SwingZ, D6JointMotion::Locked);
~~~~~~~~~~~~~

Query the motion for a specific axis:

~~~~~~~~~~~~~{.cpp}
D6JointMotion motion = joint->GetMotion(D6JointAxis::X);
~~~~~~~~~~~~~
	
### Limits
Each degree of freedom can also have a set of limits that constrain it further.

#### Linear limits
For translational degrees of freedom you use the @b3d::LimitLinear object. It contains an extent representing the maximum allowed distance (zero being the minimum). This is similar to distance and slider joints, except the minimum distance is always assumed to be zero. One limit is applied to all translational degrees of freedom.

Call @b3d::D6Joint::SetLimitLinear to apply the limit.

To enable the limit call **D6Joint::SetMotion()** with **D6JointMotion::Limited** flag for the relevant degree of freedom. You use this same approach to apply limits for all degrees of freedom.

~~~~~~~~~~~~~{.cpp}
// A limit representing a maximum distance of 20 units
LimitLinear limit;
limit.Extent = 20.0f;

// Unlock movement along the x axis, with the specified limit
joint->SetLimitLinear(limit);
joint->SetMotion(D6JointAxis::X, D6JointMotion::Limited);
~~~~~~~~~~~~~

Query the linear limit:

~~~~~~~~~~~~~{.cpp}
LimitLinear limit = joint->GetLimitLinear();
~~~~~~~~~~~~~

#### Twist limits
Use **LimitAngularRange** to define a limit for the twist degree of freedom. Call @b3d::D6Joint::SetLimitTwist to apply the limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a hinge that can swing a maximum of 90 degrees
LimitAngularRange limit;
limit.Lower = Degree(0.0f);
limit.Upper = Degree(90.0f);

joint->SetLimitTwist(limit);
joint->SetMotion(D6JointAxis::Twist, D6JointMotion::Limited);
~~~~~~~~~~~~~

Query the twist limit:

~~~~~~~~~~~~~{.cpp}
LimitAngularRange limit = joint->GetLimitTwist();
~~~~~~~~~~~~~

#### Swing limits
Use **LimitConeRange** to define a limit for the two swing degrees of freedom. Call @b3d::D6Joint::SetLimitSwing to apply the limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone
LimitConeRange limit;
limit.YLimitAngle = Degree(90.0f);
limit.ZLimitAngle = Degree(90.0f);

joint->SetLimitSwing(limit);
joint->SetMotion(D6JointAxis::SwingY, D6JointMotion::Limited);
joint->SetMotion(D6JointAxis::SwingZ, D6JointMotion::Limited);
~~~~~~~~~~~~~

Query the swing limit:

~~~~~~~~~~~~~{.cpp}
LimitConeRange limit = joint->GetLimitSwing();
~~~~~~~~~~~~~

### Drives
Each degree of freedom can optionally be assigned a drive, which applies either linear or angular force (depending on the degree of freedom).

You enable the drive for a specific degree of freedom by calling @b3d::D6Joint::SetDrive, and providing a parameter of type @b3d::D6JointDrive and the type of drive @b3d::D6JointDriveType.

Supported drive types are all the degrees of freedom, as well as a special @b3d::D6JointDriveType::SLERP drive type that performs rotation along all three axes at once.

**D6JointDrive** contains a set of properties:
 - @b3d::D6JointDrive::Stiffness - Similar purpose as with spring stiffness, except it is used for driving the object instead of stopping it.
 - @b3d::D6JointDrive::Damping - Similar purpose as with spring damping, except it is used for driving the object instead of stopping it.
 - @b3d::D6JointDrive::ForceLimit - Maximum force the drive is allowed to apply
 - @b3d::D6JointDrive::Acceleration - If true the drive will generate acceleration instead of forces. Acceleration drives are easier to tune as they account for the masses of the actors to which the joint is attached.

Finally you must call both @b3d::D6Joint::SetDriveTransform and @b3d::D6Joint::SetDriveVelocity. These methods accept the wanted position and rotation, as well as wanted linear and angular velocity. Once set the drive will apply forces to move towards that position and velocity, depending on the other parameters specified in **D6JointDrive**.

~~~~~~~~~~~~~{.cpp}
// Enable drive moving in X direction
D6JointDrive drive;
drive.Stiffness = 1.0f;
drive.Damping = 1.0f;

joint->SetDrive(D6JointDriveType::X, drive);

// Unlock the axis
joint->SetMotion(D6JointAxis::X, D6JointMotion::Free);

// Start the drive moving 50 units forward, at velocity of 1 unit per second
joint->SetDriveTransform(Vector3(0.0f, 0.0f, 50.0f), Quaternion::kIdentity);
joint->SetDriveVelocity(Vector3(0.0f, 0.0f, 1.0f), Vector3::kZero);
~~~~~~~~~~~~~

Query the drive for a specific degree of freedom:

~~~~~~~~~~~~~{.cpp}
D6JointDrive drive = joint->GetDrive(D6JointDriveType::X);
~~~~~~~~~~~~~

Query the drive transform and velocity:

~~~~~~~~~~~~~{.cpp}
Vector3 drivePosition;
Quaternion driveRotation;
joint->GetDriveTransform(drivePosition, driveRotation);

Vector3 driveLinearVelocity;
Vector3 driveAngularVelocity;
joint->GetDriveVelocity(driveLinearVelocity, driveAngularVelocity);
~~~~~~~~~~~~~

### Current value
You can find out the current angles of all the rotational degrees by calling @b3d::D6Joint::GetTwist, @b3d::D6Joint::GetSwingY or @b3d::D6Joint::GetSwingZ.

~~~~~~~~~~~~~{.cpp}
float twist = joint->GetTwist();
float swingY = joint->GetSwingY();
float swingZ = joint->GetSwingZ();
~~~~~~~~~~~~~

# Joint breaking
All joint types can be configured to break after a specific amount of force or torque is applied to them. Once a joint is broken it will no longer constrain the attached bodies.

To set the break force and torque call @b3d::Joint::SetBreakForce and @b3d::Joint::SetBreakTorque.

You can also get notified when the joint is broken by subscribing to the @b3d::Joint::OnJointBreak event.

~~~~~~~~~~~~~{.cpp}
// Break if force exceeds 500 units
joint->SetBreakForce(500.0f);

auto notify = []()
{
	B3D_LOG(Info, LogPhysics, "Joint broken!");
};

joint->OnJointBreak.Connect(notify);
~~~~~~~~~~~~~

Query the break force and torque:

~~~~~~~~~~~~~{.cpp}
float breakForce = joint->GetBreakForce();
float breakTorque = joint->GetBreakTorque();
~~~~~~~~~~~~~

# Joint collisions
By default two bodies attached to the joint will collide with each other. You can disable this by calling @b3d::Joint::SetEnableCollision.

~~~~~~~~~~~~~{.cpp}
// Disable collisions between two joint bodies
joint->SetEnableCollision(false);
~~~~~~~~~~~~~

Check if collisions are enabled:

~~~~~~~~~~~~~{.cpp}
bool collisionEnabled = joint->GetEnableCollision();
~~~~~~~~~~~~~
