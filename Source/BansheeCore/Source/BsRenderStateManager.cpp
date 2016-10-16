//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderStateManager.h"
#include "BsSamplerState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsBlendState.h"

namespace BansheeEngine
{
	SPtr<SamplerState> RenderStateManager::createSamplerState(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerState> state = _createSamplerStatePtr(desc);
		state->initialize();

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		SPtr<DepthStencilState> state = _createDepthStencilStatePtr(desc);
		state->initialize();

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		SPtr<RasterizerState> state = _createRasterizerStatePtr(desc);
		state->initialize();

		return state;
	}

	SPtr<BlendState> RenderStateManager::createBlendState(const BLEND_STATE_DESC& desc) const
	{
		SPtr<BlendState> state = _createBlendStatePtr(desc);
		state->initialize();

		return state;
	}

	SPtr<GpuPipelineState> RenderStateManager::createPipelineState(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GpuPipelineState> state = _createPipelineState(desc);
		state->initialize();

		return state;
	}

	SPtr<SamplerState> RenderStateManager::_createSamplerStatePtr(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerState> samplerState = bs_core_ptr<SamplerState>(new (bs_alloc<SamplerState>()) SamplerState(desc));
		samplerState->_setThisPtr(samplerState);

		return samplerState;
	}

	SPtr<DepthStencilState> RenderStateManager::_createDepthStencilStatePtr(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		SPtr<DepthStencilState> depthStencilState = bs_core_ptr<DepthStencilState>(new (bs_alloc<DepthStencilState>()) DepthStencilState(desc));
		depthStencilState->_setThisPtr(depthStencilState);

		return depthStencilState;
	}

	SPtr<RasterizerState> RenderStateManager::_createRasterizerStatePtr(const RASTERIZER_STATE_DESC& desc) const
	{
		SPtr<RasterizerState> rasterizerState = bs_core_ptr<RasterizerState>(new (bs_alloc<RasterizerState>()) RasterizerState(desc));
		rasterizerState->_setThisPtr(rasterizerState);

		return rasterizerState;
	}

	SPtr<BlendState> RenderStateManager::_createBlendStatePtr(const BLEND_STATE_DESC& desc) const
	{
		SPtr<BlendState> blendState = bs_core_ptr<BlendState>(new (bs_alloc<BlendState>()) BlendState(desc));
		blendState->_setThisPtr(blendState);

		return blendState;
	}

	SPtr<GpuPipelineState> RenderStateManager::_createPipelineState(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GpuPipelineState> pipelineState = 
			bs_core_ptr<GpuPipelineState>(new (bs_alloc<GpuPipelineState>()) GpuPipelineState(desc));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	const SPtr<SamplerState>& RenderStateManager::getDefaultSamplerState() const 
	{ 
		if(mDefaultSamplerState == nullptr)
			mDefaultSamplerState = createSamplerState(SAMPLER_STATE_DESC());

		return mDefaultSamplerState; 
	}

	const SPtr<BlendState>& RenderStateManager::getDefaultBlendState() const 
	{ 
		if(mDefaultBlendState == nullptr)
			mDefaultBlendState = createBlendState(BLEND_STATE_DESC());

		return mDefaultBlendState; 
	}

	const SPtr<RasterizerState>& RenderStateManager::getDefaultRasterizerState() const 
	{ 
		if(mDefaultRasterizerState == nullptr)
			mDefaultRasterizerState = createRasterizerState(RASTERIZER_STATE_DESC());

		return mDefaultRasterizerState; 
	}

	const SPtr<DepthStencilState>& RenderStateManager::getDefaultDepthStencilState() const 
	{ 
		if(mDefaultDepthStencilState == nullptr)
			mDefaultDepthStencilState = createDepthStencilState(DEPTH_STENCIL_STATE_DESC());

		return mDefaultDepthStencilState; 
	}

	RenderStateCoreManager::RenderStateCoreManager()
		:mNextBlendStateId(0), mNextRasterizerStateId(0), mNextDepthStencilStateId(0)
	{
		
	}

	SPtr<SamplerStateCore> RenderStateCoreManager::createSamplerState(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerStateCore> state = findCachedState(desc);
		if (state == nullptr)
		{
			state = createSamplerStateInternal(desc);
			state->initialize();

			notifySamplerStateCreated(desc, state);
		}

		return state;
	}

	SPtr<DepthStencilStateCore> RenderStateCoreManager::createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createDepthStencilStateInternal(desc, id);
			state->initialize();

			CachedDepthStencilState cachedData(id);
			cachedData.state = state;

			notifyDepthStencilStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<RasterizerStateCore> RenderStateCoreManager::createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createRasterizerStateInternal(desc, id);
			state->initialize();

			CachedRasterizerState cachedData(id);
			cachedData.state = state;

			notifyRasterizerStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<BlendStateCore> RenderStateCoreManager::createBlendState(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createBlendStateInternal(desc, id);
			state->initialize();

			CachedBlendState cachedData(id);
			cachedData.state = state;

			notifyBlendStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<GpuPipelineStateCore> RenderStateCoreManager::createPipelineState(const PIPELINE_STATE_CORE_DESC& desc, 
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineStateCore> state = _createPipelineState(desc, deviceMask);
		state->initialize();

		return state;
	}

	SPtr<SamplerStateCore> RenderStateCoreManager::_createSamplerState(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerStateCore> state = findCachedState(desc);
		if (state == nullptr)
		{
			state = createSamplerStateInternal(desc);

			notifySamplerStateCreated(desc, state);
		}

		return state;
	}

	SPtr<DepthStencilStateCore> RenderStateCoreManager::_createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createDepthStencilStateInternal(desc, id);

			CachedDepthStencilState cachedData(id);
			cachedData.state = state;

			notifyDepthStencilStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<RasterizerStateCore> RenderStateCoreManager::_createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createRasterizerStateInternal(desc, id);

			CachedRasterizerState cachedData(id);
			cachedData.state = state;

			notifyRasterizerStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<BlendStateCore> RenderStateCoreManager::_createBlendState(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendStateCore> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createBlendStateInternal(desc, id);

			CachedBlendState cachedData(id);
			cachedData.state = state;

			notifyBlendStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<GpuPipelineStateCore> RenderStateCoreManager::_createPipelineState(const PIPELINE_STATE_CORE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineStateCore> pipelineState =
			bs_shared_ptr<GpuPipelineStateCore>(new (bs_alloc<GpuPipelineStateCore>()) GpuPipelineStateCore(desc, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	void RenderStateCoreManager::onShutDown()
	{
		mDefaultBlendState = nullptr;
		mDefaultDepthStencilState = nullptr;
		mDefaultRasterizerState = nullptr;
		mDefaultSamplerState = nullptr;
	}

	const SPtr<SamplerStateCore>& RenderStateCoreManager::getDefaultSamplerState() const
	{
		if (mDefaultSamplerState == nullptr)
			mDefaultSamplerState = createSamplerState(SAMPLER_STATE_DESC());

		return mDefaultSamplerState;
	}

	const SPtr<BlendStateCore>& RenderStateCoreManager::getDefaultBlendState() const
	{
		if (mDefaultBlendState == nullptr)
			mDefaultBlendState = createBlendState(BLEND_STATE_DESC());

		return mDefaultBlendState;
	}

	const SPtr<RasterizerStateCore>& RenderStateCoreManager::getDefaultRasterizerState() const
	{
		if (mDefaultRasterizerState == nullptr)
			mDefaultRasterizerState = createRasterizerState(RASTERIZER_STATE_DESC());

		return mDefaultRasterizerState;
	}

	const SPtr<DepthStencilStateCore>& RenderStateCoreManager::getDefaultDepthStencilState() const
	{
		if (mDefaultDepthStencilState == nullptr)
			mDefaultDepthStencilState = createDepthStencilState(DEPTH_STENCIL_STATE_DESC());

		return mDefaultDepthStencilState;
	}

	void RenderStateCoreManager::notifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerStateCore>& state) const
	{
		Lock lock(mMutex);

		mCachedSamplerStates[desc] = state;
	}

	void RenderStateCoreManager::notifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const
	{
		Lock lock(mMutex);

		mCachedBlendStates[desc] = state;
	}

	void RenderStateCoreManager::notifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const
	{
		Lock lock(mMutex);

		mCachedRasterizerStates[desc] = state;
	}

	void RenderStateCoreManager::notifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const
	{
		Lock lock(mMutex);

		mCachedDepthStencilStates[desc] = state;
	}

	void RenderStateCoreManager::notifySamplerStateDestroyed(const SAMPLER_STATE_DESC& desc) const
	{
		Lock lock(mMutex);

		mCachedSamplerStates.erase(desc);
	}

	SPtr<SamplerStateCore> RenderStateCoreManager::findCachedState(const SAMPLER_STATE_DESC& desc) const
	{
		Lock lock(mMutex);

		auto iterFind = mCachedSamplerStates.find(desc);
		if (iterFind != mCachedSamplerStates.end())
			return iterFind->second.lock();

		return nullptr;
	}

	SPtr<BlendStateCore> RenderStateCoreManager::findCachedState(const BLEND_STATE_DESC& desc, UINT32& id) const
	{
		Lock lock(mMutex);

		auto iterFind = mCachedBlendStates.find(desc);
		if (iterFind != mCachedBlendStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.expired())
				return iterFind->second.state.lock();

			return nullptr;
		}

		id = mNextBlendStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<RasterizerStateCore> RenderStateCoreManager::findCachedState(const RASTERIZER_STATE_DESC& desc, UINT32& id) const
	{
		Lock lock(mMutex);

		auto iterFind = mCachedRasterizerStates.find(desc);
		if (iterFind != mCachedRasterizerStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.expired())
				return iterFind->second.state.lock();

			return nullptr;
		}

		id = mNextRasterizerStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<DepthStencilStateCore> RenderStateCoreManager::findCachedState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32& id) const
	{
		Lock lock(mMutex);

		auto iterFind = mCachedDepthStencilStates.find(desc);
		if (iterFind != mCachedDepthStencilStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.expired())
				return iterFind->second.state.lock();

			return nullptr;
		}

		id = mNextDepthStencilStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<SamplerStateCore> RenderStateCoreManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerStateCore> state = bs_shared_ptr<SamplerStateCore>(new (bs_alloc<SamplerStateCore>()) SamplerStateCore(desc));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<DepthStencilStateCore> RenderStateCoreManager::createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<DepthStencilStateCore> state = bs_shared_ptr<DepthStencilStateCore>(new (bs_alloc<DepthStencilStateCore>()) DepthStencilStateCore(desc, id));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<RasterizerStateCore> RenderStateCoreManager::createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<RasterizerStateCore> state = bs_shared_ptr<RasterizerStateCore>(new (bs_alloc<RasterizerStateCore>()) RasterizerStateCore(desc, id));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<BlendStateCore> RenderStateCoreManager::createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<BlendStateCore> state = bs_shared_ptr<BlendStateCore>(new (bs_alloc<BlendStateCore>()) BlendStateCore(desc, id));
		state->_setThisPtr(state);

		return state;
	}
}