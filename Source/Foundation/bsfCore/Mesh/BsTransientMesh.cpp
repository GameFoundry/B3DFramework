//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsTransientMesh.h"
#include "RenderAPI/BsVertexData.h"
#include "Math/BsBounds.h"
#include "Mesh/BsMeshHeap.h"

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

	SPtr<ct::TransientMesh> TransientMesh::getCore() const
	{
		return std::static_pointer_cast<ct::TransientMesh>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> TransientMesh::createCore() const
	{
		ct::TransientMesh* core = new (bs_alloc<ct::TransientMesh>()) ct::TransientMesh(
			mParentHeap->getCore(), mId, mProperties.mNumVertices, mProperties.mNumIndices, mProperties.mSubMeshes);

		SPtr<ct::CoreObject> meshCore = bs_shared_ptr<ct::TransientMesh>(core);
		meshCore->SetThisPtrInternal(meshCore);

		return meshCore;
	}

	namespace ct
	{
	TransientMesh::TransientMesh(const SPtr<MeshHeap>& parentHeap, UINT32 id,
		UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:MeshBase(numVertices, numIndices, subMeshes), mParentHeap(parentHeap), mId(id)
	{

	}

	SPtr<VertexData> TransientMesh::getVertexData() const
	{
		return mParentHeap->getVertexData();
	}

	SPtr<IndexBuffer> TransientMesh::getIndexBuffer() const
	{
		return mParentHeap->getIndexBuffer();
	}

	UINT32 TransientMesh::getVertexOffset() const
	{
		return mParentHeap->getVertexOffset(mId);
	}

	UINT32 TransientMesh::getIndexOffset() const
	{
		return mParentHeap->getIndexOffset(mId);
	}

	SPtr<VertexDataDesc> TransientMesh::getVertexDesc() const
	{
		return mParentHeap->getVertexDesc();
	}

	void TransientMesh::NotifyUsedOnGPUInternal()
	{
		mParentHeap->notifyUsedOnGPU(mId);
	}
	}
}
