//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuBuffer.h"

namespace bs
{
	void checkValidDesc(const VERTEX_BUFFER_DESC& desc)
	{
		if(desc.vertexSize == 0)
			BS_EXCEPT(InvalidParametersException, "Vertex buffer vertex size is not allowed to be zero.");

		if(desc.numVerts == 0)
			BS_EXCEPT(InvalidParametersException, "Vertex buffer vertex count is not allowed to be zero.");
	}

	VertexBufferProperties::VertexBufferProperties(UINT32 numVertices, UINT32 vertexSize)
		:mNumVertices(numVertices), mVertexSize(vertexSize)
	{ }

	VertexBuffer::VertexBuffer(const VERTEX_BUFFER_DESC& desc)
		: mProperties(desc.numVerts, desc.vertexSize), mUsage(desc.usage), mStreamOut(desc.streamOut)
	{
#if BS_DEBUG_MODE
		checkValidDesc(desc);
#endif
	}

	SPtr<ct::CoreObject> VertexBuffer::CreateCore() const
	{
		VERTEX_BUFFER_DESC desc;
		desc.vertexSize = mProperties.mVertexSize;
		desc.numVerts = mProperties.mNumVertices;
		desc.usage = mUsage;
		desc.streamOut = mStreamOut;

		return ct::HardwareBufferManager::Instance().CreateVertexBufferInternal(desc);
	}

	SPtr<ct::VertexBuffer> VertexBuffer::GetCore() const
	{
		return std::static_pointer_cast<ct::VertexBuffer>(mCoreSpecific);
	}

	SPtr<VertexBuffer> VertexBuffer::Create(const VERTEX_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::Instance().CreateVertexBuffer(desc);
	}

	namespace ct
	{
	VertexBuffer::VertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:HardwareBuffer(desc.vertexSize * desc.numVerts, desc.usage, deviceMask), mProperties(desc.numVerts, desc.vertexSize)
	{
#if BS_DEBUG_MODE
		checkValidDesc(desc);
#endif
	}

	VertexBuffer::~VertexBuffer()
	{
		if(mBuffer && !mSharedBuffer)
			mBufferDeleter(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexBuffer);
	}

	void VertexBuffer::Initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexBuffer);
		CoreObject::Initialize();
	}

	void* VertexBuffer::Map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
		}
#endif

		return mBuffer->Lock(offset, length, options, deviceIdx, queueIdx);
	}

	void VertexBuffer::Unmap()
	{
		mBuffer->Unlock();
	}

	void VertexBuffer::ReadData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
	}

	void VertexBuffer::WriteData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
		UINT32 queueIdx)
	{
		mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
	}

	void VertexBuffer::CopyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto& srcVertexBuffer = static_cast<VertexBuffer&>(srcBuffer);
		mBuffer->CopyData(*srcVertexBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	SPtr<GpuBuffer> VertexBuffer::GetLoadStore(GpuBufferType type, GpuBufferFormat format, UINT32 elementSize)
	{
		if((mUsage & GBU_LOADSTORE) != GBU_LOADSTORE)
			return nullptr;

		for(const auto& entry : mLoadStoreViews)
		{
			const GpuBufferProperties& props = entry->GetProperties();
			if(props.GetType() == type)
			{
				if(type == GBT_STANDARD && props.GetFormat() == format)
					return entry;

				if(type == GBT_STRUCTURED && props.GetElementSize() == elementSize)
					return entry;
			}
		}

		UINT32 elemSize = type == GBT_STANDARD ? bs::GpuBuffer::GetFormatSize(format) : elementSize;
		if((mBuffer->GetSize() % elemSize) != 0)
		{
			BS_LOG(Error, RenderBackend,
				"Size of the buffer isn't divisible by individual element size provided for the buffer view.");
			return nullptr;
		}

		GPU_BUFFER_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.usage = mUsage;
		desc.elementSize = elementSize;
		desc.elementCount = mBuffer->GetSize() / elemSize;

		if(!mSharedBuffer)
			mSharedBuffer = bs_shared_ptr(mBuffer, mBufferDeleter);

		SPtr<GpuBuffer> newView = GpuBuffer::Create(desc, mSharedBuffer);
		mLoadStoreViews.push_back(newView);
		
		return newView;
	}

	SPtr<VertexBuffer> VertexBuffer::Create(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::Instance().CreateVertexBuffer(desc, deviceMask);
	}
	}
}
