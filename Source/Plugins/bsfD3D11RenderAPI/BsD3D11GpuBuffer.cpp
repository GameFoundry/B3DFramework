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
	static void DeleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<D3D11HardwareBuffer*>(buffer));
	}

	D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11GpuBuffer::D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
		: GpuBuffer(desc, std::move(underlyingBuffer))
	{ }

	D3D11GpuBuffer::~D3D11GpuBuffer()
	{
		clearBufferViews();
	}

	void D3D11GpuBuffer::Initialize()
	{
		const GpuBufferProperties& props = getProperties();
		mBufferDeleter = &deleteBuffer;

		// Create a new buffer if not wrapping an external one
		if(!mBuffer)
		{
			D3D11HardwareBuffer::BufferType bufferType;
			D3D11RenderAPI* rapi = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());

			switch (props.GetType())
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
				BS_EXCEPT(InvalidParametersException, "Unsupported buffer type " + ToString(props.GetType()));
			}

			mBuffer = bs_pool_new<D3D11HardwareBuffer>(bufferType, props.GetUsage(), props.getElementCount(),
				props.GetElementSize(), rapi->GetPrimaryDevice(), false, false);
		}

		UINT32 usage = GVU_DEFAULT;
		if ((props.GetUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
			usage |= GVU_RANDOMWRITE;

		// Keep a single view of the entire buffer, we don't support views of sub-sets (yet)
		mBufferView = requestView(this, 0, props.GetElementCount(), (GpuViewUsage)usage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		GpuBuffer::initialize();
	}

	ID3D11Buffer* D3D11GpuBuffer::getDX11Buffer() const
	{
		return static_cast<D3D11HardwareBuffer*>(mBuffer)->GetD3DBuffer();
	}

	GpuBufferView* D3D11GpuBuffer::requestView(D3D11GpuBuffer* buffer, UINT32 firstElement, UINT32 numElements,
		GpuViewUsage usage)
	{
		const auto& props = buffer->GetProperties();

		GPU_BUFFER_VIEW_DESC key;
		key.firstElement = firstElement;
		key.elementWidth = props.GetElementSize();
		key.numElements = numElements;
		key.usage = usage;
		key.format = props.GetFormat();
		key.useCounter = false;

		auto iterFind = buffer->mBufferViews.find(key);
		if (iterFind == buffer->mBufferViews.end())
		{
			GpuBufferView* newView = bs_new<GpuBufferView>();
			newView->Initialize(buffer, key);
			buffer->mBufferViews[key] = bs_new<GpuBufferReference>(newView);

			iterFind = buffer->mBufferViews.find(key);
		}

		iterFind->second->refCount++;
		return iterFind->second->view;
	}

	void D3D11GpuBuffer::ReleaseView(GpuBufferView* view)
	{
		D3D11GpuBuffer* buffer = view->GetBuffer();

		auto iterFind = buffer->mBufferViews.find(view->GetDesc());
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

	void D3D11GpuBuffer::ClearBufferViews()
	{
		for (auto iter = mBufferViews.begin(); iter != mBufferViews.end(); ++iter)
		{
			if (iter->second->view != nullptr)
				bs_delete(iter->second->view);

			bs_delete(iter->second);
		}

		mBufferViews.Clear();
	}

	ID3D11ShaderResourceView* D3D11GpuBuffer::getSRV() const
	{
		return mBufferView->GetSRV();
	}

	ID3D11UnorderedAccessView* D3D11GpuBuffer::getUAV() const
	{
		return mBufferView->GetUAV();
	}
}}
