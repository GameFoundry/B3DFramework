//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/B3DMeshData.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DSphere.h"
#include "Math/B3DAABox.h"
#include "RTTI/B3DMeshDataRTTI.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

MeshData::MeshData(u32 vertexCount, u32 indexCount, const SPtr<VertexDescription>& vertexDescription, IndexType indexType)
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
	if(!B3D_ENSURE_LOG(mIndexType == IT_16BIT, "Attempting to get 16bit index buffer, but internally allocated buffer is 32 bit."))
		return nullptr;

	u32 indexBufferOffset = GetIndexBufferOffset();

	return (u16*)(GetData() + indexBufferOffset);
}

u32* MeshData::GetIndices32() const
{
	if(!B3D_ENSURE_LOG(mIndexType == IT_32BIT, "Attempting to get 32bit index buffer, but internally allocated buffer is 16 bit."))
		return nullptr;

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

	TInlineArray<VertexElement, 8> vertexElements;

	Vector<VertexElement> combinedVertexElements;
	for(auto& meshData : meshes)
	{
		for(u32 elementIndex = 0; elementIndex < meshData->GetVertexDescription()->GetElementCount(); elementIndex++)
		{
			const VertexElement& newElement = meshData->GetVertexDescription()->GetElement(elementIndex);

			i32 alreadyExistsIndex = -1;
			u32 existingElementIndex = 0;

			for(auto& existingElement : combinedVertexElements)
			{
				if(newElement.GetSemantic() == existingElement.GetSemantic() && newElement.GetSemanticIndex() == existingElement.GetSemanticIndex() && newElement.GetStreamIndex() == existingElement.GetStreamIndex())
				{
					B3D_ENSURE_LOG(newElement.GetType() == existingElement.GetType(), "Two elements have same semantics but different types. This is not supported.");

					alreadyExistsIndex = existingElementIndex;
					break;
				}

				existingElementIndex++;
			}

			if(alreadyExistsIndex == -1)
			{
				combinedVertexElements.push_back(newElement);
				vertexElements.Add(VertexElement(newElement.GetType(), newElement.GetSemantic(), newElement.GetSemanticIndex(), newElement.GetStreamIndex()));
			}
		}
	}

	SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
	SPtr<MeshData> combinedMeshData = B3DMakeShared<MeshData>(totalVertexCount, totalIndexCount, vertexDescription);

	// Copy indices
	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	u32* indexPointer = combinedMeshData->GetIndices32();
	for(auto& meshData : meshes)
	{
		u32 indexCount = meshData->GetIndexCount();
		u32* sourceData = meshData->GetIndices32();

		for(u32 indexIndex = 0; indexIndex < indexCount; indexIndex++)
			indexPointer[indexIndex] = sourceData[indexIndex] + vertexOffset;

		indexOffset += indexCount;
		indexPointer += indexCount;
		vertexOffset += meshData->GetVertexCount();
	}

	// Copy sub-meshes
	u32 meshIndex = 0;
	indexOffset = 0;
	for(auto& meshData : meshes)
	{
		u32 indexCount = meshData->GetIndexCount();
		const Vector<SubMesh> currentSubMeshes = allSubMeshes[meshIndex];

		for(auto& subMesh : currentSubMeshes)
		{
			subMeshes.push_back(SubMesh(subMesh.IndexOffset + indexOffset, subMesh.IndexCount, subMesh.DrawOp));
		}

		indexOffset += indexCount;
		meshIndex++;
	}

	// Copy vertices
	vertexOffset = 0;
	for(auto& meshData : meshes)
	{
		for(auto& element : combinedVertexElements)
		{
			u32 destinationVertexStride = vertexDescription->GetVertexStride(element.GetStreamIndex());
			u8* destinationData = combinedMeshData->GetElementData(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());
			destinationData += vertexOffset * destinationVertexStride;

			u32 sourceVertexCount = meshData->GetVertexCount();
			u32 vertexSize = vertexDescription->GetElementSize(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());

			if(meshData->GetVertexDescription()->HasElement(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex()))
			{
				u32 sourceVertexStride = meshData->GetVertexDescription()->GetVertexStride(element.GetStreamIndex());
				u8* sourceData = meshData->GetElementData(element.GetSemantic(), element.GetSemanticIndex(), element.GetStreamIndex());

				for(u32 vertexIndex = 0; vertexIndex < sourceVertexCount; vertexIndex++)
				{
					memcpy(destinationData, sourceData, vertexSize);
					destinationData += destinationVertexStride;
					sourceData += sourceVertexStride;
				}
			}
			else
			{
				for(u32 vertexIndex = 0; vertexIndex < sourceVertexCount; vertexIndex++)
				{
					memset(destinationData, 0, vertexSize);
					destinationData += destinationVertexStride;
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
		B3D_LOG(Warning, LogMesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIndex, streamIndex);
		return;
	}

	u32 elementSize = mVertexDescription->GetElementSize(semantic, semanticIndex, streamIndex);
	u32 totalSize = elementSize * mVertexCount;

	if(!B3D_ENSURE_LOG(totalSize == size, "Buffer sizes don't match. Expected: {0}. Got: {1}", totalSize, size))
		return;

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);
	u32 vertexStride = mVertexDescription->GetVertexStride(streamIndex);

	u8* destination = GetData() + indexBufferOffset + elementOffset;
	u8* source = (u8*)data;
	for(u32 vertexIndex = 0; vertexIndex < mVertexCount; vertexIndex++)
	{
		memcpy(destination, source, elementSize);
		destination += vertexStride;
		source += elementSize;
	}
}

void MeshData::GetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIndex, u32 streamIndex)
{
	B3D_ASSERT(data != nullptr);

	if(!mVertexDescription->HasElement(semantic, semanticIndex, streamIndex))
	{
		B3D_LOG(Warning, LogMesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
							  "Semantic index: {1}, Stream index: {2}",
			   semantic, semanticIndex, streamIndex);
		return;
	}

	u32 elementSize = mVertexDescription->GetElementSize(semantic, semanticIndex, streamIndex);
	u32 totalSize = elementSize * mVertexCount;

	if(!B3D_ENSURE_LOG(totalSize == size, "Buffer sizes don't match. Expected: {0}. Got: {1}", totalSize, size))
		return;

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);
	u32 vertexStride = mVertexDescription->GetVertexStride(streamIndex);

	u8* source = GetData() + indexBufferOffset + elementOffset;
	u8* destination = (u8*)data;
	for(u32 vertexIndex = 0; vertexIndex < mVertexCount; vertexIndex++)
	{
		memcpy(destination, source, elementSize);
		destination += elementSize;
		source += vertexStride;
	}
}

VertexElemIterator<Vector2> MeshData::GetVec2DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIterator<Vector2>(data, vertexStride, mVertexCount);
}

VertexElemIterator<Vector3> MeshData::GetVec3DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIterator<Vector3>(data, vertexStride, mVertexCount);
}

VertexElemIterator<Vector4> MeshData::GetVec4DataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIterator<Vector4>(data, vertexStride, mVertexCount);
}

VertexElemIterator<u32> MeshData::GetDwordDataIter(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex)
{
	u8* data;
	u32 vertexStride;
	GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

	return VertexElemIterator<u32>(data, vertexStride, mVertexCount);
}

void MeshData::GetDataForIterator(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex, u8*& data, u32& stride) const
{
	if(!B3D_ENSURE_LOG(mVertexDescription->HasElement(semantic, semanticIndex, streamIndex), "MeshData doesn't contain an element of specified type: Semantic: {0}, Semantic index: {1}, Stream index: {2}", semantic, semanticIndex, streamIndex))
		return;

	u32 indexBufferOffset = GetIndexBufferSize();

	u32 elementOffset = GetElementOffset(semantic, semanticIndex, streamIndex);

	data = GetData() + indexBufferOffset + elementOffset;
	stride = mVertexDescription->GetVertexStride(streamIndex);
}

u32 MeshData::GetIndexBufferOffset() const
{
	return 0;
}

u32 MeshData::GetStreamOffset(u32 streamIndex) const
{
	u32 streamOffset = mVertexDescription->GetStreamOffset(streamIndex);

	return streamOffset * mVertexCount;
}

u8* MeshData::GetElementData(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	return GetData() + GetIndexBufferSize() + GetElementOffset(semantic, semanticIndex, streamIndex);
}

u8* MeshData::GetStreamData(u32 streamIndex) const
{
	return GetData() + GetIndexBufferSize() + GetStreamOffset(streamIndex);
}

u32 MeshData::GetElementOffset(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	return GetStreamOffset(streamIndex) + mVertexDescription->GetElementOffsetFromStream(semantic, semanticIndex, streamIndex);
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
	Bounds bounds(kZeroTag);

	SPtr<VertexDescription> vertexDescription = GetVertexDescription();
	for(u32 elementIndex = 0; elementIndex < vertexDescription->GetElementCount(); elementIndex++)
	{
		const VertexElement& currentElement = vertexDescription->GetElement(elementIndex);

		if(currentElement.GetSemantic() != VES_POSITION || (currentElement.GetType() != VET_FLOAT3 && currentElement.GetType() != VET_FLOAT4))
			continue;

		u8* data = GetElementData(currentElement.GetSemantic(), currentElement.GetSemanticIndex(), currentElement.GetStreamIndex());
		u32 stride = vertexDescription->GetVertexStride(currentElement.GetStreamIndex());

		if(GetVertexCount() > 0)
		{
			Vector3 currentPosition = *(Vector3*)data;
			Vector3 accumulator = currentPosition;
			Vector3 minimum = currentPosition;
			Vector3 maximum = currentPosition;

			for(u32 vertexIndex = 1; vertexIndex < GetVertexCount(); vertexIndex++)
			{
				currentPosition = *(Vector3*)(data + stride * vertexIndex);
				accumulator += currentPosition;
				minimum = Vector3::Min(minimum, currentPosition);
				maximum = Vector3::Max(maximum, currentPosition);
			}

			Vector3 center = accumulator / (float)GetVertexCount();
			float radiusSquared = 0.0f;

			for(u32 vertexIndex = 0; vertexIndex < GetVertexCount(); vertexIndex++)
			{
				currentPosition = *(Vector3*)(data + stride * vertexIndex);
				float distance = center.SquaredDistance(currentPosition);

				if(distance > radiusSquared)
					radiusSquared = distance;
			}

			float radius = Math::SquareRoot(radiusSquared);

			bounds = Bounds(center, (maximum - minimum) * 0.5f, radius);
			break;
		}
	}

	return bounds;
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* MeshData::GetRttiStatic()
{
	return MeshDataRTTI::Instance();
}

RTTIType* MeshData::GetRtti() const
{
	return MeshData::GetRttiStatic();
}
