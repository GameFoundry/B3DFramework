//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"

namespace bs
{
	SPtr<SamplerState> RenderStateManager::CreateSamplerState(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerState> state = _createSamplerStatePtr(desc);
		state->Initialize();

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		SPtr<DepthStencilState> state = _createDepthStencilStatePtr(desc);
		state->Initialize();

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		SPtr<RasterizerState> state = _createRasterizerStatePtr(desc);
		state->Initialize();

		return state;
	}

	SPtr<BlendState> RenderStateManager::CreateBlendState(const BLEND_STATE_DESC& desc) const
	{
		SPtr<BlendState> state = _createBlendStatePtr(desc);
		state->Initialize();

		return state;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GraphicsPipelineState> state = _createGraphicsPipelineState(desc);
		state->Initialize();

		return state;
	}

	SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineState(const SPtr<GpuProgram>& program) const
	{
		SPtr<ComputePipelineState> state = _createComputePipelineState(program);
		state->Initialize();

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

	SPtr<GraphicsPipelineState> RenderStateManager::_createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GraphicsPipelineState> pipelineState =
			bs_core_ptr<GraphicsPipelineState>(new (bs_alloc<GraphicsPipelineState>()) GraphicsPipelineState(desc));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineState> RenderStateManager::_createComputePipelineState(const SPtr<GpuProgram>& program) const
	{
		SPtr<ComputePipelineState> pipelineState =
			bs_core_ptr<ComputePipelineState>(new (bs_alloc<ComputePipelineState>()) ComputePipelineState(program));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	const SPtr<SamplerState>& RenderStateManager::GetDefaultSamplerState() const
	{
		if(mDefaultSamplerState == nullptr)
			mDefaultSamplerState = createSamplerState(SAMPLER_STATE_DESC());

		return mDefaultSamplerState;
	}

	const SPtr<BlendState>& RenderStateManager::GetDefaultBlendState() const
	{
		if(mDefaultBlendState == nullptr)
			mDefaultBlendState = createBlendState(BLEND_STATE_DESC());

		return mDefaultBlendState;
	}

	const SPtr<RasterizerState>& RenderStateManager::GetDefaultRasterizerState() const
	{
		if(mDefaultRasterizerState == nullptr)
			mDefaultRasterizerState = createRasterizerState(RASTERIZER_STATE_DESC());

		return mDefaultRasterizerState;
	}

	const SPtr<DepthStencilState>& RenderStateManager::GetDefaultDepthStencilState() const
	{
		if(mDefaultDepthStencilState == nullptr)
			mDefaultDepthStencilState = createDepthStencilState(DEPTH_STENCIL_STATE_DESC());

		return mDefaultDepthStencilState;
	}

	namespace ct
	{
	SPtr<SamplerState> RenderStateManager::createSamplerState(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> state = findCachedState(desc);
		if (state == nullptr)
		{
			state = createSamplerStateInternal(desc, deviceMask);
			state->Initialize();

			notifySamplerStateCreated(desc, state);
		}

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createDepthStencilStateInternal(desc, id);
			state->Initialize();

			CachedDepthStencilState CachedData(id);
			cachedData.state = state;

			notifyDepthStencilStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createRasterizerStateInternal(desc, id);
			state->Initialize();

			CachedRasterizerState CachedData(id);
			cachedData.state = state;

			notifyRasterizerStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<BlendState> RenderStateManager::CreateBlendState(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createBlendStateInternal(desc, id);
			state->Initialize();

			CachedBlendState CachedData(id);
			cachedData.state = state;

			notifyBlendStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GraphicsPipelineState> state = _createGraphicsPipelineState(desc, deviceMask);
		state->Initialize();

		return state;
	}

	SPtr<ComputePipelineState> RenderStateManager::createComputePipelineState(const SPtr<GpuProgram>& program,
																					  GpuDeviceFlags deviceMask) const
	{
		SPtr<ComputePipelineState> state = _createComputePipelineState(program, deviceMask);
		state->Initialize();

		return state;
	}

	SPtr<GpuPipelineParamInfo> RenderStateManager::createPipelineParamInfo(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineParamInfo> paramInfo = _createPipelineParamInfo(desc, deviceMask);
		paramInfo->Initialize();

		return paramInfo;
	}

	SPtr<SamplerState> RenderStateManager::_createSamplerState(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> state = findCachedState(desc);
		if (state == nullptr)
		{
			state = createSamplerStateInternal(desc, deviceMask);

			notifySamplerStateCreated(desc, state);
		}

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::_createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createDepthStencilStateInternal(desc, id);

			CachedDepthStencilState CachedData(id);
			cachedData.state = state;

			notifyDepthStencilStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::_createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createRasterizerStateInternal(desc, id);

			CachedRasterizerState CachedData(id);
			cachedData.state = state;

			notifyRasterizerStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<BlendState> RenderStateManager::_createBlendState(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createBlendStateInternal(desc, id);

			CachedBlendState CachedData(id);
			cachedData.state = state;

			notifyBlendStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::_createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GraphicsPipelineState> pipelineState =
			bs_shared_ptr<GraphicsPipelineState>(new (bs_alloc<GraphicsPipelineState>())
			GraphicsPipelineState(desc, deviceMask));

		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineState> RenderStateManager::_createComputePipelineState(const SPtr<GpuProgram>& program,
																					   GpuDeviceFlags deviceMask) const
	{
		SPtr<ComputePipelineState> pipelineState =
			bs_shared_ptr<ComputePipelineState>(new (bs_alloc<ComputePipelineState>())
			ComputePipelineState(program, deviceMask));

		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<GpuPipelineParamInfo> RenderStateManager::_createPipelineParamInfo(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineParamInfo> paramInfo =
			bs_shared_ptr<GpuPipelineParamInfo>(new (bs_alloc<GpuPipelineParamInfo>())
													GpuPipelineParamInfo(desc, deviceMask));

		paramInfo->_setThisPtr(paramInfo);

		return paramInfo;
	}

	void RenderStateManager::OnShutDown()
	{
		mDefaultBlendState = nullptr;
		mDefaultDepthStencilState = nullptr;
		mDefaultRasterizerState = nullptr;
		mDefaultSamplerState = nullptr;
	}

	const SPtr<SamplerState>& RenderStateManager::GetDefaultSamplerState() const
	{
		if (mDefaultSamplerState == nullptr)
			mDefaultSamplerState = createSamplerState(SAMPLER_STATE_DESC());

		return mDefaultSamplerState;
	}

	const SPtr<BlendState>& RenderStateManager::GetDefaultBlendState() const
	{
		if (mDefaultBlendState == nullptr)
			mDefaultBlendState = createBlendState(BLEND_STATE_DESC());

		return mDefaultBlendState;
	}

	const SPtr<RasterizerState>& RenderStateManager::GetDefaultRasterizerState() const
	{
		if (mDefaultRasterizerState == nullptr)
			mDefaultRasterizerState = createRasterizerState(RASTERIZER_STATE_DESC());

		return mDefaultRasterizerState;
	}

	const SPtr<DepthStencilState>& RenderStateManager::GetDefaultDepthStencilState() const
	{
		if (mDefaultDepthStencilState == nullptr)
			mDefaultDepthStencilState = createDepthStencilState(DEPTH_STENCIL_STATE_DESC());

		return mDefaultDepthStencilState;
	}

	void RenderStateManager::NotifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerState>& state) const
	{
		Lock Lock(mMutex);

		mCachedSamplerStates[desc] = state;
	}

	void RenderStateManager::NotifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const
	{
		Lock Lock(mMutex);

		mCachedBlendStates[desc] = state;
	}

	void RenderStateManager::NotifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const
	{
		Lock Lock(mMutex);

		mCachedRasterizerStates[desc] = state;
	}

	void RenderStateManager::NotifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const
	{
		Lock Lock(mMutex);

		mCachedDepthStencilStates[desc] = state;
	}

	void RenderStateManager::NotifySamplerStateDestroyed(const SAMPLER_STATE_DESC& desc) const
	{
		Lock Lock(mMutex);

		mCachedSamplerStates.erase(desc);
	}

	SPtr<SamplerState> RenderStateManager::FindCachedState(const SAMPLER_STATE_DESC& desc) const
	{
		Lock Lock(mMutex);

		auto iterFind = mCachedSamplerStates.find(desc);
		if (iterFind != mCachedSamplerStates.end())
			return iterFind->second.Lock();

		return nullptr;
	}

	SPtr<BlendState> RenderStateManager::FindCachedState(const BLEND_STATE_DESC& desc, UINT32& id) const
	{
		Lock Lock(mMutex);

		auto iterFind = mCachedBlendStates.find(desc);
		if (iterFind != mCachedBlendStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.Expired())
				return iterFind->second.state.Lock();

			return nullptr;
		}

		id = mNextBlendStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<RasterizerState> RenderStateManager::FindCachedState(const RASTERIZER_STATE_DESC& desc, UINT32& id) const
	{
		Lock Lock(mMutex);

		auto iterFind = mCachedRasterizerStates.find(desc);
		if (iterFind != mCachedRasterizerStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.Expired())
				return iterFind->second.state.Lock();

			return nullptr;
		}

		id = mNextRasterizerStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<DepthStencilState> RenderStateManager::FindCachedState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32& id) const
	{
		Lock Lock(mMutex);

		auto iterFind = mCachedDepthStencilStates.find(desc);
		if (iterFind != mCachedDepthStencilStates.end())
		{
			id = iterFind->second.id;

			if (!iterFind->second.state.Expired())
				return iterFind->second.state.Lock();

			return nullptr;
		}

		id = mNextDepthStencilStateId++;
		assert(id <= 0x3FF); // 10 bits maximum

		return nullptr;
	}

	SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> state =
			bs_shared_ptr<SamplerState>(new (bs_alloc<SamplerState>()) SamplerState(desc, deviceMask));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<DepthStencilState> state = bs_shared_ptr<DepthStencilState>(new (bs_alloc<DepthStencilState>()) DepthStencilState(desc, id));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::CreateRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<RasterizerState> state = bs_shared_ptr<RasterizerState>(new (bs_alloc<RasterizerState>()) RasterizerState(desc, id));
		state->_setThisPtr(state);

		return state;
	}

	SPtr<BlendState> RenderStateManager::CreateBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<BlendState> state = bs_shared_ptr<BlendState>(new (bs_alloc<BlendState>()) BlendState(desc, id));
		state->_setThisPtr(state);

		return state;
	}
	}
}
