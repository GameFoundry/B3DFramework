//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	PhysicsMeshType PhysicsMesh::getType() const
	{
		return mInternal->mType;
	}

	SPtr<MeshData> PhysicsMesh::getMeshData() const
	{
		return mInternal->getMeshData();
	}

	HPhysicsMesh PhysicsMesh::create(const SPtr<MeshData>& meshData, PhysicsMeshType type)
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

	void PhysicsMesh::initialize()
	{
		mInitMeshData = nullptr;

		Resource::initialize();
	}

	RTTITypeBase* PhysicsMesh::getRTTIStatic()
	{
		return PhysicsMeshRTTI::instance();
	}

	RTTITypeBase* PhysicsMesh::getRTTI() const
	{
		return getRTTIStatic();
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
		return FPhysicsMeshRTTI::instance();
	}

	RTTITypeBase* FPhysicsMesh::getRTTI() const
	{
		return getRTTIStatic();
	}
}
