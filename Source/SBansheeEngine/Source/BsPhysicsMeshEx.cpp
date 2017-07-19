//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysicsMeshEx.h"
#include "BsRendererMeshData.h"

namespace bs
{
	HPhysicsMesh PhysicsMeshEx::create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type)
	{
		return PhysicsMesh::create(meshData->getData(), type);
	}

	SPtr<RendererMeshData> PhysicsMeshEx::getMeshData(const HPhysicsMesh& thisPtr)
	{
		return RendererMeshData::create(thisPtr->getMeshData());
	}
}