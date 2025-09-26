//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"

namespace b3d
{
	class ColliderShape;
	/** @addtogroup Physics
	 *  @{
	 */

	/** Information about a single contact point during physics collision. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) ContactPoint
	{
		Vector3 Position; /**< Contact point in world space. */
		Vector3 Normal; /**< Normal pointing from the second shape to the first shape. */
		/** Impulse applied to the objects to keep them from penetrating. Divide by simulation step to get the force. */
		float Impulse;
		float Separation; /**< Determines how far are the objects. Negative value denotes penetration. */
	};

	/** Information about a collision between two physics objects. */
	struct CollisionDataRaw
	{
		ColliderShape* ColliderShapes[2]; /**< Collider shapes involved in the collision. */

		// Note: Not too happy this is heap allocated, use static allocator?
		Vector<ContactPoint> ContactPoints; /**< Information about all the contact points for the hit. */
	};

	/** Information about a collision between two physics objects. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) CollisionData
	{
		/** Components of the colliders that have collided. */
		HCollider Collider[2];

		/** Shapes of that have collided. */
		SPtr<ColliderShape> ColliderShapes[2];

		// Note: Not too happy this is heap allocated, use static allocator?
		Vector<ContactPoint> ContactPoints; /**< Information about all the contact points for the hit. */
	};

	/** Determines which collision events will be reported by physics objects. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) CollisionReportMode
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
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) PhysicsQueryHit
	{
		Vector3 Point; /**< Position of the hit in world space. */
		Vector3 Normal; /**< Normal to the surface that was hit. */
		Vector2 Uv; /**< Barycentric coordinates of the triangle that was hit (only applicable when triangle meshes are hit). */
		float Distance = 0.0f; /**< Distance from the query origin to the hit position. */
		u32 TriangleIdx = 0; /**< Index of the triangle that was hit (only applicable when triangle meshes are hit). */

		/**
		 * Unmapped index of the triangle that was hit (only applicable when triangle meshes are hit).
		 * It represents an index into the original MeshData used to create the PhysicsMesh associated with @p collider.
		 * In contrast, @p triangleIdx is only a valid index for the MeshData directly obtained from #collider which can
		 * differ from the original MeshData due to the internal implementation.
		 */
		u32 UnmappedTriangleIdx = 0;

		/**
		 * Component of the collider that was hit. This may be null if the hit collider has no owner component, in which
		 * case refer to #colliderRaw.
		 */
		HCollider Collider;

		SPtr<ColliderShape> ColliderShape; /**< Collider shape that was hit. */
	};

	/** @} */
} // namespace b3d
