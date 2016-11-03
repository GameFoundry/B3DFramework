//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11GpuBufferView.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	size_t GpuBufferView::HashFunction::operator()(const GPU_BUFFER_VIEW_DESC& key) const
	{
		size_t seed = 0;
		hash_combine(seed, key.elementWidth);
		hash_combine(seed, key.firstElement);
		hash_combine(seed, key.numElements);
		hash_combine(seed, key.useCounter);
		hash_combine(seed, key.usage);
		hash_combine(seed, key.format);

		return seed;
	}

	bool GpuBufferView::EqualFunction::operator()
		(const GPU_BUFFER_VIEW_DESC& a, const GPU_BUFFER_VIEW_DESC& b) const
	{
		return a.elementWidth == b.elementWidth && a.firstElement == b.firstElement && a.numElements == b.numElements
			&& a.useCounter == b.useCounter && a.usage == b.usage && a.format == b.format;
	}

	GpuBufferView::GpuBufferView()
		:mSRV(nullptr), mUAV(nullptr)
	{

	}

	GpuBufferView::~GpuBufferView()
	{
		SAFE_RELEASE(mSRV);
		SAFE_RELEASE(mUAV);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_ResourceView);
	}

	void GpuBufferView::initialize(const SPtr<D3D11GpuBufferCore>& buffer, GPU_BUFFER_VIEW_DESC& desc)
	{
		mBuffer = buffer;
		mDesc = desc;

		D3D11GpuBufferCore* d3d11GpuBuffer = static_cast<D3D11GpuBufferCore*>(buffer.get());

		if ((desc.usage & GVU_DEFAULT) != 0)
			mSRV = createSRV(d3d11GpuBuffer, desc.firstElement, desc.elementWidth, desc.numElements);

		if((desc.usage & GVU_RANDOMWRITE) != 0)
			mUAV = createUAV(d3d11GpuBuffer, desc.firstElement, desc.numElements, desc.useCounter);

		if((desc.usage & GVU_RENDERTARGET) != 0 || (desc.usage & GVU_DEPTHSTENCIL) != 0)
		{
			BS_EXCEPT(NotImplementedException, "Invalid usage flags for a GPU buffer view.");
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_ResourceView);
	}

	ID3D11ShaderResourceView* GpuBufferView::createSRV(D3D11GpuBufferCore* buffer, UINT32 firstElement, UINT32 elementWidth, UINT32 numElements)
	{
		const GpuBufferProperties& props = buffer->getProperties();

		if (props.getType() == GBT_APPENDCONSUME)
			BS_EXCEPT(InvalidParametersException, "Cannot create ShaderResourceView for an append/consume buffer.");

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		if (props.getType() == GBT_STRUCTURED || props.getType() == GBT_STANDARD)
		{
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}
		else if (props.getType() == GBT_RAW)
		{
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			desc.BufferEx.FirstElement = firstElement;
			desc.BufferEx.NumElements = numElements;
		}
		else if (props.getType() == GBT_INDIRECTARGUMENT)
		{
			desc.Format = DXGI_FORMAT_R32_UINT;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.ElementOffset = firstElement * elementWidth;
			desc.Buffer.ElementWidth = elementWidth;
		}

		ID3D11ShaderResourceView* srv = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateShaderResourceView(buffer->getDX11Buffer(), &desc, &srv);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create ShaderResourceView: " + msg);
		}

		return srv;
	}

	ID3D11UnorderedAccessView* GpuBufferView::createUAV(D3D11GpuBufferCore* buffer, UINT32 firstElement, UINT32 numElements, bool useCounter)
	{
		const GpuBufferProperties& props = buffer->getProperties();

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

		if (props.getType() == GBT_STRUCTURED || props.getType() == GBT_STANDARD)
		{
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;

			if(useCounter)
				desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
			else
				desc.Buffer.Flags = 0;
		}
		else if (props.getType() == GBT_RAW)
		{
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}
		else if (props.getType() == GBT_INDIRECTARGUMENT)
		{
			desc.Format = DXGI_FORMAT_R32_UINT;
			desc.Buffer.Flags = 0;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}
		else if (props.getType() == GBT_APPENDCONSUME)
		{
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}

		ID3D11UnorderedAccessView* uav = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateUnorderedAccessView(buffer->getDX11Buffer(), &desc, &uav);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create UnorderedAccessView: " + msg);
		}

		return uav;
	}
}