//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsMeshEx.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	HMesh MeshEx::Create(int numVertices, int numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		MESH_DESC desc;
		desc.NumVertices = numVertices;
		desc.NumIndices = numIndices;
		desc.VertexDesc = RendererMeshData::VertexLayoutVertexDesc(vertex);
		desc.SubMeshes = { SubMesh(0, numIndices, topology) };
		desc.Usage = usage;
		desc.IndexType = index;

		return Mesh::Create(desc);
	}

	HMesh MeshEx::Create(int numVertices, int numIndices, const Vector<SubMesh>& subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		MESH_DESC desc;
		desc.NumVertices = numVertices;
		desc.NumIndices = numIndices;
		desc.VertexDesc = RendererMeshData::VertexLayoutVertexDesc(vertex);
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
			numIndices = meshData->GetNumIndices();

		MESH_DESC desc;
		desc.SubMeshes = { SubMesh(0, numIndices, topology) };
		desc.Usage = usage;

		return Mesh::Create(meshData, desc);
	}

	HMesh MeshEx::Create(const SPtr<RendererMeshData>& data, const Vector<SubMesh>& subMeshes, MeshUsage usage)
	{
		SPtr<MeshData> meshData;
		if(data != nullptr)
			meshData = data->GetData();

		MESH_DESC desc;
		desc.SubMeshes = subMeshes;
		desc.Usage = usage;

		return Mesh::Create(meshData, desc);
	}

	Vector<SubMesh> MeshEx::GetSubMeshes(const HMesh& thisPtr)
	{
		u32 numSubMeshes = thisPtr->GetProperties().GetNumSubMeshes();
		Vector<SubMesh> output(numSubMeshes);
		for(u32 i = 0; i < numSubMeshes; i++)
			output[i] = thisPtr->GetProperties().GetSubMesh(i);

		return output;
	}

	u32 MeshEx::GetSubMeshCount(const HMesh& thisPtr)
	{
		return thisPtr->GetProperties().GetNumSubMeshes();
	}

	void MeshEx::GetBounds(const HMesh& thisPtr, AABox* box, Sphere* sphere)
	{
		Bounds bounds = thisPtr->GetProperties().GetBounds();
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
} // namespace bs
