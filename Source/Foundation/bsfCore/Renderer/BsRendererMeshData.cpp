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
		SPtr<VertexDataDesc> vertexDesc = VertexLayoutVertexDesc(layout);

		mMeshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc, indexType);
	}

	RendererMeshData::RendererMeshData(const SPtr<MeshData>& meshData)
		:mMeshData(meshData)
	{

	}

	void RendererMeshData::GetPositions(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_POSITION))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->GetVertexData(VES_POSITION, buffer, size);
	}

	void RendererMeshData::SetPositions(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_POSITION))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		mMeshData->SetVertexData(VES_POSITION, buffer, size);
	}

	void RendererMeshData::GetNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		UINT8* normalSrc = mMeshData->GetElementData(VES_NORMAL);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		MeshUtility::UnpackNormals(normalSrc, buffer, numElements, stride);
	}

	void RendererMeshData::SetNormals(Vector3* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_NORMAL))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector3) == size);

		UINT8* normalDst = mMeshData->GetElementData(VES_NORMAL);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		MeshUtility::PackNormals(buffer, normalDst, numElements, sizeof(Vector3), stride);
	}

	void RendererMeshData::GetTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* tangentSrc = mMeshData->GetElementData(VES_TANGENT);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		MeshUtility::UnpackNormals(tangentSrc, buffer, numElements, stride);
	}

	void RendererMeshData::SetTangents(Vector4* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TANGENT))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* tangentDst = mMeshData->GetElementData(VES_TANGENT);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		MeshUtility::PackNormals(buffer, tangentDst, numElements, sizeof(Vector4), stride);
	}

	void RendererMeshData::GetColors(Color* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* colorSrc = mMeshData->GetElementData(VES_COLOR);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		Color* colorDst = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			PixelUtil::UnpackColor(colorDst, PF_RGBA8, (void*)colorSrc);

			colorSrc += stride;
			colorDst++;
		}
	}

	void RendererMeshData::SetColors(Color* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector4) == size);

		UINT8* colorDst = mMeshData->GetElementData(VES_COLOR);
		UINT32 stride = mMeshData->GetVertexDesc()->GetVertexStride(0);

		Color* colorSrc = buffer;
		for (UINT32 i = 0; i < numElements; i++)
		{
			PixelUtil::PackColor(*colorSrc, PF_RGBA8, (void*)colorDst);

			colorSrc++;
			colorDst += stride;
		}
	}

	void RendererMeshData::SetColors(UINT32* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_COLOR))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(UINT32) == size);

		mMeshData->SetVertexData(VES_COLOR, buffer, size);
	}

	void RendererMeshData::GetUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 0);
	}

	void RendererMeshData::SetUV0(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TEXCOORD, 0))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 0);
	}

	void RendererMeshData::GetUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 1);
	}

	void RendererMeshData::SetUV1(Vector2* buffer, UINT32 size)
	{
		if (!mMeshData->GetVertexDesc()->HasElement(VES_TEXCOORD, 1))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(Vector2) == size);

		mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 1);
	}

	void RendererMeshData::GetBoneWeights(BoneWeight* buffer, UINT32 size)
	{
		SPtr<VertexDataDesc> vertexDesc = mMeshData->GetVertexDesc();

		if (!vertexDesc->HasElement(VES_BLEND_WEIGHTS) ||
			!vertexDesc->HasElement(VES_BLEND_INDICES))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(BoneWeight) == size);

		UINT8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS);
		UINT8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES);

		UINT32 stride = vertexDesc->GetVertexStride(0);

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

	void RendererMeshData::SetBoneWeights(BoneWeight* buffer, UINT32 size)
	{
		SPtr<VertexDataDesc> vertexDesc = mMeshData->GetVertexDesc();

		if (!vertexDesc->HasElement(VES_BLEND_WEIGHTS) ||
			!vertexDesc->HasElement(VES_BLEND_INDICES))
			return;

		UINT32 numElements = mMeshData->GetNumVertices();
		assert(numElements * sizeof(BoneWeight) == size);

		UINT8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS);
		UINT8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES);

		UINT32 stride = vertexDesc->GetVertexStride(0);

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

	void RendererMeshData::GetIndices(UINT32* buffer, UINT32 size)
	{
		UINT32 indexSize = mMeshData->GetIndexElementSize();
		UINT32 numIndices = mMeshData->GetNumIndices();

		assert(numIndices * indexSize == size);

		if (mMeshData->GetIndexType() == IT_16BIT)
		{
			UINT16* src = mMeshData->GetIndices16();
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
			memcpy(buffer, mMeshData->GetIndices32(), size);
		}
	}

	void RendererMeshData::SetIndices(UINT32* buffer, UINT32 size)
	{
		UINT32 indexSize = mMeshData->GetIndexElementSize();
		UINT32 numIndices = mMeshData->GetNumIndices();

		assert(numIndices * indexSize == size);

		if (mMeshData->GetIndexType() == IT_16BIT)
		{
			UINT16* dest = mMeshData->GetIndices16();
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
			memcpy(mMeshData->GetIndices32(), buffer, size);
		}
	}

	SPtr<RendererMeshData> RendererMeshData::Create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return RendererManager::Instance().GetActive()->CreateMeshDataInternal(numVertices, numIndices, layout, indexType);
	}

	SPtr<RendererMeshData> RendererMeshData::Create(const SPtr<MeshData>& meshData)
	{
		return RendererManager::Instance().GetActive()->CreateMeshDataInternal(meshData);
	}

	SPtr<VertexDataDesc> RendererMeshData::VertexLayoutVertexDesc(VertexLayout type)
	{
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::Create();

		INT32 intType = (INT32)type;

		if (intType == 0)
			type = VertexLayout::Position;

		if ((intType & (INT32)VertexLayout::Position) != 0)
			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

		if ((intType & (INT32)VertexLayout::Normal) != 0)
			vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_NORMAL);

		if ((intType & (INT32)VertexLayout::Tangent) != 0)
			vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_TANGENT);

		if ((intType & (INT32)VertexLayout::UV0) != 0)
			vertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 0);

		if ((intType & (INT32)VertexLayout::UV1) != 0)
			vertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 1);

		if ((intType & (INT32)VertexLayout::Color) != 0)
			vertexDesc->AddVertElem(VET_COLOR, VES_COLOR);

		if ((intType & (INT32)VertexLayout::BoneWeights) != 0)
		{
			vertexDesc->AddVertElem(VET_UBYTE4, VES_BLEND_INDICES);
			vertexDesc->AddVertElem(VET_FLOAT4, VES_BLEND_WEIGHTS);
		}

		return vertexDesc;
	}

	SPtr<MeshData> RendererMeshData::Convert(const SPtr<MeshData>& meshData)
	{
		// Note: Only converting between packed normals/tangents for now
		SPtr<VertexDataDesc> vertexDesc = meshData->GetVertexDesc();

		UINT32 numVertices = meshData->GetNumVertices();
		UINT32 numIndices = meshData->GetNumIndices();
		UINT32 inputStride = vertexDesc->GetVertexStride();

		INT32 type = 0;

		const VertexElement* posElem = vertexDesc->GetElement(VES_POSITION);
		if (posElem != nullptr && posElem->GetType() == VET_FLOAT3)
			type = (INT32)VertexLayout::Position;

		const VertexElement* normalElem = vertexDesc->GetElement(VES_NORMAL);
		bool packNormals = false;
		if(normalElem != nullptr)
		{
			if (normalElem->GetType() == VET_FLOAT3)
			{
				packNormals = true;
				type |= (INT32)VertexLayout::Normal;
			}
			else if(normalElem->GetType() == VET_UBYTE4_NORM)
				type |= (INT32)VertexLayout::Normal;
		}

		const VertexElement* tanElem = vertexDesc->GetElement(VES_TANGENT);
		bool packTangents = false;
		if (tanElem != nullptr)
		{
			if (tanElem->GetType() == VET_FLOAT4)
			{
				packTangents = true;
				type |= (INT32)VertexLayout::Tangent;
			}
			else if (tanElem->GetType() == VET_UBYTE4_NORM)
				type |= (INT32)VertexLayout::Tangent;
		}

		const VertexElement* uv0Elem = vertexDesc->GetElement(VES_TEXCOORD, 0);
		if (uv0Elem != nullptr && uv0Elem->GetType() == VET_FLOAT2)
			type |= (INT32)VertexLayout::UV0;

		const VertexElement* uv1Elem = vertexDesc->GetElement(VES_TEXCOORD, 1);
		if (uv1Elem != nullptr && uv1Elem->GetType() == VET_FLOAT2)
			type |= (INT32)VertexLayout::UV1;

		const VertexElement* colorElem = vertexDesc->GetElement(VES_COLOR);
		if (colorElem != nullptr && colorElem->GetType() == VET_COLOR)
			type |= (INT32)VertexLayout::Color;

		const VertexElement* blendIndicesElem = vertexDesc->GetElement(VES_BLEND_INDICES);
		const VertexElement* blendWeightsElem = vertexDesc->GetElement(VES_BLEND_WEIGHTS);
		if (blendIndicesElem != nullptr && blendIndicesElem->GetType() == VET_UBYTE4 &&
			blendWeightsElem != nullptr && blendWeightsElem->GetType() == VET_FLOAT4)
			type |= (INT32)VertexLayout::BoneWeights;

		SPtr<RendererMeshData> rendererMeshData = Create(numVertices, numIndices, (VertexLayout)type,
			meshData->GetIndexType());

		SPtr<MeshData> output = rendererMeshData->mMeshData;
		SPtr<VertexDataDesc> outputVertexDesc = output->GetVertexDesc();
		UINT32 outputStride = outputVertexDesc->GetVertexStride();

		if((type & (INT32)VertexLayout::Position) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_POSITION);
			UINT8* outData = output->GetElementData(VES_POSITION);
			for(UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector3));
		}

		if ((type & (INT32)VertexLayout::Normal) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_NORMAL);
			UINT8* outData = output->GetElementData(VES_NORMAL);

			if (packNormals)
				MeshUtility::PackNormals((Vector3*)inData, outData, numVertices, inputStride, outputStride);
			else
			{
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}
		}

		if ((type & (INT32)VertexLayout::Tangent) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_TANGENT);
			UINT8* outData = output->GetElementData(VES_TANGENT);

			if (packTangents)
				MeshUtility::PackNormals((Vector4*)inData, outData, numVertices, inputStride, outputStride);
			else
			{
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}
		}

		if ((type & (INT32)VertexLayout::UV0) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_TEXCOORD, 0);
			UINT8* outData = output->GetElementData(VES_TEXCOORD, 0);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
		}

		if ((type & (INT32)VertexLayout::UV1) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_TEXCOORD, 1);
			UINT8* outData = output->GetElementData(VES_TEXCOORD, 1);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
		}

		if ((type & (INT32)VertexLayout::Color) != 0)
		{
			UINT8* inData = meshData->GetElementData(VES_COLOR, 0);
			UINT8* outData = output->GetElementData(VES_COLOR, 0);
			for (UINT32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
		}

		if ((type & (INT32)VertexLayout::BoneWeights) != 0)
		{
			{
				UINT8* inData = meshData->GetElementData(VES_BLEND_INDICES, 0);
				UINT8* outData = output->GetElementData(VES_BLEND_INDICES, 0);
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(UINT32));
			}

			{
				UINT8* inData = meshData->GetElementData(VES_BLEND_WEIGHTS, 0);
				UINT8* outData = output->GetElementData(VES_BLEND_WEIGHTS, 0);
				for (UINT32 i = 0; i < numVertices; i++)
					memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector4));
			}
		}

		if(meshData->GetIndexType() == IT_32BIT)
		{
			UINT32* dst = output->GetIndices32();
			memcpy(dst, meshData->GetIndices32(), numIndices * sizeof(UINT32));
		}
		else
		{
			UINT16* dst = output->GetIndices16();
			memcpy(dst, meshData->GetIndices16(), numIndices * sizeof(UINT16));
		}

		return output;
	}
}
