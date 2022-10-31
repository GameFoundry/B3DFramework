//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshHeap.h"
#include "CoreThread/BsCoreThread.h"
#include "Mesh/BsTransientMesh.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "RenderAPI/BsVertexData.h"
#include "Mesh/BsMeshData.h"
#include "Math/BsMath.h"
#include "RenderAPI/BsEventQuery.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

MeshHeap::MeshHeap(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType)
	: mNumVertices(numVertices), mNumIndices(numIndices), mVertexDesc(vertexDesc), mIndexType(indexType), mNextFreeId(0)
{
}

SPtr<MeshHeap> MeshHeap::Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType)
{
	MeshHeap* meshHeap = new(B3DAllocate<MeshHeap>()) MeshHeap(numVertices, numIndices, vertexDesc, indexType);
	SPtr<MeshHeap> meshHeapPtr = B3DMakeCoreFromExisting<MeshHeap>(meshHeap);

	meshHeapPtr->SetThisPtrInternal(meshHeapPtr);
	meshHeapPtr->Initialize();

	return meshHeapPtr;
}

SPtr<TransientMesh> MeshHeap::Alloc(const SPtr<MeshData>& meshData, DrawOperationType drawOp)
{
	u32 meshIdx = mNextFreeId++;

	SPtr<MeshHeap> thisPtr = std::static_pointer_cast<MeshHeap>(GetThisPtr());
	TransientMesh* transientMesh = new(B3DAllocate<TransientMesh>()) TransientMesh(thisPtr, meshIdx, meshData->GetNumVertices(), meshData->GetNumIndices(), drawOp);
	SPtr<TransientMesh> transientMeshPtr = B3DMakeCoreFromExisting<TransientMesh>(transientMesh);

	transientMeshPtr->SetThisPtrInternal(transientMeshPtr);
	transientMeshPtr->Initialize();

	mMeshes[meshIdx] = transientMeshPtr;

	QueueGpuCommand(GetCore(), std::bind(&ct::MeshHeap::Alloc, GetCore().get(), transientMeshPtr->GetCore(), meshData));

	return transientMeshPtr;
}

void MeshHeap::Dealloc(const SPtr<TransientMesh>& mesh)
{
	auto iterFind = mMeshes.find(mesh->mId);
	if(iterFind == mMeshes.end())
		return;

	mesh->MarkAsDestroyed();
	mMeshes.erase(iterFind);

	QueueGpuCommand(GetCore(), std::bind(&ct::MeshHeap::Dealloc, GetCore().get(), mesh->GetCore()));
}

SPtr<ct::MeshHeap> MeshHeap::GetCore() const
{
	return std::static_pointer_cast<ct::MeshHeap>(mCoreSpecific);
}

SPtr<ct::CoreObject> MeshHeap::CreateCore() const
{
	ct::MeshHeap* obj = new(B3DAllocate<ct::MeshHeap>()) ct::MeshHeap(mNumVertices, mNumIndices, mVertexDesc, mIndexType, GDF_DEFAULT);

	SPtr<ct::MeshHeap> corePtr = B3DMakeSharedFromExisting<ct::MeshHeap>(obj);
	obj->SetThisPtrInternal(corePtr);

	return corePtr;
}

namespace bs { namespace ct
{
const float MeshHeap::kGrowPercent = 1.5f;

MeshHeap::MeshHeap(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType, GpuDeviceFlags deviceMask)
	: mNumVertices(numVertices), mNumIndices(numIndices), mCPUIndexData(nullptr), mVertexDesc(vertexDesc), mIndexType(indexType), mDeviceMask(deviceMask), mNextQueryId(0)
{
	for(u32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
	{
		mCPUVertexData.push_back(nullptr);
	}
}

MeshHeap::~MeshHeap()
{
	THROW_IF_NOT_CORE_THREAD;

	for(auto& cpuVertBuffer : mCPUVertexData)
		B3DFree(cpuVertBuffer);

	if(mCPUIndexData != nullptr)
		B3DFree(mCPUIndexData);

	mVertexData = nullptr;
	mIndexBuffer = nullptr;
	mVertexDesc = nullptr;
}

void MeshHeap::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	GrowVertexBuffer(mNumVertices);
	GrowIndexBuffer(mNumIndices);

	CoreObject::Initialize();
}

void MeshHeap::Alloc(SPtr<TransientMesh> mesh, const SPtr<MeshData>& meshData)
{
	// Find free vertex chunk and grow if needed
	u32 smallestVertFit = 0;
	u32 smallestVertFitIdx = 0;

	while(smallestVertFit == 0)
	{
		u32 curIdx = 0;
		for(auto& chunkIdx : mFreeVertChunks)
		{
			ChunkData& chunk = mVertChunks[chunkIdx];

			if(chunk.Size >= meshData->GetNumVertices() && (chunk.Size < smallestVertFit || smallestVertFit == 0))
			{
				smallestVertFit = chunk.Size;
				smallestVertFitIdx = curIdx;
			}

			curIdx++;
		}

		if(smallestVertFit > 0)
			break;

		u32 newNumVertices = mNumVertices;
		while(newNumVertices < (mNumVertices + meshData->GetNumVertices()))
		{
			newNumVertices = Math::RoundToInt(newNumVertices * kGrowPercent);
		}

		GrowVertexBuffer(newNumVertices);
	}

	// Find free index chunk and grow if needed
	u32 smallestIdxFit = 0;
	u32 smallestIdxFitIdx = 0;

	while(smallestIdxFit == 0)
	{
		u32 curIdx = 0;
		for(auto& chunkIdx : mFreeIdxChunks)
		{
			ChunkData& chunk = mIdxChunks[chunkIdx];

			if(chunk.Size >= meshData->GetNumIndices() && (chunk.Size < smallestIdxFit || smallestIdxFit == 0))
			{
				smallestIdxFit = chunk.Size;
				smallestIdxFitIdx = curIdx;
			}

			curIdx++;
		}

		if(smallestIdxFit > 0)
			break;

		u32 newNumIndices = mNumIndices;
		while(newNumIndices < (mNumIndices + meshData->GetNumIndices()))
		{
			newNumIndices = Math::RoundToInt(newNumIndices * kGrowPercent);
		}

		GrowIndexBuffer(newNumIndices);
	}

	u32 freeVertChunkIdx = 0;
	u32 freeIdxChunkIdx = 0;

	auto freeVertIter = mFreeVertChunks.begin();
	freeVertChunkIdx = (*freeVertIter);
	for(u32 i = 0; i < smallestVertFitIdx; i++)
	{
		freeVertIter++;
		freeVertChunkIdx = (*freeVertIter);
	}

	mFreeVertChunks.erase(freeVertIter);

	auto freeIdxIter = mFreeIdxChunks.begin();
	freeIdxChunkIdx = (*freeIdxIter);
	for(u32 i = 0; i < smallestIdxFitIdx; i++)
	{
		freeIdxIter++;
		freeIdxChunkIdx = (*freeIdxIter);
	}

	mFreeIdxChunks.erase(freeIdxIter);

	ChunkData& vertChunk = mVertChunks[freeVertChunkIdx];
	ChunkData& idxChunk = mIdxChunks[freeIdxChunkIdx];

	u32 vertChunkStart = vertChunk.Start;
	u32 idxChunkStart = idxChunk.Start;

	u32 remainingNumVerts = vertChunk.Size - meshData->GetNumVertices();
	u32 remainingNumIdx = idxChunk.Size - meshData->GetNumIndices();

	vertChunk.Size = meshData->GetNumVertices();
	idxChunk.Size = meshData->GetNumIndices();

	if(remainingNumVerts > 0)
	{
		if(!mEmptyVertChunks.empty())
		{
			u32 emptyChunkIdx = mEmptyVertChunks.top();
			ChunkData& emptyChunk = mVertChunks[emptyChunkIdx];
			mEmptyVertChunks.pop();

			emptyChunk.Start = vertChunkStart + meshData->GetNumVertices();
			emptyChunk.Size = remainingNumVerts;
		}
		else
		{
			ChunkData newChunk;
			newChunk.Size = remainingNumVerts;
			newChunk.Start = vertChunkStart + meshData->GetNumVertices();

			mVertChunks.push_back(newChunk);
			mFreeVertChunks.push_back((u32)(mVertChunks.size() - 1));
		}
	}

	if(remainingNumIdx > 0)
	{
		if(!mEmptyIdxChunks.empty())
		{
			u32 emptyChunkIdx = mEmptyIdxChunks.top();
			ChunkData& emptyChunk = mIdxChunks[emptyChunkIdx];
			mEmptyIdxChunks.pop();

			emptyChunk.Start = idxChunkStart + meshData->GetNumIndices();
			emptyChunk.Size = remainingNumIdx;
		}
		else
		{
			ChunkData newChunk;
			newChunk.Size = remainingNumIdx;
			newChunk.Start = idxChunkStart + meshData->GetNumIndices();

			mIdxChunks.push_back(newChunk);
			mFreeIdxChunks.push_back((u32)(mIdxChunks.size() - 1));
		}
	}

	AllocatedData newAllocData;
	newAllocData.VertChunkIdx = freeVertChunkIdx;
	newAllocData.IdxChunkIdx = freeIdxChunkIdx;
	newAllocData.UseFlags = UseFlags::GPUFree;
	newAllocData.EventQueryIdx = CreateEventQuery();
	newAllocData.Mesh = mesh;

	mMeshAllocData[mesh->GetMeshHeapId()] = newAllocData;

	// Actually copy data
	for(u32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
	{
		if(!mVertexDesc->HasStream(i))
			continue;

		if(!meshData->GetVertexDesc()->HasStream(i))
			continue;

		// Ensure vertex sizes match
		u32 vertSize = mVertexData->VertexDeclaration->GetProperties().GetVertexSize(i);
		u32 otherVertSize = meshData->GetVertexDesc()->GetVertexStride(i);
		if(otherVertSize != vertSize)
		{
			BS_EXCEPT(InvalidParametersException, "Provided vertex size for stream " + toString(i) + " doesn't match meshes vertex size. Needed: " + toString(vertSize) + ". Got: " + toString(otherVertSize));
		}

		SPtr<VertexBuffer> vertexBuffer = mVertexData->GetBuffer(i);

		u8* vertDest = mCPUVertexData[i] + vertChunkStart * vertSize;
		memcpy(vertDest, meshData->GetStreamData(i), meshData->GetNumVertices() * vertSize);

		vertexBuffer->WriteData(vertChunkStart * vertSize, meshData->GetNumVertices() * vertSize, vertDest, BTW_NO_OVERWRITE);
	}

	const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

	u32 idxSize = ibProps.GetIndexSize();

	// Ensure index sizes match
	if(meshData->GetIndexElementSize() != idxSize)
	{
		BS_EXCEPT(InvalidParametersException, "Provided index size doesn't match meshes index size. Needed: " + toString(idxSize) + ". Got: " + toString(meshData->GetIndexElementSize()));
	}

	u8* idxDest = mCPUIndexData + idxChunkStart * idxSize;
	memcpy(idxDest, meshData->GetIndexData(), meshData->GetNumIndices() * idxSize);
	mIndexBuffer->WriteData(idxChunkStart * idxSize, meshData->GetNumIndices() * idxSize, idxDest, BTW_NO_OVERWRITE);
}

void MeshHeap::Dealloc(SPtr<TransientMesh> mesh)
{
	auto findIter = mMeshAllocData.find(mesh->GetMeshHeapId());
	assert(findIter != mMeshAllocData.end());

	AllocatedData& allocData = findIter->second;
	if(allocData.UseFlags == UseFlags::GPUFree)
	{
		allocData.UseFlags = UseFlags::Free;
		FreeEventQuery(allocData.EventQueryIdx);

		mFreeVertChunks.push_back(allocData.VertChunkIdx);
		mFreeIdxChunks.push_back(allocData.IdxChunkIdx);

		MergeWithNearbyChunks(allocData.VertChunkIdx, allocData.IdxChunkIdx);

		mMeshAllocData.erase(findIter);
	}
	else if(allocData.UseFlags == UseFlags::Used)
		allocData.UseFlags = UseFlags::CPUFree;
}

void MeshHeap::GrowVertexBuffer(u32 numVertices)
{
	mNumVertices = numVertices;
	mVertexData = SPtr<VertexData>(B3DNew<VertexData>());

	mVertexData->VertexCount = mNumVertices;
	mVertexData->VertexDeclaration = VertexDeclaration::Create(mVertexDesc, mDeviceMask);

	// Create buffers and copy data
	for(u32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
	{
		if(!mVertexDesc->HasStream(i))
			continue;

		u32 vertSize = mVertexData->VertexDeclaration->GetProperties().GetVertexSize(i);

		VERTEX_BUFFER_DESC desc;
		desc.VertexSize = vertSize;
		desc.NumVerts = mVertexData->VertexCount;
		desc.Usage = GBU_DYNAMIC;

		SPtr<VertexBuffer> vertexBuffer = VertexBuffer::Create(desc, mDeviceMask);
		mVertexData->SetBuffer(i, vertexBuffer);

		// Copy all data to the new buffer
		u8* oldBuffer = mCPUVertexData[i];
		u8* buffer = (u8*)B3DAllocate(vertSize * numVertices);

		u32 destOffset = 0;
		if(oldBuffer != nullptr)
		{
			for(auto& allocData : mMeshAllocData)
			{
				ChunkData& oldChunk = mVertChunks[allocData.second.VertChunkIdx];

				u8* oldData = oldBuffer + oldChunk.Start * vertSize;
				memcpy(buffer + destOffset * vertSize, oldData, oldChunk.Size * vertSize);

				destOffset += oldChunk.Size;
			}

			B3DFree(oldBuffer);
		}

		if(destOffset > 0)
			vertexBuffer->WriteData(0, destOffset * vertSize, buffer, BTW_NO_OVERWRITE);

		mCPUVertexData[i] = buffer;
	}

	// Reorder chunks
	u32 destOffset = 0;
	Vector<ChunkData> newVertChunks;
	List<u32> freeVertChunks;

	for(auto& allocData : mMeshAllocData)
	{
		ChunkData& oldChunk = mVertChunks[allocData.second.VertChunkIdx];

		ChunkData newChunk;
		newChunk.Start = destOffset;
		newChunk.Size = oldChunk.Size;

		allocData.second.VertChunkIdx = (u32)newVertChunks.size();
		newVertChunks.push_back(newChunk);

		destOffset += oldChunk.Size;
	}

	// Add free chunk
	if(destOffset != mNumVertices)
	{
		ChunkData newChunk;
		newChunk.Start = destOffset;
		newChunk.Size = mNumVertices - destOffset;

		newVertChunks.push_back(newChunk);
		freeVertChunks.push_back((u32)(newVertChunks.size() - 1));
	}

	mVertChunks = newVertChunks;
	mFreeVertChunks = freeVertChunks;

	while(!mEmptyVertChunks.empty())
		mEmptyVertChunks.pop();
}

void MeshHeap::GrowIndexBuffer(u32 numIndices)
{
	mNumIndices = numIndices;

	INDEX_BUFFER_DESC ibDesc;
	ibDesc.IndexType = mIndexType;
	ibDesc.NumIndices = mNumIndices;
	ibDesc.Usage = GBU_DYNAMIC;

	mIndexBuffer = IndexBuffer::Create(ibDesc, mDeviceMask);

	const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

	// Copy all data to the new buffer
	u32 idxSize = ibProps.GetIndexSize();

	u8* oldBuffer = mCPUIndexData;
	u8* buffer = (u8*)B3DAllocate(idxSize * numIndices);

	u32 destOffset = 0;
	if(oldBuffer != nullptr)
	{
		for(auto& allocData : mMeshAllocData)
		{
			ChunkData& oldChunk = mIdxChunks[allocData.second.IdxChunkIdx];

			u8* oldData = oldBuffer + oldChunk.Start * idxSize;
			memcpy(buffer + destOffset * idxSize, oldData, oldChunk.Size * idxSize);

			destOffset += oldChunk.Size;
		}

		B3DFree(oldBuffer);
	}

	if(destOffset > 0)
		mIndexBuffer->WriteData(0, destOffset * idxSize, buffer, BTW_NO_OVERWRITE);

	mCPUIndexData = buffer;

	// Reorder chunks
	destOffset = 0;
	Vector<ChunkData> newIdxChunks;
	List<u32> freeIdxChunks;

	for(auto& allocData : mMeshAllocData)
	{
		ChunkData& oldChunk = mIdxChunks[allocData.second.IdxChunkIdx];

		ChunkData newChunk;
		newChunk.Start = destOffset;
		newChunk.Size = oldChunk.Size;

		allocData.second.IdxChunkIdx = (u32)newIdxChunks.size();
		newIdxChunks.push_back(newChunk);

		destOffset += oldChunk.Size;
	}

	// Add free chunk
	if(destOffset != mNumIndices)
	{
		ChunkData newChunk;
		newChunk.Start = destOffset;
		newChunk.Size = mNumIndices - destOffset;

		newIdxChunks.push_back(newChunk);
		freeIdxChunks.push_back((u32)(newIdxChunks.size() - 1));
	}

	mIdxChunks = newIdxChunks;
	mFreeIdxChunks = freeIdxChunks;

	while(!mEmptyIdxChunks.empty())
		mEmptyIdxChunks.pop();
}

u32 MeshHeap::CreateEventQuery()
{
	u32 idx = 0;
	if(mFreeEventQueries.size() > 0)
	{
		idx = mFreeEventQueries.top();
		mFreeEventQueries.pop();
	}
	else
	{
		QueryData newQuery;
		newQuery.Query = EventQuery::Create();
		newQuery.QueryId = 0;

		mEventQueries.push_back(newQuery);
		idx = (u32)(mEventQueries.size() - 1);
	}

	return idx;
}

void MeshHeap::FreeEventQuery(u32 idx)
{
	mEventQueries[idx].Query->OnTriggered.Clear();
	mEventQueries[idx].QueryId = 0;
	mFreeEventQueries.push(idx);
}

SPtr<VertexData> MeshHeap::GetVertexData() const
{
	return mVertexData;
}

SPtr<IndexBuffer> MeshHeap::GetIndexBuffer() const
{
	return mIndexBuffer;
}

SPtr<VertexDataDesc> MeshHeap::GetVertexDesc() const
{
	return mVertexDesc;
}

u32 MeshHeap::GetVertexOffset(u32 meshId) const
{
	auto findIter = mMeshAllocData.find(meshId);
	assert(findIter != mMeshAllocData.end());

	u32 chunkIdx = findIter->second.VertChunkIdx;
	return mVertChunks[chunkIdx].Start;
}

u32 MeshHeap::GetIndexOffset(u32 meshId) const
{
	auto findIter = mMeshAllocData.find(meshId);
	assert(findIter != mMeshAllocData.end());

	u32 chunkIdx = findIter->second.IdxChunkIdx;
	return mIdxChunks[chunkIdx].Start;
}

void MeshHeap::NotifyUsedOnGpu(u32 meshId)
{
	auto findIter = mMeshAllocData.find(meshId);
	assert(findIter != mMeshAllocData.end());

	AllocatedData& allocData = findIter->second;
	assert(allocData.UseFlags != UseFlags::Free);

	if(allocData.UseFlags == UseFlags::GPUFree)
		allocData.UseFlags = UseFlags::Used;

	SPtr<MeshHeap> thisPtr = std::static_pointer_cast<MeshHeap>(GetThisPtr());

	QueryData& queryData = mEventQueries[allocData.EventQueryIdx];
	queryData.QueryId = mNextQueryId++;
	queryData.Query->OnTriggered.Clear();
	queryData.Query->OnTriggered.Connect(std::bind(&MeshHeap::QueryTriggered, thisPtr, meshId, queryData.QueryId));
	queryData.Query->Begin();
}

// Note: Need to use a shared ptr here to ensure MeshHeap doesn't get deallocated sometime during this callback
void MeshHeap::QueryTriggered(SPtr<MeshHeap> thisPtr, u32 meshId, u32 queryId)
{
	auto findIter = thisPtr->mMeshAllocData.find(meshId);
	assert(findIter != thisPtr->mMeshAllocData.end());

	AllocatedData& allocData = findIter->second;

	// If query ids don't match then it means there either a more recent query or
	// the buffer was discarded and we are not interested in query result
	QueryData& queryData = thisPtr->mEventQueries[allocData.EventQueryIdx];
	if(queryId == queryData.QueryId)
	{
		assert(allocData.UseFlags != UseFlags::Free && allocData.UseFlags != UseFlags::GPUFree);

		if(allocData.UseFlags == UseFlags::CPUFree)
		{
			allocData.UseFlags = UseFlags::Free;
			thisPtr->FreeEventQuery(allocData.EventQueryIdx);

			thisPtr->mFreeVertChunks.push_back(allocData.VertChunkIdx);
			thisPtr->mFreeIdxChunks.push_back(allocData.IdxChunkIdx);

			thisPtr->MergeWithNearbyChunks(allocData.VertChunkIdx, allocData.IdxChunkIdx);

			thisPtr->mMeshAllocData.erase(findIter);
		}
		else
			allocData.UseFlags = UseFlags::GPUFree;
	}

	queryData.Query->OnTriggered.Clear();
}

void MeshHeap::MergeWithNearbyChunks(u32 chunkVertIdx, u32 chunkIdxIdx)
{
	// Merge vertex chunks
	ChunkData& vertChunk = mVertChunks[chunkVertIdx];
	for(auto& freeChunkIdx : mFreeVertChunks)
	{
		if(chunkVertIdx == freeChunkIdx)
			continue;

		ChunkData& curChunk = mVertChunks[freeChunkIdx];
		if(curChunk.Size == 0) // Already merged
			continue;

		bool merged = false;
		if(curChunk.Start == (vertChunk.Start + vertChunk.Size))
		{
			vertChunk.Size += curChunk.Size;

			merged = true;
		}
		else if((curChunk.Start + curChunk.Size) == vertChunk.Start)
		{
			vertChunk.Start = curChunk.Start;
			vertChunk.Size += curChunk.Size;

			merged = true;
		}

		if(merged)
		{
			// We can't remove the chunk since that would break the indexing scheme, so
			// mark it as empty and set size to 0. It will be reused when needed.
			curChunk.Start = 0;
			curChunk.Size = 0;
			mEmptyVertChunks.push(freeChunkIdx);
		}
	}

	// Merge index chunks
	ChunkData& idxChunk = mIdxChunks[chunkIdxIdx];
	for(auto& freeChunkIdx : mFreeIdxChunks)
	{
		if(chunkIdxIdx == freeChunkIdx)
			continue;

		ChunkData& curChunk = mIdxChunks[freeChunkIdx];
		if(curChunk.Size == 0) // Already merged
			continue;

		bool merged = false;
		if(curChunk.Start == (idxChunk.Start + idxChunk.Size))
		{
			idxChunk.Size += curChunk.Size;

			merged = true;
		}
		else if((curChunk.Start + curChunk.Size) == idxChunk.Start)
		{
			idxChunk.Start = curChunk.Start;
			idxChunk.Size += curChunk.Size;

			merged = true;
		}

		if(merged)
		{
			// We can't remove the chunk since that would break the indexing scheme, so
			// mark it as empty and set size to 0. It will be reused when needed.
			curChunk.Start = 0;
			curChunk.Size = 0;
			mEmptyIdxChunks.push(freeChunkIdx);
		}
	}
}
}}
