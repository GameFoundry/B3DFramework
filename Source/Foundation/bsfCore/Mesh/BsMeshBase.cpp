//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshBase.h"
#include "Private/RTTI/BsMeshBaseRTTI.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	MeshProperties::MeshProperties()
		:mNumVertices(0), mNumIndices(0)
	{
		mSubMeshes.Reserve(10);
	}

	MeshProperties::MeshProperties(UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp)
		:mNumVertices(numVertices), mNumIndices(numIndices)
	{
		mSubMeshes.push_back(SubMesh(0, numIndices, drawOp));
	}

	MeshProperties::MeshProperties(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:mNumVertices(numVertices), mNumIndices(numIndices)
	{
		mSubMeshes = subMeshes;
	}

	const SubMesh& MeshProperties::GetSubMesh(UINT32 subMeshIdx) const
	{
		if (subMeshIdx >= mSubMeshes.Size())
		{
			BS_EXCEPT(InvalidParametersException, "Invalid sub-mesh index ("
				+ toString(subMeshIdx) + "). Number of sub-meshes available: " + toString((int)mSubMeshes.Size()));
		}

		return mSubMeshes[subMeshIdx];
	}

	UINT32 MeshProperties::GetNumSubMeshes() const
	{
		return (UINT32)mSubMeshes.Size();
	}

	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp)
		:mProperties(numVertices, numIndices, drawOp)
	{ }

	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:mProperties(numVertices, numIndices, subMeshes)
	{ }

	MeshBase::~MeshBase()
	{ }

	CoreSyncData MeshBase::SyncToCore(FrameAlloc* allocator)
	{
		UINT32 size = sizeof(Bounds);
		UINT8* buffer = allocator->Alloc(size);

		memcpy(buffer, &mProperties.mBounds, size);
		return CoreSyncData(buffer, size);
	}

	SPtr<ct::MeshBase> MeshBase::GetCore() const
	{
		return std::static_pointer_cast<ct::MeshBase>(mCoreSpecific);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* MeshBase::getRTTIStatic()
	{
		return MeshBaseRTTI::Instance();
	}

	RTTITypeBase* MeshBase::getRTTI() const
	{
		return MeshBase::GetRTTIStatic();
	}

	namespace ct
	{
	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:mProperties(numVertices, numIndices, subMeshes)
	{ }

	void MeshBase::SyncToCore(const CoreSyncData& data)
	{
		mProperties.mBounds = data.getData<Bounds>();
	}
	}
}
