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

MeshData::MeshData(u32 vertexCount, u32 indexCount, const SPtr<VertexDataDesc>& vertexDescription, IndexType indexType)
	: mVertexCount(vertexCount), mIndexCount(indexCount), mIndexType(indexType), mVertexDescription(vertexDescription)
{
	AllocateInternalBuffer();
}

MeshData::MeshData()
	: mVertexCount(0), mIndexCount(0), mIndexType(IT_32BIT)
{}

MeshData::~MeshData()
{}

u16* MeshData::GetIndices16() const
{
	if(mIndexType != IT_16BIT)
		B3D_EXCEPT(InternalErrorException, "Attempting to get 16bit index buffer, but internally allocated buffer is 32 bit.");

	u32 indexBufferOffset = GetIndexBufferOffset();

	return (u16*)(GetData() + indexBufferOffset);
}

u32* MeshData::GetIndices32() const
{
	if(mIndexType != IT_32BIT)
		B3D_EXCEPT(InternalErrorException, "Attempting to get 32bit index buffer, but internally allocated buffer is 16 bit.");

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
		totalVertexCount += meshData->GetVertexCount();
		totalIndexCount += meshData->GetIndexCount();
	}

	SmallVector<VertexElement, 8> vertexElements;

	Vector<VertexElement> combinedVertexElements;
	for(auto& meshData : meshes)
	{
		for(u32 i = 0; i < meshData->GetVertexDescription()->GetNumElements(); i++)
		{
			const VertexElement& newElement = meshData->GetVertexDescription()->GetElement(i);

			i32 alreadyExistsIdx = -1;
			u32 idx = 0;

			for(auto& existingElement : combinedVertexElements)
			{
				if(newElement.GetSemantic() == existingElement.GetSemantic() && newElement.GetSemanticIndex() == existingElement.GetSemanticIndex() && newElement.GetStreamIndex() == existingElement.GetStreamIndex())
				{
					if(newElement.GetType() != existingElement.GetType())
					{
						B3D_EXCEPT(NotImplementedException, "Two elements have same semantics but different types. This is not supported.");
					}

					alreadyExistsIdx = idx;
					break;
				}

				idx++;
			}

			if(alreadyExistsIdx == -1)
			{
				combinedVertexElements.push_back(newElement);
				vertexElements.Add(VertexElement(newElement.GetType(), newElement.GetSemantic(), newElement.GetSemanticIndex(), newElement.GetStreamIndex()));
			}
		}
	}

	SPtr<VertexDataDesc> vertexDescription = B3DMakeShared<VertexDataDesc>(vertexElements);
	SPtr<MeshData> combinedMeshData = B3DMakeShared<MeshData>(totalVertexCount, totalIndexCount, vertexDescription);

	// Copy indices
	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	u32* idxPtr = combinedMeshData->GetIndices32();
	for(auto& meshData : meshes)
	{
		u32 indexCount = meshData->GetIndexCount();
		u32* srcData = meshData->GetIndices32();

		for(u32 j = 0; j < indexCount; j++)
			idxPtr[j] = srcData[j] + vertexOffset;

		indexOffset += indexCount;
		idxPtr += indexCount;
		vertexOffset += meshData->GetVertexCount();
	}

	// Copy sub-meshes
	u32 meshIdx = 0;
	indexOffset = 0;
	for(auto& meshData : meshes)
	{
		u32 indexCount = meshData->GetIndexCount();
		const Vector<SubMesh> curSubMeshes = allSubMeshes[meshIdx];

		for(auto& subMesh : curSubMeshes)
		{
			subMeshes.push_back(SubMesh(subMesh.IndexOffset + indexOffset, subMesh.IndexCount, subMesh.DrawOp));
		}

		indexOffset += indexCount;
		meshIdx++;
	}

	// Copy vertices
	vertexOffset = 0;
	for(auto& meshData : meshes)
	{
		for(auto& element : combinedVertexElements)
		{
			u32 dstVertexStride = vertexDescription->GetVertexStride(element.GetStreamIndex());
			u8* dstData = combinedMeshData->GetElementData(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());
			dstData += vertexOffset * dstVertexStride;

			u32 sourceVertexCount = meshData->GetVertexCount();
			u32 vertexSize = vertexDescription->GetElementSize(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());

			if(meshData->GetVertexDescription()->HasElement(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex()))
			{
				u32 srcVertexStride = meshData->GetVertexDescription()->GetVertexStride(element.GetStreamIndex());
				u8* srcData = meshData->GetElementData(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());

				for(u32 i = 0; i < sourceVertexCount; i++)
				{
					memcpy(dstData, srcData, vertexSize);
					dstData += dstVertexStride;
					srcData += srcVertexStride;
				}
			}
			else
			{
				for(u32 i = 0; i < sourceVertexCount; i++)
				{
					memset(dstData, 0, vertexSize);
					dstData += dstVertexStride;
				}
			}
		}

		vertexOffset += meshData->GetVertexCount();
	}

	return combinedMeshData;
}

void MeshData::SetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIndex, u32 streamIndex)
{
	B3D_ASSERT(data != nullptr);

	if(!mVertexDescription->HasElement(semantic, semanticIndex, streamIndex))
	{
		B3D_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIndex, streamIndex);
		return;
	}

	u32 elementSize = mVertexDescription->GetElementSize(semantic, semanticIndex, streamIndex);
	u32 totalSize = elementSize * mVertexCount;

	if(totalSize != size)
	{
		B3D_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + ToString(totalSize) + ". Got: " + ToString(size));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);
	u32 vertexStride = mVertexDescription->GetVertexStride(streamIndex);

	u8* dst = GetData() + indexBufferOffset + elementOffset;
	u8* src = (u8*)data;
	for(u32 i = 0; i < mVertexCount; i++)
	{
		memcpy(dst, src, elementSize);
		dst += vertexStride;
		src += elementSize;
	}
}

void MeshData::GetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIndex, u32 streamIndex)
{
	B3D_ASSERT(data != nullptr);

	if(!mVertexDescription->HasElement(semantic, semanticIndex, streamIndex))
	{
		B3D_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIndex, streamIndex);
		return;
	}

	u32 elementSize = mVertexDescription->GetElementSize(semantic, semanticIndex, streamIndex);
	u32 totalSize = elementSize * mVertexCount;

	if(totalSize != size)
	{
		B3D_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + ToString(totalSize) + ". Got: " + ToString(size));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);
	u32 vertexStride = mVertexDescription->GetVertexStride(streamIndex);

	u8* src = GetData() + indexBufferOffset + elementOffset;
	u8* dst = (u8*)data;
	for(u32 i = 0; i < mVertexCount; i++)
	{
		memcpy(dst, src, elementSize);
		dst += elementSize;
		src += vertexStride;
	}
}

VertexElemIter<Vector2> MeshData::GetVec2DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIter<Vector2>(data, vertexStride, mVertexCount);
}

VertexElemIter<Vector3> MeshData::GetVec3DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIter<Vector3>(data, vertexStride, mVertexCount);
}

VertexElemIter<Vector4> MeshData::GetVec4DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIter<Vector4>(data, vertexStride, mVertexCount);
}

VertexElemIter<u32> MeshData::GetDwordDataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIter<u32>(data, vertexStride, mVertexCount);
}

void MeshData::GetDataForIterator(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex, u8*& data, u32& stride) const
{
	if(!mVertexDescription->HasElement(semantic, semanticIndex, streamIndex))
	{
		B3D_EXCEPT(InvalidParametersException, "MeshData doesn't contain an element of specified type: Semantic: " + ToString(semantic) + ", Semantic index: " + ToString(semanticIndex) + ", Stream index: " + ToString(streamIndex));
	}

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);

	data = GetData() + indexBufferOffset + elementOffset;
	stride = mVertexDescription->GetVertexStride(streamIndex);
}

u32 MeshData::GetIndexBufferOffset() const
{
	return 0;
}

u32 MeshData::GetStreamOffset(u32 streamIdx) const
{
	u32 streamOffset = mVertexDescription->GetStreamOffset(streamIdx);

	return streamOffset * mVertexCount;
}

u8* MeshData::GetElementData(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	return GetData() + GetIndexBufferSize() + GetElementOffset(semantic, semanticIdx, streamIdx);
}

u8* MeshData::GetStreamData(u32 streamIdx) const
{
	return GetData() + GetIndexBufferSize() + GetStreamOffset(streamIdx);
}

u32 MeshData::GetElementOffset(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	return GetStreamOffset(streamIdx) + mVertexDescription->GetElementOffsetFromStream(semantic, semanticIdx, streamIdx);
}

u32 MeshData::GetIndexBufferSize() const
{
	return mIndexCount * GetIndexElementSize();
}

u32 MeshData::GetStreamSize(u32 streamIndex) const
{
	return mVertexDescription->GetVertexStride(streamIndex) * mVertexCount;
}

u32 MeshData::GetStreamSize() const
{
	return mVertexDescription->GetVertexStride() * mVertexCount;
}

Bounds MeshData::CalculateBounds() const
{
	Bounds bounds;

	SPtr<VertexDataDesc> vertexDesc = GetVertexDescription();
	for(u32 i = 0; i < vertexDesc->GetNumElements(); i++)
	{
		const VertexElement& curElement = vertexDesc->GetElement(i);

		if(curElement.GetSemantic() != VES_POSITION || (curElement.GetType() != VET_FLOAT3 && curElement.GetType() != VET_FLOAT4))
			continue;

		u8* data = GetElementData(curElement.GetSemantic(), curElement.GetSemanticIndex(), curElement.GetStreamIndex());
		u32 stride = vertexDesc->GetVertexStride(curElement.GetStreamIndex());

		if(GetVertexCount() > 0)
		{
			Vector3 curPosition = *(Vector3*)data;
			Vector3 accum = curPosition;
			Vector3 min = curPosition;
			Vector3 max = curPosition;

			for(u32 i = 1; i < GetVertexCount(); i++)
			{
				curPosition = *(Vector3*)(data + stride * i);
				accum += curPosition;
				min = Vector3::Min(min, curPosition);
				max = Vector3::Max(max, curPosition);
			}

			Vector3 center = accum / (float)GetVertexCount();
			float radiusSqrd = 0.0f;

			for(u32 i = 0; i < GetVertexCount(); i++)
			{
				curPosition = *(Vector3*)(data + stride * i);
				float dist = center.SquaredDistance(curPosition);

				if(dist > radiusSqrd)
					radiusSqrd = dist;
			}

			float radius = Math::SquareRoot(radiusSqrd);

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
