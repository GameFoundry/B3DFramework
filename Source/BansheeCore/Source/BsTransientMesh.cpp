//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTransientMesh.h"
#include "BsVertexData.h"
#include "BsBounds.h"
#include "BsMeshHeap.h"
#include "BsFrameAlloc.h"

namespace bs
{
	TransientMesh::TransientMesh(const SPtr<MeshHeap>& parentHeap, UINT32 id, UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp)
		:MeshBase(numVertices, numIndices, drawOp), mIsDestroyed(false), mParentHeap(parentHeap), mId(id)
	{

	}

	TransientMesh::~TransientMesh()
	{
		if (!mIsDestroyed)
		{
			SPtr<TransientMesh> meshPtr = std::static_pointer_cast<TransientMesh>(getThisPtr());
			mParentHeap->dealloc(meshPtr);
		}
	}

	SPtr<ct::TransientMeshCore> TransientMesh::getCore() const
	{
		return std::static_pointer_cast<ct::TransientMeshCore>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> TransientMesh::createCore() const
	{
		ct::TransientMeshCore* core = new (bs_alloc<ct::TransientMeshCore>()) ct::TransientMeshCore(
			mParentHeap->getCore(), mId, mProperties.mNumVertices, mProperties.mNumIndices, mProperties.mSubMeshes);

		SPtr<ct::CoreObject> meshCore = bs_shared_ptr<ct::TransientMeshCore>(core);
		meshCore->_setThisPtr(meshCore);

		return meshCore;
	}

	namespace ct
	{
	TransientMeshCore::TransientMeshCore(const SPtr<MeshHeapCore>& parentHeap, UINT32 id, 
		UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:MeshCoreBase(numVertices, numIndices, subMeshes), mParentHeap(parentHeap), mId(id)
	{

	}

	SPtr<VertexData> TransientMeshCore::getVertexData() const
	{
		return mParentHeap->getVertexData();
	}

	SPtr<IndexBufferCore> TransientMeshCore::getIndexBuffer() const
	{
		return mParentHeap->getIndexBuffer();
	}

	UINT32 TransientMeshCore::getVertexOffset() const
	{
		return mParentHeap->getVertexOffset(mId);
	}

	UINT32 TransientMeshCore::getIndexOffset() const
	{
		return mParentHeap->getIndexOffset(mId);
	}

	SPtr<VertexDataDesc> TransientMeshCore::getVertexDesc() const
	{
		return mParentHeap->getVertexDesc();
	}

	void TransientMeshCore::_notifyUsedOnGPU()
	{
		mParentHeap->notifyUsedOnGPU(mId);
	}
	}
}