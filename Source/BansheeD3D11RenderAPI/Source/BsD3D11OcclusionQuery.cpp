//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11OcclusionQuery.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsRenderStats.h"
#include "BsMath.h"

namespace BansheeEngine
{
	D3D11OcclusionQuery::D3D11OcclusionQuery(bool binary, UINT32 deviceIdx)
		:OcclusionQuery(binary), mContext(nullptr), mQuery(nullptr), mNumSamples(0), mFinalized(false), mQueryEndCalled(false)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();

		D3D11_QUERY_DESC queryDesc;
		queryDesc.Query = mBinary ? D3D11_QUERY_OCCLUSION_PREDICATE : D3D11_QUERY_OCCLUSION;
		queryDesc.MiscFlags = 0;

		HRESULT hr = device.getD3D11Device()->CreateQuery(&queryDesc, &mQuery);
		if (hr != S_OK)
			BS_EXCEPT(RenderingAPIException, "Failed to create an occlusion query.");

		mContext = device.getImmediateContext();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	D3D11OcclusionQuery::~D3D11OcclusionQuery()
	{
		if (mQuery != nullptr)
			mQuery->Release();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void D3D11OcclusionQuery::begin()
	{
		mContext->Begin(mQuery);

		mNumSamples = 0;
		mQueryEndCalled = false;
		
		setActive(true);
	}

	void D3D11OcclusionQuery::end()
	{
		mContext->End(mQuery);

		mQueryEndCalled = true;
		mFinalized = false;
	}

	bool D3D11OcclusionQuery::isReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mBinary)
		{
			BOOL anySamples = FALSE;
			return mContext->GetData(mQuery, &anySamples, sizeof(anySamples), 0) == S_OK;
		}
		else
		{
			UINT64 numSamples = 0;
			return mContext->GetData(mQuery, &numSamples, sizeof(numSamples), 0) == S_OK;
		}
	}

	UINT32 D3D11OcclusionQuery::getNumSamples()
	{
		if (!mFinalized && isReady())
		{
			finalize();
		}

		return mNumSamples;
	}

	void D3D11OcclusionQuery::finalize()
	{
		mFinalized = true;

		if (mBinary)
		{
			BOOL anySamples = FALSE;
			mContext->GetData(mQuery, &anySamples, sizeof(anySamples), 0);

			mNumSamples = anySamples == TRUE ? 1 : 0;
		}
		else
		{
			UINT64 numSamples = 0;
			mContext->GetData(mQuery, &numSamples, sizeof(numSamples), 0);

			mNumSamples = (UINT32)numSamples;
		}
	}
}