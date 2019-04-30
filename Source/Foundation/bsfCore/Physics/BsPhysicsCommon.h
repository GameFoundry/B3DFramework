//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Information about a single contact point during physics collision. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) ContactPoint
	{
		Vector3 position; /**< Contact point in world space. */
		Vector3 normal; /**< Normal pointing from the second shape to the first shape. */
		/** Impulse applied to the objects to keep them from penetrating. Divide by simulation step to get the force. */
		float impulse;
		float separation; /**< Determines how far are the objects. Negative value denotes penetration. */
	};

	/** Information about a collision between two physics objects. */
	struct CollisionDataRaw
	{
		Collider* colliders[2]; /**< Colliders involved in the collision. */

		// Note: Not too happy this is heap allocated, use static allocator?
		Vector<ContactPoint> contactPoints; /**< Information about all the contact points for the hit. */ 
	};

	/** Information about a collision between two physics objects. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) CollisionData
	{
		/** Components of the colliders that have collided. */
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
	enum class BS_SCRIPT_EXPORT(m:Physics) CollisionReportMode
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
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) PhysicsQueryHit
	{
		Vector3 point; /**< Position of the hit in world space. */
		Vector3 normal; /**< Normal to the surface that was hit. */
		Vector2 uv; /**< Barycentric coordinates of the triangle that was hit (only applicable when triangle meshes are hit). */
		float distance = 0.0f; /**< Distance from the query origin to the hit position. */
		UINT32 triangleIdx = 0; /**< Index of the triangle that was hit (only applicable when triangle meshes are hit). */

		/**
		 * Unmapped index of the triangle that was hit (only applicable when triangle meshes are hit). 
		 * It represents an index into the original MeshData used to create the PhysicsMesh associated with @p collider.
		 * In contrast, @p triangleIdx is only a valid index for the MeshData directly obtained from #collider which can 
		 * differ from the original MeshData due to the internal implementation.
		 */
		UINT32 unmappedTriangleIdx = 0;

		/** 
		 * Component of the collider that was hit. This may be null if the hit collider has no owner component, in which
		 * case refer to #colliderRaw.
		 */
		HCollider collider;

		BS_SCRIPT_EXPORT(ex:true)
		Collider* colliderRaw = nullptr; /**< Collider that was hit. */
	};

	/** @} */
}