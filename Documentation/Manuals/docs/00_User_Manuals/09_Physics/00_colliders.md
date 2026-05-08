---
title: Colliders
---

Colliders are a type of component that describe a physical surface. Other physical objects cannot move through them, and as their name implies they will instead collide with them in a physically realistic way. Colliders are represented as components, and can be added or removed in the scene as any other component.

They are static physical objects, and they cannot be moved under the influence of another physical object. Moving the scene object they are attached to will also not result in realistic physical interaction. We'll show how to create dynamic physical objects in later chapters.

For example, you would use colliders to set up the floors of your level so they can be walked on, or set up the walls and obstacles so they cannot be walked through.

> Note that physical objects like colliders only affect other physical objects (other colliders or other types as described later). You can still move objects through colliders if they do not have any physical objects attached. You can think of all physical objects as a closed system that only affects its own components.

There are five collider types, that differ in the way their surface is described:
 - **Plane** - The surface is an unbounded plane
 - **Box** - The surface is a box with custom width, height, and depth
 - **Sphere** - The surface is a sphere with a radius
 - **Capsule** - The surface is a capsule with a radius and a height
 - **Mesh** - The surface is represented by custom geometry using a triangle mesh

# Plane collider
Represented by the @b3d::PlaneCollider component. Use @b3d::PlaneCollider::SetNormal to provide the direction in which the plane is oriented, and @b3d::PlaneCollider::SetDistance to provide an offset along that direction. Using these two properties, you can position and orient a plane anywhere in the scene.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HPlaneCollider planeCollider = colliderSceneObject->AddComponent<PlaneCollider>();

// A plane pointing up, parallel to the XZ plane, 10 units beneath the origin
planeCollider->SetNormal(Vector3::kUnitY);
planeCollider->SetDistance(-10.0f);
~~~~~~~~~~~~~

Query plane properties:

~~~~~~~~~~~~~{.cpp}
Vector3 normal = planeCollider->GetNormal();
float distance = planeCollider->GetDistance();
~~~~~~~~~~~~~

> Note that the plane's position and orientation are also influenced by the scene object the component is attached to. All colliders are influenced by their scene object, but most also provide properties for modifying their position, orientation, or scale locally for more control. You can use either method, or both.

![Plane collider](../../Images/PlaneCollider.png)  

# Box collider
Represented by the @b3d::BoxCollider component. Use @b3d::BoxCollider::SetExtents to provide the extents (half size) of the box. Use @b3d::BoxCollider::SetCenter to offset the box relative to the scene object.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HBoxCollider boxCollider = colliderSceneObject->AddComponent<BoxCollider>();

// Box 1x1x1 in size, centered at its scene object position
boxCollider->SetExtents(Vector3(0.5f, 0.5f, 0.5f));
boxCollider->SetCenter(Vector3::kZero);
~~~~~~~~~~~~~

Query box properties:

~~~~~~~~~~~~~{.cpp}
Vector3 extents = boxCollider->GetExtents();
Vector3 center = boxCollider->GetCenter();
~~~~~~~~~~~~~

![Box collider](../../Images/BoxCollider.png)  

# Sphere collider
Represented by the @b3d::SphereCollider component. Use @b3d::SphereCollider::SetRadius to provide the radius of the sphere.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HSphereCollider sphereCollider = colliderSceneObject->AddComponent<SphereCollider>();

// A unit sphere
sphereCollider->SetRadius(1.0f);
~~~~~~~~~~~~~

You can also offset the sphere from its scene object position using @b3d::SphereCollider::SetCenter:

~~~~~~~~~~~~~{.cpp}
sphereCollider->SetCenter(Vector3(0.0f, 1.0f, 0.0f));
~~~~~~~~~~~~~

Query sphere properties:

~~~~~~~~~~~~~{.cpp}
float radius = sphereCollider->GetRadius();
Vector3 center = sphereCollider->GetCenter();
~~~~~~~~~~~~~

![Sphere collider](../../Images/SphereCollider.png)  

# Capsule collider
Represented by the @b3d::CapsuleCollider component. A capsule is defined using a height and a radius which represents the distance from every point on the line at the capsule center (the line being the length as defined by the height). Use @b3d::CapsuleCollider::SetHalfHeight to set the half-height, and @b3d::CapsuleCollider::SetRadius to set the radius.

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HCapsuleCollider capsuleCollider = colliderSceneObject->AddComponent<CapsuleCollider>();

// A capsule 2 units in height, with 0.5 unit radius
capsuleCollider->SetHalfHeight(1.0f);
capsuleCollider->SetRadius(0.5f);
~~~~~~~~~~~~~

Control capsule orientation with @b3d::CapsuleCollider::SetNormal:

~~~~~~~~~~~~~{.cpp}
// Orient capsule along the X axis
capsuleCollider->SetNormal(Vector3::kUnitX);
~~~~~~~~~~~~~

Offset the capsule from its scene object position:

~~~~~~~~~~~~~{.cpp}
capsuleCollider->SetCenter(Vector3(0.0f, 1.0f, 0.0f));
~~~~~~~~~~~~~

Query capsule properties:

~~~~~~~~~~~~~{.cpp}
float halfHeight = capsuleCollider->GetHalfHeight();
float radius = capsuleCollider->GetRadius();
Vector3 normal = capsuleCollider->GetNormal();
Vector3 center = capsuleCollider->GetCenter();
~~~~~~~~~~~~~

![Capsule collider](../../Images/CapsuleCollider.png)  

# Mesh collider
Mesh colliders represent the most complex collider type. They are represented with the @b3d::MeshCollider component and require a **PhysicsMesh** resource to operate. The physics mesh allows you to represent complex surfaces using triangles (similar to how a mesh used for rendering works) when basic shapes like boxes and spheres are not sufficient.

We'll show how to import physics meshes in the next chapter.

Assign a physics mesh by calling @b3d::MeshCollider::SetMesh:

~~~~~~~~~~~~~{.cpp}
HSceneObject colliderSceneObject = SceneObject::Create("Collider");
HMeshCollider meshCollider = colliderSceneObject->AddComponent<MeshCollider>();

HPhysicsMesh physicsMesh = ...; // Shown in next chapter
meshCollider->SetMesh(physicsMesh);
~~~~~~~~~~~~~

Query the physics mesh:

~~~~~~~~~~~~~{.cpp}
HPhysicsMesh currentMesh = meshCollider->GetMesh();
~~~~~~~~~~~~~

> Note that triangle mesh colliders are not supported as triggers, nor are they supported for colliders that are parts of non-kinematic rigidbodies. Use convex meshes for those scenarios.

![Mesh collider](../../Images/MeshCollider.png)

# Collider shapes

All colliders internally use @b3d::ColliderShape objects to represent their geometry. When you create a collider component like **BoxCollider** or **SphereCollider**, the component automatically creates and manages an internal shape for you. However, you can also work with shapes directly for more advanced control.

Each **ColliderShape** has the following properties that can be customized:
- **Position** - Local position relative to the collider
- **Rotation** - Local rotation relative to the collider
- **Scale** - Local scale relative to the collider
- **IsTrigger** - Whether the shape acts as a trigger
- **Mass** - Mass of the shape (used for rigidbody calculations)
- **Material** - Physical material determining friction and restitution
- **ContactOffset** - Distance at which collision detection begins
- **RestOffset** - Distance at which objects come to rest
- **Layer** - Collision layer for filtering interactions
- **CollisionReportMode** - Controls which collision events are reported

## Retrieving shapes

Access the shapes associated with a collider using @b3d::Collider::GetShapes:

~~~~~~~~~~~~~{.cpp}
TInlineArray<TShared<ColliderShape>, 1> shapes = boxCollider->GetShapes();

// Modify the first shape
if (!shapes.empty())
{
	TShared<ColliderShape> shape = shapes[0];
	shape->SetPosition(Vector3(1.0f, 0.0f, 0.0f));
	shape->SetRotation(Quaternion(Vector3::kUnitZ, Degree(45.0f)));
	shape->SetScale(Vector3(2.0f, 1.0f, 1.0f));
}
~~~~~~~~~~~~~

## Creating shapes manually

You can create collider shapes manually using the static creation methods:

~~~~~~~~~~~~~{.cpp}
// Create a box shape
BoxColliderShapeInformation boxInfo(Vector3(0.5f, 0.5f, 0.5f));
TShared<ColliderShape> boxShape = ColliderShape::CreateBox(boxInfo);

// Create a sphere shape
SphereColliderShapeInformation sphereInfo(1.0f);
TShared<ColliderShape> sphereShape = ColliderShape::CreateSphere(sphereInfo);

// Create a capsule shape
CapsuleColliderShapeInformation capsuleInfo(0.5f, 1.0f);
TShared<ColliderShape> capsuleShape = ColliderShape::CreateCapsule(capsuleInfo);

// Create a plane shape
PlaneColliderShapeInformation planeInfo;
TShared<ColliderShape> planeShape = ColliderShape::CreatePlane(planeInfo);

// Create a mesh shape
MeshColliderShapeInformation meshInfo(physicsMesh);
TShared<ColliderShape> meshShape = ColliderShape::CreateMesh(meshInfo);
~~~~~~~~~~~~~

## Modifying shape geometry

Change the type or parameters of an existing shape:

~~~~~~~~~~~~~{.cpp}
TShared<ColliderShape> shape = shapes[0];

// Change to a box shape
BoxColliderShapeInformation boxInfo(Vector3(1.0f, 1.0f, 1.0f));
shape->SetShape(boxInfo);

// Or change to a sphere
SphereColliderShapeInformation sphereInfo(2.0f);
shape->SetShape(sphereInfo);
~~~~~~~~~~~~~

Query shape information:

~~~~~~~~~~~~~{.cpp}
// Check the shape type
ColliderShapeType shapeType = shape->GetType();

if (shapeType == ColliderShapeType::Box)
{
	BoxColliderShapeInformation boxInfo = shape->GetBoxShapeInformation();
	B3D_LOG(Info, LogPhysics, "Box extents: {0}", boxInfo.Extents);
}
else if (shapeType == ColliderShapeType::Sphere)
{
	SphereColliderShapeInformation sphereInfo = shape->GetSphereShapeInformation();
	B3D_LOG(Info, LogPhysics, "Sphere radius: {0}", sphereInfo.Radius);
}
~~~~~~~~~~~~~

# Multiple shapes per collider

While the basic collider components (**PlaneCollider**, **BoxCollider**, etc.) create a single shape automatically, you can assign multiple shapes to a single collider for complex collision geometry. This allows you to build compound colliders from multiple primitive shapes.

> Note: The standard collider components do not currently support multiple shapes through their public API. To use multiple shapes, you would need to work with the underlying physics implementation directly or create a custom collider component. This is an advanced feature primarily used for rigidbodies with complex shapes.

However, you can customize properties on the existing shapes:

~~~~~~~~~~~~~{.cpp}
// Customize the shape's physical properties
TShared<ColliderShape> shape = boxCollider->GetShapes()[0];
shape->SetMass(10.0f);
shape->SetIsTrigger(false);

// Set a custom physics material
HPhysicsMaterial material = ...; // Created separately
shape->SetMaterial(material);

// Adjust collision detection offsets
shape->SetContactOffset(0.05f);
shape->SetRestOffset(0.01f);

// Set collision layer
shape->SetLayer(2); // Layer 2
~~~~~~~~~~~~~
