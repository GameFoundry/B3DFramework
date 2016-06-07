//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11GpuBufferView.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11HardwareBuffer.h"
#include "BsD3D11Device.h"
#include "BsD3D11Mappings.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	D3D11GpuBufferCore::D3D11GpuBufferCore(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, 
		GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
		: GpuBufferCore(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter), mBuffer(nullptr)
	{
		if (type != GBT_STANDARD)
			assert(format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(elementSize != 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	D3D11GpuBufferCore::~D3D11GpuBufferCore()
	{ 
		bs_delete(mBuffer);
		clearBufferViews();
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void D3D11GpuBufferCore::initialize()
	{
		D3D11HardwareBuffer::BufferType bufferType;
		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());

		const GpuBufferProperties& props = getProperties();

		switch (props.getType())
		{
		case GBT_STANDARD:
		case GBT_STRUCTURED:
			bufferType = D3D11HardwareBuffer::BT_STRUCTURED;
			break;
		case GBT_RAW:
			bufferType = D3D11HardwareBuffer::BT_RAW;
			break;
		case GBT_INDIRECTARGUMENT:
			bufferType = D3D11HardwareBuffer::BT_INDIRECTARGUMENT;
			break;
		case GBT_APPENDCONSUME:
			bufferType = D3D11HardwareBuffer::BT_APPENDCONSUME;
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Unsupported buffer type " + toString(props.getType()));
		}

		mBuffer = bs_new<D3D11HardwareBuffer>(bufferType, props.getUsage(), props.getElementCount(), props.getElementSize(),
			d3d11rs->getPrimaryDevice(), false, false, props.getRandomGpuWrite(), props.getUseCounter());

		SPtr<D3D11GpuBufferCore> thisPtr = std::static_pointer_cast<D3D11GpuBufferCore>(getThisPtr());
		UINT32 usage = GVU_DEFAULT;
		if (props.getRandomGpuWrite())
			usage |= GVU_RANDOMWRITE;

		// Keep a single view of the entire buffer, we don't support views of sub-sets (yet)
		mBufferView = static_cast<D3D11GpuBufferView*>(requestView(thisPtr, 0, props.getElementCount(), (GpuViewUsage)usage));

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		GpuBufferCore::initialize();
	}

	void* D3D11GpuBufferCore::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
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

	void D3D11GpuBufferCore::unlock()
	{
		mBuffer->unlock();
	}

	void D3D11GpuBufferCore::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);

		mBuffer->readData(offset, length, pDest);
	}

	void D3D11GpuBufferCore::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);

		mBuffer->writeData(offset, length, pSource, writeFlags);
	}

	void D3D11GpuBufferCore::copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{
		D3D11GpuBufferCore* d3d11SrcBuffer = static_cast<D3D11GpuBufferCore*>(&srcBuffer);

		mBuffer->copyData(*d3d11SrcBuffer->mBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}

	ID3D11Buffer* D3D11GpuBufferCore::getDX11Buffer() const
	{ 
		return mBuffer->getD3DBuffer(); 
	}

	GpuBufferView* D3D11GpuBufferCore::createView()
	{
		return bs_new<D3D11GpuBufferView>();
	}

	void D3D11GpuBufferCore::destroyView(GpuBufferView* view)
	{
		if(view != nullptr)
			bs_delete(view);
	}

	ID3D11ShaderResourceView* D3D11GpuBufferCore::getSRV() const
	{
		return mBufferView->getSRV();
	}

	ID3D11UnorderedAccessView* D3D11GpuBufferCore::getUAV() const
	{
		return mBufferView->getUAV();
	}
}