//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsPhysicsMeshEx.h"
#include "Renderer/BsRendererMeshData.h"

namespace bs
{
	HPhysicsMesh PhysicsMeshEx::Create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type)
	{
		return PhysicsMesh::Create(meshData->GetData(), type);
	}

	SPtr<RendererMeshData> PhysicsMeshEx::GetMeshData(const HPhysicsMesh& thisPtr)
	{
		return RendererMeshData::Create(thisPtr->GetMeshData());
	}
}
