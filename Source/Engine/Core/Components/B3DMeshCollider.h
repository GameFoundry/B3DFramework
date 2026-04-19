//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DCollider.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** A collider represented by an arbitrary mesh. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) MeshCollider : public Collider
	{
	public:
		MeshCollider(const HSceneObject& parent);

		/**
		 * Determines a mesh that represents the collider geometry. This can be a generic triangle mesh, or and convex mesh.
		 * Triangle meshes are not supported as triggers, nor are they supported for colliders that are parts of a non-kinematic rigidbody.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Setter))
		void SetMesh(const HPhysicsMesh& mesh);

		/** @copydoc SetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Getter))
		HPhysicsMesh GetMesh() const { return mMesh; }

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnCreated() override;
		bool IsValidParent(const HRigidbody& parent) const override;

	protected:
		HPhysicsMesh mMesh;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class MeshColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		MeshCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
