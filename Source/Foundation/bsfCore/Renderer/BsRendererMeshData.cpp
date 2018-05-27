//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Image/BsColor.h"
#include "Image/BsPixelUtil.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Mesh/BsMeshUtility.h"

namespace bs
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

		mMeshData->getVertexData(VES_POSITION, buffer, size);
	}

	void RendererMeshData::setPositions(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_POSITION))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->setVertexData(VES_POSITION, buffer, size);
	}

	void RendererMeshData::getNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		UINT8* normalSrc = mMeshData->getElementData(VES_NORMAL);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		MeshUtility::unpackNormals(normalSrc, buffer, numElements, stride);
	}

	void RendererMeshData::setNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		UINT8* normalDst = mMeshData->getElementData(VES_NORMAL);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		MeshUtility::packNormals(buffer, normalDst, numElements, sizeof(Vector3), stride);
	}

	void RendererMeshData::getTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* tangentSrc = mMeshData->getElementData(VES_TANGENT);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		MeshUtility::unpackNormals(tangentSrc, buffer, numElements, stride);
	}

	void RendererMeshData::setTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* tangentDst = mMeshData->getElementData(VES_TANGENT);
		UINT32 stride = mMeshData->getVertexDesc()->getVertexStride(0);

		MeshUtility::packNormals(buffer, tangentDst, numElements, sizeof(Vector4), stride);
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
			PixelUtil::unpackColor(colorDst, PF_RGBA8, (void*)colorSrc);

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
			PixelUtil::packColor(*colorSrc, PF_RGBA8, (void*)colorDst);

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

		mMeshData->setVertexData(VES_COLOR, buffer, size);
	}

	void RendererMeshData::getUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->getVertexData(VES_TEXCOORD, buffer, size, 0);
	}

	void RendererMeshData::setUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->setVertexData(VES_TEXCOORD, buffer, size, 0);
	}

	void RendererMeshData::getUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->getVertexData(VES_TEXCOORD, buffer, size, 1);
	}

	void RendererMeshData::setUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->getVertexDesc()->hasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->getNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->setVertexData(VES_TEXCOORD, buffer, size, 1);
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
			vertexDesc->addVertElem(VET_UBYTE4_NORM, VES_NORMAL);

		if ((intType & (INT32)VertexLayout::Tangent) != 0)
			vertexDesc->addVertElem(VET_UBYTE4_NORM, VES_TANGENT);

		if ((intType & (INT32)VertexLayout::UV0) != 0)
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 0);

		if ((intType & (INT32)VertexLayout::UV1) != 0)
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 1);

		if ((intType & (INT32)VertexLayout::Color) != 0)
			vertexDesc->addVertElem(VET_COLOR, VES_COLOR);

		if ((intType & (INT32)VertexLayout::BoneWeights) != 0)
		{
			vertexDesc->addVertElem(VET_UBYTE4, VES_BLEND_INDICES);
			vertexDesc->addVertElem(VET_FLOAT4, VES_BLEND_WEIGHTS);
		}

		return vertexDesc;
	}

	SPtr<MeshData> RendererMeshData::convert(const SPtr<MeshData>& meshData)
	{
		// Note: Only converting between packed normals/tangents for now
		SPtr<VertexDataDesc> vertexDesc = meshData->getVertexDesc();

		UINT32 numVertices = meshData->getNumVertices();
		UINT32 numIndices = meshData->getNumIndices();
		UINT32 inputStride = vertexDesc->getVertexStride();

		INT32 type = 0;

		const VertexElement* posElem = vertexDesc->getElement(VES_POSITION);
		if (posElem != nullptr && posElem->getType() == VET_FLOAT3)
			type = (INT32)VertexLayout::Position;

		const VertexElement* normalElem = vertexDesc->getElement(VES_NORMAL);
		bool packNormals = false;
		if(normalElem != nullptr)
		{
			if (normalElem->getType() == VET_FLOAT3)
			{
				packNormals = true;
				type |= (INT32)VertexLayout::Normal;
			}
			else if(normalElem->getType() == VET_UBYTE4_NORM)
				type |= (INT32)VertexLayout::Normal;
		}

		const VertexElement* tanElem = vertexDesc->getElement(VES_TANGENT);
		bool packTangents = false;
		if (tanElem != nullptr)
		{
			if (tanElem->getType() == VET_FLOAT4)
			{
				packTangents = true;
				type |= (INT32)VertexLayout::Tangent;
			}
			else if (tanElem->getType() == VET_UBYTE4_NORM)
				type |= (INT32)VertexLayout::Tangent;
		}

		const VertexElement* uv0Elem = vertexDesc->getElement(VES_TEXCOORD, 0);
		if (uv0Elem != nullptr && uv0Elem->getType() == VET_FLOAT2)
			type |= (INT32)VertexLayout::UV0;

		const VertexElement* uv1Elem = vertexDesc->getElement(VES_TEXCOORD, 1);
		if (uv1Elem != nullptr && uv1Elem->getType() == VET_FLOAT2)
			type |= (INT32)VertexLayout::UV1;

		const VertexElement* colorElem = vertexDesc->getElement(VES_COLOR);
		if (colorElem != nullptr && colorElem->getType() == VET_COLOR)
			type |= (INT32)VertexLayout::Color;

		const VertexElement* blendIndicesElem = vertexDesc->getElement(VES_BLEND_INDICES);
		const VertexElement* blendWeightsElem = vertexDesc->getElement(VES_BLEND_WEIGHTS);
		if (blendIndicesElem != nullptr && blendIndicesElem->getType() == VET_UBYTE4 && 
			blendWeightsElem != nullptr && blendWeightsElem->getType() == VET_FLOAT4)
			type |= (INT32)VertexLayout::BoneWeights;

		SPtr<RendererMeshData> rendererMeshData = create(numVertices, numIndices, (VertexLayout)type, 
			meshData->getIndexType());

		SPtr<MeshData> output = rendererMeshData->mMeshData;
		SPtr<VertexDataDesc> outputVertexDesc = output->getVertexDesc();
		UINT32 outputStride = outputVertexDesc->getVertexStride();

		if((type & (INT32)VertexLayout::Position) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_POSITION);
			UINT8* outData = output->getElementData(VES_POSITION);
			for(UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector3));
		}

		if ((type & (INT32)VertexLayout::Normal) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_NORMAL);
			UINT8* outData = output->getElementData(VES_NORMAL);

			if (packNormals)
				MeshUtility::packNormals((Vector3*)inData, outData, numVertices, inputStride, outputStride);
			else
			{
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}
		}

		if ((type & (INT32)VertexLayout::Tangent) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_TANGENT);
			UINT8* outData = output->getElementData(VES_TANGENT);

			if (packTangents)
				MeshUtility::packNormals((Vector4*)inData, outData, numVertices, inputStride, outputStride);
			else
			{
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}
		}

		if ((type & (INT32)VertexLayout::UV0) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_TEXCOORD, 0);
			UINT8* outData = output->getElementData(VES_TEXCOORD, 0);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
		}

		if ((type & (INT32)VertexLayout::UV1) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_TEXCOORD, 1);
			UINT8* outData = output->getElementData(VES_TEXCOORD, 1);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
		}

		if ((type & (INT32)VertexLayout::Color) != 0)
		{
			UINT8* inData = meshData->getElementData(VES_COLOR, 0);
			UINT8* outData = output->getElementData(VES_COLOR, 0);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
		}

		if ((type & (INT32)VertexLayout::BoneWeights) != 0)
		{
			{
				UINT8* inData = meshData->getElementData(VES_BLEND_INDICES, 0);
				UINT8* outData = output->getElementData(VES_BLEND_INDICES, 0);
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}

			{
				UINT8* inData = meshData->getElementData(VES_BLEND_WEIGHTS, 0);
				UINT8* outData = output->getElementData(VES_BLEND_WEIGHTS, 0);
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector4));
			}
		}

		if(meshData->getIndexType() == IT_32BIT)
		{
			UINT32* dst = output->getIndices32();
			memcpy(dst, meshData->getIndices32(), numIndices * sizeof(UINT32));
		}
		else
		{
			UINT16* dst = output->getIndices16();
			memcpy(dst, meshData->getIndices16(), numIndices * sizeof(UINT16));
		}

		return output;
	}
}