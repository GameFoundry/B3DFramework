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

using namespace bs;
using namespace bs::ct;

static void deleteBuffer(HardwareBuffer* buffer)
{
	B3DPoolDelete(static_cast<D3D11HardwareBuffer*>(buffer));
}

D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(desc, deviceMask)
{
	assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
}

D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
	: GpuBuffer(desc, std::move(underlyingBuffer))
{}

D3D11GpuBuffer::~D3D11GpuBuffer()
{
	ClearBufferViews();
}

void D3D11GpuBuffer::Initialize()
{
	const GpuBufferProperties& props = GetProperties();
	mBufferDeleter = &deleteBuffer;

	// Create a new buffer if not wrapping an external one
	if(!mBuffer)
	{
		D3D11HardwareBuffer::BufferType bufferType;
		D3D11RenderAPI* rapi = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::InstancePtr());

		switch(props.GetType())
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
			BS_EXCEPT(InvalidParametersException, "Unsupported buffer type " + toString(props.GetType()));
		}

		mBuffer = B3DPoolNew<D3D11HardwareBuffer>(bufferType, props.GetUsage(), props.GetElementCount(), props.GetElementSize(), rapi->GetPrimaryDevice(), false, false);
	}

	u32 usage = GVU_DEFAULT;
	if((props.GetUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
		usage |= GVU_RANDOMWRITE;

	// Keep a single view of the entire buffer, we don't support views of sub-sets (yet)
	mBufferView = RequestView(this, 0, props.GetElementCount(), (GpuViewUsage)usage);

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

	GpuBuffer::Initialize();
}

ID3D11Buffer* D3D11GpuBuffer::GetDX11Buffer() const
{
	return static_cast<D3D11HardwareBuffer*>(mBuffer)->GetD3DBuffer();
}

GpuBufferView* D3D11GpuBuffer::RequestView(D3D11GpuBuffer* buffer, u32 firstElement, u32 numElements, GpuViewUsage usage)
{
	const auto& props = buffer->GetProperties();

	GPU_BUFFER_VIEW_DESC key;
	key.FirstElement = firstElement;
	key.ElementWidth = props.GetElementSize();
	key.NumElements = numElements;
	key.Usage = usage;
	key.Format = props.GetFormat();
	key.UseCounter = false;

	auto iterFind = buffer->mBufferViews.find(key);
	if(iterFind == buffer->mBufferViews.end())
	{
		GpuBufferView* newView = B3DNew<GpuBufferView>();
		newView->Initialize(buffer, key);
		buffer->mBufferViews[key] = B3DNew<GpuBufferReference>(newView);

		iterFind = buffer->mBufferViews.find(key);
	}

	iterFind->second->RefCount++;
	return iterFind->second->View;
}

void D3D11GpuBuffer::ReleaseView(GpuBufferView* view)
{
	D3D11GpuBuffer* buffer = view->GetBuffer();

	auto iterFind = buffer->mBufferViews.find(view->GetDesc());
	if(iterFind == buffer->mBufferViews.end())
	{
		BS_EXCEPT(InternalErrorException, "Trying to release a buffer view that doesn't exist!");
	}

	iterFind->second->RefCount--;

	if(iterFind->second->RefCount == 0)
	{
		GpuBufferReference* toRemove = iterFind->second;

		buffer->mBufferViews.erase(iterFind);

		if(toRemove->View != nullptr)
			B3DDelete(toRemove->View);

		B3DDelete(toRemove);
	}
}

void D3D11GpuBuffer::ClearBufferViews()
{
	for(auto iter = mBufferViews.begin(); iter != mBufferViews.end(); ++iter)
	{
		if(iter->second->View != nullptr)
			B3DDelete(iter->second->View);

		B3DDelete(iter->second);
	}

	mBufferViews.clear();
}

ID3D11ShaderResourceView* D3D11GpuBuffer::GetSrv() const
{
	return mBufferView->GetSrv();
}

ID3D11UnorderedAccessView* D3D11GpuBuffer::GetUav() const
{
	return mBufferView->GetUav();
}
