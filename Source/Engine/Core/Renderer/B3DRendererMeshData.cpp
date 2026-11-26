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
#include "RayTracing/B3DRayTracingGeometry.h"

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
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_POSITION);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	mMeshData->GetVertexData(VES_POSITION, buffer, size, 0, element->GetStreamIndex());
}

void RendererMeshData::SetPositions(Vector3* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_POSITION);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	mMeshData->SetVertexData(VES_POSITION, buffer, size, 0, element->GetStreamIndex());
}

void RendererMeshData::GetNormals(Vector3* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_NORMAL);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	u8* source = mMeshData->GetElementData(VES_NORMAL, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

	MeshUtility::UnpackNormals(source, buffer, numElements, stride);
}

void RendererMeshData::SetNormals(Vector3* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_NORMAL);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector3) == size);

	u8* destination = mMeshData->GetElementData(VES_NORMAL, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

	MeshUtility::PackNormals(buffer, destination, numElements, sizeof(Vector3), stride);
}

void RendererMeshData::GetTangents(Vector4* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TANGENT);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* source = mMeshData->GetElementData(VES_TANGENT, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

	MeshUtility::UnpackNormals(source, buffer, numElements, stride);
}

void RendererMeshData::SetTangents(Vector4* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TANGENT);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* destination = mMeshData->GetElementData(VES_TANGENT, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

	MeshUtility::PackNormals(buffer, destination, numElements, sizeof(Vector4), stride);
}

void RendererMeshData::GetColors(Color* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_COLOR);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* colorSrc = mMeshData->GetElementData(VES_COLOR, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

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
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_COLOR);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector4) == size);

	u8* colorDst = mMeshData->GetElementData(VES_COLOR, 0, element->GetStreamIndex());
	u32 stride = mMeshData->GetVertexDescription()->GetVertexStride(element->GetStreamIndex());

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
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_COLOR);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(u32) == size);

	mMeshData->SetVertexData(VES_COLOR, buffer, size, 0, element->GetStreamIndex());
}

void RendererMeshData::GetUV0(Vector2* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TEXCOORD);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 0, element->GetStreamIndex());
}

void RendererMeshData::SetUV0(Vector2* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TEXCOORD);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 0, element->GetStreamIndex());
}

void RendererMeshData::GetUV1(Vector2* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TEXCOORD, 1);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->GetVertexData(VES_TEXCOORD, buffer, size, 1, element->GetStreamIndex());
}

void RendererMeshData::SetUV1(Vector2* buffer, u32 size)
{
	const VertexElement* element = mMeshData->GetVertexDescription()->FindElement(VES_TEXCOORD, 1);
	if(element == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(Vector2) == size);

	mMeshData->SetVertexData(VES_TEXCOORD, buffer, size, 1, element->GetStreamIndex());
}

void RendererMeshData::GetBoneWeights(BoneWeight* buffer, u32 size)
{
	TShared<VertexDescription> vertexDesc = mMeshData->GetVertexDescription();

	const VertexElement* weightElement = vertexDesc->FindElement(VES_BLEND_WEIGHTS);
	const VertexElement* indexElement = vertexDesc->FindElement(VES_BLEND_INDICES);
	if(weightElement == nullptr || indexElement == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(BoneWeight) == size);

	u8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS, 0, weightElement->GetStreamIndex());
	u8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES, 0, indexElement->GetStreamIndex());

	u32 weightStride = vertexDesc->GetVertexStride(weightElement->GetStreamIndex());
	u32 indexStride = vertexDesc->GetVertexStride(indexElement->GetStreamIndex());

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
		indexPtr += indexStride;
		weightPtr += weightStride;
	}
}

void RendererMeshData::SetBoneWeights(BoneWeight* buffer, u32 size)
{
	TShared<VertexDescription> vertexDesc = mMeshData->GetVertexDescription();

	const VertexElement* weightElement = vertexDesc->FindElement(VES_BLEND_WEIGHTS);
	const VertexElement* indexElement = vertexDesc->FindElement(VES_BLEND_INDICES);
	if(weightElement == nullptr || indexElement == nullptr)
		return;

	u32 numElements = mMeshData->GetVertexCount();
	B3D_ASSERT(numElements * sizeof(BoneWeight) == size);

	u8* weightPtr = mMeshData->GetElementData(VES_BLEND_WEIGHTS, 0, weightElement->GetStreamIndex());
	u8* indexPtr = mMeshData->GetElementData(VES_BLEND_INDICES, 0, indexElement->GetStreamIndex());

	u32 weightStride = vertexDesc->GetVertexStride(weightElement->GetStreamIndex());
	u32 indexStride = vertexDesc->GetVertexStride(indexElement->GetStreamIndex());

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
		indexPtr += indexStride;
		weightPtr += weightStride;
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
	// Ray tracing geometry reads meshes in these formats, so imported meshes can be traced
	static_assert(render::RayTracingGeometry::kPositionType == VET_FLOAT3 && render::RayTracingGeometry::kNormalType == VET_UBYTE4_NORM &&
		render::RayTracingGeometry::kTangentType == VET_UBYTE4_NORM && render::RayTracingGeometry::kUvType == VET_FLOAT2,
		"The vertex formats of renderer mesh data must match the formats of ray tracing geometry.");

	TInlineArray<VertexElement, 8> vertexElements;
	i32 intType = (i32)type;

	if(intType == 0)
		type = VertexLayout::Position;

	// Attributes are split into streams by how they are used. Positions get their own stream so depth-only passes and acceleration structure builds
	// can read them alone. Deformable attributes and blend data come next, so the skin cache can replace them with its own streams and bind only the
	// last stream of the source mesh.
	const bool hasTangentBasis = (intType & ((i32)VertexLayout::Normal | (i32)VertexLayout::Tangent | (i32)VertexLayout::BoneWeights)) != 0;
	const u32 tangentBasisStreamIndex = 1;
	const u32 surfaceStreamIndex = hasTangentBasis ? 2 : 1;

	if((intType & (i32)VertexLayout::Position) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	if((intType & (i32)VertexLayout::Normal) != 0)
		vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL, 0, tangentBasisStreamIndex));

	if((intType & (i32)VertexLayout::Tangent) != 0)
		vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_TANGENT, 0, tangentBasisStreamIndex));

	if((intType & (i32)VertexLayout::UV0) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 0, surfaceStreamIndex));

	if((intType & (i32)VertexLayout::UV1) != 0)
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 1, surfaceStreamIndex));

	if((intType & (i32)VertexLayout::Color) != 0)
		vertexElements.Add(VertexElement(VET_COLOR, VES_COLOR, 0, surfaceStreamIndex));

	if((intType & (i32)VertexLayout::BoneWeights) != 0)
	{
		vertexElements.Add(VertexElement(VET_UBYTE4, VES_BLEND_INDICES, 0, tangentBasisStreamIndex));
		vertexElements.Add(VertexElement(VET_FLOAT4, VES_BLEND_WEIGHTS, 0, tangentBasisStreamIndex));
	}

	return B3DMakeShared<VertexDescription>(vertexElements);
}

TShared<MeshData> RendererMeshData::Convert(const TShared<MeshData>& meshData)
{
	// Note: Only converting between packed normals/tangents for now
	TShared<VertexDescription> vertexDesc = meshData->GetVertexDescription();

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();

	i32 type = 0;

	const VertexElement* posElem = vertexDesc->FindElement(VES_POSITION);
	if(posElem != nullptr && posElem->GetType() == VET_FLOAT3)
		type = (i32)VertexLayout::Position;

	const VertexElement* normalElem = vertexDesc->FindElement(VES_NORMAL);
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

	const VertexElement* tanElem = vertexDesc->FindElement(VES_TANGENT);
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

	const VertexElement* uv0Elem = vertexDesc->FindElement(VES_TEXCOORD);
	if(uv0Elem != nullptr && uv0Elem->GetType() == VET_FLOAT2)
		type |= (i32)VertexLayout::UV0;

	const VertexElement* uv1Elem = vertexDesc->FindElement(VES_TEXCOORD, 1);
	if(uv1Elem != nullptr && uv1Elem->GetType() == VET_FLOAT2)
		type |= (i32)VertexLayout::UV1;

	const VertexElement* colorElem = vertexDesc->FindElement(VES_COLOR);
	if(colorElem != nullptr && colorElem->GetType() == VET_COLOR)
		type |= (i32)VertexLayout::Color;

	const VertexElement* blendIndicesElem = vertexDesc->FindElement(VES_BLEND_INDICES);
	const VertexElement* blendWeightsElem = vertexDesc->FindElement(VES_BLEND_WEIGHTS);
	if(blendIndicesElem != nullptr && blendIndicesElem->GetType() == VET_UBYTE4 &&
	   blendWeightsElem != nullptr && blendWeightsElem->GetType() == VET_FLOAT4)
		type |= (i32)VertexLayout::BoneWeights;

	TShared<RendererMeshData> rendererMeshData = Create(numVertices, numIndices, (VertexLayout)type, meshData->GetIndexType());

	TShared<MeshData> output = rendererMeshData->mMeshData;
	TShared<VertexDescription> outputVertexDesc = output->GetVertexDescription();

	// Input and output may place an element in different streams, so offsets and strides are looked up for each separately
	const auto fnCopyElement = [&meshData, &vertexDesc, &output, &outputVertexDesc, numVertices](const VertexElement* inputElement, u32 elementSize)
	{
		const VertexElement* outputElement = outputVertexDesc->FindElement(inputElement->GetSemantic(), inputElement->GetSemanticIndex());

		u32 inputStride = vertexDesc->GetVertexStride(inputElement->GetStreamIndex());
		u32 outputStride = outputVertexDesc->GetVertexStride(outputElement->GetStreamIndex());

		u8* inData = meshData->GetElementData(inputElement->GetSemantic(), inputElement->GetSemanticIndex(), inputElement->GetStreamIndex());
		u8* outData = output->GetElementData(outputElement->GetSemantic(), outputElement->GetSemanticIndex(), outputElement->GetStreamIndex());
		for(u32 i = 0; i < numVertices; i++)
			memcpy(outData + i * outputStride, inData + i * inputStride, elementSize);
	};

	if((type & (i32)VertexLayout::Position) != 0)
		fnCopyElement(posElem, sizeof(Vector3));

	if((type & (i32)VertexLayout::Normal) != 0)
	{
		if(packNormals)
		{
			const VertexElement* outputElement = outputVertexDesc->FindElement(VES_NORMAL);

			u8* inData = meshData->GetElementData(VES_NORMAL, 0, normalElem->GetStreamIndex());
			u8* outData = output->GetElementData(VES_NORMAL, 0, outputElement->GetStreamIndex());
			MeshUtility::PackNormals((Vector3*)inData, outData, numVertices, vertexDesc->GetVertexStride(normalElem->GetStreamIndex()), outputVertexDesc->GetVertexStride(outputElement->GetStreamIndex()));
		}
		else
			fnCopyElement(normalElem, sizeof(u32));
	}

	if((type & (i32)VertexLayout::Tangent) != 0)
	{
		if(packTangents)
		{
			const VertexElement* outputElement = outputVertexDesc->FindElement(VES_TANGENT);

			u8* inData = meshData->GetElementData(VES_TANGENT, 0, tanElem->GetStreamIndex());
			u8* outData = output->GetElementData(VES_TANGENT, 0, outputElement->GetStreamIndex());
			MeshUtility::PackNormals((Vector4*)inData, outData, numVertices, vertexDesc->GetVertexStride(tanElem->GetStreamIndex()), outputVertexDesc->GetVertexStride(outputElement->GetStreamIndex()));
		}
		else
			fnCopyElement(tanElem, sizeof(u32));
	}

	if((type & (i32)VertexLayout::UV0) != 0)
		fnCopyElement(uv0Elem, sizeof(Vector2));

	if((type & (i32)VertexLayout::UV1) != 0)
		fnCopyElement(uv1Elem, sizeof(Vector2));

	if((type & (i32)VertexLayout::Color) != 0)
		fnCopyElement(colorElem, sizeof(u32));

	if((type & (i32)VertexLayout::BoneWeights) != 0)
	{
		fnCopyElement(blendIndicesElem, sizeof(u32));
		fnCopyElement(blendWeightsElem, sizeof(Vector4));
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
