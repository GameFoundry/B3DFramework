//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/BsMesh.h"
#include "Private/RTTI/BsMeshRTTI.h"
#include "Mesh/BsMeshData.h"
#include "Debug/BsDebug.h"
#include "Managers/BsMeshManager.h"
#include "CoreObject/BsRenderThread.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Threading/BsAsyncOp.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Resources/BsResources.h"

using namespace bs;

const MeshCreateInformation MeshCreateInformation::kDefault = MeshCreateInformation();

Mesh::Mesh(const MeshCreateInformation& meshCreateInformation)
	: MeshBase(meshCreateInformation.VertexCount, meshCreateInformation.IndexCount, meshCreateInformation.SubMeshes), mVertexDescription(meshCreateInformation.VertexDescription), mUsage(meshCreateInformation.Usage), mIndexType(meshCreateInformation.IndexType), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)
{
}

Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
	: MeshBase(initialMeshData->GetVertexCount(), initialMeshData->GetIndexCount(), meshCreateInformation.SubMeshes), mCPUData(initialMeshData), mVertexDescription(initialMeshData->GetVertexDescription()), mUsage(meshCreateInformation.Usage), mIndexType(initialMeshData->GetIndexType()), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)
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
		asyncOp.CompleteOperation();
	};

	AsyncOp asyncOp;
	GetRenderThread().PostCommand([func = std::move(func), renderProxy = B3DGetRenderProxy(this), data, discardEntireBuffer, asyncOp]() mutable { func(renderProxy, data, discardEntireBuffer, asyncOp); }, "Mesh::WriteData", false, GetName());

	return asyncOp;
}

AsyncOp Mesh::ReadData(const SPtr<MeshData>& data)
{
	data->LockInternal();

	std::function<void(const SPtr<ct::Mesh>&, const SPtr<MeshData>&, AsyncOp&)> func =
		[&](const SPtr<ct::Mesh>& mesh, const SPtr<MeshData>& _meshData, AsyncOp& asyncOp)
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		mesh->ReadData(*_meshData);
		_meshData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	AsyncOp asyncOp;
	GetRenderThread().PostCommand([func = std::move(func), renderProxy = B3DGetRenderProxy(this), data, asyncOp]() mutable { func(renderProxy, data, asyncOp); }, "Mesh::ReadData", false, GetName());

	return asyncOp;
}

SPtr<MeshData> Mesh::AllocBuffer() const
{
	SPtr<MeshData> meshData = B3DMakeShared<MeshData>(mProperties.VertexCount, mProperties.IndexCount, mVertexDescription, mIndexType);

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
	mProperties.Bounds = meshData.CalculateBounds();
	MarkRenderProxyDataDirty();
}

SPtr<ct::RenderProxy> Mesh::CreateRenderProxy() const
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = mProperties.VertexCount;
	meshCreateInformation.IndexCount = mProperties.IndexCount;
	meshCreateInformation.VertexDescription = mVertexDescription;
	meshCreateInformation.SubMeshes = mProperties.SubMeshes;
	meshCreateInformation.Usage = mUsage;
	meshCreateInformation.IndexType = mIndexType;
	meshCreateInformation.Skeleton = mSkeleton;
	meshCreateInformation.MorphShapes = mMorphShapes;

	ct::Mesh* renderProxy = new(B3DAllocate<ct::Mesh>()) ct::Mesh(mCPUData, meshCreateInformation);

	SPtr<ct::RenderProxy> renderProxyShared = B3DMakeSharedFromExisting<ct::Mesh>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	if((mUsage & MU_CPUCACHED) == 0)
		mCPUData = nullptr;

	return renderProxyShared;
}

void Mesh::UpdateCpuBuffer(u32 subresourceIndex, const MeshData& pixelData)
{
	if((mUsage & MU_CPUCACHED) == 0)
		return;

	if(subresourceIndex > 0)
	{
		B3D_LOG(Error, Mesh, "Invalid subresource index: {0}. Supported range: 0 .. 1.", subresourceIndex);
		return;
	}

	if(pixelData.GetIndexCount() != mProperties.IndexCount ||
	   pixelData.GetVertexCount() != mProperties.VertexCount ||
	   pixelData.GetIndexType() != mIndexType ||
	   pixelData.GetVertexDescription()->GetVertexStride() != mVertexDescription->GetVertexStride())
	{
		B3D_LOG(Error, Mesh, "Provided buffer is not of valid dimensions or format in order to update this mesh.");
		return;
	}

	if(mCPUData->GetSize() != pixelData.GetSize())
		B3D_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

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

HMesh Mesh::Create(u32 vertexCount, u32 indexCount, const SPtr<VertexDescription>& vertexDescription, int usage, DrawOperationType primitiveType, IndexType indexType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = vertexCount;
	meshCreateInformation.IndexCount = indexCount;
	meshCreateInformation.VertexDescription = vertexDescription;
	meshCreateInformation.Usage = usage;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, indexCount, primitiveType));
	meshCreateInformation.IndexType = indexType;

	SPtr<Mesh> meshPtr = CreateShared(meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const MeshCreateInformation& meshCreateInformation)
{
	SPtr<Mesh> meshPtr = CreateShared(meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	SPtr<Mesh> meshPtr = CreateShared(initialMeshData, meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType primitiveType)
{
	SPtr<Mesh> meshPtr = CreateShared(initialMeshData, usage, primitiveType);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

SPtr<Mesh> Mesh::CreateShared(const MeshCreateInformation& meshCreateInformation)
{
	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreateShared(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreateShared(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType primitiveType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.Usage = usage;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, initialMeshData->GetIndexCount(), primitiveType));

	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::CreateEmptyShared()
{
	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh());
	mesh->SetShared(mesh);

	return mesh;
}

namespace bs { namespace ct
{
Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
	: MeshBase(meshCreateInformation.VertexCount, meshCreateInformation.IndexCount, meshCreateInformation.SubMeshes), mVertexData(nullptr), mIndexBuffer(nullptr), mVertexDescription(meshCreateInformation.VertexDescription), mUsage(meshCreateInformation.Usage), mIndexType(meshCreateInformation.IndexType), mTempInitialMeshData(initialMeshData), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)

{}

Mesh::~Mesh()
{
	ASSERT_IF_NOT_RENDER_THREAD;

	mVertexData = nullptr;
	mIndexBuffer = nullptr;
	mVertexDescription = nullptr;
	mTempInitialMeshData = nullptr;
}

void Mesh::Initialize()
{
	ASSERT_IF_NOT_RENDER_THREAD;

	const bool isDynamic = (mUsage & MU_DYNAMIC) != 0;
	const GpuBufferFlags flags = isDynamic ? GpuBufferFlag::StoreOnCPUWithGPUAccess : GpuBufferFlag::StoreOnGPU;

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	GpuBufferInformation indexBufferCreateInformation;
	indexBufferCreateInformation.Type = GpuBufferType::Index;
	indexBufferCreateInformation.Flags = flags;
	indexBufferCreateInformation.Index.Type = mIndexType;
	indexBufferCreateInformation.Index.Count = mProperties.IndexCount;

	mIndexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

	mVertexData = B3DMakeShared<VertexData>();
	mVertexData->VertexCount = mProperties.VertexCount;
	mVertexData->VertexDescription = mVertexDescription;

	for(u32 i = 0; i <= mVertexDescription->GetLargestStreamIndex(); i++)
	{
		if(!mVertexDescription->HasStream(i))
			continue;

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = flags;
		vertexBufferCreateInformation.Vertex.ElementSize = mVertexData->VertexDescription->GetVertexStride(i);
		vertexBufferCreateInformation.Vertex.Count = mVertexData->VertexCount;

		SPtr<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);
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
	ASSERT_IF_NOT_RENDER_THREAD;

	return mVertexData;
}

SPtr<GpuBuffer> Mesh::GetIndexBuffer() const
{
	ASSERT_IF_NOT_RENDER_THREAD;

	return mIndexBuffer;
}

SPtr<VertexDescription> Mesh::GetVertexDescription() const
{
	ASSERT_IF_NOT_RENDER_THREAD;

	return mVertexDescription;
}

void Mesh::WriteData(const MeshData& meshData, bool discardEntireBuffer, bool performUpdateBounds, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	if(discardEntireBuffer)
	{
		if((mUsage & MU_STATIC) != 0)
		{
			B3D_LOG(Warning, Mesh, "Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
			discardEntireBuffer = false;
		}
	}
	else
	{
		if((mUsage & MU_DYNAMIC) != 0)
		{
			B3D_LOG(Warning, Mesh, "Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
			discardEntireBuffer = true;
		}
	}

	// Indices
	const GpuBufferInformation& indexBufferInformation = mIndexBuffer->GetInformation();

	B3D_ENSURE(indexBufferInformation.Type == GpuBufferType::Index);
	const u32 indexBufferIndexSize = bs::GpuBuffer::GetIndexSize(indexBufferInformation.Index.Type);

	u32 indicesSize = meshData.GetIndexBufferSize();
	u8* srcIdxData = meshData.GetIndexData();

	if(meshData.GetIndexElementSize() != indexBufferIndexSize)
	{
		B3D_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", indexBufferIndexSize, meshData.GetIndexElementSize());

		return;
	}

	if(indicesSize > mIndexBuffer->GetTotalSize())
	{
		indicesSize = mIndexBuffer->GetTotalSize();
		B3D_LOG(Error, Mesh, "Index buffer values are being written out of valid range.");
	}

	mIndexBuffer->WriteData(0, indicesSize, srcIdxData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL, commandBuffer);

	// Vertices
	for(u32 i = 0; i <= mVertexDescription->GetLargestStreamIndex(); i++)
	{
		if(!mVertexDescription->HasStream(i))
			continue;

		if(!meshData.GetVertexDescription()->HasStream(i))
			continue;

		// Ensure both have the same sized vertices
		u32 myVertSize = mVertexDescription->GetVertexStride(i);
		u32 otherVertSize = meshData.GetVertexDescription()->GetVertexStride(i);
		if(myVertSize != otherVertSize)
		{
			B3D_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
								"Needed: {1}. Got: {2}",
				   i, myVertSize, otherVertSize);

			continue;
		}

		SPtr<GpuBuffer> vertexBuffer = mVertexData->GetBuffer(i);

		u32 bufferSize = meshData.GetStreamSize(i);
		u8* srcVertBufferData = meshData.GetStreamData(i);

		if(bufferSize > vertexBuffer->GetTotalSize())
		{
			bufferSize = vertexBuffer->GetTotalSize();
			B3D_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being written out of valid range.", i);
		}

		vertexBuffer->WriteData(0, bufferSize, srcVertBufferData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL, commandBuffer);
	}

	if(performUpdateBounds)
		UpdateBounds(meshData);
}

void Mesh::ReadData(MeshData& meshData, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	const GpuBufferInformation& indexBufferInformation = mIndexBuffer->GetInformation();
	B3D_ENSURE(indexBufferInformation.Type == GpuBufferType::Index);

	IndexType indexType = IT_32BIT;
	if(mIndexBuffer)
		indexType = indexBufferInformation.Index.Type;

	const u32 indexBufferIndexSize = bs::GpuBuffer::GetIndexSize(indexType);

	if(mIndexBuffer)
	{
		if(meshData.GetIndexElementSize() != indexBufferIndexSize)
		{
			B3D_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", indexBufferIndexSize, meshData.GetIndexElementSize());
			return;
		}

		u32 idxElemSize = indexBufferIndexSize;

		u8* indices = nullptr;
		if(indexType == IT_16BIT)
			indices = (u8*)meshData.GetIndices16();
		else
			indices = (u8*)meshData.GetIndices32();

		u32 numIndicesToCopy = std::min(mProperties.IndexCount, meshData.GetIndexCount());

		u32 indicesSize = numIndicesToCopy * idxElemSize;
		if(indicesSize > meshData.GetIndexBufferSize())
		{
			B3D_LOG(Error, Mesh, "Provided buffer doesn't have enough space to store mesh indices.");
			return;
		}

		mIndexBuffer->ReadData(0, indicesSize, indices);
	}

	if(mVertexData)
	{
		auto vertexBuffers = mVertexData->GetBuffers();

		u32 streamIdx = 0;
		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			if(!meshData.GetVertexDescription()->HasStream(streamIdx))
				continue;

			SPtr<GpuBuffer> vertexBuffer = iter->second;

			const GpuBufferInformation& vertexBufferInformation = vertexBuffer->GetInformation();
			B3D_ENSURE(vertexBufferInformation.Type == GpuBufferType::Vertex);

			// Ensure both have the same sized vertices
			u32 myVertSize = mVertexDescription->GetVertexStride(streamIdx);
			u32 otherVertSize = meshData.GetVertexDescription()->GetVertexStride(streamIdx);
			if(myVertSize != otherVertSize)
			{
				B3D_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
									"Needed: {1}. Got: {2}",
					   streamIdx, myVertSize, otherVertSize);

				continue;
			}

			u32 numVerticesToCopy = meshData.GetVertexCount();
			u32 bufferSize = vertexBufferInformation.Vertex.ElementSize * numVerticesToCopy;

			if(bufferSize > vertexBuffer->GetTotalSize())
			{
				B3D_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being read out of valid range.", streamIdx);
				continue;
			}

			u8* dest = meshData.GetStreamData(streamIdx);
			vertexBuffer->ReadData(0, bufferSize, dest);

			streamIdx++;
		}
	}
}

void Mesh::UpdateBounds(const MeshData& meshData)
{
	mProperties.Bounds = meshData.CalculateBounds();

	// TODO - Sync this to main-thread possibly?
}

SPtr<Mesh> Mesh::Create(u32 vertexCount, u32 indexCount, const SPtr<VertexDescription>& vertexDescription, int usage, DrawOperationType primitiveType, IndexType indexType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = vertexCount;
	meshCreateInformation.IndexCount = indexCount;
	meshCreateInformation.VertexDescription = vertexDescription;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, indexCount, primitiveType));
	meshCreateInformation.Usage = usage;
	meshCreateInformation.IndexType = indexType;

	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const MeshCreateInformation& meshCreateInformation)
{
	SPtr<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, meshCreateInformation));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	MeshCreateInformation meshCreateInformationCopy = meshCreateInformation;
	meshCreateInformationCopy.VertexCount = initialMeshData->GetVertexCount();
	meshCreateInformationCopy.IndexCount = initialMeshData->GetIndexCount();
	meshCreateInformationCopy.VertexDescription = initialMeshData->GetVertexDescription();
	meshCreateInformationCopy.IndexType = initialMeshData->GetIndexType();

	SPtr<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformationCopy));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

SPtr<Mesh> Mesh::Create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = initialMeshData->GetVertexCount();
	meshCreateInformation.IndexCount = initialMeshData->GetIndexCount();
	meshCreateInformation.VertexDescription = initialMeshData->GetVertexDescription();
	meshCreateInformation.IndexType = initialMeshData->GetIndexType();
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, initialMeshData->GetIndexCount(), drawOp));
	meshCreateInformation.Usage = usage;

	SPtr<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}
}}
