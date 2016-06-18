//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMeshManager.h"
#include "BsCoreThreadAccessor.h"
#include "BsCoreApplication.h"
#include "BsVector3.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"

namespace BansheeEngine
{
	MeshManager::MeshManager()
	{

	}

	MeshManager::~MeshManager()
	{

	}

	SPtr<Mesh> MeshManager::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, 
		int usage, DrawOperationType drawOp, IndexType indexType, const SPtr<Skeleton>& skeleton)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) 
			Mesh(numVertices, numIndices, vertexDesc, usage, drawOp, indexType, skeleton));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> MeshManager::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, 
		const Vector<SubMesh>& subMeshes, int usage, IndexType indexType, const SPtr<Skeleton>& skeleton)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>())
			Mesh(numVertices, numIndices, vertexDesc, subMeshes, usage, indexType, skeleton));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> MeshManager::create(const SPtr<MeshData>& initialData, int usage, DrawOperationType drawOp, 
		const SPtr<Skeleton>& skeleton)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialData, usage, drawOp, skeleton));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> MeshManager::create(const SPtr<MeshData>& initialData, const Vector<SubMesh>& subMeshes, int usage, 
		const SPtr<Skeleton>& skeleton)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialData, subMeshes, usage, skeleton));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> MeshManager::createEmpty()
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh());
		mesh->_setThisPtr(mesh);

		return mesh;
	}

	void MeshManager::onStartUp()
	{
		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

		mDummyMeshData = bs_shared_ptr_new<MeshData>(1, 3, vertexDesc);

		auto vecIter = mDummyMeshData->getVec3DataIter(VES_POSITION);
		vecIter.setValue(Vector3(0, 0, 0));

		auto indices = mDummyMeshData->getIndices32();
		indices[0] = 0;
		indices[1] = 0;
		indices[2] = 0;

		mDummyMesh = Mesh::create(mDummyMeshData);
	}
}