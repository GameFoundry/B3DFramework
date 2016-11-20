//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVector3.h"
#include "BsVector2.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Information about a single contact point during physics collision. */
	struct ContactPoint
	{
		Vector3 position; /**< Contact point in world space. */
		Vector3 normal; /**< Normal pointing from the second shape to the first shape. */
		/** Impulse applied to the objects to keep them from penetrating. Divide by simulation step to get the force. */
		float impulse;
		float separation; /**< Determines how far are the objects. Negative value denotes penetration. */
	};

	/** Information about a collision between two physics objects. */
	struct CollisionData
	{
		Collider* collidersRaw[2]; /**< Colliders involved in the collision. */

		/** 
		 * Components of the colliders that were hit. Can be null if collider is not owned by a component, in which case
		 * use #collidersRaw directly. 
		 */
		HCollider collider[2]; 
		// Note: Not too happy this is heap allocated, use static allocator?
		Vector<ContactPoint> contactPoints; /**< Information about all the contact points for the hit. */ 
	};

	/** Determines what parent, if any, owns a physics object. */
	enum class PhysicsOwnerType
	{
		None, /** No parent, object is used directly. */
		Component, /** Object is used by a C++ Component. */
		Script /** Object is used by the scripting system. */
	};

	/** Contains information about a parent for a physics object. */
	struct PhysicsObjectOwner
	{
		PhysicsOwnerType type = PhysicsOwnerType::None; /**< Type of owner. */
		void* ownerData = nullptr; /**< Data managed by the owner. */
	};

	/** Determines which collision events will be reported by physics objects. */
	enum class CollisionReportMode
	{
		None, /**< No collision events will be triggered. */
		Report, /**< Collision events will be triggered when object enters and/or leaves collision. */
		/** 
		 * Collision events will be triggered when object enters and/or leaves collision, but also every frame the object
		 * remains in collision. 
		 */
		ReportPersistent, 
	};

	/** Hit information from a physics query. */
	struct PhysicsQueryHit
	{
		Vector3 point; /**< Position of the hit in world space. */
		Vector3 normal; /**< Normal to the surface that was hit. */
		Vector2 uv; /**< UV coordinates of the triangle that was hit (only applicable when triangle meshes are hit). */
		float distance = 0.0f; /**< Distance from the query origin to the hit position. */
		UINT32 triangleIdx = 0; /**< Index of the triangle that was hit (only applicable when triangle meshes are hit). */
		Collider* colliderRaw = nullptr; /**< Collider that was hit. */
		/** 
		 * Component of the collider that was hit. This may be null if the hit collider has no owner component, in which
		 * case refer to #colliderRaw.
		 */
		HCollider collider;
	};

	/** @} */
}