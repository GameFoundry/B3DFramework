//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"

using namespace bs;

SPtr<SamplerState> RenderStateManager::CreateSamplerState(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> state = CreateSamplerStatePtrInternal(desc);
	state->Initialize();

	return state;
}

SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
{
	SPtr<DepthStencilState> state = CreateDepthStencilStatePtrInternal(desc);
	state->Initialize();

	return state;
}

SPtr<RasterizerState> RenderStateManager::CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const
{
	SPtr<RasterizerState> state = CreateRasterizerStatePtrInternal(desc);
	state->Initialize();

	return state;
}

SPtr<BlendState> RenderStateManager::CreateBlendState(const BLEND_STATE_DESC& desc) const
{
	SPtr<BlendState> state = CreateBlendStatePtrInternal(desc);
	state->Initialize();

	return state;
}

SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const
{
	SPtr<GraphicsPipelineState> state = CreateGraphicsPipelineStateInternal(desc);
	state->Initialize();

	return state;
}

SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineState(const SPtr<GpuProgram>& program) const
{
	SPtr<ComputePipelineState> state = CreateComputePipelineStateInternal(program);
	state->Initialize();

	return state;
}

SPtr<SamplerState> RenderStateManager::CreateSamplerStatePtrInternal(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> samplerState = B3DMakeCoreFromExisting<SamplerState>(new(B3DAllocate<SamplerState>()) SamplerState(desc));
	samplerState->SetShared(samplerState);

	return samplerState;
}

SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStatePtrInternal(const DEPTH_STENCIL_STATE_DESC& desc) const
{
	SPtr<DepthStencilState> depthStencilState = B3DMakeCoreFromExisting<DepthStencilState>(new(B3DAllocate<DepthStencilState>()) DepthStencilState(desc));
	depthStencilState->SetShared(depthStencilState);

	return depthStencilState;
}

SPtr<RasterizerState> RenderStateManager::CreateRasterizerStatePtrInternal(const RASTERIZER_STATE_DESC& desc) const
{
	SPtr<RasterizerState> rasterizerState = B3DMakeCoreFromExisting<RasterizerState>(new(B3DAllocate<RasterizerState>()) RasterizerState(desc));
	rasterizerState->SetShared(rasterizerState);

	return rasterizerState;
}

SPtr<BlendState> RenderStateManager::CreateBlendStatePtrInternal(const BLEND_STATE_DESC& desc) const
{
	SPtr<BlendState> blendState = B3DMakeCoreFromExisting<BlendState>(new(B3DAllocate<BlendState>()) BlendState(desc));
	blendState->SetShared(blendState);

	return blendState;
}

SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc) const
{
	SPtr<GraphicsPipelineState> pipelineState =
		B3DMakeCoreFromExisting<GraphicsPipelineState>(new(B3DAllocate<GraphicsPipelineState>()) GraphicsPipelineState(desc));
	pipelineState->SetShared(pipelineState);

	return pipelineState;
}

SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program) const
{
	SPtr<ComputePipelineState> pipelineState =
		B3DMakeCoreFromExisting<ComputePipelineState>(new(B3DAllocate<ComputePipelineState>()) ComputePipelineState(program));
	pipelineState->SetShared(pipelineState);

	return pipelineState;
}

const SPtr<SamplerState>& RenderStateManager::GetDefaultSamplerState() const
{
	if(mDefaultSamplerState == nullptr)
		mDefaultSamplerState = CreateSamplerState(SamplerStateInformation());

	return mDefaultSamplerState;
}

const SPtr<BlendState>& RenderStateManager::GetDefaultBlendState() const
{
	if(mDefaultBlendState == nullptr)
		mDefaultBlendState = CreateBlendState(BLEND_STATE_DESC());

	return mDefaultBlendState;
}

const SPtr<RasterizerState>& RenderStateManager::GetDefaultRasterizerState() const
{
	if(mDefaultRasterizerState == nullptr)
		mDefaultRasterizerState = CreateRasterizerState(RASTERIZER_STATE_DESC());

	return mDefaultRasterizerState;
}

const SPtr<DepthStencilState>& RenderStateManager::GetDefaultDepthStencilState() const
{
	if(mDefaultDepthStencilState == nullptr)
		mDefaultDepthStencilState = CreateDepthStencilState(DEPTH_STENCIL_STATE_DESC());

	return mDefaultDepthStencilState;
}

namespace bs { namespace ct
{
SPtr<SamplerState> RenderStateManager::CreateSamplerState(const SamplerStateCreateInformation& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<SamplerState> state = FindCachedState(desc);
	if(state == nullptr)
	{
		state = CreateSamplerStateInternalInternal(desc, deviceMask);
		state->Initialize();

		NotifySamplerStateCreated(desc, state);
	}

	return state;
}

SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<DepthStencilState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateDepthStencilStateInternalInternal(desc, id);
		state->Initialize();

		CachedDepthStencilState cachedData(id);
		cachedData.State = state;

		NotifyDepthStencilStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<RasterizerState> RenderStateManager::CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<RasterizerState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateRasterizerStateInternalInternal(desc, id);
		state->Initialize();

		CachedRasterizerState cachedData(id);
		cachedData.State = state;

		NotifyRasterizerStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<BlendState> RenderStateManager::CreateBlendState(const BLEND_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<BlendState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateBlendStateInternalInternal(desc, id);
		state->Initialize();

		CachedBlendState cachedData(id);
		cachedData.State = state;

		NotifyBlendStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<GraphicsPipelineState> state = CreateGraphicsPipelineStateInternal(desc, deviceMask);
	state->Initialize();

	return state;
}

SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask) const
{
	SPtr<ComputePipelineState> state = CreateComputePipelineStateInternal(program, deviceMask);
	state->Initialize();

	return state;
}

SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternal(const SamplerStateCreateInformation& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<SamplerState> state = FindCachedState(desc);
	if(state == nullptr)
	{
		state = CreateSamplerStateInternalInternal(desc, deviceMask);

		NotifySamplerStateCreated(desc, state);
	}

	return state;
}

SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<DepthStencilState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateDepthStencilStateInternalInternal(desc, id);

		CachedDepthStencilState cachedData(id);
		cachedData.State = state;

		NotifyDepthStencilStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<RasterizerState> RenderStateManager::CreateRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<RasterizerState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateRasterizerStateInternalInternal(desc, id);

		CachedRasterizerState cachedData(id);
		cachedData.State = state;

		NotifyRasterizerStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<BlendState> RenderStateManager::CreateBlendStateInternal(const BLEND_STATE_DESC& desc) const
{
	u32 id = 0;
	SPtr<BlendState> state = FindCachedState(desc, id);
	if(state == nullptr)
	{
		state = CreateBlendStateInternalInternal(desc, id);

		CachedBlendState cachedData(id);
		cachedData.State = state;

		NotifyBlendStateCreated(desc, cachedData);
	}

	return state;
}

SPtr<GraphicsPipelineState> RenderStateManager::CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<GraphicsPipelineState> pipelineState =
		B3DMakeSharedFromExisting<GraphicsPipelineState>(new(B3DAllocate<GraphicsPipelineState>())
												 GraphicsPipelineState(desc, deviceMask));

	pipelineState->SetShared(pipelineState);

	return pipelineState;
}

SPtr<ComputePipelineState> RenderStateManager::CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask) const
{
	SPtr<ComputePipelineState> pipelineState =
		B3DMakeSharedFromExisting<ComputePipelineState>(new(B3DAllocate<ComputePipelineState>())
												ComputePipelineState(program, deviceMask));

	pipelineState->SetShared(pipelineState);

	return pipelineState;
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
	if(mDefaultSamplerState == nullptr)
		mDefaultSamplerState = CreateSamplerState(SamplerStateInformation());

	return mDefaultSamplerState;
}

const SPtr<BlendState>& RenderStateManager::GetDefaultBlendState() const
{
	if(mDefaultBlendState == nullptr)
		mDefaultBlendState = CreateBlendState(BLEND_STATE_DESC());

	return mDefaultBlendState;
}

const SPtr<RasterizerState>& RenderStateManager::GetDefaultRasterizerState() const
{
	if(mDefaultRasterizerState == nullptr)
		mDefaultRasterizerState = CreateRasterizerState(RASTERIZER_STATE_DESC());

	return mDefaultRasterizerState;
}

const SPtr<DepthStencilState>& RenderStateManager::GetDefaultDepthStencilState() const
{
	if(mDefaultDepthStencilState == nullptr)
		mDefaultDepthStencilState = CreateDepthStencilState(DEPTH_STENCIL_STATE_DESC());

	return mDefaultDepthStencilState;
}

void RenderStateManager::NotifySamplerStateCreated(const SamplerStateCreateInformation& desc, const SPtr<SamplerState>& state) const
{
	Lock lock(mMutex);

	mCachedSamplerStates[desc] = state;
}

void RenderStateManager::NotifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const
{
	Lock lock(mMutex);

	mCachedBlendStates[desc] = state;
}

void RenderStateManager::NotifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const
{
	Lock lock(mMutex);

	mCachedRasterizerStates[desc] = state;
}

void RenderStateManager::NotifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const
{
	Lock lock(mMutex);

	mCachedDepthStencilStates[desc] = state;
}

void RenderStateManager::NotifySamplerStateDestroyed(const SamplerStateInformation& desc) const
{
	Lock lock(mMutex);

	mCachedSamplerStates.erase(desc);
}

SPtr<SamplerState> RenderStateManager::FindCachedState(const SamplerStateInformation& desc) const
{
	Lock lock(mMutex);

	auto iterFind = mCachedSamplerStates.find(desc);
	if(iterFind != mCachedSamplerStates.end())
		return iterFind->second.lock();

	return nullptr;
}

SPtr<BlendState> RenderStateManager::FindCachedState(const BLEND_STATE_DESC& desc, u32& id) const
{
	Lock lock(mMutex);

	auto iterFind = mCachedBlendStates.find(desc);
	if(iterFind != mCachedBlendStates.end())
	{
		id = iterFind->second.Id;

		if(!iterFind->second.State.expired())
			return iterFind->second.State.lock();

		return nullptr;
	}

	id = mNextBlendStateId++;
	B3D_ASSERT(id <= 0x3FF); // 10 bits maximum

	return nullptr;
}

SPtr<RasterizerState> RenderStateManager::FindCachedState(const RASTERIZER_STATE_DESC& desc, u32& id) const
{
	Lock lock(mMutex);

	auto iterFind = mCachedRasterizerStates.find(desc);
	if(iterFind != mCachedRasterizerStates.end())
	{
		id = iterFind->second.Id;

		if(!iterFind->second.State.expired())
			return iterFind->second.State.lock();

		return nullptr;
	}

	id = mNextRasterizerStateId++;
	B3D_ASSERT(id <= 0x3FF); // 10 bits maximum

	return nullptr;
}

SPtr<DepthStencilState> RenderStateManager::FindCachedState(const DEPTH_STENCIL_STATE_DESC& desc, u32& id) const
{
	Lock lock(mMutex);

	auto iterFind = mCachedDepthStencilStates.find(desc);
	if(iterFind != mCachedDepthStencilStates.end())
	{
		id = iterFind->second.Id;

		if(!iterFind->second.State.expired())
			return iterFind->second.State.lock();

		return nullptr;
	}

	id = mNextDepthStencilStateId++;
	B3D_ASSERT(id <= 0x3FF); // 10 bits maximum

	return nullptr;
}

SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternalInternal(const SamplerStateCreateInformation& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<SamplerState> state =
		B3DMakeSharedFromExisting<SamplerState>(new(B3DAllocate<SamplerState>()) SamplerState(desc, deviceMask));
	state->SetShared(state);

	return state;
}

SPtr<DepthStencilState> RenderStateManager::CreateDepthStencilStateInternalInternal(const DEPTH_STENCIL_STATE_DESC& desc, u32 id) const
{
	SPtr<DepthStencilState> state = B3DMakeSharedFromExisting<DepthStencilState>(new(B3DAllocate<DepthStencilState>()) DepthStencilState(desc, id));
	state->SetShared(state);

	return state;
}

SPtr<RasterizerState> RenderStateManager::CreateRasterizerStateInternalInternal(const RASTERIZER_STATE_DESC& desc, u32 id) const
{
	SPtr<RasterizerState> state = B3DMakeSharedFromExisting<RasterizerState>(new(B3DAllocate<RasterizerState>()) RasterizerState(desc, id));
	state->SetShared(state);

	return state;
}

SPtr<BlendState> RenderStateManager::CreateBlendStateInternalInternal(const BLEND_STATE_DESC& desc, u32 id) const
{
	SPtr<BlendState> state = B3DMakeSharedFromExisting<BlendState>(new(B3DAllocate<BlendState>()) BlendState(desc, id));
	state->SetShared(state);

	return state;
}
}}
