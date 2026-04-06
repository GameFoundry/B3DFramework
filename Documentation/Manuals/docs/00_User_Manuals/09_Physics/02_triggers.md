---
title: Triggers
---

Aside from participating in collisions with other physical objects, colliders can also report such collisions to the programmer, allowing you to add functionality that triggers when a specific action occurs. The collider can report when:
 - Collision began
 - Collision ended
 - Collision is in progress

This is handled through the following events:
 - @b3d::Collider::OnCollisionBegin - Triggered whenever the collider starts colliding with another object.
 - @b3d::Collider::OnCollisionEnd - Triggered whenever the collider no longer collides (touches) an object.
 - @b3d::Collider::OnCollisionStay - Triggered every frame while the collider is in collision with (is touching) an object.

By default, these events will not be triggered until they are enabled by calling @b3d::Collider::SetCollisionReportMode with a parameter of type @b3d::CollisionReportMode. It can have one of the following values:
 - **CollisionReportMode::None** - None of the events will trigger.
 - **CollisionReportMode::Report** - Only begin and end events will trigger.
 - **CollisionReportMode::ReportPersistent** - Begin and end events will trigger, as well as the stay event.
 
Each of the event callbacks will provide as a parameter a @b3d::CollisionData structure, which contains various relevant information about the collision. It contains:
 - **Collider** - Array of two collider components that interacted
 - **ColliderShapes** - Array of two collider shapes that collided
 - **ContactPoints** - Set of contact points at which the collider shapes are touching

Each @b3d::ContactPoint in the contact points array provides:
 - **Position** - Contact point in world space
 - **Normal** - Normal pointing from the second shape to the first shape
 - **Impulse** - Impulse applied to the objects to keep them from penetrating (divide by simulation step to get force)
 - **Separation** - Distance between the objects (negative value denotes penetration)

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HBoxCollider boxCollider = colliderSceneObject->AddComponent<BoxCollider>();

auto collisionStarted = [](const CollisionData& data)
{
	HCollider otherCollider = data.Collider[1];
	String otherSceneObjectName = otherCollider->SO()->GetName();

	Vector3 contactPoint = data.ContactPoints[0].Position;
	Vector3 contactNormal = data.ContactPoints[0].Normal;
	float impulse = data.ContactPoints[0].Impulse;

	B3D_LOG(Info, LogPhysics, "Started colliding with {0} at point {1}", otherSceneObjectName, contactPoint);
	B3D_LOG(Info, LogPhysics, "Contact normal: {0}, Impulse: {1}", contactNormal, impulse);
};

boxCollider->SetCollisionReportMode(CollisionReportMode::Report);
boxCollider->OnCollisionBegin.Connect(collisionStarted);
~~~~~~~~~~~~~

Handle collision end and stay events:

~~~~~~~~~~~~~{.cpp}
auto collisionEnded = [](const CollisionData& data)
{
	HCollider otherCollider = data.Collider[1];
	B3D_LOG(Info, LogPhysics, "Stopped colliding with {0}", otherCollider->SO()->GetName());
};

auto collisionStay = [](const CollisionData& data)
{
	// Called every frame while colliding
	u32 contactCount = static_cast<u32>(data.ContactPoints.size());
	B3D_LOG(Info, LogPhysics, "Still colliding with {0} contact points", contactCount);
};

boxCollider->OnCollisionEnd.Connect(collisionEnded);
boxCollider->OnCollisionStay.Connect(collisionStay);

// Enable persistent reporting to receive OnCollisionStay events
boxCollider->SetCollisionReportMode(CollisionReportMode::ReportPersistent);
~~~~~~~~~~~~~

Query collision report mode:

~~~~~~~~~~~~~{.cpp}
CollisionReportMode mode = boxCollider->GetCollisionReportMode();
~~~~~~~~~~~~~

# Pure triggers
In some cases you might only be interested in trigger events reported by a collider, without requiring the collider to be an actual physical object. This way you can set up "invisible" triggers within game levels that start executing code when the player enters their bounds or interacts with them in some other way. Physical objects will go through such colliders as if they are not there, but the events will be reported just the same.

To do this, call @b3d::Collider::SetIsTrigger:

~~~~~~~~~~~~~{.cpp}
boxCollider->SetIsTrigger(true);
~~~~~~~~~~~~~

Check if a collider is a trigger:

~~~~~~~~~~~~~{.cpp}
bool isTrigger = boxCollider->GetIsTrigger();
~~~~~~~~~~~~~

When a collider is set as a trigger, it will not participate in physical collisions, but all collision events will still fire when other physical objects pass through it. This is particularly useful for creating detection zones, level triggers, or checkpoint areas.

~~~~~~~~~~~~~{.cpp}
// Create a trigger zone
HSceneObject triggerZoneSceneObject = SceneObject::Create("TriggerZone");
HBoxCollider triggerZone = triggerZoneSceneObject->AddComponent<BoxCollider>();

// Make it a trigger
triggerZone->SetIsTrigger(true);

// Set up the trigger zone size
triggerZone->SetExtents(Vector3(5.0f, 2.0f, 5.0f));

// Handle objects entering the trigger
auto onTriggerEnter = [](const CollisionData& data)
{
	HCollider other = data.Collider[1];
	B3D_LOG(Info, LogPhysics, "Object entered trigger zone: {0}", other->SO()->GetName());
};

triggerZone->SetCollisionReportMode(CollisionReportMode::Report);
triggerZone->OnCollisionBegin.Connect(onTriggerEnter);
~~~~~~~~~~~~~
