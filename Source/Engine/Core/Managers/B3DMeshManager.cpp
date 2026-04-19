//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DMeshManager.h"
#include "Math/B3DVector3.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"

using namespace b3d;

void MeshManager::OnStartUp()
{
	TInlineArray<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	SPtr<VertexDescription> vertexDesc = B3DMakeShared<VertexDescription>(vertexElements);

	mDummyMeshData = B3DMakeShared<MeshData>(1, 3, vertexDesc);

	auto vecIter = mDummyMeshData->GetVec3DataIter(VES_POSITION);
	vecIter.SetValue(Vector3(0, 0, 0));

	auto indices = mDummyMeshData->GetIndices32();
	indices[0] = 0;
	indices[1] = 0;
	indices[2] = 0;

	mDummyMesh = Mesh::Create(mDummyMeshData);
}
