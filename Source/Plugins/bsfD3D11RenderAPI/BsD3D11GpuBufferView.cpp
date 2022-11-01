//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11GpuBufferView.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsD3D11Mappings.h"

using namespace bs;
using namespace bs::ct;

size_t GpuBufferView::HashFunction::operator()(const GPU_BUFFER_VIEW_DESC& key) const
{
	size_t seed = 0;
	B3DCombineHash(seed, key.ElementWidth);
	B3DCombineHash(seed, key.FirstElement);
	B3DCombineHash(seed, key.NumElements);
	B3DCombineHash(seed, key.UseCounter);
	B3DCombineHash(seed, key.Usage);
	B3DCombineHash(seed, key.Format);

	return seed;
}

bool GpuBufferView::EqualFunction::operator()(const GPU_BUFFER_VIEW_DESC& a, const GPU_BUFFER_VIEW_DESC& b) const
{
	return a.ElementWidth == b.ElementWidth && a.FirstElement == b.FirstElement && a.NumElements == b.NumElements && a.UseCounter == b.UseCounter && a.Usage == b.Usage && a.Format == b.Format;
}

GpuBufferView::~GpuBufferView()
{
	SAFE_RELEASE(mSRV);
	SAFE_RELEASE(mUAV);

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_ResourceView);
}

void GpuBufferView::Initialize(D3D11GpuBuffer* buffer, GPU_BUFFER_VIEW_DESC& desc)
{
	mBuffer = buffer;
	mDesc = desc;

	if((desc.Usage & GVU_DEFAULT) != 0)
		mSRV = CreateSrv(buffer, desc.FirstElement, desc.ElementWidth, desc.NumElements);

	if((desc.Usage & GVU_RANDOMWRITE) != 0)
		mUAV = CreateUav(buffer, desc.FirstElement, desc.NumElements, desc.UseCounter);

	if((desc.Usage & GVU_RENDERTARGET) != 0 || (desc.Usage & GVU_DEPTHSTENCIL) != 0)
	{
		BS_EXCEPT(NotImplementedException, "Invalid usage flags for a GPU buffer view.");
	}

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_ResourceView);
}

ID3D11ShaderResourceView* GpuBufferView::CreateSrv(D3D11GpuBuffer* buffer, u32 firstElement, u32 elementWidth, u32 numElements)
{
	const GpuBufferProperties& props = buffer->GetProperties();

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	if(props.GetType() == GBT_STANDARD)
	{
		desc.Format = D3D11Mappings::GetBf(props.GetFormat());
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
	}
	else if(props.GetType() == GBT_STRUCTURED)
	{
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
	}
	else if(props.GetType() == GBT_INDIRECTARGUMENT)
	{
		desc.Format = DXGI_FORMAT_R32_UINT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.ElementOffset = firstElement * elementWidth;
		desc.Buffer.ElementWidth = elementWidth;
	}

	ID3D11ShaderResourceView* srv = nullptr;

	D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::InstancePtr());
	HRESULT hr = d3d11rs->GetPrimaryDevice().GetD3D11Device()->CreateShaderResourceView(buffer->GetDX11Buffer(), &desc, &srv);

	if(FAILED(hr) || d3d11rs->GetPrimaryDevice().HasError())
	{
		String msg = d3d11rs->GetPrimaryDevice().GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Cannot create ShaderResourceView: " + msg);
	}

	return srv;
}

ID3D11UnorderedAccessView* GpuBufferView::CreateUav(D3D11GpuBuffer* buffer, u32 firstElement, u32 numElements, bool useCounter)
{
	const GpuBufferProperties& props = buffer->GetProperties();

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	if(props.GetType() == GBT_STANDARD)
	{
		desc.Format = D3D11Mappings::GetBf(props.GetFormat());
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;

		if(useCounter)
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
		else
			desc.Buffer.Flags = 0;
	}
	else if(props.GetType() == GBT_STRUCTURED)
	{
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;

		if(useCounter)
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
		else
			desc.Buffer.Flags = 0;
	}
	else if(props.GetType() == GBT_INDIRECTARGUMENT)
	{
		desc.Format = DXGI_FORMAT_R32_UINT;
		desc.Buffer.Flags = 0;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
	}

	ID3D11UnorderedAccessView* uav = nullptr;

	D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::InstancePtr());
	HRESULT hr = d3d11rs->GetPrimaryDevice().GetD3D11Device()->CreateUnorderedAccessView(buffer->GetDX11Buffer(), &desc, &uav);

	if(FAILED(hr) || d3d11rs->GetPrimaryDevice().HasError())
	{
		String msg = d3d11rs->GetPrimaryDevice().GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Cannot create UnorderedAccessView: " + msg);
	}

	return uav;
}
