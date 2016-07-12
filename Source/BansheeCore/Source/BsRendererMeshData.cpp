//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererMeshData.h"
#include "BsVertexDataDesc.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsColor.h"
#include "BsPixelUtil.h"
#include "BsRendererManager.h"
#include "BsCoreRenderer.h"

namespace BansheeEngine
{
	RendererMeshData::RendererMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		SPtr<VertexDataDesc> vertexDesc = vertexLayoutVertexDesc(layout);

		mMeshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc, indexType);
	}

	RendererMeshData::RendererMeshData(const SPtr<MeshData>& meshData)
		:mMeshData(meshData)
	{

	}

	void RendererMeshData::getPositions(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_POSITION))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->getVertexData(VES_POSITION, (UINT8*)buffer, size);
	}

	void RendererMeshData::setPositions(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_POSITION))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->setVertexData(VES_POSITION, (UINT8*)buffer, size);
	}

	void RendererMeshData::getNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->getVertexData(VES_NORMAL, (UINT8*)buffer, size);
	}

	void RendererMeshData::setNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->setVertexData(VES_NORMAL, (UINT8*)buffer, size);
	}

	void RendererMeshData::getTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		mMeshData->getVertexData(VES_TANGENT, (UINT8*)buffer, size);
	}

	void RendererMeshData::setTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		mMeshData->setVertexData(VES_TANGENT, (UINT8*)buffer, size);
	}

	void RendererMeshData::getColors(Color* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* colorSrc = mMeshData->getElementData(VES_COLOR);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		Color* colorDst = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			PixelUtil::unpackColor(colorDst, PF_R8G8B8A8, (void*)colorSrc);

			colorSrc += stride;
			colorDst++;
		}
	}

	void RendererMeshData::setColors(Color* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* colorDst = mMeshData->getElementData(VES_COLOR);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		Color* colorSrc = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			PixelUtil::packColor(*colorSrc, PF_R8G8B8A8, (void*)colorDst);

			colorSrc++;
			colorDst += stride;
		}
	}

	void RendererMeshData::setColors(UINT32* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(UINT32) == size);

		UINT8* colorDst = mMeshData->getElementData(VES_COLOR);
		memcpy(colorDst, buffer, size);
	}

	void RendererMeshData::getUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->getVertexData(VES_TEXCOORD, (UINT8*)buffer, size, 0);
	}

	void RendererMeshData::setUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->setVertexData(VES_TEXCOORD, (UINT8*)buffer, size, 0);
	}

	void RendererMeshData::getUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->getVertexData(VES_TEXCOORD, (UINT8*)buffer, size, 1);
	}

	void RendererMeshData::setUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->setVertexData(VES_TEXCOORD, (UINT8*)buffer, size, 1);
	}

	void RendererMeshData::getBoneWeights(BoneWeight* buffer, UINT32 size)
	{
		SPtr<VertexDataDesc> vertexDesc = mMeshData->getVertexDesc();

		if (!vertexDesc->hasElement(VES_BLEND_WEIGHTS) ||
			!vertexDesc->hasElement(VES_BLEND_INDICES))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(BoneWeight) == size);

		UINT8* weightPtr = mMeshData->getElementData(VES_BLEND_WEIGHTS);
		UINT8* indexPtr = mMeshData->getElementData(VES_BLEND_INDICES);

		UINT32 stride = vertexDesc->getVertexStride(0);

		BoneWeight* weightDst = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			UINT8* indices = indexPtr;
			float* weights = (float*)weightPtr;

			weightDst->index0 = indices[0];
			weightDst->index1 = indices[1];
			weightDst->index2 = indices[2];
			weightDst->index3 = indices[3];

			weightDst->weight0 = weights[0];
			weightDst->weight1 = weights[1];
			weightDst->weight2 = weights[2];
			weightDst->weight3 = weights[3];

			weightDst++;
			indexPtr += stride;
			weightPtr += stride;
		}
	}

	void RendererMeshData::setBoneWeights(BoneWeight* buffer, UINT32 size)
	{
		SPtr<VertexDataDesc> vertexDesc = mMeshData->getVertexDesc();

		if (!vertexDesc->hasElement(VES_BLEND_WEIGHTS) ||
			!vertexDesc->hasElement(VES_BLEND_INDICES))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(BoneWeight) == size);

		UINT8* weightPtr = mMeshData->getElementData(VES_BLEND_WEIGHTS);
		UINT8* indexPtr = mMeshData->getElementData(VES_BLEND_INDICES);

		UINT32 stride = vertexDesc->getVertexStride(0);

		BoneWeight* weightSrc = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			UINT8* indices = indexPtr;
			float* weights = (float*)weightPtr;

			indices[0] = weightSrc->index0;
			indices[1] = weightSrc->index1;
			indices[2] = weightSrc->index2;
			indices[3] = weightSrc->index3;

			weights[0] = weightSrc->weight0;
			weights[1] = weightSrc->weight1;
			weights[2] = weightSrc->weight2;
			weights[3] = weightSrc->weight3;

			weightSrc++;
			indexPtr += stride;
			weightPtr += stride;
		}
	}

	void RendererMeshData::getIndices(UINT32* buffer, UINT32 size)
	{
		UINT32 indexSize = mMeshData->getIndexElementSize();
		UINT32 numIndices = mMeshData->getNumIndices();

		assert(numIndices * indexSize == size);

		if (mMeshData->getIndexType() == IT_16BIT)
		{
			UINT16* src = mMeshData->getIndices16();
			UINT32* dest = buffer;
			
			for (UINT32 i = 0; i < numIndices; i++)
			{
				*dest = *src;

				src++;
				dest++;
			}
		}
		else
		{
			memcpy(buffer, mMeshData->getIndices32(), size);
		}
	}

	void RendererMeshData::setIndices(UINT32* buffer, UINT32 size)
	{
		UINT32 indexSize = mMeshData->getIndexElementSize();
		UINT32 numIndices = mMeshData->getNumIndices();

		assert(numIndices * indexSize == size);

		if (mMeshData->getIndexType() == IT_16BIT)
		{
			UINT16* dest = mMeshData->getIndices16();
			UINT32* src = buffer;

			for (UINT32 i = 0; i < numIndices; i++)
			{
				*dest = *src;

				src++;
				dest++;
			}
		}
		else
		{
			memcpy(mMeshData->getIndices32(), buffer, size);
		}
	}

	SPtr<RendererMeshData> RendererMeshData::create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return RendererManager::instance().getActive()->_createMeshData(numVertices, numIndices, layout, indexType);
	}

	SPtr<RendererMeshData> RendererMeshData::create(const SPtr<MeshData>& meshData)
	{
		return RendererManager::instance().getActive()->_createMeshData(meshData);
	}

	SPtr<VertexDataDesc> RendererMeshData::vertexLayoutVertexDesc(VertexLayout type)
	{
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();

		INT32 intType = (INT32)type;

		if (intType == 0)
			type = VertexLayout::Position;

		if ((intType & (INT32)VertexLayout::Position) != 0)
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

		if ((intType & (INT32)VertexLayout::Normal) != 0)
			vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);

		if ((intType & (INT32)VertexLayout::Tangent) != 0)
			vertexDesc->addVertElem(VET_FLOAT4, VES_TANGENT);

		if ((intType & (INT32)VertexLayout::UV0) != 0)
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 0);

		if ((intType & (INT32)VertexLayout::UV1) != 0)
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 1);

		if ((intType & (INT32)VertexLayout::Color) != 0)
			vertexDesc->addVertElem(VET_FLOAT4, VES_COLOR);

		if ((intType & (INT32)VertexLayout::BoneWeights) != 0)
		{
			vertexDesc->addVertElem(VET_UBYTE4, VES_BLEND_INDICES);
			vertexDesc->addVertElem(VET_FLOAT4, VES_BLEND_WEIGHTS);
		}

		return vertexDesc;
	}
}