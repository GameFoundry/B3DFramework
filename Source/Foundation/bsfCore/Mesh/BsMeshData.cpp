//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshData.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsSphere.h"
#include "Math/BsAABox.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Private/RTTI/BsMeshDataRTTI.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"

using namespace bs;

MeshData::MeshData(u32 numVertices, u32 numIndexes, const SPtr<VertexDataDesc>& vertexData, IndexType indexType)
	: mNumVertices(numVertices), mNumIndices(numIndexes), mIndexType(indexType), mVertexData(vertexData)
{
	AllocateInternalBuffer();
}

MeshData::MeshData()
	: mNumVertices(0), mNumIndices(0), mIndexType(IT_32BIT)
{}

MeshData::~MeshData()
{}

u32 MeshData::GetNumIndices() const
{
	return mNumIndices;
}

u16* MeshData::GetIndices16() const
{
	if(mIndexType != IT_16BIT)
		BS_EXCEPT(InternalErrorException, "Attempting to get 16bit index buffer, but internally allocated buffer is 32 bit.");

	u32 indexBufferOffset = GetIndexBufferOffset();

	return (u16*)(GetData() + indexBufferOffset);
}

u32* MeshData::GetIndices32() const
{
	if(mIndexType != IT_32BIT)
		BS_EXCEPT(InternalErrorException, "Attempting to get 32bit index buffer, but internally allocated buffer is 16 bit.");

	u32 indexBufferOffset = GetIndexBufferOffset();

	return (u32*)(GetData() + indexBufferOffset);
}

u32 MeshData::GetInternalBufferSize() const
{
	return GetIndexBufferSize() + GetStreamSize();
}

// TODO - This doesn't handle the case where multiple elements in same slot have different data types
SPtr<MeshData> MeshData::Combine(const Vector<SPtr<MeshData>>& meshes, const Vector<Vector<SubMesh>>& allSubMeshes, Vector<SubMesh>& subMeshes)
{
	u32 totalVertexCount = 0;
	u32 totalIndexCount = 0;
	for(auto& meshData : meshes)
	{
		totalVertexCount += meshData->GetNumVertices();
		totalIndexCount += meshData->GetNumIndices();
	}

	SPtr<VertexDataDesc> vertexData = bs_shared_ptr_new<VertexDataDesc>();

	Vector<VertexElement> combinedVertexElements;
	for(auto& meshData : meshes)
	{
		for(u32 i = 0; i < meshData->GetVertexDesc()->GetNumElements(); i++)
		{
			const VertexElement& newElement = meshData->GetVertexDesc()->GetElement(i);

			i32 alreadyExistsIdx = -1;
			u32 idx = 0;

			for(auto& existingElement : combinedVertexElements)
			{
				if(newElement.GetSemantic() == existingElement.GetSemantic() && newElement.GetSemanticIdx() == existingElement.GetSemanticIdx() && newElement.GetStreamIdx() == existingElement.GetStreamIdx())
				{
					if(newElement.GetType() != existingElement.GetType())
					{
						BS_EXCEPT(NotImplementedException, "Two elements have same semantics but different types. This is not supported.");
					}

					alreadyExistsIdx = idx;
					break;
				}

				idx++;
			}

			if(alreadyExistsIdx == -1)
			{
				combinedVertexElements.push_back(newElement);
				vertexData->AddVertElem(newElement.GetType(), newElement.GetSemantic(), newElement.GetSemanticIdx(), newElement.GetStreamIdx());
			}
		}
	}

	SPtr<MeshData> combinedMeshData = bs_shared_ptr_new<MeshData>(totalVertexCount, totalIndexCount, vertexData);

	// Copy indices
	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	u32* idxPtr = combinedMeshData->GetIndices32();
	for(auto& meshData : meshes)
	{
		u32 numIndices = meshData->GetNumIndices();
		u32* srcData = meshData->GetIndices32();

		for(u32 j = 0; j < numIndices; j++)
			idxPtr[j] = srcData[j] + vertexOffset;

		indexOffset += numIndices;
		idxPtr += numIndices;
		vertexOffset += meshData->GetNumVertices();
	}

	// Copy sub-meshes
	u32 meshIdx = 0;
	indexOffset = 0;
	for(auto& meshData : meshes)
	{
		u32 numIndices = meshData->GetNumIndices();
		const Vector<SubMesh> curSubMeshes = allSubMeshes[meshIdx];

		for(auto& subMesh : curSubMeshes)
		{
			subMeshes.push_back(SubMesh(subMesh.IndexOffset + indexOffset, subMesh.IndexCount, subMesh.DrawOp));
		}

		indexOffset += numIndices;
		meshIdx++;
	}

	// Copy vertices
	vertexOffset = 0;
	for(auto& meshData : meshes)
	{
		for(auto& element : combinedVertexElements)
		{
			u32 dstVertexStride = vertexData->GetVertexStride(element.GetStreamIdx());
			u8* dstData = combinedMeshData->GetElementData(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());
			dstData += vertexOffset * dstVertexStride;

			u32 numSrcVertices = meshData->GetNumVertices();
			u32 vertexSize = vertexData->GetElementSize(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());

			if(meshData->GetVertexDesc()->HasElement(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx()))
			{
				u32 srcVertexStride = meshData->GetVertexDesc()->GetVertexStride(element.GetStreamIdx());
				u8* srcData = meshData->GetElementData(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());

				for(u32 i = 0; i < numSrcVertices; i++)
				{
					memcpy(dstData, srcData, vertexSize);
					dstData += dstVertexStride;
					srcData += srcVertexStride;
				}
			}
			else
			{
				for(u32 i = 0; i < numSrcVertices; i++)
				{
					memset(dstData, 0, vertexSize);
					dstData += dstVertexStride;
				}
			}
		}

		vertexOffset += meshData->GetNumVertices();
	}

	return combinedMeshData;
}

void MeshData::SetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIdx, u32 streamIdx)
{
	assert(data != nullptr);

	if(!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
	{
		BS_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIdx, streamIdx);
		return;
	}

	u32 elementSize = mVertexData->GetElementSize(semantic, semanticIdx, streamIdx);
	u32 totalSize = elementSize * mNumVertices;

	if(totalSize != size)
	{
		BS_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + toString(totalSize) + ". Got: " + toString(size));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIdx, streamIdx);
	u32 vertexStride = mVertexData->GetVertexStride(streamIdx);

	u8* dst = GetData() + indexBufferOffset + elementOffset;
	u8* src = (u8*)data;
	for(u32 i = 0; i < mNumVertices; i++)
	{
		memcpy(dst, src, elementSize);
		dst += vertexStride;
		src += elementSize;
	}
}

void MeshData::GetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIdx, u32 streamIdx)
{
	assert(data != nullptr);

	if(!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
	{
		BS_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIdx, streamIdx);
		return;
	}

	u32 elementSize = mVertexData->GetElementSize(semantic, semanticIdx, streamIdx);
	u32 totalSize = elementSize * mNumVertices;

	if(totalSize != size)
	{
		BS_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + toString(totalSize) + ". Got: " + toString(size));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIdx, streamIdx);
	u32 vertexStride = mVertexData->GetVertexStride(streamIdx);

	u8* src = GetData() + indexBufferOffset + elementOffset;
	u8* dst = (u8*)data;
	for(u32 i = 0; i < mNumVertices; i++)
	{
		memcpy(dst, src, elementSize);
		dst += elementSize;
		src += vertexStride;
	}
}

VertexElemIter<Vector2> MeshData::GetVec2DataIter(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

	return VertexElemIter<Vector2>(data, vertexStride, mNumVertices);
}

VertexElemIter<Vector3> MeshData::GetVec3DataIter(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

	return VertexElemIter<Vector3>(data, vertexStride, mNumVertices);
}

VertexElemIter<Vector4> MeshData::GetVec4DataIter(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

	return VertexElemIter<Vector4>(data, vertexStride, mNumVertices);
}

VertexElemIter<u32> MeshData::GetDwordDataIter(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

	return VertexElemIter<u32>(data, vertexStride, mNumVertices);
}

void MeshData::GetDataForIterator(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx, u8*& data, u32& stride) const
{
	if(!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
	{
		BS_EXCEPT(InvalidParametersException, "MeshData doesn't contain an element of specified type: Semantic: " + toString(semantic) + ", Semantic index: " + toString(semanticIdx) + ", Stream index: " + toString(streamIdx));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIdx, streamIdx);

	data = GetData() + indexBufferOffset + elementOffset;
	stride = mVertexData->GetVertexStride(streamIdx);
}

u32 MeshData::GetIndexBufferOffset() const
{
	return 0;
}

u32 MeshData::GetStreamOffset(u32 streamIdx) const
{
	u32 streamOffset = mVertexData->GetStreamOffset(streamIdx);

	return streamOffset * mNumVertices;
}

u8* MeshData::GetElementData(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	return GetData() + GetIndexBufferSize() + GetElementOffset(semantic, semanticIdx, streamIdx);
}

u8* MeshData::GetStreamData(u32 streamIdx) const
{
	return GetData() + GetIndexBufferSize() + GetStreamOffset(streamIdx);
}

u32 MeshData::GetIndexElementSize() const
{
	return mIndexType == IT_32BIT ? sizeof(u32) : sizeof(u16);
}

u32 MeshData::GetElementOffset(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	return GetStreamOffset(streamIdx) + mVertexData->GetElementOffsetFromStream(semantic, semanticIdx, streamIdx);
}

u32 MeshData::GetIndexBufferSize() const
{
	return mNumIndices * GetIndexElementSize();
}

u32 MeshData::GetStreamSize(u32 streamIdx) const
{
	return mVertexData->GetVertexStride(streamIdx) * mNumVertices;
}

u32 MeshData::GetStreamSize() const
{
	return mVertexData->GetVertexStride() * mNumVertices;
}

Bounds MeshData::CalculateBounds() const
{
	Bounds bounds;

	SPtr<VertexDataDesc> vertexDesc = GetVertexDesc();
	for(u32 i = 0; i < vertexDesc->GetNumElements(); i++)
	{
		const VertexElement& curElement = vertexDesc->GetElement(i);

		if(curElement.GetSemantic() != VES_POSITION || (curElement.GetType() != VET_FLOAT3 && curElement.GetType() != VET_FLOAT4))
			continue;

		u8* data = GetElementData(curElement.GetSemantic(), curElement.GetSemanticIdx(), curElement.GetStreamIdx());
		u32 stride = vertexDesc->GetVertexStride(curElement.GetStreamIdx());

		if(GetNumVertices() > 0)
		{
			Vector3 curPosition = *(Vector3*)data;
			Vector3 accum = curPosition;
			Vector3 min = curPosition;
			Vector3 max = curPosition;

			for(u32 i = 1; i < GetNumVertices(); i++)
			{
				curPosition = *(Vector3*)(data + stride * i);
				accum += curPosition;
				min = Vector3::Min(min, curPosition);
				max = Vector3::Max(max, curPosition);
			}

			Vector3 center = accum / (float)GetNumVertices();
			float radiusSqrd = 0.0f;

			for(u32 i = 0; i < GetNumVertices(); i++)
			{
				curPosition = *(Vector3*)(data + stride * i);
				float dist = center.SquaredDistance(curPosition);

				if(dist > radiusSqrd)
					radiusSqrd = dist;
			}

			float radius = Math::Sqrt(radiusSqrd);

			bounds = Bounds(AABox(min, max), Sphere(center, radius));
			break;
		}
	}

	return bounds;
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* MeshData::GetRttiStatic()
{
	return MeshDataRTTI::Instance();
}

RTTITypeBase* MeshData::GetRtti() const
{
	return MeshData::GetRttiStatic();
}
