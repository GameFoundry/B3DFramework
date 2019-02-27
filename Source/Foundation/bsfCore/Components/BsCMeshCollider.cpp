//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCMeshCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Physics/BsPhysicsMesh.h"
#include "Private/RTTI/BsCMeshColliderRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CMeshCollider::CMeshCollider()
	{
		setName("MeshCollider");
	}

	CMeshCollider::CMeshCollider(const HSceneObject& parent)
		: CCollider(parent)
	{
		setName("MeshCollider");
	}

	void CMeshCollider::setMesh(const HPhysicsMesh& mesh)
	{
		if (mMesh == mesh)
			return;

		if(getIsTrigger() && mesh->getType() == PhysicsMeshType::Triangle)
		{
			LOGWRN("Triangle meshes are not supported on Trigger colliders.");
			return;
		}

		mMesh = mesh;

		if (mInternal != nullptr)
		{
			_getInternal()->setMesh(mesh);

			if (mParent != nullptr)
			{
				// If triangle mesh its possible the parent can no longer use this collider (they're not supported for 
				// non-kinematic rigidbodies)
				if (mMesh.isLoaded() && mMesh->getType() == PhysicsMeshType::Triangle)
					updateParentRigidbody();
				else
					mParent->_updateMassDistribution();
			}
		}
	}

	SPtr<Collider> CMeshCollider::createInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->getScene();
		const Transform& tfrm = SO()->getTransform();

		SPtr<MeshCollider> collider = MeshCollider::create(*scene->getPhysicsScene(), tfrm.getPosition(), 
			tfrm.getRotation());
		collider->setMesh(mMesh);
		collider->_setOwner(PhysicsOwnerType::Component, this);

		return collider;
	}

	bool CMeshCollider::isValidParent(const HRigidbody& parent) const
	{
		// Triangle mesh colliders cannot be used for non-kinematic rigidbodies
		return !mMesh.isLoaded() || mMesh->getType() == PhysicsMeshType::Convex || parent->getIsKinematic();
	}

	RTTITypeBase* CMeshCollider::getRTTIStatic()
	{
		return CMeshColliderRTTI::instance();
	}

	RTTITypeBase* CMeshCollider::getRTTI() const
	{
		return CMeshCollider::getRTTIStatic();
	}
}
