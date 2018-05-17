Triggers						{#triggers}
===============
[TOC]

Aside from participating in collisions with other physical objects, colliders can also report such collisions to the programmer, allowing the programmer to add functionality that triggers when a specific action occurs. The collider can report when:
 - Collision began
 - Collision ended
 - Collision is in progress
 
This is handled through the following events:
 - @ref bs::CCollider::onCollisionBegin "CCollider::onCollisionBegin" - Triggered whenever the collider starts colliding with another object.
 - @ref bs::CCollider::onCollisionEnd "CCollider::onCollisionEnd" - Triggered whenever the collider no longer collides (touches) an object.
 - @ref bs::CCollider::onCollisionStay "CCollider::onCollisionStay" - Triggered every frame while the collider is in collision with (is touching) an object.
 
By default those events will not be triggered until they are enabled by calling @ref bs::CCollider::setCollisionReportMode "CCollider::setCollisionReportMode()", with the parameter of type @ref bs::CollisionReportMode "CollisionReportMode". It can have one of the following values:
 - **CollisionReportMode::None** - None of the events will trigger.
 - **CollisionReportMode::Report** - Only begin and end events will trigger.
 - **CollisionReportMode::ReportPersistent** - Begin/end events will trigger, as well as the *stay* event.
 
Each of the event callbacks will provide as a parameter a @ref bs::CollisionData "CollisionData" structure, which contains various relevant information about the collision. It contains:
 - References to components of the colliders that interacted
 - A set of contact points at which the collider shapes are touching
  - Each contact point provides a position & normal, impulse applied during collision and distance between the objects
 
~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HBoxCollider collider = colliderSO->addComponent<CBoxCollider>();

auto collisionStarted = [](const CollisionData& data)
{
	HCollider otherCollider = data.collider[1];
	String otherSceneObjectName = otherCollider->SO()->getName();
	
	Vector3 contactPoint = data.contactPoints[0].position;
	gDebug().logDebug("Started colliding with " + otherSceneObjectName + " at point " + toString(contactPoint));
};

collider->setCollisionReportMode(CollisionReportMode::Report);
collider->onCollisionBegin.connect(collisionStarted);
~~~~~~~~~~~~~

# Pure triggers {#triggers_a}
In some cases you might only be interested in trigger events reported by a collider, without requiring the collider to be an actual physical object. This way you can set up "invisible" triggers within game levels that start executing code when player enters their bounds or interacts with them in some other way. Physical objects will go through such colliders as if they are not there - but the events will be reported just the same.

To do this call @ref bs::CCollider::setIsTrigger() "CCollider::setIsTrigger()".

~~~~~~~~~~~~~{.cpp}
collider->setIsTrigger(true);
~~~~~~~~~~~~~