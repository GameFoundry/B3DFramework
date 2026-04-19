//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/B3DMeshBase.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "RTTI/B3DMeshBaseRTTI.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

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

namespace b3d
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

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* MeshBase::GetRttiStatic()
{
	return MeshBaseRTTI::Instance();
}

RTTIType* MeshBase::GetRtti() const
{
	return MeshBase::GetRttiStatic();
}

namespace b3d { namespace render
{
MeshBase::MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes)
	: mProperties(vertexCount, indexCount, subMeshes)
{}

void MeshBase::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	const auto* const syncPacket = data.GetSyncPacket<b3d::MeshBase::SyncPacket>();
	if(!syncPacket)
		return;

	mProperties.Bounds = syncPacket->Bounds;
}
}}
