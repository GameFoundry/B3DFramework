//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsMesh.h"
#include "RTTI/BsNullPhysicsMeshRTTI.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "BsNullPhysics.h"
#include "Math/BsAABox.h"

namespace bs
{
	NullPhysicsMesh::NullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:PhysicsMesh(meshData, type)
	{ }

	void NullPhysicsMesh::Initialize()
	{
		if(mInternal == nullptr) // Could be not-null if we're deserializing
			mInternal = bs_shared_ptr_new<FNullPhysicsMesh>(mInitMeshData, mType);

		PhysicsMesh::initialize();
	}

	void NullPhysicsMesh::Destroy()
	{
		mInternal = nullptr;

		PhysicsMesh::destroy();
	}

	FNullPhysicsMesh::FNullPhysicsMesh()
		:FPhysicsMesh(nullptr, PhysicsMeshType::Convex)
	{ }

	FNullPhysicsMesh::FNullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:FPhysicsMesh(meshData, type)
	{ }

	SPtr<MeshData> FNullPhysicsMesh::GetMeshData() const
	{
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

		return MeshData::Create(0, 0, vertexDesc);
	}

	RTTITypeBase* FNullPhysicsMesh::getRTTIStatic()
	{
		return FNullPhysicsMeshRTTI::Instance();
	}

	RTTITypeBase* FNullPhysicsMesh::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
