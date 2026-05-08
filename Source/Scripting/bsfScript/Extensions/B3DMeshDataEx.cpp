//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DMeshDataEx.h"
#include "Image/B3DPixelUtility.h"
#include "Math/B3DVector2.h"
#include "Image/B3DColor.h"

using namespace b3d;
template <int Semantic>
struct TVertexDataAccessor
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size) {}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size) {}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::Position>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetPositions((Vector3*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetPositions((Vector3*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::Normal>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetNormals((Vector3*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetNormals((Vector3*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::Tangent>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetTangents((Vector4*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetTangents((Vector4*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::Color>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetColors((Color*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetColors((Color*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::UV0>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetUV0((Vector2*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetUV0((Vector2*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::UV1>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetUV1((Vector2*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetUV1((Vector2*)buffer, size);
	}
};

template <>
struct TVertexDataAccessor<(int)VertexLayout::BoneWeights>
{
	static void Get(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->GetBoneWeights((BoneWeight*)buffer, size);
	}

	static void Set(const TShared<RendererMeshData>& meshData, u8* buffer, u32 size)
	{
		meshData->SetBoneWeights((BoneWeight*)buffer, size);
	}
};

template <int Semantic, class TNative>
Vector<TNative> GetVertexDataArray(const TShared<RendererMeshData>& meshData)
{
	u32 numElements = meshData->GetData()->GetVertexCount();
	Vector<TNative> output(numElements);

	TVertexDataAccessor<Semantic>::Get(meshData, (u8*)output.data(), numElements * sizeof(TNative));
	return output;
}

template <int Semantic, class TNative>
void SetVertexDataArray(const TShared<RendererMeshData>& meshData, const Vector<TNative>& input)
{
	u32 numElements = meshData->GetData()->GetVertexCount();

	TVertexDataAccessor<Semantic>::Set(meshData, (u8*)input.data(), numElements * sizeof(TNative));
}

TShared<RendererMeshData> MeshDataEx::Create(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	return RendererMeshData::Create(numVertices, numIndices, layout, indexType);
}

Vector<Vector3> MeshDataEx::GetPositions(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::Position, Vector3>(thisPtr);
}

void MeshDataEx::SetPositions(const TShared<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
{
	SetVertexDataArray<(int)VertexLayout::Position>(thisPtr, value);
}

Vector<Vector3> MeshDataEx::GetNormals(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::Normal, Vector3>(thisPtr);
}

void MeshDataEx::SetNormals(const TShared<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
{
	SetVertexDataArray<(int)VertexLayout::Normal>(thisPtr, value);
}

Vector<Vector4> MeshDataEx::GetTangents(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::Tangent, Vector4>(thisPtr);
}

void MeshDataEx::SetTangents(const TShared<RendererMeshData>& thisPtr, const Vector<Vector4>& value)
{
	SetVertexDataArray<(int)VertexLayout::Tangent>(thisPtr, value);
}

Vector<Color> MeshDataEx::GetColors(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::Color, Color>(thisPtr);
}

void MeshDataEx::SetColors(const TShared<RendererMeshData>& thisPtr, const Vector<Color>& value)
{
	SetVertexDataArray<(int)VertexLayout::Color>(thisPtr, value);
}

Vector<Vector2> MeshDataEx::GetUV0(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::UV0, Vector2>(thisPtr);
}

void MeshDataEx::SetUV0(const TShared<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
{
	SetVertexDataArray<(int)VertexLayout::UV0>(thisPtr, value);
}

Vector<Vector2> MeshDataEx::GetUV1(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::UV1, Vector2>(thisPtr);
}

void MeshDataEx::SetUV1(const TShared<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
{
	SetVertexDataArray<(int)VertexLayout::UV1>(thisPtr, value);
}

Vector<BoneWeight> MeshDataEx::GetBoneWeights(const TShared<RendererMeshData>& thisPtr)
{
	return GetVertexDataArray<(int)VertexLayout::BoneWeights, BoneWeight>(thisPtr);
}

void MeshDataEx::SetBoneWeights(const TShared<RendererMeshData>& thisPtr, const Vector<BoneWeight>& value)
{
	SetVertexDataArray<(int)VertexLayout::BoneWeights>(thisPtr, value);
}

Vector<u32> MeshDataEx::GetIndices(const TShared<RendererMeshData>& thisPtr)
{
	u32 numElements = thisPtr->GetData()->GetIndexCount();

	Vector<u32> output(numElements);
	thisPtr->GetIndices(output.data(), numElements * sizeof(u32));

	return output;
}

void MeshDataEx::SetIndices(const TShared<RendererMeshData>& thisPtr, const Vector<u32>& value)
{
	u32 numElements = thisPtr->GetData()->GetIndexCount();

	thisPtr->SetIndices((u32*)value.data(), numElements * sizeof(u32));
}

int MeshDataEx::GetVertexCount(const TShared<RendererMeshData>& thisPtr)
{
	return (int)thisPtr->GetData()->GetVertexCount();
}

int MeshDataEx::GetIndexCount(const TShared<RendererMeshData>& thisPtr)
{
	return (int)thisPtr->GetData()->GetIndexCount();
}
