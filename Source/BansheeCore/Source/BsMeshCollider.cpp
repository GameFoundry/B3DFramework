//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMeshCollider.h"
#include "BsPhysics.h"

namespace bs
{
	MeshCollider::MeshCollider()
	{ }

	SPtr<MeshCollider> MeshCollider::create(const Vector3& position, const Quaternion& rotation)
	{
		return Physics::instance().createMeshCollider(position, rotation);
	}

	void MeshCollider::getListenerResources(Vector<HResource>& resources)
	{
		if(mMesh != nullptr)
			resources.push_back(mMesh);
	}

	void MeshCollider::notifyResourceLoaded(const HResource& resource)
	{
		onMeshChanged();
	}

	void MeshCollider::notifyResourceChanged(const HResource& resource)
	{
		onMeshChanged();
	}
}