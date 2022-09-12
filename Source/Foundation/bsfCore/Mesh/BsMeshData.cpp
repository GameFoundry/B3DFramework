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

namespace bs
{
	MeshData::MeshData(UINT32 numVertices, UINT32 numIndexes, const SPtr<VertexDataDesc>& vertexData, IndexType indexType)
	   :mNumVertices(numVertices), mNumIndices(numIndexes), mIndexType(indexType), mVertexData(vertexData)
	{
		allocateInternalBuffer();
	}

	MeshData::MeshData()
		:mNumVertices(0), mNumIndices(0), mIndexType(IT_32BIT)
	{ }

	MeshData::~MeshData()
	{ }

	UINT32 MeshData::GetNumIndices() const
	{
		return mNumIndices;
	}

	UINT16* MeshData::getIndices16() const
	{
		if(mIndexType != IT_16BIT)
			BS_EXCEPT(InternalErrorException, "Attempting to get 16bit index buffer, but internally allocated buffer is 32 bit.");

		UINT32 indexBufferOffset = getIndexBufferOffset();

		return (UINT16*)(getData() + indexBufferOffset);
	}

	UINT32* MeshData::getIndices32() const
	{
		if(mIndexType != IT_32BIT)
			BS_EXCEPT(InternalErrorException, "Attempting to get 32bit index buffer, but internally allocated buffer is 16 bit.");

		UINT32 indexBufferOffset = getIndexBufferOffset();

		return (UINT32*)(getData() + indexBufferOffset);
	}

	UINT32 MeshData::GetInternalBufferSize() const
	{
		return GetIndexBufferSize() + getStreamSize();
	}

	// TODO - This doesn't handle the case where multiple elements in same slot have different data types
	SPtr<MeshData> MeshData::combine(const Vector<SPtr<MeshData>>& meshes, const Vector<Vector<SubMesh>>& allSubMeshes,
		Vector<SubMesh>& subMeshes)
	{
		UINT32 totalVertexCount = 0;
		UINT32 totalIndexCount = 0;
		for(auto& meshData : meshes)
		{
			totalVertexCount += meshData->GetNumVertices();
			totalIndexCount += meshData->GetNumIndices();
		}

		SPtr<VertexDataDesc> vertexData = bs_shared_ptr_new<VertexDataDesc>();
		
		Vector<VertexElement> combinedVertexElements;
		for(auto& meshData : meshes)
		{
			for(UINT32 i = 0; i < meshData->GetVertexDesc()->getNumElements(); i++)
			{
				const VertexElement& newElement = meshData->GetVertexDesc()->getElement(i);

				INT32 alreadyExistsIdx = -1;
				UINT32 idx = 0;

				for(auto& existingElement : combinedVertexElements)
				{
					if(newElement.GetSemantic() == existingElement.getSemantic() && newElement.getSemanticIdx() == existingElement.getSemanticIdx()
						&& newElement.GetStreamIdx() == existingElement.getStreamIdx())
					{
						if(newElement.GetType() != existingElement.getType())
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
					vertexData->AddVertElem(newElement.GetType(), newElement.getSemantic(), newElement.getSemanticIdx(), newElement.getStreamIdx());
				}
			}
		}

		SPtr<MeshData> combinedMeshData = bs_shared_ptr_new<MeshData>(totalVertexCount, totalIndexCount, vertexData);

		// Copy indices
		UINT32 vertexOffset = 0;
		UINT32 indexOffset = 0;
		UINT32* idxPtr = combinedMeshData->GetIndices32();
		for(auto& meshData : meshes)
		{
			UINT32 numIndices = meshData->GetNumIndices();
			UINT32* srcData = meshData->GetIndices32();

			for(UINT32 j = 0; j < numIndices; j++)
				idxPtr[j] = srcData[j] + vertexOffset;

			indexOffset += numIndices;
			idxPtr += numIndices;
			vertexOffset += meshData->GetNumVertices();
		}

		// Copy sub-meshes
		UINT32 meshIdx = 0;
		indexOffset = 0;
		for(auto& meshData : meshes)
		{
			UINT32 numIndices = meshData->GetNumIndices();
			const Vector<SubMesh> curSubMeshes = allSubMeshes[meshIdx];

			for(auto& subMesh : curSubMeshes)
			{
				subMeshes.push_back(SubMesh(subMesh.indexOffset + indexOffset, subMesh.indexCount, subMesh.drawOp));
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
				UINT32 dstVertexStride = vertexData->GetVertexStride(element.GetStreamIdx());
				UINT8* dstData = combinedMeshData->GetElementData(element.GetSemantic(), element.getSemanticIdx(), element.getStreamIdx());
				dstData += vertexOffset * dstVertexStride;

				UINT32 numSrcVertices = meshData->GetNumVertices();
				UINT32 vertexSize = vertexData->GetElementSize(element.GetSemantic(), element.getSemanticIdx(), element.getStreamIdx());

				if(meshData->GetVertexDesc()->hasElement(element.GetSemantic(), element.getSemanticIdx(), element.getStreamIdx()))
				{
					UINT32 srcVertexStride = meshData->GetVertexDesc()->getVertexStride(element.GetStreamIdx());
					UINT8* srcData = meshData->GetElementData(element.GetSemantic(), element.getSemanticIdx(), element.getStreamIdx());

					for(UINT32 i = 0; i < numSrcVertices; i++)
					{
						memcpy(dstData, srcData, vertexSize);
						dstData += dstVertexStride;
						srcData += srcVertexStride;
					}
				}
				else
				{
					for(UINT32 i = 0; i < numSrcVertices; i++)
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

	void MeshData::SetVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx, UINT32 streamIdx)
	{
		assert(data != nullptr);

		if(!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
		{
			BS_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
				"Semantic index: {1}, Stream index: {2}", semantic, semanticIdx, streamIdx);
			return;
		}

		UINT32 elementSize = mVertexData->GetElementSize(semantic, semanticIdx, streamIdx);
		UINT32 totalSize = elementSize * mNumVertices;

		if(totalSize != size)
		{
			BS_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + toString(totalSize) + ". Got: " + toString(size));
		}

		UINT32 indexBufferOffset = getIndexBufferSize();

		UINT32 elementOffset = getElementOffset(semantic, semanticIdx, streamIdx);
		UINT32 vertexStride = mVertexData->GetVertexStride(streamIdx);

		UINT8* dst = getData() + indexBufferOffset + elementOffset;
		UINT8* src = (UINT8*)data;
		for(UINT32 i = 0; i < mNumVertices; i++)
		{
			memcpy(dst, src, elementSize);
			dst += vertexStride;
			src += elementSize;
		}
	}

	void MeshData::GetVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx, UINT32 streamIdx)
	{
		assert(data != nullptr);

		if (!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
		{
			BS_LOG(Warning, Mesh, "MeshData doesn't contain an element of specified type: Semantic: {0}, "
				"Semantic index: {1}, Stream index: {2}", semantic, semanticIdx, streamIdx);
			return;
		}

		UINT32 elementSize = mVertexData->GetElementSize(semantic, semanticIdx, streamIdx);
		UINT32 totalSize = elementSize * mNumVertices;

		if (totalSize != size)
		{
			BS_EXCEPT(InvalidParametersException, "Buffer sizes don't match. Expected: " + toString(totalSize) + ". Got: " + toString(size));
		}

		UINT32 indexBufferOffset = getIndexBufferSize();

		UINT32 elementOffset = getElementOffset(semantic, semanticIdx, streamIdx);
		UINT32 vertexStride = mVertexData->GetVertexStride(streamIdx);

		UINT8* src = getData() + indexBufferOffset + elementOffset;
		UINT8* dst = (UINT8*)data;
		for (UINT32 i = 0; i < mNumVertices; i++)
		{
			memcpy(dst, src, elementSize);
			dst += elementSize;
			src += vertexStride;
		}
	}

	VertexElemIter<Vector2> MeshData::GetVec2DataIter(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx)
	{
		UINT8* data;
		UINT32 vertexStride;
		getDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

		return VertexElemIter<Vector2>(data, vertexStride, mNumVertices);
	}

	VertexElemIter<Vector3> MeshData::GetVec3DataIter(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx)
	{
		UINT8* data;
		UINT32 vertexStride;
		getDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

		return VertexElemIter<Vector3>(data, vertexStride, mNumVertices);
	}

	VertexElemIter<Vector4> MeshData::GetVec4DataIter(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx)
	{
		UINT8* data;
		UINT32 vertexStride;
		getDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

		return VertexElemIter<Vector4>(data, vertexStride, mNumVertices);
	}

	VertexElemIter<UINT32> MeshData::GetDWORDDataIter(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx)
	{
		UINT8* data;
		UINT32 vertexStride;
		getDataForIterator(semantic, semanticIdx, streamIdx, data, vertexStride);

		return VertexElemIter<UINT32>(data, vertexStride, mNumVertices);
	}

	void MeshData::GetDataForIterator(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx, UINT8*& data, UINT32& stride) const
	{
		if(!mVertexData->HasElement(semantic, semanticIdx, streamIdx))
		{
			BS_EXCEPT(InvalidParametersException, "MeshData doesn't contain an element of specified type: Semantic: " + toString(semantic) + ", Semantic index: "
				+ toString(semanticIdx) + ", Stream index: " + toString(streamIdx));
		}

		UINT32 indexBufferOffset = getIndexBufferSize();

		UINT32 elementOffset = getElementOffset(semantic, semanticIdx, streamIdx);

		data = getData() + indexBufferOffset + elementOffset;
		stride = mVertexData->GetVertexStride(streamIdx);
	}

	UINT32 MeshData::GetIndexBufferOffset() const
	{
		return 0;
	}

	UINT32 MeshData::GetStreamOffset(UINT32 streamIdx) const
	{
		UINT32 streamOffset = mVertexData->GetStreamOffset(streamIdx);

		return streamOffset * mNumVertices;
	}

	UINT8* MeshData::getElementData(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		return GetData() + getIndexBufferSize() + getElementOffset(semantic, semanticIdx, streamIdx);
	}

	UINT8* MeshData::getStreamData(UINT32 streamIdx) const
	{
		return GetData() + getIndexBufferSize() + getStreamOffset(streamIdx);
	}

	UINT32 MeshData::GetIndexElementSize() const
	{
		return mIndexType == IT_32BIT ? sizeof(UINT32) : sizeof(UINT16);
	}

	UINT32 MeshData::GetElementOffset(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		return GetStreamOffset(streamIdx) + mVertexData->GetElementOffsetFromStream(semantic, semanticIdx, streamIdx);
	}

	UINT32 MeshData::GetIndexBufferSize() const
	{
		return mNumIndices * getIndexElementSize();
	}

	UINT32 MeshData::GetStreamSize(UINT32 streamIdx) const
	{
		return mVertexData->GetVertexStride(streamIdx) * mNumVertices;
	}

	UINT32 MeshData::GetStreamSize() const
	{
		return mVertexData->GetVertexStride() * mNumVertices;
	}

	Bounds MeshData::CalculateBounds() const
	{
		Bounds bounds;

		SPtr<VertexDataDesc> vertexDesc = getVertexDesc();
		for (UINT32 i = 0; i < vertexDesc->GetNumElements(); i++)
		{
			const VertexElement& curElement = vertexDesc->GetElement(i);

			if (curElement.GetSemantic() != VES_POSITION || (curElement.getType() != VET_FLOAT3 && curElement.getType() != VET_FLOAT4))
				continue;

			UINT8* data = getElementData(curElement.GetSemantic(), curElement.getSemanticIdx(), curElement.getStreamIdx());
			UINT32 stride = vertexDesc->GetVertexStride(curElement.GetStreamIdx());

			if (getNumVertices() > 0)
			{
				Vector3 curPosition = *(Vector3*)data;
				Vector3 accum = curPosition;
				Vector3 min = curPosition;
				Vector3 max = curPosition;

				for (UINT32 i = 1; i < getNumVertices(); i++)
				{
					curPosition = *(Vector3*)(data + stride * i);
					accum += curPosition;
					min = Vector3::min(min, curPosition);
					max = Vector3::max(max, curPosition);
				}

				Vector3 center = accum / (float)getNumVertices();
				float radiusSqrd = 0.0f;

				for (UINT32 i = 0; i < getNumVertices(); i++)
				{
					curPosition = *(Vector3*)(data + stride * i);
					float dist = center.SquaredDistance(curPosition);

					if (dist > radiusSqrd)
						radiusSqrd = dist;
				}

				float radius = Math::sqrt(radiusSqrd);

				bounds = Bounds(AABox(min, max), Sphere(center, radius));
				break;
			}
		}

		return bounds;
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* MeshData::getRTTIStatic()
	{
		return MeshDataRTTI::Instance();
	}

	RTTITypeBase* MeshData::getRTTI() const
	{
		return MeshData::GetRTTIStatic();
	}
}
