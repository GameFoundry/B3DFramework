//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11EventQuery.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11CommandBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"

using namespace bs;
using namespace bs::ct;

D3D11EventQuery::D3D11EventQuery(u32 deviceIdx)
{
	B3D_ASSERT(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();

	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_EVENT;
	queryDesc.MiscFlags = 0;

	HRESULT hr = device.GetD3D11Device()->CreateQuery(&queryDesc, &mQuery);
	if(hr != S_OK)
	{
		B3D_EXCEPT(RenderingAPIException, "Failed to create an Event query.");
	}

	mContext = device.GetImmediateContext();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

D3D11EventQuery::~D3D11EventQuery()
{
	if(mQuery != nullptr)
	{
		mQuery->Release();
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void D3D11EventQuery::Begin(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		mContext->End(mQuery);
		SetActive(true);
	};

	if(cb == nullptr)
		execute();
	else
	{
		SPtr<D3D11CommandBuffer> d3d11cb = std::static_pointer_cast<D3D11CommandBuffer>(cb);
		d3d11cb->QueueCommand(execute);
	}
}

bool D3D11EventQuery::IsReady() const
{
	BOOL queryData;
	return mContext->GetData(mQuery, &queryData, sizeof(BOOL), 0) == S_OK;
}
