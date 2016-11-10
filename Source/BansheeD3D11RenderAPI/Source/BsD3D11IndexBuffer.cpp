//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11Device.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	D3D11IndexBufferCore::D3D11IndexBufferCore(D3D11Device& device, const INDEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
		:IndexBufferCore(desc, deviceMask), mBuffer(nullptr), mDevice(device), mUsage(desc.usage)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX.");
	}

	D3D11IndexBufferCore::~D3D11IndexBufferCore()
	{
		if (mBuffer != nullptr)
			bs_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBufferCore::initialize()
	{
		mBuffer = bs_new<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_INDEX, mUsage, 1, mSize, std::ref(mDevice), false);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBufferCore::initialize();
	}

	void* D3D11IndexBufferCore::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options);
	}

	void D3D11IndexBufferCore::unmap()
	{
		mBuffer->unlock();
	}

	void D3D11IndexBufferCore::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, pDest);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBufferCore::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, pSource, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}
}