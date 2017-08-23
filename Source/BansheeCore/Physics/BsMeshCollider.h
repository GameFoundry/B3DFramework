//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Resources/BsIResourceListener.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** A collider represented by an arbitrary mesh. */
	class BS_CORE_EXPORT MeshCollider : public Collider, public IResourceListener
	{
	public:
		MeshCollider();

		/** 
		 * Sets a mesh that represents the collider geometry. This can be a generic triangle mesh, or and convex mesh. 
		 * Triangle meshes are not supported as triggers, nor are they supported for colliders that are parts of a 
		 * non-kinematic rigidbody.
		 */
		void setMesh(const HPhysicsMesh& mesh) { mMesh = mesh; onMeshChanged(); markListenerResourcesDirty(); }

		/** @copydoc setMesh() */
		HPhysicsMesh getMesh() const { return mMesh; }

		/** 
		 * Creates a new mesh collider. 
		 *
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		static SPtr<MeshCollider> create(const Vector3& position = Vector3::ZERO, 
			const Quaternion& rotation = Quaternion::IDENTITY);

	protected:
		/** @copydoc IResourceListener::getListenerResources */
		void getListenerResources(Vector<HResource>& resources) override;

		/** @copydoc IResourceListener::notifyResourceLoaded */
		void notifyResourceLoaded(const HResource& resource) override;

		/** @copydoc IResourceListener::notifyResourceChanged */
		void notifyResourceChanged(const HResource& resource) override;

		/** 
		 * Triggered by the resources system whenever the attached collision mesh changed (e.g. was reimported) or loaded. 
		 */
		virtual void onMeshChanged() { }

		HPhysicsMesh mMesh;
	};

	/** @} */
}