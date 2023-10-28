//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsMeshEx.h"
#include "CoreObject/BsRenderThread.h"

using namespace bs;
HMesh MeshEx::Create(int numVertices, int numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
{
	MeshCreateInformation desc;
	desc.VertexCount = numVertices;
	desc.IndexCount = numIndices;
	desc.VertexDescription = RendererMeshData::VertexLayoutVertexDesc(vertex);
	desc.SubMeshes = { SubMesh(0, numIndices, topology) };
	desc.Usage = usage;
	desc.IndexType = index;

	return Mesh::Create(desc);
}

HMesh MeshEx::Create(int numVertices, int numIndices, const Vector<SubMesh>& subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
{
	MeshCreateInformation desc;
	desc.VertexCount = numVertices;
	desc.IndexCount = numIndices;
	desc.VertexDescription = RendererMeshData::VertexLayoutVertexDesc(vertex);
	desc.SubMeshes = subMeshes;
	desc.Usage = usage;
	desc.IndexType = index;

	return Mesh::Create(desc);
}

HMesh MeshEx::Create(const SPtr<RendererMeshData>& data, DrawOperationType topology, MeshUsage usage)
{
	SPtr<MeshData> meshData;
	if(data != nullptr)
		meshData = data->GetData();

	u32 numIndices = 0;
	if(meshData != nullptr)
		numIndices = meshData->GetIndexCount();

	MeshCreateInformation desc;
	desc.SubMeshes = { SubMesh(0, numIndices, topology) };
	desc.Usage = usage;

	return Mesh::Create(meshData, desc);
}

HMesh MeshEx::Create(const SPtr<RendererMeshData>& data, const Vector<SubMesh>& subMeshes, MeshUsage usage)
{
	SPtr<MeshData> meshData;
	if(data != nullptr)
		meshData = data->GetData();

	MeshCreateInformation desc;
	desc.SubMeshes = subMeshes;
	desc.Usage = usage;

	return Mesh::Create(meshData, desc);
}

Vector<SubMesh> MeshEx::GetSubMeshes(const HMesh& thisPtr)
{
	const u32 subMeshCount = (u32)thisPtr->GetProperties().SubMeshes.size();
	Vector<SubMesh> output(subMeshCount);
	for(u32 i = 0; i < subMeshCount; i++)
		output[i] = thisPtr->GetProperties().SubMeshes[i];

	return output;
}

u32 MeshEx::GetSubMeshCount(const HMesh& thisPtr)
{
	return (u32)thisPtr->GetProperties().SubMeshes.size();
}

void MeshEx::GetBounds(const HMesh& thisPtr, AABox* box, Sphere* sphere)
{
	Bounds bounds = thisPtr->GetProperties().Bounds;
	*box = bounds.GetBox();
	*sphere = bounds.GetSphere();
}

SPtr<RendererMeshData> MeshEx::GetMeshData(const HMesh& thisPtr)
{
	const SPtr<MeshData>& meshData = thisPtr->GetCachedData();
	return RendererMeshData::Create(meshData);
}

void MeshEx::SetMeshData(const HMesh& thisPtr, const SPtr<RendererMeshData>& value)
{
	if(value != nullptr)
	{
		SPtr<MeshData> meshData = value->GetData();
		thisPtr->WriteData(meshData, true);
	}
}
