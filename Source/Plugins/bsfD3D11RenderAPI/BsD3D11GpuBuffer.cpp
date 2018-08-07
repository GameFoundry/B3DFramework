//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11GpuBufferView.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11HardwareBuffer.h"
#include "BsD3D11Device.h"
#include "BsD3D11Mappings.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"

namespace bs { namespace ct
{
	D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<D3D11HardwareBuffer>& underlyingBuffer)
		: GpuBuffer(desc, underlyingBuffer), mBuffer(underlyingBuffer.get())
	{ }

	D3D11GpuBuffer::~D3D11GpuBuffer()
	{ 
		if(mBuffer && !mExternalBuffer)
			bs_pool_delete(mBuffer);

		clearBufferViews();
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void D3D11GpuBuffer::initialize()
	{
		const GpuBufferProperties& props = getProperties();

		// Create a new buffer if not wrapping an external one
		if(!mBuffer)
		{
			D3D11HardwareBuffer::BufferType bufferType;
			D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());

			switch (props.getType())
			{
			case GBT_STANDARD:
				bufferType = D3D11HardwareBuffer::BT_STANDARD;
				break;
			case GBT_STRUCTURED:
				bufferType = D3D11HardwareBuffer::BT_STRUCTURED;
				break;
			case GBT_INDIRECTARGUMENT:
				bufferType = D3D11HardwareBuffer::BT_INDIRECTARGUMENT;
				break;
			default:
				BS_EXCEPT(InvalidParametersException, "Unsupported buffer type " + toString(props.getType()));
			}

			mBuffer = bs_pool_new<D3D11HardwareBuffer>(bufferType, props.getUsage(), props.getElementCount(),
				props.getElementSize(), d3d11rs->getPrimaryDevice(), false, false);
		}

		UINT32 usage = GVU_DEFAULT;
		if ((props.getUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
			usage |= GVU_RANDOMWRITE;

		// Keep a single view of the entire buffer, we don't support views of sub-sets (yet)
		mBufferView = requestView(this, 0, props.getElementCount(), (GpuViewUsage)usage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		GpuBuffer::initialize();
	}

	void* D3D11GpuBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options);
	}

	void D3D11GpuBuffer::unlock()
	{
		mBuffer->unlock();
	}

	void D3D11GpuBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);

		mBuffer->readData(offset, length, dest);
	}

	void D3D11GpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, 
		UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);

		mBuffer->writeData(offset, length, source, writeFlags);
	}

	void D3D11GpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		D3D11GpuBuffer* d3d11SrcBuffer = static_cast<D3D11GpuBuffer*>(&srcBuffer);

		mBuffer->copyData(*d3d11SrcBuffer->mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	ID3D11Buffer* D3D11GpuBuffer::getDX11Buffer() const
	{ 
		return mBuffer->getD3DBuffer(); 
	}

	GpuBufferView* D3D11GpuBuffer::requestView(D3D11GpuBuffer* buffer, UINT32 firstElement, UINT32 numElements, 
		GpuViewUsage usage)
	{
		const auto& props = buffer->getProperties();

		GPU_BUFFER_VIEW_DESC key;
		key.firstElement = firstElement;
		key.elementWidth = props.getElementSize();
		key.numElements = numElements;
		key.usage = usage;
		key.format = props.getFormat();
		key.useCounter = false;

		auto iterFind = buffer->mBufferViews.find(key);
		if (iterFind == buffer->mBufferViews.end())
		{
			GpuBufferView* newView = bs_new<GpuBufferView>();
			newView->initialize(buffer, key);
			buffer->mBufferViews[key] = bs_new<GpuBufferReference>(newView);

			iterFind = buffer->mBufferViews.find(key);
		}

		iterFind->second->refCount++;
		return iterFind->second->view;
	}

	void D3D11GpuBuffer::releaseView(GpuBufferView* view)
	{
		D3D11GpuBuffer* buffer = view->getBuffer();

		auto iterFind = buffer->mBufferViews.find(view->getDesc());
		if (iterFind == buffer->mBufferViews.end())
		{
			BS_EXCEPT(InternalErrorException, "Trying to release a buffer view that doesn't exist!");
		}

		iterFind->second->refCount--;

		if (iterFind->second->refCount == 0)
		{
			GpuBufferReference* toRemove = iterFind->second;

			buffer->mBufferViews.erase(iterFind);

			if (toRemove->view != nullptr)
				bs_delete(toRemove->view);

			bs_delete(toRemove);
		}
	}

	void D3D11GpuBuffer::clearBufferViews()
	{
		for (auto iter = mBufferViews.begin(); iter != mBufferViews.end(); ++iter)
		{
			if (iter->second->view != nullptr)
				bs_delete(iter->second->view);

			bs_delete(iter->second);
		}

		mBufferViews.clear();
	}

	ID3D11ShaderResourceView* D3D11GpuBuffer::getSRV() const
	{
		return mBufferView->getSRV();
	}

	ID3D11UnorderedAccessView* D3D11GpuBuffer::getUAV() const
	{
		return mBufferView->getUAV();
	}
}}