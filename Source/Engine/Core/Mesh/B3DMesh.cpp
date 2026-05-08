//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/B3DMesh.h"

#include "B3DApplication.h"
#include "RTTI/B3DMeshRTTI.h"
#include "Mesh/B3DMeshData.h"
#include "Debug/B3DDebug.h"
#include "Managers/B3DMeshManager.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Threading/B3DAsyncOp.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Resources/B3DResources.h"

using namespace b3d;

const MeshCreateInformation MeshCreateInformation::kDefault = MeshCreateInformation();

Mesh::Mesh(const MeshCreateInformation& meshCreateInformation)
	: MeshBase(meshCreateInformation.VertexCount, meshCreateInformation.IndexCount, meshCreateInformation.SubMeshes), mVertexDescription(meshCreateInformation.VertexDescription), mFlags(meshCreateInformation.Flags), mIndexType(meshCreateInformation.IndexType), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)
{
}

Mesh::Mesh(const TShared<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
	: MeshBase(initialMeshData->GetVertexCount(), initialMeshData->GetIndexCount(), meshCreateInformation.SubMeshes), mCPUData(initialMeshData), mVertexDescription(initialMeshData->GetVertexDescription()), mFlags(meshCreateInformation.Flags), mIndexType(initialMeshData->GetIndexType()), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)
{}

Mesh::Mesh()
	: MeshBase(0, 0, DOT_TRIANGLE_LIST)
{}

TAsyncOp<void> Mesh::WriteData(const TShared<MeshData>& data, bool discardEntireBuffer)
{
	UpdateBounds(*data);
	UpdateCpuBuffer(0, *data);

	data->LockInternal();

	auto fnWriteMeshData = [&](const TShared<render::Mesh>& mesh, const TShared<MeshData>& meshData, bool shouldDiscardEntireBuffer, TAsyncOp<void>& asyncOp)
	{
		mesh->WriteData(*meshData, shouldDiscardEntireBuffer, false);
		meshData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnWriteMeshData = std::move(fnWriteMeshData), renderProxy = B3DGetRenderProxy(this), data, discardEntireBuffer, asyncOp]() mutable { fnWriteMeshData(renderProxy, data, discardEntireBuffer, asyncOp); }, "Mesh::WriteData", false, GetName());

	return asyncOp;
}

TAsyncOp<void> Mesh::ReadData(const TShared<MeshData>& data)
{
	data->LockInternal();

	auto fnReadMeshData = [&](const TShared<render::Mesh>& mesh, const TShared<MeshData>& meshData, TAsyncOp<void>& asyncOp)
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		mesh->ReadData(*meshData);
		meshData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnReadMeshData = std::move(fnReadMeshData), renderProxy = B3DGetRenderProxy(this), data, asyncOp]() mutable { fnReadMeshData(renderProxy, data, asyncOp); }, "Mesh::ReadData", false, GetName());

	return asyncOp;
}

TShared<MeshData> Mesh::AllocBuffer() const
{
	TShared<MeshData> meshData = B3DMakeShared<MeshData>(mProperties.VertexCount, mProperties.IndexCount, mVertexDescription, mIndexType);

	return meshData;
}

void Mesh::Initialize()
{
	if(mCPUData != nullptr)
		UpdateBounds(*mCPUData);

	MeshBase::Initialize();

	if(mFlags.IsSet(MeshFlag::KeepCPUCopy) && mCPUData == nullptr)
		CreateCpuBuffer();
}

void Mesh::UpdateBounds(const MeshData& meshData)
{
	mProperties.Bounds = meshData.CalculateBounds();
	MarkRenderProxyDataDirty();
}

TShared<render::RenderProxy> Mesh::CreateRenderProxy() const
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = mProperties.VertexCount;
	meshCreateInformation.IndexCount = mProperties.IndexCount;
	meshCreateInformation.VertexDescription = mVertexDescription;
	meshCreateInformation.SubMeshes = mProperties.SubMeshes;
	meshCreateInformation.Flags = mFlags;
	meshCreateInformation.IndexType = mIndexType;
	meshCreateInformation.Skeleton = mSkeleton;
	meshCreateInformation.MorphShapes = mMorphShapes;

	render::Mesh* renderProxy = new(B3DAllocate<render::Mesh>()) render::Mesh(mCPUData, meshCreateInformation);

	TShared<render::RenderProxy> renderProxyShared = B3DMakeSharedFromExisting<render::Mesh>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	if(!mFlags.IsSet(MeshFlag::KeepCPUCopy))
		mCPUData = nullptr;

	return renderProxyShared;
}

void Mesh::UpdateCpuBuffer(u32 subresourceIndex, const MeshData& pixelData)
{
	if(!mFlags.IsSet(MeshFlag::KeepCPUCopy))
		return;

	if(subresourceIndex > 0)
	{
		B3D_LOG(Error, LogMesh, "Invalid subresource index: {0}. Supported range: 0 .. 1.", subresourceIndex);
		return;
	}

	if(pixelData.GetIndexCount() != mProperties.IndexCount ||
	   pixelData.GetVertexCount() != mProperties.VertexCount ||
	   pixelData.GetIndexType() != mIndexType ||
	   pixelData.GetVertexDescription()->GetVertexStride() != mVertexDescription->GetVertexStride())
	{
		B3D_LOG(Error, LogMesh, "Provided buffer is not of valid dimensions or format in order to update this mesh.");
		return;
	}

	if(!B3D_ENSURE_LOG(mCPUData->GetSize() == pixelData.GetSize(), "Buffer sizes don't match."))
		return;

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

RTTIType* Mesh::GetRttiStatic()
{
	return MeshRTTI::Instance();
}

RTTIType* Mesh::GetRtti() const
{
	return Mesh::GetRttiStatic();
}

/************************************************************************/
/* 								STATICS		                     		*/
/************************************************************************/

HMesh Mesh::Create(u32 vertexCount, u32 indexCount, const TShared<VertexDescription>& vertexDescription, MeshFlags flags, DrawOperationType primitiveType, IndexType indexType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = vertexCount;
	meshCreateInformation.IndexCount = indexCount;
	meshCreateInformation.VertexDescription = vertexDescription;
	meshCreateInformation.Flags = flags;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, indexCount, primitiveType));
	meshCreateInformation.IndexType = indexType;

	TShared<Mesh> meshPtr = CreateShared(meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const MeshCreateInformation& meshCreateInformation)
{
	TShared<Mesh> meshPtr = CreateShared(meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const TShared<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	TShared<Mesh> meshPtr = CreateShared(initialMeshData, meshCreateInformation);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

HMesh Mesh::Create(const TShared<MeshData>& initialMeshData, MeshFlags flags, DrawOperationType primitiveType)
{
	TShared<Mesh> meshPtr = CreateShared(initialMeshData, flags, primitiveType);
	return B3DStaticResourceCast<Mesh>(GetResources().CreateResourceHandle(meshPtr));
}

TShared<Mesh> Mesh::CreateShared(const MeshCreateInformation& meshCreateInformation)
{
	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::CreateShared(const TShared<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::CreateShared(const TShared<MeshData>& initialMeshData, MeshFlags flags, DrawOperationType primitiveType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.Flags = flags;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, initialMeshData->GetIndexCount(), primitiveType));

	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::CreateEmptyShared()
{
	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh());
	mesh->SetShared(mesh);

	return mesh;
}

namespace b3d { namespace render
{
Mesh::Mesh(const TShared<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
	: MeshBase(meshCreateInformation.VertexCount, meshCreateInformation.IndexCount, meshCreateInformation.SubMeshes), mVertexData(nullptr), mIndexBuffer(nullptr), mVertexDescription(meshCreateInformation.VertexDescription), mFlags(meshCreateInformation.Flags), mIndexType(meshCreateInformation.IndexType), mTempInitialMeshData(initialMeshData), mSkeleton(meshCreateInformation.Skeleton), mMorphShapes(meshCreateInformation.MorphShapes)

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

	const bool isDynamic = mFlags.IsSet(MeshFlag::Dynamic);
	GpuBufferFlags flags = isDynamic ? GpuBufferFlag::StoreOnCPUWithGPUAccess : GpuBufferFlag::StoreOnGPU;
	if(mFlags.IsSet(MeshFlag::UnorderedAccess))
		flags |= GpuBufferFlag::AllowUnorderedAccessOnTheGPU;

	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	GpuBufferInformation indexBufferCreateInformation;
	indexBufferCreateInformation.Type = GpuBufferType::Index;
	indexBufferCreateInformation.Flags = flags;
	indexBufferCreateInformation.Index.Type = mIndexType;
	indexBufferCreateInformation.Index.Count = mProperties.IndexCount;

	mIndexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

	mVertexData = B3DMakeShared<VertexData>();
	mVertexData->VertexCount = mProperties.VertexCount;
	mVertexData->VertexDescription = mVertexDescription;

	for(u32 streamIndex = 0; streamIndex <= mVertexDescription->GetLargestStreamIndex(); streamIndex++)
	{
		if(!mVertexDescription->HasStream(streamIndex))
			continue;

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = flags;
		vertexBufferCreateInformation.Vertex.ElementSize = mVertexData->VertexDescription->GetVertexStride(streamIndex);
		vertexBufferCreateInformation.Vertex.Count = mVertexData->VertexCount;

		TShared<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);
		mVertexData->SetBuffer(streamIndex, vertexBuffer);
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

TShared<VertexData> Mesh::GetVertexData() const
{
	ASSERT_IF_NOT_RENDER_THREAD;

	return mVertexData;
}

TShared<GpuBuffer> Mesh::GetIndexBuffer() const
{
	ASSERT_IF_NOT_RENDER_THREAD;

	return mIndexBuffer;
}

TShared<VertexDescription> Mesh::GetVertexDescription() const
{
	ASSERT_IF_NOT_RENDER_THREAD;

	return mVertexDescription;
}

void Mesh::WriteData(const MeshData& meshData, bool discardEntireBuffer, bool performUpdateBounds, const TShared<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	if(discardEntireBuffer)
	{
		if(mFlags.IsSet(MeshFlag::Static))
		{
			B3D_LOG(Warning, LogMesh, "Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
			discardEntireBuffer = false;
		}
	}
	else
	{
		if(mFlags.IsSet(MeshFlag::Dynamic))
		{
			B3D_LOG(Warning, LogMesh, "Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
			discardEntireBuffer = true;
		}
	}

	// Indices
	const GpuBufferInformation& indexBufferInformation = mIndexBuffer->GetInformation();

	B3D_ENSURE(indexBufferInformation.Type == GpuBufferType::Index);
	const u32 indexBufferIndexSize = b3d::GpuBuffer::GetIndexSize(indexBufferInformation.Index.Type);

	u32 indicesSize = meshData.GetIndexBufferSize();
	u8* sourceIndexData = meshData.GetIndexData();

	if(meshData.GetIndexElementSize() != indexBufferIndexSize)
	{
		B3D_LOG(Error, LogMesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", indexBufferIndexSize, meshData.GetIndexElementSize());

		return;
	}

	if(indicesSize > mIndexBuffer->GetTotalSize())
	{
		indicesSize = mIndexBuffer->GetTotalSize();
		B3D_LOG(Error, LogMesh, "Index buffer values are being written out of valid range.");
	}

	GpuBufferUtility::Write(mIndexBuffer, 0, indicesSize, sourceIndexData, discardEntireBuffer ? GpuBufferWriteFlag::Discard : GpuBufferWriteFlag::Normal, commandBuffer);

	// Vertices
	for(u32 streamIndex = 0; streamIndex <= mVertexDescription->GetLargestStreamIndex(); streamIndex++)
	{
		if(!mVertexDescription->HasStream(streamIndex))
			continue;

		if(!meshData.GetVertexDescription()->HasStream(streamIndex))
			continue;

		// Ensure both have the same sized vertices
		u32 myVertexSize = mVertexDescription->GetVertexStride(streamIndex);
		u32 otherVertexSize = meshData.GetVertexDescription()->GetVertexStride(streamIndex);
		if(myVertexSize != otherVertexSize)
		{
			B3D_LOG(Error, LogMesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
								"Needed: {1}. Got: {2}",
				   streamIndex, myVertexSize, otherVertexSize);

			continue;
		}

		TShared<GpuBuffer> vertexBuffer = mVertexData->GetBuffer(streamIndex);

		u32 bufferSize = meshData.GetStreamSize(streamIndex);
		u8* sourceVertexBufferData = meshData.GetStreamData(streamIndex);

		if(bufferSize > vertexBuffer->GetTotalSize())
		{
			bufferSize = vertexBuffer->GetTotalSize();
			B3D_LOG(Error, LogMesh, "Vertex buffer values for stream \"{0}\" are being written out of valid range.", streamIndex);
		}

		GpuBufferUtility::Write(vertexBuffer, 0, bufferSize, sourceVertexBufferData, discardEntireBuffer ? GpuBufferWriteFlag::Discard : GpuBufferWriteFlag::Normal, commandBuffer);
	}

	if(performUpdateBounds)
		UpdateBounds(meshData);
}

void Mesh::ReadData(MeshData& meshData, const TShared<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	const GpuBufferInformation& indexBufferInformation = mIndexBuffer->GetInformation();
	B3D_ENSURE(indexBufferInformation.Type == GpuBufferType::Index);

	IndexType indexType = IT_32BIT;
	if(mIndexBuffer)
		indexType = indexBufferInformation.Index.Type;

	const u32 indexBufferIndexSize = b3d::GpuBuffer::GetIndexSize(indexType);

	if(mIndexBuffer)
	{
		if(meshData.GetIndexElementSize() != indexBufferIndexSize)
		{
			B3D_LOG(Error, LogMesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}", indexBufferIndexSize, meshData.GetIndexElementSize());
			return;
		}

		u32 indexElementSize = indexBufferIndexSize;

		u8* indices = nullptr;
		if(indexType == IT_16BIT)
			indices = (u8*)meshData.GetIndices16();
		else
			indices = (u8*)meshData.GetIndices32();

		u32 indexCountToCopy = std::min(mProperties.IndexCount, meshData.GetIndexCount());

		u32 indicesSize = indexCountToCopy * indexElementSize;
		if(indicesSize > meshData.GetIndexBufferSize())
		{
			B3D_LOG(Error, LogMesh, "Provided buffer doesn't have enough space to store mesh indices.");
			return;
		}

		GpuBufferUtility::Read(mIndexBuffer, 0, indicesSize, indices);
	}

	if(mVertexData)
	{
		auto vertexBuffers = mVertexData->GetBuffers();

		u32 streamIndex = 0;
		for(auto iterator = vertexBuffers.begin(); iterator != vertexBuffers.end(); ++iterator)
		{
			if(!meshData.GetVertexDescription()->HasStream(streamIndex))
				continue;

			TShared<GpuBuffer> vertexBuffer = iterator->second;

			const GpuBufferInformation& vertexBufferInformation = vertexBuffer->GetInformation();
			B3D_ENSURE(vertexBufferInformation.Type == GpuBufferType::Vertex);

			// Ensure both have the same sized vertices
			u32 myVertexSize = mVertexDescription->GetVertexStride(streamIndex);
			u32 otherVertexSize = meshData.GetVertexDescription()->GetVertexStride(streamIndex);
			if(myVertexSize != otherVertexSize)
			{
				B3D_LOG(Error, LogMesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
									"Needed: {1}. Got: {2}",
					   streamIndex, myVertexSize, otherVertexSize);

				continue;
			}

			u32 vertexCountToCopy = meshData.GetVertexCount();
			u32 bufferSize = vertexBufferInformation.Vertex.ElementSize * vertexCountToCopy;

			if(bufferSize > vertexBuffer->GetTotalSize())
			{
				B3D_LOG(Error, LogMesh, "Vertex buffer values for stream \"{0}\" are being read out of valid range.", streamIndex);
				continue;
			}

			u8* destination = meshData.GetStreamData(streamIndex);
			GpuBufferUtility::Read(vertexBuffer, 0, bufferSize, destination);

			streamIndex++;
		}
	}
}

void Mesh::UpdateBounds(const MeshData& meshData)
{
	mProperties.Bounds = meshData.CalculateBounds();

	// TODO - Sync this to main-thread possibly?
}

TShared<Mesh> Mesh::Create(u32 vertexCount, u32 indexCount, const TShared<VertexDescription>& vertexDescription, MeshFlags flags, DrawOperationType primitiveType, IndexType indexType)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = vertexCount;
	meshCreateInformation.IndexCount = indexCount;
	meshCreateInformation.VertexDescription = vertexDescription;
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, indexCount, primitiveType));
	meshCreateInformation.Flags = flags;
	meshCreateInformation.IndexType = indexType;

	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, meshCreateInformation));
	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::Create(const MeshCreateInformation& meshCreateInformation)
{
	TShared<Mesh> mesh = B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(nullptr, meshCreateInformation));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::Create(const TShared<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation)
{
	MeshCreateInformation meshCreateInformationCopy = meshCreateInformation;
	meshCreateInformationCopy.VertexCount = initialMeshData->GetVertexCount();
	meshCreateInformationCopy.IndexCount = initialMeshData->GetIndexCount();
	meshCreateInformationCopy.VertexDescription = initialMeshData->GetVertexDescription();
	meshCreateInformationCopy.IndexType = initialMeshData->GetIndexType();

	TShared<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformationCopy));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}

TShared<Mesh> Mesh::Create(const TShared<MeshData>& initialMeshData, MeshFlags flags, DrawOperationType drawOp)
{
	MeshCreateInformation meshCreateInformation;
	meshCreateInformation.VertexCount = initialMeshData->GetVertexCount();
	meshCreateInformation.IndexCount = initialMeshData->GetIndexCount();
	meshCreateInformation.VertexDescription = initialMeshData->GetVertexDescription();
	meshCreateInformation.IndexType = initialMeshData->GetIndexType();
	meshCreateInformation.SubMeshes.push_back(SubMesh(0, initialMeshData->GetIndexCount(), drawOp));
	meshCreateInformation.Flags = flags;

	TShared<Mesh> mesh =
		B3DMakeSharedFromExisting<Mesh>(new(B3DAllocate<Mesh>()) Mesh(initialMeshData, meshCreateInformation));

	mesh->SetShared(mesh);
	mesh->Initialize();

	return mesh;
}
}}
