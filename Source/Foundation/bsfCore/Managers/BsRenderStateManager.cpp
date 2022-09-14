//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"

namespace bs
{
	SPtr<SamplerState> RenderStateManager::createSamplerState(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerState> state = CreateSamplerStatePtrInternal(desc);
		state->initialize();

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		SPtr<DepthStencilState> state = CreateDepthStencilStatePtrInternal(desc);
		state->initialize();

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		SPtr<RasterizerState> state = CreateRasterizerStatePtrInternal(desc);
		state->initialize();

		return state;
	}

	SPtr<BlendState> RenderStateManager::createBlendState(const BLEND_STATE_DESC& desc) const
	{
		SPtr<BlendState> state = CreateBlendStatePtrInternal(desc);
		state->initialize();

		return state;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GraphicsPipelineState> state = CreateGraphicsPipelineStateInternal(desc);
		state->initialize();

		return state;
	}

	SPtr<ComputePipelineState> RenderStateManager::createComputePipelineState(const SPtr<GpuProgram>& program) const
	{
		SPtr<ComputePipelineState> state = CreateComputePipelineStateInternal(program);
		state->initialize();

		return state;
	}

	SPtr<SamplerState> RenderStateManager::CreateSamplerStatePtrInternal(const SAMPLER_STATE_DESC& desc) const
	{
		SPtr<SamplerState> samplerState = bs_core_ptr<SamplerState>(new (bs_alloc<SamplerState>()) SamplerState(desc));
		samplerState->SetThisPtrInternal(samplerState);

		return samplerState;
	}

	SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStatePtrInternal(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		SPtr<DepthStencilState> depthStencilState = bs_core_ptr<DepthStencilState>(new (bs_alloc<DepthStencilState>()) DepthStencilState(desc));
		depthStencilState->SetThisPtrInternal(depthStencilState);

		return depthStencilState;
	}

	SPtr<RasterizerState> RenderStateManager::CreateRasterizerStatePtrInternal(const RASTERIZER_STATE_DESC& desc) const
	{
		SPtr<RasterizerState> rasterizerState = bs_core_ptr<RasterizerState>(new (bs_alloc<RasterizerState>()) RasterizerState(desc));
		rasterizerState->SetThisPtrInternal(rasterizerState);

		return rasterizerState;
	}

	SPtr<BlendState> RenderStateManager::CreateBlendStatePtrInternal(const BLEND_STATE_DESC& desc) const
	{
		SPtr<BlendState> blendState = bs_core_ptr<BlendState>(new (bs_alloc<BlendState>()) BlendState(desc));
		blendState->SetThisPtrInternal(blendState);

		return blendState;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc) const
	{
		SPtr<GraphicsPipelineState> pipelineState =
			bs_core_ptr<GraphicsPipelineState>(new (bs_alloc<GraphicsPipelineState>()) GraphicsPipelineState(desc));
		pipelineState->SetThisPtrInternal(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program) const
	{
		SPtr<ComputePipelineState> pipelineState =
			bs_core_ptr<ComputePipelineState>(new (bs_alloc<ComputePipelineState>()) ComputePipelineState(program));
		pipelineState->SetThisPtrInternal(pipelineState);

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

	namespace ct
	{
	SPtr<SamplerState> RenderStateManager::createSamplerState(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> state = findCachedState(desc);
		if (state == nullptr)
		{
			state = createSamplerStateInternal(desc, deviceMask);
			state->initialize();

			notifySamplerStateCreated(desc, state);
		}

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilState> state = findCachedState(desc, id);
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

	SPtr<RasterizerState> RenderStateManager::createRasterizerState(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerState> state = findCachedState(desc, id);
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

	SPtr<BlendState> RenderStateManager::createBlendState(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendState> state = findCachedState(desc, id);
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

	SPtr<GraphicsPipelineState> RenderStateManager::createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GraphicsPipelineState> state = CreateGraphicsPipelineStateInternal(desc, deviceMask);
		state->initialize();

		return state;
	}

	SPtr<ComputePipelineState> RenderStateManager::createComputePipelineState(const SPtr<GpuProgram>& program,
																					  GpuDeviceFlags deviceMask) const
	{
		SPtr<ComputePipelineState> state = CreateComputePipelineStateInternal(program, deviceMask);
		state->initialize();

		return state;
	}

	SPtr<GpuPipelineParamInfo> RenderStateManager::createPipelineParamInfo(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineParamInfo> paramInfo = CreatePipelineParamInfoInternal(desc, deviceMask);
		paramInfo->initialize();

		return paramInfo;
	}

	SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
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

	SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<DepthStencilState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createDepthStencilStateInternal(desc, id);

			CachedDepthStencilState cachedData(id);
			cachedData.state = state;

			notifyDepthStencilStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::CreateRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<RasterizerState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createRasterizerStateInternal(desc, id);

			CachedRasterizerState cachedData(id);
			cachedData.state = state;

			notifyRasterizerStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<BlendState> RenderStateManager::CreateBlendStateInternal(const BLEND_STATE_DESC& desc) const
	{
		UINT32 id = 0;
		SPtr<BlendState> state = findCachedState(desc, id);
		if (state == nullptr)
		{
			state = createBlendStateInternal(desc, id);

			CachedBlendState cachedData(id);
			cachedData.state = state;

			notifyBlendStateCreated(desc, cachedData);
		}

		return state;
	}

	SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<GraphicsPipelineState> pipelineState =
			bs_shared_ptr<GraphicsPipelineState>(new (bs_alloc<GraphicsPipelineState>())
			GraphicsPipelineState(desc, deviceMask));

		pipelineState->SetThisPtrInternal(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program,
																					   GpuDeviceFlags deviceMask) const
	{
		SPtr<ComputePipelineState> pipelineState =
			bs_shared_ptr<ComputePipelineState>(new (bs_alloc<ComputePipelineState>())
			ComputePipelineState(program, deviceMask));

		pipelineState->SetThisPtrInternal(pipelineState);

		return pipelineState;
	}

	SPtr<GpuPipelineParamInfo> RenderStateManager::CreatePipelineParamInfoInternal(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<GpuPipelineParamInfo> paramInfo =
			bs_shared_ptr<GpuPipelineParamInfo>(new (bs_alloc<GpuPipelineParamInfo>())
													GpuPipelineParamInfo(desc, deviceMask));

		paramInfo->SetThisPtrInternal(paramInfo);

		return paramInfo;
	}

	void RenderStateManager::onShutDown()
	{
		mDefaultBlendState = nullptr;
		mDefaultDepthStencilState = nullptr;
		mDefaultRasterizerState = nullptr;
		mDefaultSamplerState = nullptr;
	}

	const SPtr<SamplerState>& RenderStateManager::getDefaultSamplerState() const
	{
		if (mDefaultSamplerState == nullptr)
			mDefaultSamplerState = createSamplerState(SAMPLER_STATE_DESC());

		return mDefaultSamplerState;
	}

	const SPtr<BlendState>& RenderStateManager::getDefaultBlendState() const
	{
		if (mDefaultBlendState == nullptr)
			mDefaultBlendState = createBlendState(BLEND_STATE_DESC());

		return mDefaultBlendState;
	}

	const SPtr<RasterizerState>& RenderStateManager::getDefaultRasterizerState() const
	{
		if (mDefaultRasterizerState == nullptr)
			mDefaultRasterizerState = createRasterizerState(RASTERIZER_STATE_DESC());

		return mDefaultRasterizerState;
	}

	const SPtr<DepthStencilState>& RenderStateManager::getDefaultDepthStencilState() const
	{
		if (mDefaultDepthStencilState == nullptr)
			mDefaultDepthStencilState = createDepthStencilState(DEPTH_STENCIL_STATE_DESC());

		return mDefaultDepthStencilState;
	}

	void RenderStateManager::notifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerState>& state) const
	{
		Lock lock(mMutex);

		mCachedSamplerStates[desc] = state;
	}

	void RenderStateManager::notifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const
	{
		Lock lock(mMutex);

		mCachedBlendStates[desc] = state;
	}

	void RenderStateManager::notifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const
	{
		Lock lock(mMutex);

		mCachedRasterizerStates[desc] = state;
	}

	void RenderStateManager::notifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const
	{
		Lock lock(mMutex);

		mCachedDepthStencilStates[desc] = state;
	}

	void RenderStateManager::notifySamplerStateDestroyed(const SAMPLER_STATE_DESC& desc) const
	{
		Lock lock(mMutex);

		mCachedSamplerStates.erase(desc);
	}

	SPtr<SamplerState> RenderStateManager::findCachedState(const SAMPLER_STATE_DESC& desc) const
	{
		Lock lock(mMutex);

		auto iterFind = mCachedSamplerStates.find(desc);
		if (iterFind != mCachedSamplerStates.end())
			return iterFind->second.lock();

		return nullptr;
	}

	SPtr<BlendState> RenderStateManager::findCachedState(const BLEND_STATE_DESC& desc, UINT32& id) const
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

	SPtr<RasterizerState> RenderStateManager::findCachedState(const RASTERIZER_STATE_DESC& desc, UINT32& id) const
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

	SPtr<DepthStencilState> RenderStateManager::findCachedState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32& id) const
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

	SPtr<SamplerState> RenderStateManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<SamplerState> state =
			bs_shared_ptr<SamplerState>(new (bs_alloc<SamplerState>()) SamplerState(desc, deviceMask));
		state->SetThisPtrInternal(state);

		return state;
	}

	SPtr<DepthStencilState> RenderStateManager::createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<DepthStencilState> state = bs_shared_ptr<DepthStencilState>(new (bs_alloc<DepthStencilState>()) DepthStencilState(desc, id));
		state->SetThisPtrInternal(state);

		return state;
	}

	SPtr<RasterizerState> RenderStateManager::createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<RasterizerState> state = bs_shared_ptr<RasterizerState>(new (bs_alloc<RasterizerState>()) RasterizerState(desc, id));
		state->SetThisPtrInternal(state);

		return state;
	}

	SPtr<BlendState> RenderStateManager::createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const
	{
		SPtr<BlendState> state = bs_shared_ptr<BlendState>(new (bs_alloc<BlendState>()) BlendState(desc, id));
		state->SetThisPtrInternal(state);

		return state;
	}
	}
}
