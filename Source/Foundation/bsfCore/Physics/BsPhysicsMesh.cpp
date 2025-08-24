//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMesh.h"
#include "Private/RTTI/BsPhysicsMeshRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

using namespace b3d;

PhysicsMesh::PhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: mInitMeshData(meshData), mType(type)
{ }

SPtr<MeshData> PhysicsMesh::GetMeshData() const
{
	return mImplementation->GetMeshData();
}

HPhysicsMesh PhysicsMesh::Create(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	SPtr<PhysicsMesh> newMesh = CreateShared(meshData, type);

	return B3DStaticResourceCast<PhysicsMesh>(GetResources().CreateResourceHandle(newMesh));
}

SPtr<PhysicsMesh> PhysicsMesh::CreateShared(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	SPtr<PhysicsMesh> newMesh = B3DMakeShared<PhysicsMesh>(meshData, type);
	newMesh->SetShared(newMesh);
	newMesh->Initialize();

	return newMesh;
}

SPtr<PhysicsMesh> PhysicsMesh::CreateEmpty()
{
	SPtr<PhysicsMesh> newMesh = B3DMakeShared<PhysicsMesh>(nullptr, PhysicsMeshType::Convex);
	newMesh->SetShared(newMesh);

	return newMesh;
}

void PhysicsMesh::Initialize()
{
	if(mImplementation == nullptr) // Could be not-null if we're deserializing
		mImplementation = GetPhysics().CreateMesh(mInitMeshData, mType);

	mInitMeshData = nullptr;

	Resource::Initialize();
}

void PhysicsMesh::Destroy()
{
	mImplementation = nullptr;
	
	Resource::Destroy();
}


RTTIType* PhysicsMesh::GetRttiStatic()
{
	return PhysicsMeshRTTI::Instance();
}

RTTIType* PhysicsMesh::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* IPhysicsMeshImplementation::GetRttiStatic()
{
	return PhysicsMeshImplementationRTTI::Instance();
}

RTTIType* IPhysicsMeshImplementation::GetRtti() const
{
	return GetRttiStatic();
}
