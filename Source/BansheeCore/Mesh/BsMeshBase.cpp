//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Mesh/BsMeshBase.h"
#include "Private/RTTI/BsMeshBaseRTTI.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	MeshProperties::MeshProperties()
		:mNumVertices(0), mNumIndices(0)
	{
		mSubMeshes.reserve(10);
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

	const SubMesh& MeshProperties::getSubMesh(UINT32 subMeshIdx) const
	{
		if (subMeshIdx >= mSubMeshes.size())
		{
			BS_EXCEPT(InvalidParametersException, "Invalid sub-mesh index ("
				+ toString(subMeshIdx) + "). Number of sub-meshes available: " + toString((int)mSubMeshes.size()));
		}

		return mSubMeshes[subMeshIdx];
	}

	UINT32 MeshProperties::getNumSubMeshes() const
	{
		return (UINT32)mSubMeshes.size();
	}

	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp)
		:mProperties(numVertices, numIndices, drawOp)
	{ }

	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:mProperties(numVertices, numIndices, subMeshes)
	{ }

	MeshBase::~MeshBase()
	{ }

	CoreSyncData MeshBase::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = sizeof(Bounds);
		UINT8* buffer = allocator->alloc(size);

		memcpy(buffer, &mProperties.mBounds, size);
		return CoreSyncData(buffer, size);
	}

	SPtr<ct::MeshBase> MeshBase::getCore() const
	{
		return std::static_pointer_cast<ct::MeshBase>(mCoreSpecific);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* MeshBase::getRTTIStatic()
	{
		return MeshBaseRTTI::instance();
	}

	RTTITypeBase* MeshBase::getRTTI() const
	{
		return MeshBase::getRTTIStatic();
	}

	namespace ct
	{
	MeshBase::MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes)
		:mProperties(numVertices, numIndices, subMeshes)
	{ }

	void MeshBase::syncToCore(const CoreSyncData& data)
	{
		mProperties.mBounds = data.getData<Bounds>();
	}
	}
}