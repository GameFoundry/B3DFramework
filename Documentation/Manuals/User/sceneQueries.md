Scene queries						{#sceneQueries}
===============
[TOC]

Scene queries allow you to specify a geometric object and perform a query if that object intersects or overlaps any physics object in the scene. Most queries also return detailed information about the intersection. All queries are performed through the global @ref bs::Physics "Physics" interface, accessible through @ref bs::gPhysics "gPhysics()".

Here is a short example of a basic query, to give you a better idea:

~~~~~~~~~~~~~{.cpp}
// Ray starting at origin, traveling towards negative Z
Vector3 origin(0, 0, 0);
Vector3 direction(0, 0, -1);
Ray ray(origin, direction);

PhysicsQueryHit hitInfo;

// Cast a ray into the scene and return information about first object hit
if(gPhysics().rayCast(ray, hitInfo))
{
	HCollider hitCollider = hitInfo.collider;
	String hitSceneObjectName = hitCollider->SO()->getName();
	
	Vector3 contactPoint = hitInfo.point;
	gDebug().logDebug("Hit " + hitSceneObjectName + " at point " + toString(contactPoint));
}
~~~~~~~~~~~~~

There are two major types of queries:
  - Casts - Check if an object hits something if traveling along a certain direction
  - Overlaps - Check if object overlaps something at a specific position
  
# Casts {#sceneQueries_a}
Casts are types of queries in which you provide a shape, origin point and a direction, and the system check if the shape intersects anything along the way.

Cast can be used with different objects (shapes):
 - Ray
 - Box
 - Sphere
 - Capsule
 - Convex mesh
 
> Note that only convex meshes are supported when performing mesh casts. Read the physics mesh manual to learn about convex meshes.

They can also be categorized by the type of values they return:
  - All hits - Most expensive type of cast query, returns information about all hit objects.
  - Any hit - Cheapest type of cast query, just returns a boolean if hit occurred or not.
  - Closest - Returns information only about the closest hit. Cheaper than *All*, more expensive than *Any*.
  
## All hit casts {#sceneQueries_a_a}
As the name implies, these type of queries perform a cast and then return information about all hit objects. Relevant methods are:
 - @ref bs::Physics::rayCastAll "Physics::rayCastAll()"
 - @ref bs::Physics::boxCastAll "Physics::boxCastAll()"
 - @ref bs::Physics::sphereCastAll "Physics::sphereCastAll()"
 - @ref bs::Physics::capsuleCastAll "Physics::capsuleCastAll()"
 - @ref bs::Physics::convexCastAll "Physics::convexCastAll()"
 
They all share a common interface, where as the first parameter they accept a shape with its starting position and orientation, travel direction, and finally an optional maximum range. They return an array of @ref bs::PhysicsQueryHit "PhysicsQueryHit" objects.

~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3(0, 0, 0), 0.5f);

// Check negative Z direction
Vector3 direction(0, 0, -1);

// Find all objects intersecting the sphere traveling in the specified direction
Vector<PhysicsQueryHit> hits = gPhysics().sphereCastAll(sphere, direction);

for(auto& entry : hits)
{
	HCollider hitCollider = entry.collider;
	String hitSceneObjectName = hitCollider->SO()->getName();
	
	Vector3 contactPoint = entry.point;
	gDebug().logDebug("Found hit with " + hitSceneObjectName + " at point " + toString(contactPoint));
}
~~~~~~~~~~~~~

**PhysicsQueryHit** object contains information about each individual hit, and it contains:
 - **Collider** component that was hit
 - Position and normal of the contact point
 - Distance of the contact point from the query origin
 - Index of the triangle that was hit (if hit collider is a **MeshCollider**)
 - Barycentric coordinates of the triangle that was hit (if hit collider is a **MeshCollider**)
 
## Closest hit casts {#sceneQueries_a_b}
Closest hit casts are nearly identical to all hit casts, with the main difference is that they return a boolean value if a hit occurred or not, and output a single **PhysicsQueryHit** object. Hit information returned always concerns the closest found hit. 

Checking for closest hit is cheaper than checking for them all, and is usually adequate for most applications. Relevant methods are:
 - @ref bs::Physics::rayCast "Physics::rayCast()"
 - @ref bs::Physics::boxCast "Physics::boxCast()"
 - @ref bs::Physics::sphereCast "Physics::sphereCast()"
 - @ref bs::Physics::capsuleCast "Physics::capsuleCast()"
 - @ref bs::Physics::convexCast "Physics::convexCast()"
 
They also share a common interface where as the first parameter they accept a shape with its starting position and orientation, travel direction, reference to a **PhysicsQueryHit** object to receive the results, and finally an optional maximum range. They return a boolean value that returns true if anything was hit.

~~~~~~~~~~~~~{.cpp}
// Axis aligned box centered at origin with extents 0.5 in all directions
AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

// Check negative Z direction
Vector3 direction(0, 0, -1);

// Find closest object intersecting the box traveling in the specified direction
PhysicsQueryHit hitInfo;
if(gPhysics().boxCast(box, direction, hitInfo))
{
	HCollider hitCollider = hitInfo.collider;
	String hitSceneObjectName = hitCollider->SO()->getName();
	
	Vector3 contactPoint = hitInfo.point;
	gDebug().logDebug("Found hit with " + hitSceneObjectName + " at point " + toString(contactPoint));
}
~~~~~~~~~~~~~

## Any hit casts {#sceneQueries_a_c}
Finally, any hit casts are the simplest (and cheapest) of them all. They simply return a boolean value if a hit occurred or not. They do not return any further information about the hit.

Relevant methods are:
 - @ref bs::Physics::rayCastAny "Physics::rayCastAny()"
 - @ref bs::Physics::boxCastAny "Physics::boxCastAny()"
 - @ref bs::Physics::sphereCastAny "Physics::sphereCastAny()"
 - @ref bs::Physics::capsuleCastAny "Physics::capsuleCastAny()"
 - @ref bs::Physics::convexCastAny "Physics::convexCastAny()"
 
~~~~~~~~~~~~~{.cpp}
// Ray starting at origin, traveling towards negative Z
Vector3 origin(0, 0, 0);
Vector3 direction(0, 0, -1);
Ray ray(origin, direction);

// See if the ray intersects anything while traveling in the specified direction
if(gPhysics().rayCastAny(ray))
	gDebug().logDebug("Found hit!");
~~~~~~~~~~~~~

# Overlaps {#sceneQueries_b}
Overlap queries simply check if an object standing still at a specific position/orientation overlaps any other objects. 

Overlaps can be used with different shapes:
 - Box
 - Sphere
 - Capsule
 - Convex mesh
 
They can also be categorized by the type of values they return:
  - All overlaps - Most expensive type of overlap query, returns all overlapping objects.
  - Any overlap - Cheapest type of overlap query, just returns a boolean if overlap occurred or not.
  
## All overlap methods {#sceneQueries_b_a}
These overlap methods return an array of **Collider**%s consisting of all the objects the provided shape is currently overlapping. Relevant methods are:
 - @ref bs::Physics::boxOverlap "Physics::boxOverlap()"
 - @ref bs::Physics::sphereOverlap "Physics::sphereOverlap()"
 - @ref bs::Physics::capsuleOverlap "Physics::capsuleOverlap()"
 - @ref bs::Physics::convexOverlap "Physics::convexOverlap()"
 
They all share a common interface. As input they take a shape with its starting position and orientation, and return an array of overlapping objects. 

~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3(0, 0, 0), 0.5f);

// Find all objects overlapping the sphere
Vector<HCollider> overlaps = gPhysics().sphereOverlap(sphere);

for(auto& entry : overlaps)
{
	String overlappingSceneObjectName = entry->SO()->getName();
	gDebug().logDebug("Found overlap with " + overlappingSceneObjectName);
}
~~~~~~~~~~~~~

## Any overlap methods {#sceneQueries_b_b}
This is a set of overlap methods that returns only a boolean value if the overlap occurred or not, without a list of colliders that are overlapping. This is cheaper than querying for all overlaps. The relevant methods are:
 - @ref bs::Physics::boxOverlapAny "Physics::boxOverlapAny()"
 - @ref bs::Physics::sphereOverlapAny "Physics::sphereOverlapAny()"
 - @ref bs::Physics::capsuleOverlapAny "Physics::capsuleOverlapAny()"
 - @ref bs::Physics::convexOverlapAny "Physics::convexOverlapAny()"
 
 ~~~~~~~~~~~~~{.cpp}
// Sphere centered at origin with radius 0.5
Sphere sphere(Vector3(0, 0, 0), 0.5f);

// Check if sphere overlaps anything 
if(gPhysics().sphereOverlapAny(sphere))
	gDebug().logDebug("Found overlap!");
~~~~~~~~~~~~~