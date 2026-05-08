//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/B3DPhysicsMesh.h"
#include "RTTI/B3DPhysicsMeshRTTI.h"
#include "Resources/B3DResources.h"
#include "Physics/B3DPhysics.h"

using namespace b3d;

PhysicsMesh::PhysicsMesh(const TShared<MeshData>& meshData, PhysicsMeshType type)
	: mInitMeshData(meshData), mType(type)
{ }

TShared<MeshData> PhysicsMesh::GetMeshData() const
{
	return mImplementation->GetMeshData();
}

HPhysicsMesh PhysicsMesh::Create(const TShared<MeshData>& meshData, PhysicsMeshType type)
{
	TShared<PhysicsMesh> newMesh = CreateShared(meshData, type);

	return B3DStaticResourceCast<PhysicsMesh>(GetResources().CreateResourceHandle(newMesh));
}

TShared<PhysicsMesh> PhysicsMesh::CreateShared(const TShared<MeshData>& meshData, PhysicsMeshType type)
{
	TShared<PhysicsMesh> newMesh = B3DMakeShared<PhysicsMesh>(meshData, type);
	newMesh->SetShared(newMesh);
	newMesh->Initialize();

	return newMesh;
}

TShared<PhysicsMesh> PhysicsMesh::CreateEmpty()
{
	TShared<PhysicsMesh> newMesh = B3DMakeShared<PhysicsMesh>(nullptr, PhysicsMeshType::Convex);
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
