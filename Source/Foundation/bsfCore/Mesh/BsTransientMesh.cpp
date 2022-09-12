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
			mParentHeap->Dealloc(meshPtr);
		}
	}

	SPtr<ct::TransientMesh> TransientMesh::GetCore() const
	{
		return std::static_pointer_cast<ct::TransientMesh>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> TransientMesh::CreateCore() const
	{
		ct::TransientMesh* core = new (bs_alloc<ct::TransientMesh>()) ct::TransientMesh(
			mParentHeap->GetCore(), mId, mProperties.mNumVertices, mProperties.mNumIndices, mProperties.mSubMeshes);

		SPtr<ct::CoreObject> meshCore = bs_shared_ptr<ct::TransientMesh>(core);
		meshCore->_setThisPtr(meshCore);

		return meshCore;
	}

	namespace ct
	{
	TransientMesh::TransientMesh(const SPtr<MeshHeap>& parentHeap, UINT32 id,
		UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:MeshBase(numVertices, numIndices, subMeshes), mParentHeap(parentHeap), mId(id)
	{

	}

	SPtr<VertexData> TransientMesh::GetVertexData() const
	{
		return mParentHeap->GetVertexData();
	}

	SPtr<IndexBuffer> TransientMesh::GetIndexBuffer() const
	{
		return mParentHeap->GetIndexBuffer();
	}

	UINT32 TransientMesh::GetVertexOffset() const
	{
		return mParentHeap->GetVertexOffset(mId);
	}

	UINT32 TransientMesh::GetIndexOffset() const
	{
		return mParentHeap->GetIndexOffset(mId);
	}

	SPtr<VertexDataDesc> TransientMesh::GetVertexDesc() const
	{
		return mParentHeap->GetVertexDesc();
	}

	void TransientMesh::_notifyUsedOnGPU()
	{
		mParentHeap->NotifyUsedOnGPU(mId);
	}
	}
}
