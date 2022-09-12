//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsMesh.h"
#include "Private/RTTI/BsPhysicsMeshRTTI.h"
#include "Resources/BsResources.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	PhysicsMesh::PhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:mInitMeshData(meshData), mType(type)
	{
		// Derived class is responsible for initializing mInternal
	}

	PhysicsMeshType PhysicsMesh::GetType() const
	{
		return mInternal->mType;
	}

	SPtr<MeshData> PhysicsMesh::GetMeshData() const
	{
		return mInternal->getMeshData();
	}

	HPhysicsMesh PhysicsMesh::Create(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	{
		SPtr<PhysicsMesh> newMesh = _createPtr(meshData, type);

		return static_resource_cast<PhysicsMesh>(gResources()._createResourceHandle(newMesh));
	}

	SPtr<PhysicsMesh> PhysicsMesh::_createPtr(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	{
		SPtr<PhysicsMesh> newMesh = gPhysics().createMesh(meshData, type);
		newMesh->_setThisPtr(newMesh);
		newMesh->initialize();

		return newMesh;
	}

	void PhysicsMesh::Initialize()
	{
		mInitMeshData = nullptr;

		Resource::initialize();
	}

	RTTITypeBase* PhysicsMesh::getRTTIStatic()
	{
		return PhysicsMeshRTTI::Instance();
	}

	RTTITypeBase* PhysicsMesh::getRTTI() const
	{
		return GetRTTIStatic();
	}

	FPhysicsMesh::FPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:mType(type)
	{
		
	}

	FPhysicsMesh::~FPhysicsMesh()
	{
		
	}

	RTTITypeBase* FPhysicsMesh::getRTTIStatic()
	{
		return FPhysicsMeshRTTI::Instance();
	}

	RTTITypeBase* FPhysicsMesh::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
