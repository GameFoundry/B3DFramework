//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMesh.h"
#include "Private/RTTI/BsMeshRTTI.h"
#include "Mesh/BsMeshData.h"
#include "Debug/BsDebug.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Managers/BsMeshManager.h"
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsAsyncOp.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Resources/BsResources.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

MESH_DESC MESH_DESC::DEFAULT = MESH_DESC();

Mesh::Mesh(const MESH_DESC& desc)
	: MeshBase(desc.NumVertices, desc.NumIndices, desc.SubMeshes), mVertexDesc(desc.VertexDesc), mUsage(desc.Usage), mIndexType(desc.IndexType), mSkeleton(desc.Skeleton), mMorphShapes(desc.MorphShapes)
{
}

Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	: MeshBase(initialMeshData->GetNumVertices(), initialMeshData->GetNumIndices(), desc.SubMeshes), mCPUData(initialMeshData), mVertexDesc(initialMeshData->GetVertexDesc()), mUsage(desc.Usage), mIndexType(initialMeshData->GetIndexType()), mSkeleton(desc.Skeleton), mMorphShapes(desc.MorphShapes)
{}

Mesh::Mesh()
	: MeshBase(0, 0, DOT_TRIANGLE_LIST)
{}

AsyncOp Mesh::WriteData(const SPtr<MeshData>& data, bool discardEntireBuffer)
{
	UpdateBounds(*data);
	UpdateCpuBuffer(0, *data);

	data->LockInternal();

	std::function<void(const SPtr<ct::Mesh>&, const SPtr<MeshData>&, bool, AsyncOp&)> func =
		[&](const SPtr<ct::Mesh>& mesh, const SPtr<MeshData>& _meshData, bool _discardEntireBuffer, AsyncOp& asyncOp)
	{
		mesh->WriteData(*_meshData, _discardEntireBuffer, false);
		_meshData->UnlockInternal();
		asyncOp.CompleteOperationInternal();
	};

	return GetCoreThread().QueueReturnCommand(std::bind(func, GetCore(), data, discardEntireBuffer, std::placeholders::_1));
}

AsyncOp Mesh::ReadData(const SPtr<MeshData>& data)
{
	data->LockInternal();

	std::function<void(const SPtr<ct::Mesh>&, const SPtr<MeshData>&, AsyncOp&)> func =
		[&](const SPtr<ct::Mesh>& mesh, const SPtr<MeshData>& _meshData, AsyncOp& asyncOp)
	{
		// Make sure any queued command start executing before reading
		ct::RenderAPI::Instance().SubmitCommandBuffer(nullptr);

		mesh->ReadData(*_meshData);
		_meshData->UnlockInternal();
		asyncOp.CompleteOperationInternal();
	};

	return GetCoreThread().QueueReturnCommand(std::bind(func, GetCore(), data, std::placeholders::_1));
}

SPtr<MeshData> Mesh::AllocBuffer() const
{
	SPtr<MeshData> meshData = B3DMakeShared<MeshData>(mProperties.mNumVertices, mProperties.mNumIndices, mVertexDesc, mIndexType);

	return meshData;
}

void Mesh::Initialize()
{
	if(mCPUData != nullptr)
		UpdateBounds(*mCPUData);

	MeshBase::Initialize();

	if((mUsage & MU_CPUCACHED) != 0 && mCPUData == nullptr)
		CreateCpuBuffer();
}

void Mesh::UpdateBounds(const MeshData& meshData)
{
	mProperties.mBounds = meshData.CalculateBounds();
	MarkCoreDirty();
}

SPtr<ct::Mesh> Mesh::GetCore() const
{
	return std::static_pointer_cast<ct::Mesh>(mCoreSpecific);
}

SPtr<ct::CoreObject> Mesh::CreateCore() const
{
	MESH_DESC desc;
	desc.NumVertices = mProperties.mNumVertices;
	desc.NumIndices = mProperties.mNumIndices;
	desc.VertexDesc = mVertexDesc;
	desc.SubMeshes = mProperties.mSubMeshes;
	desc.Usage = mUsage;
	desc.IndexType = mIndexType;
	desc.Skeleton = mSkeleton;
	desc.MorphShapes = mMorphShapes;

	ct::Mesh* obj = new(B3DAllocate<ct::Mesh>()) ct::Mesh(mCPUData, desc, GDF_DEFAULT);

	SPtr<ct::CoreObject> meshCore = B3DMakeSharedFromExisting<ct::Mesh>(obj);
	meshCore->SetThisPtrInternal(meshCore);

	if((mUsage & MU_CPUCACHED) == 0)
		mCPUData = nullptr;

	return meshCore;
}

void Mesh::UpdateCpuBuffer(u32 subresourceIdx, const MeshData& pixelData)
{
	if((mUsage & MU_CPUCACHED) == 0)
		return;

	if(subresourceIdx > 0)
	{
		BS_LOG(Error, Mesh, "Invalid subresource index: {0}. Supported range: 0 .. 1.", subresourceIdx);
		return;
	}

	if(pixelData.GetNumIndices() != mProperties.GetNumIndices() ||
	   pixelData.GetNumVertices() != mProperties.GetNumVertices() ||
	   pixelData.GetIndexType() != mIndexType ||
	   pixelData.GetVertexDesc()->GetVertexStride() != mVertexDesc->GetVertexStride())
	{
		BS_LOG(Error, Mesh, "Provided buffer is not of valid dimensions or format in order to update this mesh.");
		return;
	}

	if(mCPUData->GetSize() != pixelData.GetSize())
		BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

	u8* dest = mCPUData->GetData();
	u8* src = pixelData.GetData();

	memcpy(dest, src, pixelData.GetSize());
}

void Mesh::CreateCpuBuffer()
{
	mCPUData = AllocBuffer();
}

HMesh Mesh::Dummy()
{
	return MeshManager::Instance().GetDummyMesh();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* Mesh::GetRttiStatic()
{
	return MeshRTTI::Instance();
}

RTTITypeBase* Mesh::GetRtti() const
{
	return Mesh::GetRttiStatic();
}

/************************************************************************/
/* 								STATICS		                     		*/
/************************************************************************/

HMesh Mesh::Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, int usage, DrawOperationType drawOp, IndexType indexType)
{
	MESH_DESC desc;
	desc.NumVertices = numVertices;
	desc.NumIndices = numIndices;
	desc.VertexDesc = vertexDesc;
	desc.Usage = usage;
	desc.SubMeshes.push_back(SubMesh(0, numIndices, drawOp));
	desc.IndexType = indexType;

	SPtr<Mesh> meshPtr = CreatePtrInternal(desc);
	return static_resource_cast<Mesh>(GetResources().CreateResourceHandleInternal(meshPtr));
}

HMesh Mesh::Create(const MESH_DESC& desc)
{
	SPtr<Mesh> meshPtr = CreatePtrInternal(desc);
	return static_resource_cast<Mesh>(GetResources().CreateResourceHandleInternal(meshPtr));
}

HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
{
	SPtr<Mesh> meshPtr = CreatePtrInternal(initialMeshData, desc);
	return static_resource_cast<Mesh>(GetResources().CreateResourceHandleInternal(meshPtr));
}

HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp)
{
	SPtr<Mesh> meshPtr = CreatePtrInternal(initialMeshData, usage, drawOp);
	return static_resource_cast<Mesh>(GetResources().CreateResourceHandleInternal(meshPtr));
}

SPtr<Mesh> Mesh::CreatePtrInternal(const MESH_DESC& desc)
{
	SPtr<Mesh> mesh = B3DMakeCoreFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(desc));
	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreatePtrInternal(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
{
	SPtr<Mesh> mesh = B3DMakeCoreFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, desc));
	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreatePtrInternal(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp)
{
	MESH_DESC desc;
	desc.Usage = usage;
	desc.SubMeshes.push_back(SubMesh(0, initialMeshData->GetNumIndices(), drawOp));

	SPtr<Mesh> mesh = B3DMakeCoreFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, desc));
	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreateEmpty()
{
	SPtr<Mesh> mesh = B3DMakeCoreFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh());
	mesh->SetThisPtrInternal(mesh);

	return mesh;
}

namespace bs { namespace ct
{
Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc, GpuDeviceFlags deviceMask)
	: MeshBase(desc.NumVertices, desc.NumIndices, desc.SubMeshes), mVertexData(nullptr), mIndexBuffer(nullptr), mVertexDesc(desc.VertexDesc), mUsage(desc.Usage), mIndexType(desc.IndexType), mDeviceMask(deviceMask), mTempInitialMeshData(initialMeshData), mSkeleton(desc.Skeleton), mMorphShapes(desc.MorphShapes)

{}

Mesh::~Mesh()
{
	THROW_IF_NOT_CORE_THREAD;

	mVertexData = nullptr;
	mIndexBuffer = nullptr;
	mVertexDesc = nullptr;
	mTempInitialMeshData = nullptr;
}

void Mesh::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	bool isDynamic = (mUsage & MU_DYNAMIC) != 0;
	int usage = isDynamic ? GBU_DYNAMIC : GBU_STATIC;

	INDEX_BUFFER_DESC ibDesc;
	ibDesc.IndexType = mIndexType;
	ibDesc.NumIndices = mProperties.mNumIndices;
	ibDesc.Usage = (GpuBufferUsage)usage;

	mIndexBuffer = IndexBuffer::Create(ibDesc, mDeviceMask);

	mVertexData = B3DMakeShared<VertexData>();
	mVertexData->VertexCount = mProperties.mNumVertices;
	mVertexData->VertexDeclaration = VertexDeclaration::Create(mVertexDesc, mDeviceMask);

	for(u32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
	{
		if(!mVertexDesc->HasStream(i))
			continue;

		VERTEX_BUFFER_DESC vbDesc;
		vbDesc.VertexSize = mVertexData->VertexDeclaration->GetProperties().GetVertexSize(i);
		vbDesc.NumVerts = mVertexData->VertexCount;
		vbDesc.Usage = (GpuBufferUsage)usage;

		SPtr<VertexBuffer> vertexBuffer = VertexBuffer::Create(vbDesc, mDeviceMask);
		mVertexData->SetBuffer(i, vertexBuffer);
	}

	// TODO Low priority - DX11 (and maybe OpenGL)? allow an optimization that allows you to set
	// buffer data upon buffer construction, instead of setting it in a second step like I do here
	if(mTempInitialMeshData != nullptr)
	{
		WriteData(*mTempInitialMeshData, isDynamic);
		mTempInitialMeshData = nullptr;
	}

	MeshBase::Initialize();
}

SPtr<VertexData> Mesh::GetVertexData() const
{
	THROW_IF_NOT_CORE_THREAD;

	return mVertexData;
}

SPtr<IndexBuffer> Mesh::GetIndexBuffer() const
{
	THROW_IF_NOT_CORE_THREAD;

	return mIndexBuffer;
}

SPtr<VertexDataDesc> Mesh::GetVertexDesc() const
{
	THROW_IF_NOT_CORE_THREAD;

	return mVertexDesc;
}

void Mesh::WriteData(const MeshData& meshData, bool discardEntireBuffer, bool performUpdateBounds, u32 queueIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	if(discardEntireBuffer)
	{
		if((mUsage & MU_STATIC) != 0)
		{
			BS_LOG(Warning, Mesh, "Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
			discardEntireBuffer = false;
		}
	}
	else
	{
		if((mUsage & MU_DYNAMIC) != 0)
		{
			BS_LOG(Warning, Mesh, "Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
			discardEntireBuffer = true;
		}
	}

	// Indices
	const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

	u32 indicesSize = meshData.GetIndexBufferSize();
	u8* srcIdxData = meshData.GetIndexData();

	if(meshData.GetIndexElementSize() != ibProps.GetIndexSize())
	{
		BS_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", ibProps.GetIndexSize(), meshData.GetIndexElementSize());

		return;
	}

	if(indicesSize > mIndexBuffer->GetSize())
	{
		indicesSize = mIndexBuffer->GetSize();
		BS_LOG(Error, Mesh, "Index buffer values are being written out of valid range.");
	}

	mIndexBuffer->WriteData(0, indicesSize, srcIdxData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL, queueIdx);

	// Vertices
	for(u32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
	{
		if(!mVertexDesc->HasStream(i))
			continue;

		if(!meshData.GetVertexDesc()->HasStream(i))
			continue;

		// Ensure both have the same sized vertices
		u32 myVertSize = mVertexDesc->GetVertexStride(i);
		u32 otherVertSize = meshData.GetVertexDesc()->GetVertexStride(i);
		if(myVertSize != otherVertSize)
		{
			BS_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
								"Needed: {1}. Got: {2}",
				   i, myVertSize, otherVertSize);

			continue;
		}

		SPtr<VertexBuffer> vertexBuffer = mVertexData->GetBuffer(i);

		u32 bufferSize = meshData.GetStreamSize(i);
		u8* srcVertBufferData = meshData.GetStreamData(i);

		if(bufferSize > vertexBuffer->GetSize())
		{
			bufferSize = vertexBuffer->GetSize();
			BS_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being written out of valid range.", i);
		}

		vertexBuffer->WriteData(0, bufferSize, srcVertBufferData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL, queueIdx);
	}

	if(performUpdateBounds)
		UpdateBounds(meshData);
}

void Mesh::ReadData(MeshData& meshData, u32 deviceIdx, u32 queueIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	IndexType indexType = IT_32BIT;
	if(mIndexBuffer)
		indexType = mIndexBuffer->GetProperties().GetType();

	if(mIndexBuffer)
	{
		const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

		if(meshData.GetIndexElementSize() != ibProps.GetIndexSize())
		{
			BS_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", ibProps.GetIndexSize(), meshData.GetIndexElementSize());
			return;
		}

		u8* idxData = static_cast<u8*>(mIndexBuffer->Lock(GBL_READ_ONLY, deviceIdx, queueIdx));
		u32 idxElemSize = ibProps.GetIndexSize();

		u8* indices = nullptr;

		if(indexType == IT_16BIT)
			indices = (u8*)meshData.GetIndices16();
		else
			indices = (u8*)meshData.GetIndices32();

		u32 numIndicesToCopy = std::min(mProperties.mNumIndices, meshData.GetNumIndices());

		u32 indicesSize = numIndicesToCopy * idxElemSize;
		if(indicesSize > meshData.GetIndexBufferSize())
		{
			BS_LOG(Error, Mesh, "Provided buffer doesn't have enough space to store mesh indices.");
			return;
		}

		memcpy(indices, idxData, numIndicesToCopy * idxElemSize);

		mIndexBuffer->Unlock();
	}

	if(mVertexData)
	{
		auto vertexBuffers = mVertexData->GetBuffers();

		u32 streamIdx = 0;
		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			if(!meshData.GetVertexDesc()->HasStream(streamIdx))
				continue;

			SPtr<VertexBuffer> vertexBuffer = iter->second;
			const VertexBufferProperties& vbProps = vertexBuffer->GetProperties();

			// Ensure both have the same sized vertices
			u32 myVertSize = mVertexDesc->GetVertexStride(streamIdx);
			u32 otherVertSize = meshData.GetVertexDesc()->GetVertexStride(streamIdx);
			if(myVertSize != otherVertSize)
			{
				BS_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
									"Needed: {1}. Got: {2}",
					   streamIdx, myVertSize, otherVertSize);

				continue;
			}

			u32 numVerticesToCopy = meshData.GetNumVertices();
			u32 bufferSize = vbProps.GetVertexSize() * numVerticesToCopy;

			if(bufferSize > vertexBuffer->GetSize())
			{
				BS_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being read out of valid range.", streamIdx);
				continue;
			}

			u8* vertDataPtr = static_cast<u8*>(vertexBuffer->Lock(GBL_READ_ONLY, deviceIdx, queueIdx));

			u8* dest = meshData.GetStreamData(streamIdx);
			memcpy(dest, vertDataPtr, bufferSize);

			vertexBuffer->Unlock();

			streamIdx++;
		}
	}
}

void Mesh::UpdateBounds(const MeshData& meshData)
{
	mProperties.mBounds = meshData.CalculateBounds();

	// TODO - Sync this to sim-thread possibly?
}

SPtr<Mesh> Mesh::Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, int usage, DrawOperationType drawOp, IndexType indexType, GpuDeviceFlags deviceMask)
{
	MESH_DESC desc;
	desc.NumVertices = numVertices;
	desc.NumIndices = numIndices;
	desc.VertexDesc = vertexDesc;
	desc.SubMeshes.push_back(SubMesh(0, numIndices, drawOp));
	desc.Usage = usage;
	desc.IndexType = indexType;

	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, desc, deviceMask));
	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const MESH_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, desc, deviceMask));

	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc, GpuDeviceFlags deviceMask)
{
	MESH_DESC descCopy = desc;
	descCopy.NumVertices = initialMeshData->GetNumVertices();
	descCopy.NumIndices = initialMeshData->GetNumIndices();
	descCopy.VertexDesc = initialMeshData->GetVertexDesc();
	descCopy.IndexType = initialMeshData->GetIndexType();

	SPtr<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, descCopy, deviceMask));

	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp, GpuDeviceFlags deviceMask)
{
	MESH_DESC desc;
	desc.NumVertices = initialMeshData->GetNumVertices();
	desc.NumIndices = initialMeshData->GetNumIndices();
	desc.VertexDesc = initialMeshData->GetVertexDesc();
	desc.IndexType = initialMeshData->GetIndexType();
	desc.SubMeshes.push_back(SubMesh(0, initialMeshData->GetNumIndices(), drawOp));
	desc.Usage = usage;

	SPtr<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, desc, deviceMask));

	mesh->SetThisPtrInternal(mesh);
	mesh->Initialize();

	return mesh;
}
}}
