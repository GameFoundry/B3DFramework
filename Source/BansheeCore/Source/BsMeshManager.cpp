//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMeshManager.h"
#include "BsCoreThreadAccessor.h"
#include "BsCoreApplication.h"
#include "BsVector3.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"

namespace bs
{
	MeshManager::MeshManager()
	{ }

	MeshManager::~MeshManager()
	{ }

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