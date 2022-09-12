//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsMeshCollider.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	SPtr<MeshCollider> MeshCollider::Create(PhysicsScene& scene, const Vector3& position, const Quaternion& rotation)
	{
		return scene.CreateMeshCollider(position, rotation);
	}

	void MeshCollider::GetListenerResources(Vector<HResource>& resources)
	{
		if(mMesh != nullptr)
			resources.push_back(mMesh);
	}

	void MeshCollider::NotifyResourceLoaded(const HResource& resource)
	{
		onMeshChanged();
	}

	void MeshCollider::NotifyResourceChanged(const HResource& resource)
	{
		onMeshChanged();
	}
}
