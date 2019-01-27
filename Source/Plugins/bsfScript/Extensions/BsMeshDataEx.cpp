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
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Position >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getPositions((Vector3*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setPositions((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Normal >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getNormals((Vector3*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setNormals((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Tangent >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getTangents((Vector4*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setTangents((Vector4*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Color >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getColors((Color*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setColors((Color*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV0 >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getUV0((Vector2*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setUV0((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV1 >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getUV1((Vector2*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setUV1((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::BoneWeights >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getBoneWeights((BoneWeight*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setBoneWeights((BoneWeight*)buffer, size); }
	};

	template<int Semantic, class TNative>
	Vector<TNative> getVertexDataArray(const SPtr<RendererMeshData>& meshData)
	{
		UINT32 numElements = meshData->getData()->getNumVertices();
		Vector<TNative> output(numElements);

		TVertexDataAccessor<Semantic>::get(meshData, (UINT8*)output.data(), numElements * sizeof(TNative));
		return output;
	}

	template<int Semantic, class TNative>
	void setVertexDataArray(const SPtr<RendererMeshData>& meshData, const Vector<TNative>& input)
	{
		UINT32 numElements = meshData->getData()->getNumVertices();

		TVertexDataAccessor<Semantic>::set(meshData, (UINT8*)input.data(), numElements * sizeof(TNative));
	}

	SPtr<RendererMeshData> MeshDataEx::create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return RendererMeshData::create(numVertices, numIndices, layout, indexType);
	}

	Vector<Vector3> MeshDataEx::getPositions(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Position, Vector3>(thisPtr);
	}

	void MeshDataEx::setPositions(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
	{
		setVertexDataArray<(int)VertexLayout::Position>(thisPtr, value);
	}

	Vector<Vector3> MeshDataEx::getNormals(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Normal, Vector3>(thisPtr);
	}

	void MeshDataEx::setNormals(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value)
	{
		setVertexDataArray<(int)VertexLayout::Normal>(thisPtr, value);
	}

	Vector<Vector4> MeshDataEx::getTangents(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Tangent, Vector4>(thisPtr);
	}

	void MeshDataEx::setTangents(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector4>& value)
	{
		setVertexDataArray<(int)VertexLayout::Tangent>(thisPtr, value);
	}

	Vector<Color> MeshDataEx::getColors(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Color, Color>(thisPtr);
	}

	void MeshDataEx::setColors(const SPtr<RendererMeshData>& thisPtr, const Vector<Color>& value)
	{
		setVertexDataArray<(int)VertexLayout::Color>(thisPtr, value);
	}

	Vector<Vector2> MeshDataEx::getUV0(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV0, Vector2>(thisPtr);
	}

	void MeshDataEx::setUV0(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
	{
		setVertexDataArray<(int)VertexLayout::UV0>(thisPtr, value);
	}

	Vector<Vector2> MeshDataEx::getUV1(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV1, Vector2>(thisPtr);
	}

	void MeshDataEx::setUV1(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value)
	{
		setVertexDataArray<(int)VertexLayout::UV1>(thisPtr, value);
	}

	Vector<BoneWeight> MeshDataEx::getBoneWeights(const SPtr<RendererMeshData>& thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::BoneWeights, BoneWeight>(thisPtr);
	}

	void MeshDataEx::setBoneWeights(const SPtr<RendererMeshData>& thisPtr, const Vector<BoneWeight>& value)
	{
		setVertexDataArray<(int)VertexLayout::BoneWeights>(thisPtr, value);
	}

	Vector<UINT32> MeshDataEx::getIndices(const SPtr<RendererMeshData>& thisPtr)
	{
		UINT32 numElements = thisPtr->getData()->getNumIndices();

		Vector<UINT32> output(numElements);
		thisPtr->getIndices(output.data(), numElements * sizeof(UINT32));

		return output;
	}

	void MeshDataEx::setIndices(const SPtr<RendererMeshData>& thisPtr, const Vector<UINT32>& value)
	{
		UINT32 numElements = thisPtr->getData()->getNumIndices();

		thisPtr->setIndices((UINT32*)value.data(), numElements * sizeof(UINT32));
	}

	int MeshDataEx::getVertexCount(const SPtr<RendererMeshData>& thisPtr)
	{
		return (int)thisPtr->getData()->getNumVertices();
	}

	int MeshDataEx::getIndexCount(const SPtr<RendererMeshData>& thisPtr)
	{
		return (int)thisPtr->getData()->getNumIndices();
	}
}