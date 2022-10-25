//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshBase.h"
#include "Private/RTTI/BsMeshBaseRTTI.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

MeshProperties::MeshProperties()
	: mNumVertices(0), mNumIndices(0)
{
	mSubMeshes.reserve(10);
}

MeshProperties::MeshProperties(u32 numVertices, u32 numIndices, DrawOperationType drawOp)
	: mNumVertices(numVertices), mNumIndices(numIndices)
{
	mSubMeshes.push_back(SubMesh(0, numIndices, drawOp));
}

MeshProperties::MeshProperties(u32 numVertices, u32 numIndices, const Vector<SubMesh>& subMeshes)
	: mNumVertices(numVertices), mNumIndices(numIndices)
{
	mSubMeshes = subMeshes;
}

const SubMesh& MeshProperties::GetSubMesh(u32 subMeshIdx) const
{
	if(subMeshIdx >= mSubMeshes.size())
	{
		BS_EXCEPT(InvalidParametersException, "Invalid sub-mesh index (" + toString(subMeshIdx) + "). Number of sub-meshes available: " + toString((int)mSubMeshes.size()));
	}

	return mSubMeshes[subMeshIdx];
}

u32 MeshProperties::GetNumSubMeshes() const
{
	return (u32)mSubMeshes.size();
}

MeshBase::MeshBase(u32 numVertices, u32 numIndices, DrawOperationType drawOp)
	: mProperties(numVertices, numIndices, drawOp)
{}

MeshBase::MeshBase(u32 numVertices, u32 numIndices, const Vector<SubMesh>& subMeshes)
	: mProperties(numVertices, numIndices, subMeshes)
{}

MeshBase::~MeshBase()
{}

CoreSyncData MeshBase::SyncToCore(FrameAlloc* allocator)
{
	u32 size = sizeof(Bounds);
	u8* buffer = allocator->Alloc(size);

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

RTTITypeBase* MeshBase::GetRttiStatic()
{
	return MeshBaseRTTI::Instance();
}

RTTITypeBase* MeshBase::GetRtti() const
{
	return MeshBase::GetRttiStatic();
}

namespace bs { namespace ct
{
MeshBase::MeshBase(u32 numVertices, u32 numIndices, const Vector<SubMesh>& subMeshes)
	: mProperties(numVertices, numIndices, subMeshes)
{}

void MeshBase::SyncToCore(const CoreSyncData& data)
{
	mProperties.mBounds = data.GetData<Bounds>();
}
}}
