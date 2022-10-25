//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsMeshManager.h"
#include "BsCoreApplication.h"
#include "Math/BsVector3.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"

using namespace bs;

void MeshManager::OnStartUp()
{
	SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
	vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

	mDummyMeshData = bs_shared_ptr_new<MeshData>(1, 3, vertexDesc);

	auto vecIter = mDummyMeshData->GetVec3DataIter(VES_POSITION);
	vecIter.SetValue(Vector3(0, 0, 0));

	auto indices = mDummyMeshData->GetIndices32();
	indices[0] = 0;
	indices[1] = 0;
	indices[2] = 0;

	mDummyMesh = Mesh::Create(mDummyMeshData);
}
