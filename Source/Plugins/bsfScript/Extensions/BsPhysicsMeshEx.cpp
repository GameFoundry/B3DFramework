//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsPhysicsMeshEx.h"
#include "Renderer/BsRendererMeshData.h"

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