Physics material						{#physicsMaterial}
===============
[TOC]

Physics material is a type of object that can be applied to a **Collider** to control the physical properties of its surface. In particular it can be used to control friction coefficients that determine how much damping is there when two objects are touching and moving laterally. As well as a restitution coefficient that determines how ellastic are collisions between two objects.

It is represented by @ref bs::PhysicsMaterial "PhysicsMaterial" and created by calling @ref bs::PhysicsMaterial::create "PhysicsMaterial::create()". It is a resource, and as such can be saved and loaded as described as any other resource.

~~~~~~~~~~~~~{.cpp}
// Create physics material with default properties
HPhysicsMaterial material = PhysicsMaterial::create();
~~~~~~~~~~~~~

Once created it can be applied to a collider by calling @ref bs::CCollider::setMaterial "CCollider::setMaterial()".

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HBoxCollider collider = colliderSO->addComponent<CBoxCollider>();

collider->setMaterial(material);
~~~~~~~~~~~~~

# Friction coefficients {#physicsMaterial_a}
Friction coefficients determine how much damping to the object's movement will occurr when objects are moving laterally to each other. Each object involved in friction can have its own physics material, and friction coefficients from them all will be accounted for.

There are two types of friction coefficients:
 - Static
 - Dynamic
 
## Static friction {#physicsMaterial_a_a}
Static friction coefficient determines how hard is to get an object moving from a stand-still, if it is touching another object. Use @ref bs::PhysicsMaterial::setStaticFriction "PhysicsMaterial::setStaticFriction()" to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// No static friction (e.g. ice)
material->setStaticFriction(0);

// A little bit of static friction (e.g. wooden floor)
material->setStaticFriction(1);
~~~~~~~~~~~~~

## Dynamic friction {#physicsMaterial_a_b}
Dynamic friction coefficient determines how much will a moving object be slowed down, when it is touching another object. Use @ref bs::PhysicsMaterial::setDynamicFriction "PhysicsMaterial::setDynamicFriction()" to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// No dynamic friction (e.g. ice)
material->setDynamicFriction(0);

// A little bit of dynamic friction (e.g. wooden floor)
material->setDynamicFriction(1);
~~~~~~~~~~~~~

# Restitution {#physicsMaterial_b}
Resitution coefficient controls the elasticity of collisions between two objects (i.e. how "bouncy" they are). Coefficient of 0 means the collisions are non-elastic, and coefficient of 1 means the collisions are fully elastic. Use @ref bs::PhysicsMaterial::setRestitutionCoefficient "PhysicsMaterial::setRestitutionCoefficient()" to set the coefficient.

~~~~~~~~~~~~~{.cpp}
// Barely elastic collisions (e.g. metal ball)
material->setRestitutionCoefficient(0.2f);

// Very elastic collisions (e.g. a basketball)
material->setRestitutionCoefficient(0.8f);
~~~~~~~~~~~~~

# Offsets {#physicsMaterial_c}
Offsets are not part of the **PhysicsMaterial** interface, but are rather set directly on a **Collider**. Since they also refer to properties of the collider's surface, they're included in this manual.

Offsets can be used to control the precision of the physics simulation when two objects are interacting. There are two types of offsets:
 - Contact
 - Rest
 
You generally only need to tweak offsets if you notice inter-penetration or gaps during collisions and while objects are touching.
 
## Contact offset {#physicsMaterial_c_a}
Contact offset determines a "buffer" that is formed around the surface of a collider. Collider will start colliding with other objects when their surfaces (modified by contact offset) start touching.

Generally you want to keep this at a low value, but not too low. When set to a high value the collisions may appear unrealistic as objects moving toward each other will start colliding before their actual collision bounds start touching.

When set to a too low value then the opposite may happen and the objects might start colliding too late, after they are already visually inter-penetrating.

It is generally a matter of tweaking the value to what looks best. Faster moving objects generally need larger contact offset values if inter-penetration is not acceptable. Smaller objects might need smaller contact offset values to avoid collision area much larger than the object.

Use @ref bs::CCollider::setContactOffset "CCollider::setContactOffset()" to change the offset.
 
~~~~~~~~~~~~~{.cpp}
// Contact offset of 0.01 units (e.g. one centimeter)
collider->setContactOffset(0.01f);
~~~~~~~~~~~~~
 
## Rest offset {#physicsMaterial_c_b}
Rest offset is similar to contact offset, as it also defines a "buffer" formed around the surface. But it is only used when physical objects are statically touching each other (e.g. a stack of boxes). It can be used to tweak the collision bounds to make the objects appear perfectly touching. If your collision bounds perfectly match the visual representation of the object, no rest offset adjustment is necessary. The value can be negative to reduce the collision bounds.

~~~~~~~~~~~~~{.cpp}
// No rest offset
collider->setRestOffset(0f);
~~~~~~~~~~~~~