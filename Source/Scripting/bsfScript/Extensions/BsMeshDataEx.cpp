//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsMeshDataEx.h"
#include "Image/BsPixelUtil.h"
#include "Math/BsVector2.h"

namespace bs
{
	template<int Semantic>
	struct TVertexDataAccessor
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Position >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetPositions((Vector3*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetPositions((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Normal >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetNormals((Vector3*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetNormals((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Tangent >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetTangents((Vector4*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetTangents((Vector4*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Color >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetColors((Color*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetColors((Color*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV0 >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetUV0((Vector2*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetUV0((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV1 >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetUV1((Vector2*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetUV1((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::BoneWeights >
	{
		static void Get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->GetBoneWeights((BoneWeight*)buffer, size); }

		static void Set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->SetBoneWeights((BoneWeight*)buffer, size); }
	};

	template<int Semantic, class TNative>
	Vector<TNative> getVertexDataArray(const SPtr<RendererMeshData>& meshData)
	{
		UINT32 numElements = meshData->GetData()->GetNumVertices();
		Vector<TNative> output(numElements);

		TVertexDataAccessor<Semantic>::Get(meshData, (UINT8*)output.data(), numElements * sizeof(TNative));
		return output;
	}

	template<int Semantic, class TNative>
	void setVertexDataArray(const SPtr<RendererMeshData>& meshData, const Vector<TNative>& input)
	{
		UINT32 numElements = meshData->GetData()->GetNumVertices();

		TVertexDataAccessor<Semantic>::set(meshData, (UINT8*)input.data(), numElements * sizeof(TNative));
	}

	SPtr<RendererMeshData> MeshDataEx::Create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return RendererMeshData::Create(numVertices, numIndices, layout, indexType);
	}

	Vector<Vector3> MeshDataEx::GetPositions(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Position, Vector3>(thisPtr);
	}

	void MeshDataEx::SetPositions(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
	{
		setVertexDataArray<(int)VertexLayout::Position>(thisPtr, value);
	}

	Vector<Vector3> MeshDataEx::GetNormals(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Normal, Vector3>(thisPtr);
	}

	void MeshDataEx::SetNormals(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
	{
		setVertexDataArray<(int)VertexLayout::Normal>(thisPtr, value);
	}

	Vector<Vector4> MeshDataEx::GetTangents(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Tangent, Vector4>(thisPtr);
	}

	void MeshDataEx::SetTangents(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector4>& value)
	{
		setVertexDataArray<(int)VertexLayout::Tangent>(thisPtr, value);
	}

	Vector<Color> MeshDataEx::GetColors(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Color, Color>(thisPtr);
	}

	void MeshDataEx::SetColors(const SPtr<RendererMeshData>& thisPtr, const Vector<Color>& value)
	{
		setVertexDataArray<(int)VertexLayout::Color>(thisPtr, value);
	}

	Vector<Vector2> MeshDataEx::GetUV0(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV0, Vector2>(thisPtr);
	}

	void MeshDataEx::SetUV0(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
	{
		setVertexDataArray<(int)VertexLayout::UV0>(thisPtr, value);
	}

	Vector<Vector2> MeshDataEx::GetUV1(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV1, Vector2>(thisPtr);
	}

	void MeshDataEx::SetUV1(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
	{
		setVertexDataArray<(int)VertexLayout::UV1>(thisPtr, value);
	}

	Vector<BoneWeight> MeshDataEx::GetBoneWeights(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::BoneWeights, BoneWeight>(thisPtr);
	}

	void MeshDataEx::SetBoneWeights(const SPtr<RendererMeshData>& thisPtr, const Vector<BoneWeight>& value)
	{
		setVertexDataArray<(int)VertexLayout::BoneWeights>(thisPtr, value);
	}

	Vector<UINT32> MeshDataEx::GetIndices(const SPtr<RendererMeshData>& thisPtr)
	{
		UINT32 numElements = thisPtr->GetData()->GetNumIndices();

		Vector<UINT32> output(numElements);
		thisPtr->GetIndices(output.data(), numElements * sizeof(UINT32));

		return output;
	}

	void MeshDataEx::SetIndices(const SPtr<RendererMeshData>& thisPtr, const Vector<UINT32>& value)
	{
		UINT32 numElements = thisPtr->GetData()->GetNumIndices();

		thisPtr->SetIndices((UINT32*)value.data(), numElements * sizeof(UINT32));
	}

	int MeshDataEx::GetVertexCount(const SPtr<RendererMeshData>& thisPtr)
	{
		return (int)thisPtr->GetData()->GetNumVertices();
	}

	int MeshDataEx::GetIndexCount(const SPtr<RendererMeshData>& thisPtr)
	{
		return (int)thisPtr->GetData()->GetNumIndices();
	}
}
