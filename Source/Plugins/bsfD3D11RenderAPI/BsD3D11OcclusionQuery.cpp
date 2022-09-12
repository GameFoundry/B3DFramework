//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11OcclusionQuery.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11CommandBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Math/BsMath.h"

namespace bs { namespace ct
{
	D3D11OcclusionQuery::D3D11OcclusionQuery(bool binary, UINT32 deviceIdx)
		:OcclusionQuery(binary)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on DirectX 11.");

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = rs->GetPrimaryDevice();

		D3D11_QUERY_DESC queryDesc;
		queryDesc.Query = mBinary ? D3D11_QUERY_OCCLUSION_PREDICATE : D3D11_QUERY_OCCLUSION;
		queryDesc.MiscFlags = 0;

		HRESULT hr = device.GetD3D11Device()->CreateQuery(&queryDesc, &mQuery);
		if (hr != S_OK)
			BS_EXCEPT(RenderingAPIException, "Failed to create an occlusion query.");

		mContext = device.GetImmediateContext();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	D3D11OcclusionQuery::~D3D11OcclusionQuery()
	{
		if (mQuery != nullptr)
			mQuery->Release();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void D3D11OcclusionQuery::Begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			mContext->Begin(mQuery);

			mNumSamples = 0;
			mQueryEndCalled = false;

			setActive(true);
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<D3D11CommandBuffer> d3d11CB = std::static_pointer_cast<D3D11CommandBuffer>(cb);
			d3d11CB->QueueCommand(execute);
		}
	}

	void D3D11OcclusionQuery::End(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			mContext->End(mQuery);

			mQueryEndCalled = true;
			mFinalized = false;
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<D3D11CommandBuffer> d3d11CB = std::static_pointer_cast<D3D11CommandBuffer>(cb);
			d3d11CB->QueueCommand(execute);
		}
	}

	bool D3D11OcclusionQuery::IsReady() const
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

	UINT32 D3D11OcclusionQuery::GetNumSamples()
	{
		if (!mFinalized && isReady())
		{
			finalize();
		}

		return mNumSamples;
	}

	void D3D11OcclusionQuery::Finalize()
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
}}
