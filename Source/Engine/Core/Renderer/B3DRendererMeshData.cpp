//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererMeshData.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Image/B3DColor.h"
#include "Image/B3DPixelUtility.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRenderer.h"
#include "Mesh/B3DMeshUtility.h"

using namespace b3d;

RendererMeshData::RendererMeshData(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	TShared<VertexDescription> vertexDesc = VertexLayoutVertexDesc(layout);

	mMeshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDesc, indexType);
}

RendererMeshData::RendererMeshData(const TShared<MeshData>& meshData)
	: mMeshData(meshData)
{
}

void RendererMeshData::GetPositions(Vector3* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_POSITION))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	mMeshData->GetVertexData(VES_POSITION, buffer, size);
}

void RendererMeshData::SetPositions(Vector3* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_POSITION))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	mMeshData->SetVertexData(VES_POSITION, buffer, size);
}

void RendererMeshData::GetNormals(Vector3* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_NORMAL))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	u8* normalSrc = mMeshData->GetElementData(VES_NORMAL);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	MeshUtility::UnpackNormals(normalSrc, buffer, numElements, stride);
}

void RendererMeshData::SetNormals(Vector3* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_NORMAL))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	u8* normalDst = mMeshData->GetElementData(VES_NORMAL);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	MeshUtility::PackNormals(buffer, normalDst, numElements, sizeof(Vector3), stride);
}

void RendererMeshData::GetTangents(Vector4* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TANGENT))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* tangentSrc = mMeshData->GetElementData(VES_TANGENT);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	MeshUtility::UnpackNormals(tangentSrc, buffer, numElements, stride);
}

void RendererMeshData::SetTangents(Vector4* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TANGENT))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* tangentDst = mMeshData->GetElementData(VES_TANGENT);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	MeshUtility::PackNormals(buffer, tangentDst, numElements, sizeof(Vector4), stride);
}

void RendererMeshData::GetColors(Color* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_COLOR))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* colorSrc = mMeshData->GetElementData(VES_COLOR);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	Color* colorDst = buffer;
	for(u32 i = 0; i < numElements; i++)
	{
		PixelUtility::UnpackColor(colorDst, PF_RGBA8, (void*)colorSrc);

		colorSrc += stride;
		colorDst++;
	}
}

void RendererMeshData::SetColors(Color* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_COLOR))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* colorDst = mMeshData->GetElementData(VES_COLOR);
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(0);

	Color* colorSrc = buffer;
	for(u32 i = 0; i < numElements; i++)
	{
		PixelUtility::PackColor(*colorSrc, PF_RGBA8, (void*)colorDst);

		colorSrc++;
		colorDst += stride;
	}
}

void RendererMeshData::SetColors(u32* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_COLOR))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(u32) == size);

	mMeshData->SetVertexData(VES_COLOR, buffer, size);
}

void RendererMeshData::GetUV0(Vector2* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TEXCOORD, 0))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 0);
}

void RendererMeshData::SetUV0(Vector2* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TEXCOORD, 0))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 0);
}

void RendererMeshData::GetUV1(Vector2* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TEXCOORD, 1))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 1);
}

void RendererMeshData::SetUV1(Vector2* buffer, u32 size)
{
	if(!mMeshData->GetVertexDescription()->HasElement(VES_TEXCOORD, 1))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 1);
}

void RendererMeshData::GetBoneWeights(BoneWeight* buffer, u32 size)
{
	TShared<VertexDescription> vertexDesc = mMeshData->GetVertexDescription();

	if(!vertexDesc->HasElement(VES_BLEND_WEIGHTS) ||
	   !vertexDesc->HasElement(VES_BLEND_INDICES))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(BoneWeight) == size);

	u8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS);
	u8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES);

	u32 stride = vertexDesc->GetVertexStride(0);

	BoneWeight* weightDst = buffer;
	for(u32 i = 0; i < numElements; i++)
	{
		u8* indices = indexPtr;
		float* weights = (float*)weightPtr;

		weightDst->Index0 = indices[0];
		weightDst->Index1 = indices[1];
		weightDst->Index2 = indices[2];
		weightDst->Index3 = indices[3];

		weightDst->Weight0 = weights[0];
		weightDst->Weight1 = weights[1];
		weightDst->Weight2 = weights[2];
		weightDst->Weight3 = weights[3];

		weightDst++;
		indexPtr += stride;
		weightPtr += stride;
	}
}

void RendererMeshData::SetBoneWeights(BoneWeight* buffer, u32 size)
{
	TShared<VertexDescription> vertexDesc = mMeshData->GetVertexDescription();

	if(!vertexDesc->HasElement(VES_BLEND_WEIGHTS) ||
	   !vertexDesc->HasElement(VES_BLEND_INDICES))
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(BoneWeight) == size);

	u8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS);
	u8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES);

	u32 stride = vertexDesc->GetVertexStride(0);

	BoneWeight* weightSrc = buffer;
	for(u32 i = 0; i < numElements; i++)
	{
		u8* indices = indexPtr;
		float* weights = (float*)weightPtr;

		indices[0] = weightSrc->Index0;
		indices[1] = weightSrc->Index1;
		indices[2] = weightSrc->Index2;
		indices[3] = weightSrc->Index3;

		weights[0] = weightSrc->Weight0;
		weights[1] = weightSrc->Weight1;
		weights[2] = weightSrc->Weight2;
		weights[3] = weightSrc->Weight3;

		weightSrc++;
		indexPtr += stride;
		weightPtr += stride;
	}
}

void RendererMeshData::GetIndices(u32* buffer, u32 size)
{
	u32 indexSize = mMeshData->GetIndexElementSize();
	u32 numIndices = mMeshData->GetIndexCount();

	B3D_ASSERT(numIndices * indexSize == size);

	if(mMeshData->GetIndexType() == IT_16BIT)
	{
		u16* src = mMeshData->GetIndices16();
		u32* dest = buffer;

		for(u32 i = 0; i < numIndices; i++)
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

void RendererMeshData::SetIndices(u32* buffer, u32 size)
{
	u32 indexSize = mMeshData->GetIndexElementSize();
	u32 numIndices = mMeshData->GetIndexCount();

	B3D_ASSERT(numIndices * indexSize == size);

	if(mMeshData->GetIndexType() == IT_16BIT)
	{
		u16* dest = mMeshData->GetIndices16();
		u32* src = buffer;

		for(u32 i = 0; i < numIndices; i++)
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

TShared<RendererMeshData> RendererMeshData::Create(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	return RendererManager::Instance().GetActive()->CreateMeshDataInternal(numVertices, numIndices, layout, indexType);
}

TShared<RendererMeshData> RendererMeshData::Create(const TShared<MeshData>& meshData)
{
	return RendererManager::Instance().GetActive()->CreateMeshDataInternal(meshData);
}

TShared<VertexDescription> RendererMeshData::VertexLayoutVertexDesc(VertexLayout type)
{
	TInlineArray<VertexElement, 8> vertexElements;
	i32 intType = (i32)type;

	if(intType == 0)
		type = VertexLayout::Position;

	if((intType & (i32)VertexLayout::Position) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	if((intType & (i32)VertexLayout::Normal) != 0)
		vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL));

	if((intType & (i32)VertexLayout::Tangent) != 0)
		vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_TANGENT));

	if((intType & (i32)VertexLayout::UV0) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 0));

	if((intType & (i32)VertexLayout::UV1) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 1));

	if((intType & (i32)VertexLayout::Color) != 0)
		vertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	if((intType & (i32)VertexLayout::BoneWeights) != 0)
	{
		vertexElements.Add(VertexElement(VET_UBYTE4, VES_BLEND_INDICES));
		vertexElements.Add(VertexElement(VET_FLOAT4, VES_BLEND_WEIGHTS));
	}

	return B3DMakeShared<VertexDescription>(vertexElements);
}

TShared<MeshData> RendererMeshData::Convert(const TShared<MeshData>& meshData)
{
	// Note: Only converting between packed normals/tangents for now
	TShared<VertexDescription> vertexDesc = meshData->GetVertexDescription();

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 inputStride = vertexDesc->GetVertexStride();

	i32 type = 0;

	const VertexElement* posElem = vertexDesc->GetElement(VES_POSITION);
	if(posElem != nullptr && posElem->GetType() == VET_FLOAT3)
		type = (i32)VertexLayout::Position;

	const VertexElement* normalElem = vertexDesc->GetElement(VES_NORMAL);
	bool packNormals = false;
	if(normalElem != nullptr)
	{
		if(normalElem->GetType() == VET_FLOAT3)
		{
			packNormals = true;
			type |= (i32)VertexLayout::Normal;
		}
		else if(normalElem->GetType() == VET_UBYTE4_NORM)
			type |= (i32)VertexLayout::Normal;
	}

	const VertexElement* tanElem = vertexDesc->GetElement(VES_TANGENT);
	bool packTangents = false;
	if(tanElem != nullptr)
	{
		if(tanElem->GetType() == VET_FLOAT4)
		{
			packTangents = true;
			type |= (i32)VertexLayout::Tangent;
		}
		else if(tanElem->GetType() == VET_UBYTE4_NORM)
			type |= (i32)VertexLayout::Tangent;
	}

	const VertexElement* uv0Elem = vertexDesc->GetElement(VES_TEXCOORD, 0);
	if(uv0Elem != nullptr && uv0Elem->GetType() == VET_FLOAT2)
		type |= (i32)VertexLayout::UV0;

	const VertexElement* uv1Elem = vertexDesc->GetElement(VES_TEXCOORD, 1);
	if(uv1Elem != nullptr && uv1Elem->GetType() == VET_FLOAT2)
		type |= (i32)VertexLayout::UV1;

	const VertexElement* colorElem = vertexDesc->GetElement(VES_COLOR);
	if(colorElem != nullptr && colorElem->GetType() == VET_COLOR)
		type |= (i32)VertexLayout::Color;

	const VertexElement* blendIndicesElem = vertexDesc->GetElement(VES_BLEND_INDICES);
	const VertexElement* blendWeightsElem = vertexDesc->GetElement(VES_BLEND_WEIGHTS);
	if(blendIndicesElem != nullptr && blendIndicesElem->GetType() == VET_UBYTE4 &&
	   blendWeightsElem != nullptr && blendWeightsElem->GetType() == VET_FLOAT4)
		type |= (i32)VertexLayout::BoneWeights;

	TShared<RendererMeshData> rendererMeshData = Create(numVertices, numIndices, (VertexLayout)type, meshData->GetIndexType());

	TShared<MeshData> output = rendererMeshData->mMeshData;
	TShared<VertexDescription> outputVertexDesc = output->GetVertexDescription();
	u32 outputStride = outputVertexDesc->GetVertexStride();

	if((type & (i32)VertexLayout::Position) != 0)
	{
		u8* inData = meshData->GetElementData(VES_POSITION);
		u8* outData = output->GetElementData(VES_POSITION);
		for(u32 i = 0; i < numVertices; i++)
			memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector3));
	}

	if((type & (i32)VertexLayout::Normal) != 0)
	{
		u8* inData = meshData->GetElementData(VES_NORMAL);
		u8* outData = output->GetElementData(VES_NORMAL);

		if(packNormals)
			MeshUtility::PackNormals((Vector3*)inData, outData, numVertices, inputStride, outputStride);
		else
		{
			for(u32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(u32));
		}
	}

	if((type & (i32)VertexLayout::Tangent) != 0)
	{
		u8* inData = meshData->GetElementData(VES_TANGENT);
		u8* outData = output->GetElementData(VES_TANGENT);

		if(packTangents)
			MeshUtility::PackNormals((Vector4*)inData, outData, numVertices, inputStride, outputStride);
		else
		{
			for(u32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(u32));
		}
	}

	if((type & (i32)VertexLayout::UV0) != 0)
	{
		u8* inData = meshData->GetElementData(VES_TEXCOORD, 0);
		u8* outData = output->GetElementData(VES_TEXCOORD, 0);
		for(u32 i = 0; i < numVertices; i++)
			memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
	}

	if((type & (i32)VertexLayout::UV1) != 0)
	{
		u8* inData = meshData->GetElementData(VES_TEXCOORD, 1);
		u8* outData = output->GetElementData(VES_TEXCOORD, 1);
		for(u32 i = 0; i < numVertices; i++)
			memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector2));
	}

	if((type & (i32)VertexLayout::Color) != 0)
	{
		u8* inData = meshData->GetElementData(VES_COLOR, 0);
		u8* outData = output->GetElementData(VES_COLOR, 0);
		for(u32 i = 0; i < numVertices; i++)
			memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(u32));
	}

	if((type & (i32)VertexLayout::BoneWeights) != 0)
	{
		{
			u8* inData = meshData->GetElementData(VES_BLEND_INDICES, 0);
			u8* outData = output->GetElementData(VES_BLEND_INDICES, 0);
			for(u32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(u32));
		}

		{
			u8* inData = meshData->GetElementData(VES_BLEND_WEIGHTS, 0);
			u8* outData = output->GetElementData(VES_BLEND_WEIGHTS, 0);
			for(u32 i = 0; i < numVertices; i++)
				memcpy(outData + i * outputStride, inData + i * inputStride, sizeof(Vector4));
		}
	}

	if(meshData->GetIndexType() == IT_32BIT)
	{
		u32* dst = output->GetIndices32();
		memcpy(dst, meshData->GetIndices32(), numIndices * sizeof(u32));
	}
	else
	{
		u16* dst = output->GetIndices16();
		memcpy(dst, meshData->GetIndices16(), numIndices * sizeof(u16));
	}

	return output;
}
