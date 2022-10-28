//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Resources/BsIResourceListener.h"

namespace bs
{
	class PhysicsScene;

	/** @addtogroup Physics
	 *  @{
	 */

	/** A collider represented by an arbitrary mesh. */
	class BS_CORE_EXPORT MeshCollider : public Collider, public IResourceListener
	{
	public:
		MeshCollider() = default;

		/**
		 * Sets a mesh that represents the collider geometry. This can be a generic triangle mesh, or and convex mesh.
		 * Triangle meshes are not supported as triggers, nor are they supported for colliders that are parts of a
		 * non-kinematic rigidbody.
		 */
		void SetMesh(const HPhysicsMesh& mesh)
		{
			mMesh = mesh;
			OnMeshChanged();
			MarkListenerResourcesDirty();
		}

		/** @copydoc SetMesh() */
		HPhysicsMesh GetMesh() const { return mMesh; }

		/**
		 * Creates a new mesh collider.
		 *
		 * @param[in]	scene		Scene into which to add the collider to.
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		static SPtr<MeshCollider> Create(PhysicsScene& scene, const Vector3& position = Vector3::kZero, const Quaternion& rotation = Quaternion::kIdentity);

	protected:
		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;

		/**
		 * Triggered by the resources system whenever the attached collision mesh changed (e.g. was reimported) or loaded.
		 */
		virtual void OnMeshChanged() {}

		HPhysicsMesh mMesh;
	};

	/** @} */
} // namespace bs
