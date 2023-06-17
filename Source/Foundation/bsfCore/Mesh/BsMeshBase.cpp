//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMeshBase.h"
#include "Private/RTTI/BsMeshBaseRTTI.h"
#include "CoreThread/BsCoreThread.h"

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

CoreSyncData MeshBase::SyncToCore(FrameAlloc* allocator)
{
	u32 size = sizeof(Bounds);
	u8* buffer = allocator->Alloc(size);

	memcpy(buffer, &mProperties.Bounds, size);
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
MeshBase::MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes)
	: mProperties(vertexCount, indexCount, subMeshes)
{}

void MeshBase::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	mProperties.Bounds = data.GetData<Bounds>();
}
}}
