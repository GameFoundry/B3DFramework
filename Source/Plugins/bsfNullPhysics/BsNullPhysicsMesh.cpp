//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsMesh.h"
#include "RTTI/BsNullPhysicsMeshRTTI.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "BsNullPhysics.h"
#include "Math/BsAABox.h"

using namespace bs;

NullPhysicsMesh::NullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: PhysicsMesh(meshData, type)
{}

void NullPhysicsMesh::Initialize()
{
	if(mInternal == nullptr) // Could be not-null if we're deserializing
		mInternal = B3DMakeShared<FNullPhysicsMesh>(mInitMeshData, mType);

	PhysicsMesh::Initialize();
}

void NullPhysicsMesh::Destroy()
{
	mInternal = nullptr;

	PhysicsMesh::Destroy();
}

FNullPhysicsMesh::FNullPhysicsMesh()
	: FPhysicsMesh(nullptr, PhysicsMeshType::Convex)
{}

FNullPhysicsMesh::FNullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: FPhysicsMesh(meshData, type)
{}

SPtr<MeshData> FNullPhysicsMesh::GetMeshData() const
{
	SmallVector<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>(vertexElements);
	return MeshData::Create(0, 0, vertexDesc);
}

RTTITypeBase* FNullPhysicsMesh::GetRttiStatic()
{
	return FNullPhysicsMeshRTTI::Instance();
}

RTTITypeBase* FNullPhysicsMesh::GetRtti() const
{
	return GetRttiStatic();
}
