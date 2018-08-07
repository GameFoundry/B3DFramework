//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	D3D11IndexBuffer::D3D11IndexBuffer(D3D11Device& device, const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask), mDevice(device), mUsage(desc.usage)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX.");
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		if (mBuffer != nullptr)
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_INDEX, mUsage, 1, mSize, mDevice);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBuffer::initialize();
	}

	void* D3D11IndexBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
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

	void D3D11IndexBuffer::unmap()
	{
		mBuffer->unlock();
	}

	void D3D11IndexBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, pSource, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}
}}