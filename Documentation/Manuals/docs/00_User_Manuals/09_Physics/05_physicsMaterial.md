---
title: Physics material
---

Physics material is a type of object that can be applied to a **Collider** to control the physical properties of its surface. In particular, it can be used to control friction coefficients that determine how much damping occurs when two objects are touching and moving laterally, as well as a restitution coefficient that determines how elastic collisions are between two objects.

It is represented by @b3d::PhysicsMaterial and created by calling @b3d::PhysicsMaterial::Create. It is a resource, and as such can be saved and loaded as any other resource.

~~~~~~~~~~~~~{.cpp}
// Create physics material with default properties
HPhysicsMaterial physicsMaterial = PhysicsMaterial::Create();
~~~~~~~~~~~~~

Once created, it can be applied to a collider by calling @b3d::Collider::SetMaterial.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HBoxCollider boxCollider = colliderSceneObject->AddComponent<BoxCollider>();

boxCollider->SetMaterial(physicsMaterial);
~~~~~~~~~~~~~

Query the material assigned to a collider:

~~~~~~~~~~~~~{.cpp}
HPhysicsMaterial currentMaterial = boxCollider->GetMaterial();
~~~~~~~~~~~~~

# Friction coefficients
Friction coefficients determine how much damping to the object's movement will occur when objects are moving laterally to each other. Each object involved in friction can have its own physics material, and friction coefficients from them all will be accounted for.

There are two types of friction coefficients:
 - **Static** - Resistance to starting motion
 - **Dynamic** - Resistance to continued motion

## Static friction
Static friction coefficient determines how hard it is to get an object moving from a standstill if it is touching another object. Use @b3d::PhysicsMaterial::SetStaticFriction to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// No static friction (e.g. ice)
physicsMaterial->SetStaticFriction(0.0f);

// A little bit of static friction (e.g. wooden floor)
physicsMaterial->SetStaticFriction(1.0f);
~~~~~~~~~~~~~

Query static friction:

~~~~~~~~~~~~~{.cpp}
float staticFriction = physicsMaterial->GetStaticFriction();
~~~~~~~~~~~~~

## Dynamic friction
Dynamic friction coefficient determines how much a moving object will be slowed down when it is touching another object. Use @b3d::PhysicsMaterial::SetDynamicFriction to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// No dynamic friction (e.g. ice)
physicsMaterial->SetDynamicFriction(0.0f);

// A little bit of dynamic friction (e.g. wooden floor)
physicsMaterial->SetDynamicFriction(1.0f);
~~~~~~~~~~~~~

Query dynamic friction:

~~~~~~~~~~~~~{.cpp}
float dynamicFriction = physicsMaterial->GetDynamicFriction();
~~~~~~~~~~~~~

# Restitution
Restitution coefficient controls the elasticity of collisions between two objects (i.e. how "bouncy" they are). Coefficient of 0 means the collisions are non-elastic, and coefficient of 1 means the collisions are fully elastic. Use @b3d::PhysicsMaterial::SetRestitutionCoefficient to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// Barely elastic collisions (e.g. metal ball)
physicsMaterial->SetRestitutionCoefficient(0.2f);

// Very elastic collisions (e.g. a basketball)
physicsMaterial->SetRestitutionCoefficient(0.8f);
~~~~~~~~~~~~~

Query restitution coefficient:

~~~~~~~~~~~~~{.cpp}
float restitution = physicsMaterial->GetRestitutionCoefficient();
~~~~~~~~~~~~~

# Offsets
Offsets are not part of the **PhysicsMaterial** interface but are rather set directly on a **Collider**. Since they also refer to properties of the collider's surface, they're included in this manual.

Offsets can be used to control the precision of the physics simulation when two objects are interacting. There are two types of offsets:
 - **Contact** - Buffer zone when collisions begin
 - **Rest** - Buffer zone when objects are at rest

You generally only need to tweak offsets if you notice inter-penetration or gaps during collisions and while objects are touching.

## Contact offset
Contact offset determines a "buffer" that is formed around the surface of a collider. The collider will start colliding with other objects when their surfaces (modified by contact offset) start touching.

Generally, you want to keep this at a low value, but not too low. When set to a high value, the collisions may appear unrealistic as objects moving toward each other will start colliding before their actual collision bounds start touching.

When set to too low a value, the opposite may happen and the objects might start colliding too late, after they are already visually inter-penetrating.

It is generally a matter of tweaking the value to what looks best. Faster-moving objects generally need larger contact offset values if inter-penetration is not acceptable. Smaller objects might need smaller contact offset values to avoid a collision area much larger than the object.

Use @b3d::Collider::SetContactOffset to change the offset.

~~~~~~~~~~~~~{.cpp}
// Contact offset of 0.01 units (e.g. one centimeter)
boxCollider->SetContactOffset(0.01f);
~~~~~~~~~~~~~

Query contact offset:

~~~~~~~~~~~~~{.cpp}
float contactOffset = boxCollider->GetContactOffset();
~~~~~~~~~~~~~

## Rest offset
Rest offset is similar to contact offset, as it also defines a "buffer" formed around the surface. But it is only used when physical objects are statically touching each other (e.g. a stack of boxes). It can be used to tweak the collision bounds to make the objects appear perfectly touching. If your collision bounds perfectly match the visual representation of the object, no rest offset adjustment is necessary. The value can be negative to reduce the collision bounds.

~~~~~~~~~~~~~{.cpp}
// No rest offset
boxCollider->SetRestOffset(0.0f);
~~~~~~~~~~~~~

Query rest offset:

~~~~~~~~~~~~~{.cpp}
float restOffset = boxCollider->GetRestOffset();
~~~~~~~~~~~~~
