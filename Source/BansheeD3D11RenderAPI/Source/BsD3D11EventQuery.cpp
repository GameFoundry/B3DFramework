//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11EventQuery.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11CommandBuffer.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace bs
{
	D3D11EventQuery::D3D11EventQuery(UINT32 deviceIdx)
		:mQuery(nullptr)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();

		D3D11_QUERY_DESC queryDesc;
		queryDesc.Query = D3D11_QUERY_EVENT;
		queryDesc.MiscFlags = 0;

		HRESULT hr = device.getD3D11Device()->CreateQuery(&queryDesc, &mQuery);
		if(hr != S_OK)
		{
			BS_EXCEPT(RenderingAPIException, "Failed to create an Event query.");
		}

		mContext = device.getImmediateContext();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	D3D11EventQuery::~D3D11EventQuery()
	{
		if(mQuery != nullptr)
		{
			mQuery->Release();
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void D3D11EventQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			mContext->End(mQuery);
			setActive(true);
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<D3D11CommandBuffer> d3d11cb = std::static_pointer_cast<D3D11CommandBuffer>(cb);
			d3d11cb->queueCommand(execute);
		}
	}

	bool D3D11EventQuery::isReady() const
	{
		BOOL queryData;
		return mContext->GetData(mQuery, &queryData, sizeof(BOOL), 0) == S_OK;
	}
}