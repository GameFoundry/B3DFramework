//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMesh.h"
#include "Private/RTTI/BsPhysicsMeshRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

using namespace bs;

PhysicsMesh::PhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: mInitMeshData(meshData), mType(type)
{
	// Derived class is responsible for initializing mInternal
}

PhysicsMeshType PhysicsMesh::GetType() const
{
	return mInternal->mType;
}

SPtr<MeshData> PhysicsMesh::GetMeshData() const
{
	return mInternal->GetMeshData();
}

HPhysicsMesh PhysicsMesh::Create(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	SPtr<PhysicsMesh> newMesh = CreatePtrInternal(meshData, type);

	return B3DStaticResourceCast<PhysicsMesh>(GetResources().CreateResourceHandle(newMesh));
}

SPtr<PhysicsMesh> PhysicsMesh::CreatePtrInternal(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	SPtr<PhysicsMesh> newMesh = GetPhysics().CreateMesh(meshData, type);
	newMesh->SetShared(newMesh);
	newMesh->Initialize();

	return newMesh;
}

void PhysicsMesh::Initialize()
{
	mInitMeshData = nullptr;

	Resource::Initialize();
}

RTTITypeBase* PhysicsMesh::GetRttiStatic()
{
	return PhysicsMeshRTTI::Instance();
}

RTTITypeBase* PhysicsMesh::GetRtti() const
{
	return GetRttiStatic();
}

FPhysicsMesh::FPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: mType(type)
{
}

FPhysicsMesh::~FPhysicsMesh()
{
}

RTTITypeBase* FPhysicsMesh::GetRttiStatic()
{
	return FPhysicsMeshRTTI::Instance();
}

RTTITypeBase* FPhysicsMesh::GetRtti() const
{
	return GetRttiStatic();
}
