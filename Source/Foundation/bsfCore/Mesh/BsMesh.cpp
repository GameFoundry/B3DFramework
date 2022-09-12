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

namespace bs
{
	MESH_DESC MESH_DESC::DEFAULT = MESH_DESC();

	Mesh::Mesh(const MESH_DESC& desc)
		:MeshBase(desc.numVertices, desc.numIndices, desc.subMeshes), mVertexDesc(desc.vertexDesc), mUsage(desc.usage),
		mIndexType(desc.indexType), mSkeleton(desc.skeleton), mMorphShapes(desc.morphShapes)
	{

	}

	Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
		:MeshBase(initialMeshData->GetNumVertices(), initialMeshData->getNumIndices(), desc.subMeshes),
		mCPUData(initialMeshData), mVertexDesc(initialMeshData->GetVertexDesc()),
		mUsage(desc.usage), mIndexType(initialMeshData->GetIndexType()), mSkeleton(desc.skeleton),
		mMorphShapes(desc.morphShapes)
	{ }

	Mesh::Mesh()
		:MeshBase(0, 0, DOT_TRIANGLE_LIST)
	{ }

	AsyncOp Mesh::WriteData(const SPtr<MeshData>& data, bool discardEntireBuffer)
	{
		updateBounds(*data);
		updateCPUBuffer(0, *data);

		data->_lock();

		std::function<void(const SPtr<ct::Mesh>&, const SPtr<MeshData>&, bool, AsyncOp&)> func =
			[&](const SPtr<ct::Mesh>& mesh, const SPtr<MeshData>& _meshData, bool _discardEntireBuffer, AsyncOp& asyncOp)
		{
			mesh->WriteData(*_meshData, _discardEntireBuffer, false);
			_meshData->_unlock();
			asyncOp._completeOperation();

		};

		return GCoreThread().QueueReturnCommand(std::bind(func, getCore(),
			 data, discardEntireBuffer, std::placeholders::_1));
	}

	AsyncOp Mesh::ReadData(const SPtr<MeshData>& data)
	{
		data->_lock();

		std::function<void(const SPtr<ct::Mesh>&, const SPtr<MeshData>&, AsyncOp&)> func =
			[&](const SPtr<ct::Mesh>& mesh, const SPtr<MeshData>& _meshData, AsyncOp& asyncOp)
		{
			// Make sure any queued command start executing before reading
			ct::RenderAPI::instance().SubmitCommandBuffer(nullptr);

			mesh->ReadData(*_meshData);
			_meshData->_unlock();
			asyncOp._completeOperation();

		};

		return GCoreThread().QueueReturnCommand(std::bind(func, getCore(),
			data, std::placeholders::_1));
	}

	SPtr<MeshData> Mesh::AllocBuffer() const
	{
		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(mProperties.mNumVertices, mProperties.mNumIndices,
			mVertexDesc, mIndexType);

		return meshData;
	}

	void Mesh::Initialize()
	{
		if (mCPUData != nullptr)
			updateBounds(*mCPUData);

		MeshBase::initialize();

		if ((mUsage & MU_CPUCACHED) != 0 && mCPUData == nullptr)
			createCPUBuffer();
	}

	void Mesh::UpdateBounds(const MeshData& meshData)
	{
		mProperties.mBounds = meshData.CalculateBounds();
		markCoreDirty();
	}

	SPtr<ct::Mesh> Mesh::GetCore() const
	{
		return std::static_pointer_cast<ct::Mesh>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Mesh::CreateCore() const
	{
		MESH_DESC desc;
		desc.numVertices = mProperties.mNumVertices;
		desc.numIndices = mProperties.mNumIndices;
		desc.vertexDesc = mVertexDesc;
		desc.subMeshes = mProperties.mSubMeshes;
		desc.usage = mUsage;
		desc.indexType = mIndexType;
		desc.skeleton = mSkeleton;
		desc.morphShapes = mMorphShapes;

		ct::Mesh* obj = new (bs_alloc<ct::Mesh>()) ct::Mesh(mCPUData, desc, GDF_DEFAULT);

		SPtr<ct::CoreObject> meshCore = bs_shared_ptr<ct::Mesh>(obj);
		meshCore->_setThisPtr(meshCore);

		if ((mUsage & MU_CPUCACHED) == 0)
			mCPUData = nullptr;

		return meshCore;
	}

	void Mesh::UpdateCPUBuffer(UINT32 subresourceIdx, const MeshData& pixelData)
	{
		if ((mUsage & MU_CPUCACHED) == 0)
			return;

		if (subresourceIdx > 0)
		{
			BS_LOG(Error, Mesh, "Invalid subresource index: {0}. Supported range: 0 .. 1.", subresourceIdx);
			return;
		}

		if (pixelData.GetNumIndices() != mProperties.getNumIndices() ||
			pixelData.GetNumVertices() != mProperties.getNumVertices() ||
			pixelData.GetIndexType() != mIndexType ||
			pixelData.GetVertexDesc()->GetVertexStride() != mVertexDesc->getVertexStride())
		{
			BS_LOG(Error, Mesh, "Provided buffer is not of valid dimensions or format in order to update this mesh.");
			return;
		}

		if (mCPUData->GetSize() != pixelData.GetSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* dest = mCPUData->GetData();
		UINT8* src = pixelData.GetData();

		memcpy(dest, src, pixelData.GetSize());
	}

	void Mesh::CreateCPUBuffer()
	{
		mCPUData = allocBuffer();
	}

	HMesh Mesh::Dummy()
	{
		return MeshManager::Instance().GetDummyMesh();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* Mesh::getRTTIStatic()
	{
		return MeshRTTI::Instance();
	}

	RTTITypeBase* Mesh::getRTTI() const
	{
		return Mesh::GetRTTIStatic();
	}

	/************************************************************************/
	/* 								STATICS		                     		*/
	/************************************************************************/

	HMesh Mesh::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc,
		int usage, DrawOperationType drawOp, IndexType indexType)
	{
		MESH_DESC desc;
		desc.numVertices = numVertices;
		desc.numIndices = numIndices;
		desc.vertexDesc = vertexDesc;
		desc.usage = usage;
		desc.subMeshes.push_back(SubMesh(0, numIndices, drawOp));
		desc.indexType = indexType;

		SPtr<Mesh> meshPtr = _createPtr(desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::Create(const MESH_DESC& desc)
	{
		SPtr<Mesh> meshPtr = _createPtr(desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	{
		SPtr<Mesh> meshPtr = _createPtr(initialMeshData, desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::Create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp)
	{
		SPtr<Mesh> meshPtr = _createPtr(initialMeshData, usage, drawOp);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	SPtr<Mesh> Mesh::_createPtr(const MESH_DESC& desc)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(desc));
		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::_createPtr(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialMeshData, desc));
		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::_createPtr(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp)
	{
		MESH_DESC desc;
		desc.usage = usage;
		desc.subMeshes.push_back(SubMesh(0, initialMeshData->GetNumIndices(), drawOp));

		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialMeshData, desc));
		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::CreateEmpty()
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh());
		mesh->_setThisPtr(mesh);

		return mesh;
	}

	namespace ct
	{
	Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc, GpuDeviceFlags deviceMask)
		: MeshBase(desc.numVertices, desc.numIndices, desc.subMeshes), mVertexData(nullptr), mIndexBuffer(nullptr)
		, MVertexDesc(desc.vertexDesc), mUsage(desc.usage), mIndexType(desc.indexType), mDeviceMask(deviceMask)
		, MTempInitialMeshData(initialMeshData), mSkeleton(desc.skeleton), mMorphShapes(desc.morphShapes)

	{ }

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
		ibDesc.indexType = mIndexType;
		ibDesc.numIndices = mProperties.mNumIndices;
		ibDesc.usage = (GpuBufferUsage)usage;

		mIndexBuffer = IndexBuffer::create(ibDesc, mDeviceMask);

		mVertexData = bs_shared_ptr_new<VertexData>();
		mVertexData->vertexCount = mProperties.mNumVertices;
		mVertexData->vertexDeclaration = VertexDeclaration::create(mVertexDesc, mDeviceMask);

		for (UINT32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
		{
			if (!mVertexDesc->HasStream(i))
				continue;

			VERTEX_BUFFER_DESC vbDesc;
			vbDesc.vertexSize = mVertexData->vertexDeclaration->GetProperties().GetVertexSize(i);
			vbDesc.numVerts = mVertexData->vertexCount;
			vbDesc.usage = (GpuBufferUsage)usage;

			SPtr<VertexBuffer> vertexBuffer = VertexBuffer::create(vbDesc, mDeviceMask);
			mVertexData->SetBuffer(i, vertexBuffer);
		}

		// TODO Low priority - DX11 (and maybe OpenGL)? allow an optimization that allows you to set
		// buffer data upon buffer construction, instead of setting it in a second step like I do here
		if (mTempInitialMeshData != nullptr)
		{
			writeData(*mTempInitialMeshData, isDynamic);
			mTempInitialMeshData = nullptr;
		}

		MeshBase::initialize();
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

	void Mesh::WriteData(const MeshData& meshData, bool discardEntireBuffer, bool performUpdateBounds, UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (discardEntireBuffer)
		{
			if ((mUsage & MU_STATIC) != 0)
			{
				BS_LOG(Warning, Mesh, "Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
				discardEntireBuffer = false;
			}
		}
		else
		{
			if ((mUsage & MU_DYNAMIC) != 0)
			{
				BS_LOG(Warning, Mesh, "Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
				discardEntireBuffer = true;
			}
		}

		// Indices
		const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

		UINT32 indicesSize = meshData.GetIndexBufferSize();
		UINT8* srcIdxData = meshData.GetIndexData();

		if (meshData.GetIndexElementSize() != ibProps.getIndexSize())
		{
			BS_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}",
				ibProps.GetIndexSize(), meshData.getIndexElementSize());

			return;
		}

		if (indicesSize > mIndexBuffer->GetSize())
		{
			indicesSize = mIndexBuffer->GetSize();
			BS_LOG(Error, Mesh, "Index buffer values are being written out of valid range.");
		}

		mIndexBuffer->WriteData(0, indicesSize, srcIdxData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL, queueIdx);

		// Vertices
		for (UINT32 i = 0; i <= mVertexDesc->GetMaxStreamIdx(); i++)
		{
			if (!mVertexDesc->HasStream(i))
				continue;

			if (!meshData.GetVertexDesc()->HasStream(i))
				continue;

			// Ensure both have the same sized vertices
			UINT32 myVertSize = mVertexDesc->GetVertexStride(i);
			UINT32 otherVertSize = meshData.GetVertexDesc()->GetVertexStride(i);
			if (myVertSize != otherVertSize)
			{
				BS_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
					"Needed: {1}. Got: {2}", i, myVertSize, otherVertSize);

				continue;
			}

			SPtr<VertexBuffer> vertexBuffer = mVertexData->GetBuffer(i);

			UINT32 bufferSize = meshData.GetStreamSize(i);
			UINT8* srcVertBufferData = meshData.GetStreamData(i);

			if (bufferSize > vertexBuffer->GetSize())
			{
				bufferSize = vertexBuffer->GetSize();
				BS_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being written out of valid range.", i);
			}

			vertexBuffer->writeData(0, bufferSize, srcVertBufferData, discardEntireBuffer ? BWT_DISCARD : BWT_NORMAL,
				queueIdx);
		}

		if (performUpdateBounds)
			updateBounds(meshData);
	}

	void Mesh::ReadData(MeshData& meshData, UINT32 deviceIdx, UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		IndexType indexType = IT_32BIT;
		if (mIndexBuffer)
			indexType = mIndexBuffer->GetProperties().GetType();

		if (mIndexBuffer)
		{
			const IndexBufferProperties& ibProps = mIndexBuffer->GetProperties();

			if (meshData.GetIndexElementSize() != ibProps.getIndexSize())
			{
				BS_LOG(Error, Mesh, "Provided index size doesn't match meshes index size. Needed: {0}. Got: {1}",
					ibProps.GetIndexSize(), meshData.getIndexElementSize());
				return;
			}

			UINT8* idxData = static_cast<UINT8*>(mIndexBuffer->Lock(GBL_READ_ONLY, deviceIdx, queueIdx));
			UINT32 idxElemSize = ibProps.GetIndexSize();

			UINT8* indices = nullptr;

			if (indexType == IT_16BIT)
				indices = (UINT8*)meshData.GetIndices16();
			else
				indices = (UINT8*)meshData.GetIndices32();

			UINT32 numIndicesToCopy = std::min(mProperties.mNumIndices, meshData.GetNumIndices());

			UINT32 indicesSize = numIndicesToCopy * idxElemSize;
			if (indicesSize > meshData.GetIndexBufferSize())
			{
				BS_LOG(Error, Mesh, "Provided buffer doesn't have enough space to store mesh indices.");
				return;
			}

			memcpy(indices, idxData, numIndicesToCopy * idxElemSize);

			mIndexBuffer->Unlock();
		}

		if (mVertexData)
		{
			auto vertexBuffers = mVertexData->GetBuffers();

			UINT32 streamIdx = 0;
			for (auto iter = vertexBuffers.Begin(); iter != vertexBuffers.end(); ++iter)
			{
				if (!meshData.GetVertexDesc()->HasStream(streamIdx))
					continue;

				SPtr<VertexBuffer> vertexBuffer = iter->second;
				const VertexBufferProperties& vbProps = vertexBuffer->GetProperties();

				// Ensure both have the same sized vertices
				UINT32 myVertSize = mVertexDesc->GetVertexStride(streamIdx);
				UINT32 otherVertSize = meshData.GetVertexDesc()->GetVertexStride(streamIdx);
				if (myVertSize != otherVertSize)
				{
					BS_LOG(Error, Mesh, "Provided vertex size for stream {0} doesn't match meshes vertex size. "
						"Needed: {1}. Got: {2}", streamIdx, myVertSize, otherVertSize);

					continue;
				}

				UINT32 numVerticesToCopy = meshData.GetNumVertices();
				UINT32 bufferSize = vbProps.GetVertexSize() * numVerticesToCopy;

				if (bufferSize > vertexBuffer->GetSize())
				{
					BS_LOG(Error, Mesh, "Vertex buffer values for stream \"{0}\" are being read out of valid range.",
						streamIdx);
					continue;
				}

				UINT8* vertDataPtr = static_cast<UINT8*>(vertexBuffer->Lock(GBL_READ_ONLY, deviceIdx, queueIdx));

				UINT8* dest = meshData.GetStreamData(streamIdx);
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

	SPtr<Mesh> Mesh::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc,
		int usage, DrawOperationType drawOp, IndexType indexType, GpuDeviceFlags deviceMask)
	{
		MESH_DESC desc;
		desc.numVertices = numVertices;
		desc.numIndices = numIndices;
		desc.vertexDesc = vertexDesc;
		desc.subMeshes.push_back(SubMesh(0, numIndices, drawOp));
		desc.usage = usage;
		desc.indexType = indexType;

		SPtr<Mesh> mesh = bs_shared_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(nullptr, desc, deviceMask));
		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::Create(const MESH_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<Mesh> mesh = bs_shared_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(nullptr, desc, deviceMask));

		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::Create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc, GpuDeviceFlags deviceMask)
	{
		MESH_DESC descCopy = desc;
		descCopy.numVertices = initialMeshData->GetNumVertices();
		descCopy.numIndices = initialMeshData->GetNumIndices();
		descCopy.vertexDesc = initialMeshData->GetVertexDesc();
		descCopy.indexType = initialMeshData->GetIndexType();

		SPtr<Mesh> mesh =
			bs_shared_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialMeshData, descCopy, deviceMask));

		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::create(const SPtr<MeshData>& initialMeshData, int usage, DrawOperationType drawOp,
		GpuDeviceFlags deviceMask)
	{
		MESH_DESC desc;
		desc.numVertices = initialMeshData->GetNumVertices();
		desc.numIndices = initialMeshData->GetNumIndices();
		desc.vertexDesc = initialMeshData->GetVertexDesc();
		desc.indexType = initialMeshData->GetIndexType();
		desc.subMeshes.push_back(SubMesh(0, initialMeshData->GetNumIndices(), drawOp));
		desc.usage = usage;

		SPtr<Mesh> mesh =
			bs_shared_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialMeshData, desc, deviceMask));

		mesh->_setThisPtr(mesh);
		mesh->Initialize();

		return mesh;
	}
	}
}
