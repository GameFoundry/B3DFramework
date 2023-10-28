//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshBase.h"

#include "CoreObject/BsCoreObjectSync.h"
#include "Private/RTTI/BsMeshBaseRTTI.h"
#include "CoreObject/BsRenderThread.h"

using namespace bs;

MeshProperties::MeshProperties()
	: VertexCount(0), IndexCount(0)
{
	SubMeshes.reserve(10);
}

MeshProperties::MeshProperties(u32 vertexCount, u32 indexCount, DrawOperationType primitiveType)
	: VertexCount(vertexCount), IndexCount(indexCount)
{
	SubMeshes.push_back(SubMesh(0, indexCount, primitiveType));
}

MeshProperties::MeshProperties(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes)
	: VertexCount(vertexCount), IndexCount(indexCount)
{
	SubMeshes = subMeshes;
}

MeshBase::MeshBase(u32 vertexCount, u32 indexCount, DrawOperationType drawOp)
	: mProperties(vertexCount, indexCount, drawOp)
{}

MeshBase::MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes)
	: mProperties(vertexCount, indexCount, subMeshes)
{}

MeshBase::~MeshBase()
{}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(MeshBase, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Bounds, Bounds)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* MeshBase::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->Bounds = mProperties.Bounds;

	return syncPacket;
}

SPtr<ct::MeshBase> MeshBase::GetCore() const
{
	return std::static_pointer_cast<ct::MeshBase>(mRenderProxy);
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
MeshBase::MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes)
	: mProperties(vertexCount, indexCount, subMeshes)
{}

void MeshBase::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	const auto* const syncPacket = data.GetSyncPacket<bs::MeshBase::SyncPacket>();
	if(!syncPacket)
		return;

	mProperties.Bounds = syncPacket->Bounds;
}
}}
