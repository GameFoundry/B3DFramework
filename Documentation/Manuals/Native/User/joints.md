Joints 						{#joints}
===============

Joints allow you to constrain movement of two rigidbodies in some way. A typical example would be a door hinge. 

Banshee supports six different joint types:
 - Fixed - Locks origins and orientations together
 - Distance - Keeps origins within a certain distance range
 - Spherical - Keeps origins together but allows rotation with no restrictions (also known as ball-and-socket joint)
 - Hinge - Keeps origins together but allows rotation around X axis
 - Slider - Keeps orientations together but allows movement on the X axis
 - D6 - Fully customizable joint type that can be used for implementing all previously mentioned joints, as well as configure new types
 
All joints are components derived from @ref bs::CJoint "Joint". Before we talk about individual joint types, lets cover the functionality shared by all joints.

# Basics
All joints require two bodies to influence. You can also choose to specify *null* for one of the bodies, in which case the other body is assumed to move relative to an immovable reference frame. Note that at least one body attached to a joint must be movable.

To assign bodies to a joint call @ref bs::CJoint::setBody "CJoint::setBody()". The method accepts a @ref bs::JointBody "JointBody" parameter specifiying which of the two bodies two assign the value to, and a **Rigidbody** component representing the body (or null).

~~~~~~~~~~~~~{.cpp}
// Create a rigidbody to manipulate with the joint
HSceneObject rigidbodySO = SceneObject::create("Rigidbody");
HRigidbody rigidbody = rigidbodySO->addComponent<CRigidbody>();

HBoxCollider boxCollider = rigidbodySO->addComponent<CBoxCollider>();
boxCollider->setExtents(Vector3(0.5f, 0.5f, 0.5f));

// Add a fixed joint (covered later)
HSceneObject jointSO = SceneObject::create("Joint");
HJoint joint = jointSO->addComponent<CFixedJoint>();

// Move the body so its not at the same position as the joint
rigidbodySO->setPosition(Vector3(0, 5, 0));

// Add a body to the joint. Since we didn't add a second body this one will be anchored to an immovable reference frame
joint->setBody(JointBody::Target, rigidbody);

// This joint will now ensure the origins and orientations of the rigidbody and joint remain constant. In this particular case since we attached the body to an immovable reference frame using a fixed joint, we have made the body immovable.
~~~~~~~~~~~~~

# Joint types

## Fixed joint
Fixed joints are the simplest joint types. As the name implies they fix the origins and orientations of two bodies together. For example imagine two wheels welded together with a metal bar - when one rotates so must the other, and when one moves so must the other.

@ref TODO_IMAGE

They are represented with the @ref bs::CFixedJoint "FixedJoint" component.They don't allow any additional properties to be set, aside from the bodies to influence.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HFixedJoint joint = jointSO->addComponent<CFixedJoint>();
~~~~~~~~~~~~~

## Distance joint
Distance joint simply keeps two bodies together at the specified distance range. You can specify a minimum and a maximum distance, as well as a spring property that makes the objects spring back or forth as they reach the range limits. For example you might use this joint type to model a rope.

@ref TODO_IMAGE

Distance joints are represented using the @ref bs::CDistanceJoint "DistanceJoint" component. 

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HDistanceJoint joint = jointSO->addComponent<CDistanceJoint>();
~~~~~~~~~~~~~

### Limits
To specify the distance range call @ref bs::CDistanceJoint::setMinDistance "CDistanceJoint::setMinDistance()" and @ref bs::CDistanceJoint::setMaxDistance "CDistanceJoint::setMaxDistance()".

You must also explicitly enable the distance limits by calling @ref bs::CDistanceJoint::setFlag "CDistanceJoint::setFlag()" with @ref bs::DistanceJoint::Flag::MinDistance "DistanceJoint::Flag::MinDistance" and @ref bs::DistanceJoint::Flag::MaxDistance "DistanceJoint::Flag::MaxDistance" flags. This allows you to only enable one or another, in case no restrictions are needed in one extreme.

~~~~~~~~~~~~~{.cpp}
// Keep the bodies in between 5 and 20 units between each other
joint->setMinDistance(5.0f);
joint->setMaxDistance(20.0f);

// Enable the limits
joint->setFlag(DistanceJoint::Flag::MinDistance, true);
joint->setFlag(DistanceJoint::Flag::MaxDistance, true);
~~~~~~~~~~~~~

### Spring
By default when the objects reach the specified limits they will come to a dead stop. Generally it's more realistic to have them be pushed back towards valid range slowly, in which case you can apply a spring parameter by calling @ref bs::CDistanceJoint::setSpring "CDistanceJoint::setSpring()".

It accepts an object of type @ref bs::Spring "Spring" which has two properties:
 - @ref bs::Spring::stiffness "Spring::stiffness" - Determines the strength of the spring when greater than zero. 
 - @ref bs::Spring::damping "Spring::damping" - Determines the damping of the spring when greater than zero.
 
You must also explicitly enable springs by calling @ref bs::CDistanceJoint::setFlag "CDistanceJoint::setFlag()" with the @ref bs::DistanceJoint::Flag::Spring "DistanceJoint::Flag::Spring" parameter.

~~~~~~~~~~~~~{.cpp}
Spring spring(1.0f, 1.0f);

joint->setSpring(spring);
joint->setFlag(DistanceJoint::Flag::Spring, true);
~~~~~~~~~~~~~

### Tolerance
To ensure springs have enough time to activate, so they can begin pushing back the object before it actually breaches the valid range, you can specify an additional tolerance parameter. This parameter adds a "buffer" on the inner edges of the valid range, at which the spring will activate.

Use @ref bs::CDistanceJoint::setTolerance "CDistanceJoint::setTolerance()" to set the tolerance. This property is only relevant if using the spring.

### Current value
Sometimes it is useful to find out how much is the joint currently "stretched". Call @ref bs::CDistanceJoint::getDistance "CDistanceJoint::getDistance()" to get the current distance between its two bodies.

## Spherical joint
Spherical joint allows for a full range of rotations, while keeping the origins of the two bodies together. This allows you to create a ball-and-socket functionality, for example.

@ref TODO_IMAGE

Spherical joint is represented with the @ref bs::CSphericalJoint "SphericalJoint" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HSphericalJoint joint = jointSO->addComponent<CSphericalJoint>();
~~~~~~~~~~~~~

### Limits
You can limit the rotation along Y and Z axes by specifying the limit angles in @ref bs::LimitConeRange "LimitConeRange". As the name implies the angles represent the cone in which rotation is valid.

You apply the limits by calling @ref bs::CSphericalJoint::setLimit "CSphericalJoint::setLimit()", and enabling the limit by calling @ref bs::CSphericalJoint::setFlag "CSphericalJoint::setFlag()" with @ref bs::SphericalJoint::Flag::Limit "SphericalJoint::Flag::Limit".

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone
LimitConeRange limit;
limit.yLimitAngle = Degree(90.0f);
limit.zLimitAngle = Degree(90.0f);

joint->setLimit(limit);
joint->setFlag(SphericalJoint::Flag::Limit, true);
~~~~~~~~~~~~~

Rotation around the X axis cannot be limited using this joint type and is always free.

### Spring
**LimitConeRange** also contains a @ref bs::LimitConeRange::restitution "LimitConeRange::restitution" parameter, which has the same meaning as in **PhysicsMaterial**. If restitution is zero the body will just stop when it reaches the limit. If it is non-zero the object will bounce instead.

You can also specify as **Spring** parameter as @ref bs::LimitConeRange::spring "LimitConeRange::spring", which controls how is the body pulled back after the limit has been breached. Its values have the same meaning as we described earlier.

Finally, you can specify a contact distance parameter as @ref bs::LimitConeRange::contactDist "LimitConeRange::contactDist", which ensures springs have enough time to activate before the body goes past the valid range. This is similar to the tolerance value we talked about regarding distance joints.

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone, with a soft limit
LimitConeRange limit;
limit.yLimitAngle = Degree(90.0f);
limit.zLimitAngle = Degree(90.0f);

limit.restitution = 0.8f;
limit.contactDist = -1.0f;
limit.spring.stiffness = 1.0f;
limit.spring.damping = 1.0f;

joint->setLimit(limit);
joint->setFlag(SphericalJoint::Flag::Limit, true);
~~~~~~~~~~~~~

Note that all the joint types that are about to follow have these four properties in their *Limit* structures, so we won't talk about them any further.

## Slider joint
As the name implies slider joint constrains the body movement to slide along a single axis (X axis in particular). No movement along other axes, or rotation is allowed. For example this could be used for modeling some kind of a piston moving up and down.

@ref TODO_IMAGE

It is represented using the @ref bs::CSliderJoint "SliderJoint" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HSliderJoint joint = jointSO->addComponent<CSliderJoint>();
~~~~~~~~~~~~~

### Limits
@ref bs::LimitLinearRange "LimitLinearRange" can be used to specify minimum and maximum allowed distance between the two bodies (similar to the distance joint).

You apply the limit by calling @ref bs::CSliderJoint::setLimit "CSliderJoint::setLimit()", and enable the limit by calling @ref bs::CSliderJoint::setFlag "CSliderJoint::setFlag()" with @ref bs::SliderJoint::Flag::Limit "SliderJoint::Flag::Limit".

~~~~~~~~~~~~~{.cpp}
// A limit representing a distance range [5, 20] units
LimitLinearRange limit;
limit.lower = 5.0f;
limit.upper = 20.0f;

joint->setLimit(limit);
joint->setFlag(SliderJoint::Flag::Limit, true);
~~~~~~~~~~~~~

### Current value
You can find out the current position of the slider by calling @ref bs::CSliderJoint::getPosition "CSliderJoint::getPosition()", and the current speed of the slider by calling @ref bs::CSliderJoint::getSpeed "CSliderJoint::getSpeed()".

## Hinge joint
Hinge joint contrains rotation around a single axis (X axis specifically). Rotation around other axes, or translation is not allowed. This can be used for modeling door hinges or similar behaviour.

@ref TODO_IMAGE

It is represented using the @ref bs::CHingeJoint "HingeJoint" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HHingeJoint joint = jointSO->addComponent<CHingeJoint>();
~~~~~~~~~~~~~

### Limits
@ref bs::LimitAngularRange "LimitAngularRange" can be used to specify minimum and maximum allowed angle between the two bodies.

You apply the limit by calling @ref bs::CHingeJoint::setLimit "CHingeJoint::setLimit()", and enable the limit by calling @ref bs::CHingeJoint::setFlag "CHingeJoint::setFlag()" with @ref bs::HingeJoint::Flag::Limit "HingeJoint::Flag::Limit".

~~~~~~~~~~~~~{.cpp}
// A limit representing a hinge that can swing a maximum of 90 degrees
LimitAngularRange limit;
limit.lower = Degree(0.0f);
limit.upper = Degree(90.0f);

joint->setLimit(limit);
joint->setFlag(HingeJoint::Flag::Limit, true);
~~~~~~~~~~~~~

### Drive
Drive is a special object of type @ref bs::HingeJoint::Drive "HingeJoint::Drive" that can be assigned to a hinge joint, to make the joint move without external forces. For example if you wanted to make a propeller, you could set up a drive on the hinge joint so it keeps on spinning.

Drive can be assigned through @ref bs::CHingeJoint::setDrive "CHingeJoint::setDrive()", and must be explicitly enabled by calling **CHingeJoint::setFlag()** with @ref bs::HingeJoint::Flag::Drive "HingeJoint::Flag::Drive".

**HingeJoint::Drive** object has a few properties:
 - @ref bs::HingeJoint::Drive::speed "HingeJoint::Drive::speed" - Speed at which the drive will try to spin at
 - @ref bs::HingeJoint::Drive::forceLimit "HingeJoint::Drive::forceLimit" - Optional maximum force the drive is allowed to apply
 - @ref bs::HingeJoint::Drive::freeSpin "HingeJoint::Drive::freeSpin" - If the joint ends up moving faster than the drive (due to an external force), the drive will try to break in order to bring it down to drive's speed. If you don't want that to happen enable free spin.
 - @ref bs::HingeJoint::Drive::gearRatio "HingeJoint::Drive::gearRatio" - Scales the velocity of the target body by this value.

### Current value
You can find out the current angle of the hinge by calling @ref bs::CHingeJoint::getAngle "CHingeJoint::getAngle()", and the current angular speed of the joint by calling @ref bs::CHingeJoint::getSpeed "CHingeJoint::getSpeed()".

## D6 joint
D6 is the most complex, and the most customizable type of joint. It can be used to create all joint types we have talked about so far, and to configure new custom joint types.

It is represented using the @ref bs::D6Joint "D6Joint" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject jointSO = SceneObject::create("Joint");
HD6Joint joint = jointSO->addComponent<CD6Joint>();
~~~~~~~~~~~~~

### Degrees of freedom
The joint has six degrees of freedom as represented by the @ref bs::D6Joint::Axis "D6Joint::Axis" enum:
 - X - Movement on the X axis
 - Y - Movement on the Y axis
 - Z - Movement on the Z axis
 - Twist - Rotation around the X axis
 - SwingY - Rotation around the Y axis
 - SwingZ - Rotation around the Z axis
 
Each of those degrees can be in one of three states as represented by @ref bs::D6Joint::Motion "D6Joint::Motion" enum:
 - Free - Axis is not constrained at all
 - Locked - Axis is immovable
 - Limited - Axis is constrained according to the provided *Limit* object
 
Rotational degrees of freedom are partitioned as *twist* and *swing*. Different effects can be achieved be unlocking different combinations of them:
	- If a single degree of angular freedom is unlocked it should be the twist degree as it has extra options for that case (for example a hinge joint).
	- If both swing degrees are unlocked but twist is locked the result is a zero-twist joint. 
	- If one swing and one twist degree of freedom are unlocked the result is a zero-swing joint (for example an arm attached at the elbow)
	- If all angular degrees of freedom are unlocked the result is the same as the spherical joint.

You can lock/unlock different axes by calling @ref bs::CD6Joint::setMotion "CD6Joint::setMotion()".

~~~~~~~~~~~~~{.cpp}
// Create an equivalent of a hinge joint
joint->setMotion(D6Joint::Axis::X, D6Joint::Motion::Locked);
joint->setMotion(D6Joint::Axis::Y, D6Joint::Motion::Locked);
joint->setMotion(D6Joint::Axis::Z, D6Joint::Motion::Locked);
joint->setMotion(D6Joint::Axis::Twist, D6Joint::Motion::Free);
joint->setMotion(D6Joint::Axis::SwingY, D6Joint::Motion::Locked);
joint->setMotion(D6Joint::Axis::SwingZ, D6Joint::Motion::Locked);
~~~~~~~~~~~~~
	
### Limits
Each degree of freedom can also have a set of limits that constrain it further. 

#### Linear limits
For translational degrees of freedom you use the @ref bs::LimitLinear "LimitLinear" object. It contains an extent representing the maximum allowed distance (zero being the minimum). This is similar to distance and slider joints, except the minimum distance is always assumed to be zero. One limit is applied to all translational degrees of freedom.

Call @ref bs::CD6Joint::setLimitLinear "CD6Joint::setLimitLinear()" to apply the limit.

To enable the limit call **CD6Joint::setMotion()** with **D6Joint::Motion::Limited** flag for the relevant degree of freedom. You use this same approach to apply limits for all degrees of freedom.

~~~~~~~~~~~~~{.cpp}
// A limit representing a maximum distance of 20 units
LimitLinear limit;
limit.extent = 20.0f;

// Unlock movement along the x axis, with the specified limit
joint->setLimitLinear(limit);
joint->setMotion(D6Joint::Axis::X, D6Joint::Motion::Limited);
~~~~~~~~~~~~~

#### Twist limits
Use **LimitAngularRange** to define a limit for the twist degree of freedom. Call @ref bs::CD6Joint::setLimitTwist "CD6Joint::setLimitTwist()" to apply the limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a hinge that can swing a maximum of 90 degrees
LimitAngularRange limit;
limit.lower = Degree(0.0f);
limit.upper = Degree(90.0f);

joint->setLimitTwist(limit);
joint->setMotion(D6Joint::Axis::Twist, D6Joint::Motion::Limited);
~~~~~~~~~~~~~

#### Swing limits
Use **LimitConeRange** to define a limit for the two swing degrees of freedom. Call @ref bs::CD6Joint::setLimitSwing "CD6Joint::setLimitSwing()" to apply the limit.

~~~~~~~~~~~~~{.cpp}
// A limit representing a 90 degree cone
LimitConeRange limit;
limit.yLimitAngle = Degree(90.0f);
limit.zLimitAngle = Degree(90.0f);

joint->setLimitSwing(limit);
joint->setMotion(D6Joint::Axis::SwingY, D6Joint::Motion::Limited);
joint->setMotion(D6Joint::Axis::SwingZ, D6Joint::Motion::Limited);
~~~~~~~~~~~~~

### Drives
Each degree of freedom can optionally be assigned a drive, which applies either linear or angular force (depending on the degree of freedom).

You enable the drive for a specific degree of freedom by calling @ref bs::CD6Joint::setDrive "CD6Joint::setDrive()", and providing a parameter of type @ref bs::D6Joint::Drive "D6Joint::Drive" and the type of drive @ref bs::D6Joint::DriveType "D6Joint::DriveType".

Supported drive types are all the degrees of freedom, as well as a special @ref bs::D6Joint::DriveType::SLERP "D6Joint::DriveType::SLERP" drive type that performs rotation along all three axes at once.

**D6Joint::Drive** contains a set of properties:
 - @ref bs::D6Joint::Drive::stiffness "D6Joint::Drive::stiffness" - Similar purpose as with string stiffness, except it is used for driving the object instead of stopping it.
 - @ref bs::D6Joint::Drive::damping "D6Joint::Drive::damping" - Similar purpose as with string damping, except it is used for driving the object instead of stopping it.
 - @ref bs::D6Joint::Drive::forceLimit "D6Joint::Drive::forceLimit" - Maximum force the drive is allowed to apply
 - @ref bs::D6Joint::Drive::acceleration "D6Joint::Drive::acceleration" - If true the drive will generate acceleration instead of forces. Acceleration drives are easier to tune as they account for the masses of the actors to which the joint is attached.

Finally you must call both @ref bs::CD6Joint::setDriveTransform "CD6Joint::setDriveTransform()" and @ref bs::CD6Joint::setDriveVelocity "CD6Joint::setDriveVelocity()". These methods accept the wanted position and rotation, as well as wanted linear and angular velocity. Once set the drive will apply forces to move towards that position and velocity, depending on the other parameters specified in **D6Joint::Drive**.
 
~~~~~~~~~~~~~{.cpp}
// Enable drive moving in X direction
D6Joint::Drive drive;
drive.stiffness = 1.0f;
drive.damping = 1.0f;

joint->setDrive(D6Joint::DriveType::X, drive);

// Unlock the axis
joint->setMotion(D6Joint::Axis::X, D6Joint::Motion::Free);

// Start the drive moving 50 units forward, at velocity of 1 unit per second
joint->setDriveTransform(Vector3(0, 0, 50), Quaternion::IDENTITY);
joint->setDriveVelocity(Vector3(0, 0, 1), Vector3(0, 0, 0));
~~~~~~~~~~~~~

### Current value
You can find out the current angles of all the rotational degrees by calling @ref bs::CD6Joint::getTwist "CD6Joint::getTwist()", @ref bs::CD6Joint::getSwingY "CD6Joint::getSwingY()" or @ref bs::CD6Joint::getSwingZ "CD6Joint::getSwingZ()".

# Joint breaking
All joint types can be configured to break after a specific amount of force or torque is applied to them. Once a joint is broken it will no longer constrain the attached bodies.

To set the break force and torque call @ref bs::CJoint::setBreakForce "CJoint::setBreakForce()" and @ref bs::CJoint::setBreakTorque "CJoint::setBreakTorque()".

You can also get notified when the joint is broken by subscribing to the @ref bs::CJoint::onJointBreak "CJoint::onJointBreak" event.

~~~~~~~~~~~~~{.cpp}
// Break if force exceeds 500 units
joint->setBreakForce(500.0f);

auto notify = []()
{
	gDebug().logDebug("Joint broken!");
};

joint->onJointBreak.connect(notify);
~~~~~~~~~~~~~

# Joint collisions
By default two bodies attached to the joint will collide with each other. You can disable this by calling @ref bs::CJoint::setEnableCollision "CJoint::setEnableCollision()".

~~~~~~~~~~~~~{.cpp}
// Disable collisions between two joint bodies
joint->setEnableCollision(false);
~~~~~~~~~~~~~
