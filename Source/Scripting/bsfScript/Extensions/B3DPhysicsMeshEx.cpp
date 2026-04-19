//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DPhysicsMeshEx.h"
#include "Renderer/B3DRendererMeshData.h"

using namespace b3d;
HPhysicsMesh PhysicsMeshEx::Create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type)
{
	return PhysicsMesh::Create(meshData->GetData(), type);
}

SPtr<RendererMeshData> PhysicsMeshEx::GetMeshData(const HPhysicsMesh& thisPtr)
{
	return RendererMeshData::Create(thisPtr->GetMeshData());
}
