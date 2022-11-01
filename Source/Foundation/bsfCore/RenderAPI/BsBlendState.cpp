//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsBlendState.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsBlendStateRTTI.h"
#include "Resources/BsResources.h"

using namespace bs;

bool RENDER_TARGET_BLEND_STATE_DESC::operator==(const RENDER_TARGET_BLEND_STATE_DESC& rhs) const
{
	return BlendEnable == rhs.BlendEnable &&
		SrcBlend == rhs.SrcBlend &&
		DstBlend == rhs.DstBlend &&
		BlendOp == rhs.BlendOp &&
		SrcBlendAlpha == rhs.SrcBlendAlpha &&
		DstBlendAlpha == rhs.DstBlendAlpha &&
		BlendOpAlpha == rhs.BlendOpAlpha &&
		RenderTargetWriteMask == rhs.RenderTargetWriteMask;
}

bool BLEND_STATE_DESC::operator==(const BLEND_STATE_DESC& rhs) const
{
	bool equals = AlphaToCoverageEnable == rhs.AlphaToCoverageEnable &&
		IndependantBlendEnable == rhs.IndependantBlendEnable;

	if(equals)
	{
		for(u32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			equals &= RenderTargetDesc[i] == rhs.RenderTargetDesc[i];
		}
	}

	return equals;
}

BlendProperties::BlendProperties(const BLEND_STATE_DESC& desc)
	: mData(desc), mHash(BlendState::GenerateHash(desc))
{}

bool BlendProperties::GetBlendEnabled(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].BlendEnable;
}

BlendFactor BlendProperties::GetSrcBlend(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].SrcBlend;
}

BlendFactor BlendProperties::GetDstBlend(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].DstBlend;
}

BlendOperation BlendProperties::GetBlendOperation(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].BlendOp;
}

BlendFactor BlendProperties::GetAlphaSrcBlend(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].SrcBlendAlpha;
}

BlendFactor BlendProperties::GetAlphaDstBlend(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].DstBlendAlpha;
}

BlendOperation BlendProperties::GetAlphaBlendOperation(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].BlendOpAlpha;
}

u8 BlendProperties::GetRenderTargetWriteMask(u32 renderTargetIdx) const
{
	assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

	return mData.RenderTargetDesc[renderTargetIdx].RenderTargetWriteMask;
}

BlendState::BlendState(const BLEND_STATE_DESC& desc)
	: mProperties(desc), mId(0)
{}

BlendState::~BlendState()
{
}

SPtr<ct::BlendState> BlendState::GetCore() const
{
	return std::static_pointer_cast<ct::BlendState>(mCoreSpecific);
}

SPtr<ct::CoreObject> BlendState::CreateCore() const
{
	SPtr<ct::BlendState> core = ct::RenderStateManager::Instance().CreateBlendStateInternal(mProperties.mData);
	mId = core->GetId(); // Accessing core from sim thread is okay here since core ID is immutable

	return core;
}

const BlendProperties& BlendState::GetProperties() const
{
	return mProperties;
}

const SPtr<BlendState>& BlendState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultBlendState();
}

SPtr<BlendState> BlendState::Create(const BLEND_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateBlendState(desc);
}

u64 BlendState::GenerateHash(const BLEND_STATE_DESC& desc)
{
	size_t hash = 0;
	B3DCombineHash(hash, desc.AlphaToCoverageEnable);
	B3DCombineHash(hash, desc.IndependantBlendEnable);

	for(u32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
	{
		B3DCombineHash(hash, desc.RenderTargetDesc[i].BlendEnable);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].SrcBlend);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].DstBlend);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].BlendOp);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].SrcBlendAlpha);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].DstBlendAlpha);
		B3DCombineHash(hash, (u32)desc.RenderTargetDesc[i].BlendOpAlpha);
		B3DCombineHash(hash, desc.RenderTargetDesc[i].RenderTargetWriteMask);
	}

	return (u64)hash;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTITypeBase* BlendState::GetRttiStatic()
{
	return BlendStateRTTI::Instance();
}

RTTITypeBase* BlendState::GetRtti() const
{
	return BlendState::GetRttiStatic();
}

namespace bs { namespace ct
{
BlendState::BlendState(const BLEND_STATE_DESC& desc, u32 id)
	: mProperties(desc), mId(id)
{
}

BlendState::~BlendState()
{
}

void BlendState::Initialize()
{
	// Since we cache states it's possible this object was already initialized
	// (i.e. multiple sim-states can share a single core-state)
	if(IsInitialized())
		return;

	CreateInternal();
	CoreObject::Initialize();
}

const BlendProperties& BlendState::GetProperties() const
{
	return mProperties;
}

SPtr<BlendState> BlendState::Create(const BLEND_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateBlendState(desc);
}

const SPtr<BlendState>& BlendState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultBlendState();
}
}}
