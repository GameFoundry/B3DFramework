//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsSamplerState.h"

using namespace bs;

SPtr<SamplerState> RenderStateManager::CreateSamplerState(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> state = CreateSamplerStatePtrInternal(desc);
	state->Initialize();

	return state;
}

SPtr<SamplerState> RenderStateManager::CreateSamplerStatePtrInternal(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> samplerState = B3DMakeCoreFromExisting<SamplerState>(new(B3DAllocate<SamplerState>()) SamplerState(desc));
	samplerState->SetShared(samplerState);

	return samplerState;
}

const SPtr<SamplerState>& RenderStateManager::GetDefaultSamplerState() const
{
	if(mDefaultSamplerState == nullptr)
		mDefaultSamplerState = CreateSamplerState(SamplerStateInformation());

	return mDefaultSamplerState;
}

namespace bs { namespace ct
{
SPtr<SamplerState> RenderStateManager::CreateSamplerState(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> state = FindCachedState(desc);
	if(state == nullptr)
	{
		state = CreateSamplerStateInternalInternal(desc);
		state->Initialize();

		NotifySamplerStateCreated(desc, state);
	}

	return state;
}

SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternal(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> state = FindCachedState(desc);
	if(state == nullptr)
	{
		state = CreateSamplerStateInternalInternal(desc);

		NotifySamplerStateCreated(desc, state);
	}

	return state;
}

void RenderStateManager::OnShutDown()
{
	mDefaultSamplerState = nullptr;
}

const SPtr<SamplerState>& RenderStateManager::GetDefaultSamplerState() const
{
	if(mDefaultSamplerState == nullptr)
		mDefaultSamplerState = CreateSamplerState(SamplerStateInformation());

	return mDefaultSamplerState;
}

void RenderStateManager::NotifySamplerStateCreated(const SamplerStateCreateInformation& desc, const SPtr<SamplerState>& state) const
{
	Lock lock(mMutex);

	mCachedSamplerStates[desc] = state;
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

SPtr<SamplerState> RenderStateManager::CreateSamplerStateInternalInternal(const SamplerStateCreateInformation& desc) const
{
	SPtr<SamplerState> state =
		B3DMakeSharedFromExisting<SamplerState>(new(B3DAllocate<SamplerState>()) SamplerState(desc));
	state->SetShared(state);

	return state;
}
}}
