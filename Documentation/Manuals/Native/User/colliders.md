Colliders 						{#colliders}
===============

Colliders are a type of component that describe a physical surface. Other physical objects can't move through them, and as their name implies they will instead collide with them in a physically realistic way. Colliders are represented as components, and can be added/removed in the scene as any other component. 

They are static physical objects, and they cannot be moved under the influence of another physical object. Moving the scene object they are attached to will also not result in realistic physical interaction either. We'll show how to create dynamic physical objects in later chapters.

For example you would use colliders to set up the floors of your level so they can be walked on, or set up the walls and obstacles so they can't be walked through.

> Note that physical objects like colliders only affect other physical objects (other colliders or other types as described later). You can still move objects through colliders if they don't have any physical objects attached. You can think of all physical objects as a closed system that only affects its own components.

There are five collider types, that differ in the way how is their surface described:
 - Plane - The surface is an non-bounded plane
 - Box - The surface is a box with custom width/height/depth
 - Sphere - The surface is a sphere with a radius
 - Capsule - The surface is a capsule with a radius and a height
 - Mesh - The surface is represented by custom geometry using with a triangle mesh
  
# Plane collider
Represented by @ref bs::CPlaneCollider "PlaneCollider" component. Use @ref bs::CPlaneCollider::setNormal "CPlaneCollider::setNormal()" to provide the direction in which the plane is oriented in, and @ref bs::CPlaneCollider::setDistance "CPlaneCollider::setDistance()" to provide an offset along that direction. Using these two properties you can position and orient a plane anywhere in the scene.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HPlaneCollider collider = colliderSO->addComponent<CPlaneCollider>();

// A plane pointing up, parallel to the XZ plane, 10 units beneath the origin
collider->setNormal(Vector3(0, 1, 0));
collider->setDistance(-10.0f);
~~~~~~~~~~~~~

> Note that plane's position and orientation is also influenced by the scene object the component is attached to. All colliders are influenced by their scene object, but most also provide properties for modifying their position/orientation/scale locally for more control. You can use either method, or both.

![Plane collider](PlaneCollider.png)  

# Box collider
Represented by @ref bs::CBoxCollider "BoxCollider" component. Use @ref bs::CBoxCollider::setExtents "CBoxCollider::setExtents()" to provide the extents (half size) of the box. Use @ref bs::CBoxCollider::setCenter "CBoxCollider::setCenter()" to offset the box relative to the scene object.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HBoxCollider collider = colliderSO->addComponent<CBoxCollider>();

// Box 1x1x1 in size, centered at its scene object position
collider->setExtents(Vector3(0.5f, 0.5f, 0.5f));
collider->setCenter(Vector3(0.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~

![Box collider](BoxCollider.png)  

# Sphere collider
Represented by @ref bs::CSphereCollider "SphereCollider" component. Use @ref bs::CSphereCollider::setRadius "CSphereCollider::setRadius()" to provide the radius of the sphere.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HSphereCollider collider = colliderSO->addComponent<CSphereCollider>();

// A unit sphere
collider->setRadius(1.0f);
~~~~~~~~~~~~~

![Sphere collider](SphereCollider.png)  

# Capsule collider
Represented by @ref bs::CCapsuleCollider "CapsuleCollider" component. Capsule is defined using a height, and a radius which represents a distance from every point on the line at the capsule center (the line being the length as defined by "height"). Use @ref bs::CCapsuleCollider::setHalfHeight "CCapsuleCollider::setHalfHeight()" to set the half-height, and @ref bs::CCapsuleCollider::setRadius "CCapsuleCollider::setRadius()" to set the radius.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HCapsuleCollider collider = colliderSO->addComponent<CCapsuleCollider>();

// A capsule 2 units in height, with 0.5 unit radius
collider->setHalfHeight(1.0f);
collider->setRadius(0.5f);
~~~~~~~~~~~~~

![Capsule collider](CapsuleCollider.png)  

# Mesh collider
Mesh colliders represent the most complex collider types. They're repesented with the @ref bs::CMeshCollider "MeshCollider" component and they require a **PhysicsMesh** resource to operate. The physics mesh allows you to represent complex surfaces using triangles (similar to how a mesh used for rendering works) when basic shapes like boxes and spheres don't cut it.

We'll show how to import physics meshes in the next chapter.

You can assign a physics mesh by calling @ref bs::CMeshCollider::setMesh "CMeshCollider::setMesh()".

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSO = SceneObject::create("Collider");
HMeshCollider collider = colliderSO->addComponent<CMeshCollider>();

HPhysicsMesh physicsMesh = ...; // Shown in next chapter
collider->setMesh(physicsMesh);
~~~~~~~~~~~~~

![Mesh collider](MeshCollider.png)  