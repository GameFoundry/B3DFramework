---
title: Scene queries
---

Scene queries allow you to specify a geometric object and perform a query to check if that object intersects or overlaps any physics object in the scene. Most queries also return detailed information about the intersection. All queries are performed through the @b3d::PhysicsScene interface.

All queries will operate only on objects in a particular scene. Usually you will only have one scene, and you can retrieve it through @b3d::SceneManager::GetMainScene. This will return a @b3d::SceneInstance object through which you can get a **PhysicsScene** by calling @b3d::SceneInstance::GetPhysicsScene.

Here is a short example of a basic query:

~~~~~~~~~~~~~{.cpp}
// Ray starting at origin, traveling towards negative Z
Vector3 origin = Vector3::kZero;
Vector3 direction(0.0f, 0.0f, -1.0f);
Ray ray(origin, direction);

PhysicsQueryHit hitInfo;

// Cast a ray into the scene and return information about first object hit
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
if (physicsScene->RayCast(ray, hitInfo))
{
	HCollider hitCollider = hitInfo.Collider;
	String hitSceneObjectName = hitCollider->SO()->GetName();

	Vector3 contactPoint = hitInfo.Point;
	B3D_LOG(Info, LogPhysics, "Hit {0} at point {1}", hitSceneObjectName, contactPoint);
}
~~~~~~~~~~~~~

There are two major types of queries:
  - Casts - Check if an object hits something if traveling along a certain direction
  - Overlaps - Check if object overlaps something at a specific position
  
# Casts
Casts are types of queries in which you provide a shape, origin point, and a direction, and the system checks if the shape intersects anything along the way.

Casts can be used with different objects (shapes):
 - **Ray** - A line with origin and direction
 - **Box** - An axis-aligned box
 - **Sphere** - A sphere with center and radius
 - **Capsule** - A capsule with center, radius, and height
 - **Convex mesh** - A convex physics mesh

> Note that only convex meshes are supported when performing mesh casts. Read the physics mesh manual to learn about convex meshes.

They can also be categorized by the type of values they return:
  - **All hits** - Most expensive type of cast query, returns information about all hit objects.
  - **Any hit** - Cheapest type of cast query, just returns a boolean if a hit occurred or not.
  - **Closest** - Returns information only about the closest hit. Cheaper than All, more expensive than Any.
  
## All hit casts
As the name implies, these types of queries perform a cast and then return information about all hit objects. Relevant methods are:
 - @b3d::PhysicsScene::RayCastAll
 - @b3d::PhysicsScene::BoxCastAll
 - @b3d::PhysicsScene::SphereCastAll
 - @b3d::PhysicsScene::CapsuleCastAll
 - @b3d::PhysicsScene::ConvexCastAll

They all share a common interface, where as the first parameter they accept a shape with its starting position and orientation, travel direction, and finally an optional maximum range. They return an array of @b3d::PhysicsQueryHit objects.

~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3::kZero, 0.5f);

// Check negative Z direction
Vector3 direction(0.0f, 0.0f, -1.0f);

// Find all objects intersecting the sphere traveling in the specified direction
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
Vector<PhysicsQueryHit> hits = physicsScene->SphereCastAll(sphere, direction);

for (auto& entry : hits)
{
	HCollider hitCollider = entry.Collider;
	String hitSceneObjectName = hitCollider->SO()->GetName();

	Vector3 contactPoint = entry.Point;
	B3D_LOG(Info, LogPhysics, "Found hit with {0} at point {1}", hitSceneObjectName, contactPoint);
}
~~~~~~~~~~~~~

The @b3d::PhysicsQueryHit object contains information about each individual hit:
 - **Collider** - The collider component that was hit
 - **Point** - Position of the hit in world space
 - **Normal** - Normal to the surface that was hit
 - **Distance** - Distance from the query origin to the hit position
 - **TriangleIdx** - Index of the triangle that was hit (only applicable when triangle meshes are hit)
 - **UnmappedTriangleIdx** - Unmapped index of the triangle in the original mesh data
 - **Uv** - Barycentric coordinates of the triangle that was hit (only applicable when triangle meshes are hit)
 
## Closest hit casts
Closest hit casts are nearly identical to all hit casts, with the main difference being that they return a boolean value indicating if a hit occurred or not, and output a single **PhysicsQueryHit** object. Hit information returned always concerns the closest found hit.

Checking for the closest hit is cheaper than checking for all of them, and is usually adequate for most applications. Relevant methods are:
 - @b3d::PhysicsScene::RayCast
 - @b3d::PhysicsScene::BoxCast
 - @b3d::PhysicsScene::SphereCast
 - @b3d::PhysicsScene::CapsuleCast
 - @b3d::PhysicsScene::ConvexCast

They also share a common interface where as the first parameter they accept a shape with its starting position and orientation, travel direction, reference to a **PhysicsQueryHit** object to receive the results, and finally an optional maximum range. They return a boolean value that is true if anything was hit.

~~~~~~~~~~~~~{.cpp}
// Axis aligned box centered at origin with extents 0.5 in all directions
AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

// Check negative Z direction
Vector3 direction(0.0f, 0.0f, -1.0f);

// Find closest object intersecting the box traveling in the specified direction
PhysicsQueryHit hitInfo;
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
if (physicsScene->BoxCast(box, direction, hitInfo))
{
	HCollider hitCollider = hitInfo.Collider;
	String hitSceneObjectName = hitCollider->SO()->GetName();

	Vector3 contactPoint = hitInfo.Point;
	Vector3 contactNormal = hitInfo.Normal;
	float distance = hitInfo.Distance;

	B3D_LOG(Info, LogPhysics, "Found hit with {0} at point {1}", hitSceneObjectName, contactPoint);
	B3D_LOG(Info, LogPhysics, "Normal: {0}, Distance: {1}", contactNormal, distance);
}
~~~~~~~~~~~~~

## Any hit casts
Finally, any hit casts are the simplest (and cheapest) of them all. They simply return a boolean value indicating if a hit occurred or not. They do not return any further information about the hit.

Relevant methods are:
 - @b3d::PhysicsScene::RayCastAny
 - @b3d::PhysicsScene::BoxCastAny
 - @b3d::PhysicsScene::SphereCastAny
 - @b3d::PhysicsScene::CapsuleCastAny
 - @b3d::PhysicsScene::ConvexCastAny

~~~~~~~~~~~~~{.cpp}
// Ray starting at origin, traveling towards negative Z
Vector3 origin = Vector3::kZero;
Vector3 direction(0.0f, 0.0f, -1.0f);
Ray ray(origin, direction);

// See if the ray intersects anything while traveling in the specified direction
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
if (physicsScene->RayCastAny(ray))
	B3D_LOG(Info, LogPhysics, "Found hit!");
~~~~~~~~~~~~~

# Overlaps
Overlap queries simply check if an object standing still at a specific position and orientation overlaps any other objects.

Overlaps can be used with different shapes:
 - **Box** - An axis-aligned box
 - **Sphere** - A sphere with center and radius
 - **Capsule** - A capsule with center, radius, and height
 - **Convex mesh** - A convex physics mesh

They can also be categorized by the type of values they return:
  - **All overlaps** - Most expensive type of overlap query, returns all overlapping objects.
  - **Any overlap** - Cheapest type of overlap query, just returns a boolean if an overlap occurred or not.
  
## All overlap methods
These overlap methods return an array of **Collider** components consisting of all the objects the provided shape is currently overlapping. Relevant methods are:
 - @b3d::PhysicsScene::BoxOverlap
 - @b3d::PhysicsScene::SphereOverlap
 - @b3d::PhysicsScene::CapsuleOverlap
 - @b3d::PhysicsScene::ConvexOverlap

They all share a common interface. As input they take a shape with its position and orientation, and return an array of overlapping objects.

~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3::kZero, 0.5f);

// Find all objects overlapping the sphere
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
Vector<HCollider> overlaps = physicsScene->SphereOverlap(sphere);

for (auto& entry : overlaps)
{
	String overlappingSceneObjectName = entry->SO()->GetName();
	B3D_LOG(Info, LogPhysics, "Found overlap with {0}", overlappingSceneObjectName);
}
~~~~~~~~~~~~~

## Any overlap methods
This is a set of overlap methods that returns only a boolean value indicating if an overlap occurred or not, without a list of colliders that are overlapping. This is cheaper than querying for all overlaps. The relevant methods are:
 - @b3d::PhysicsScene::BoxOverlapAny
 - @b3d::PhysicsScene::SphereOverlapAny
 - @b3d::PhysicsScene::CapsuleOverlapAny
 - @b3d::PhysicsScene::ConvexOverlapAny

~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3::kZero, 0.5f);

// Check if sphere overlaps anything
const TShared<PhysicsScene>& physicsScene = GetSceneManager().GetMainScene()->GetPhysicsScene();
if (physicsScene->SphereOverlapAny(sphere))
	B3D_LOG(Info, LogPhysics, "Found overlap!");
~~~~~~~~~~~~~
