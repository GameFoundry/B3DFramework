//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRasterizerState.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsRasterizerStateRTTI.h"
#include "Resources/BsResources.h"

using namespace bs;

bool RASTERIZER_STATE_DESC::operator==(const RASTERIZER_STATE_DESC& rhs) const
{
	return PolygonMode == rhs.PolygonMode &&
		CullMode == rhs.CullMode &&
		DepthBias == rhs.DepthBias &&
		DepthBiasClamp == rhs.DepthBiasClamp &&
		SlopeScaledDepthBias == rhs.SlopeScaledDepthBias &&
		DepthClipEnable == rhs.DepthClipEnable &&
		ScissorEnable == rhs.ScissorEnable &&
		MultisampleEnable == rhs.MultisampleEnable &&
		AntialiasedLineEnable == rhs.AntialiasedLineEnable;
}

RasterizerProperties::RasterizerProperties(const RASTERIZER_STATE_DESC& desc)
	: mData(desc), mHash(RasterizerState::GenerateHash(desc))
{}

RasterizerState::RasterizerState(const RASTERIZER_STATE_DESC& desc)
	: mProperties(desc), mId(0)
{
}

RasterizerState::~RasterizerState()
{
}

SPtr<ct::RasterizerState> RasterizerState::GetCore() const
{
	return std::static_pointer_cast<ct::RasterizerState>(mCoreSpecific);
}

SPtr<ct::CoreObject> RasterizerState::CreateCore() const
{
	SPtr<ct::RasterizerState> core = ct::RenderStateManager::Instance().CreateRasterizerStateInternal(mProperties.mData);
	mId = core->GetId(); // Accessing core from sim thread is okay here since core ID is immutable

	return core;
}

const RasterizerProperties& RasterizerState::GetProperties() const
{
	return mProperties;
}

const SPtr<RasterizerState>& RasterizerState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultRasterizerState();
}

SPtr<RasterizerState> RasterizerState::Create(const RASTERIZER_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateRasterizerState(desc);
}

u64 RasterizerState::GenerateHash(const RASTERIZER_STATE_DESC& desc)
{
	size_t hash = 0;
	bs_hash_combine(hash, (u32)desc.PolygonMode);
	bs_hash_combine(hash, (u32)desc.CullMode);
	bs_hash_combine(hash, desc.DepthBias);
	bs_hash_combine(hash, desc.DepthBiasClamp);
	bs_hash_combine(hash, desc.SlopeScaledDepthBias);
	bs_hash_combine(hash, desc.DepthClipEnable);
	bs_hash_combine(hash, desc.ScissorEnable);
	bs_hash_combine(hash, desc.MultisampleEnable);
	bs_hash_combine(hash, desc.AntialiasedLineEnable);

	return (u64)hash;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTITypeBase* RasterizerState::GetRttiStatic()
{
	return RasterizerStateRTTI::Instance();
}

RTTITypeBase* RasterizerState::GetRtti() const
{
	return RasterizerState::GetRttiStatic();
}

namespace bs { namespace ct
{
RasterizerState::RasterizerState(const RASTERIZER_STATE_DESC& desc, u32 id)
	: mProperties(desc), mId(id)
{
}

RasterizerState::~RasterizerState()
{
}

void RasterizerState::Initialize()
{
	// Since we cache states it's possible this object was already initialized
	// (i.e. multiple sim-states can share a single core-state)
	if(IsInitialized())
		return;

	CreateInternal();
	CoreObject::Initialize();
}

const RasterizerProperties& RasterizerState::GetProperties() const
{
	return mProperties;
}

SPtr<RasterizerState> RasterizerState::Create(const RASTERIZER_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateRasterizerState(desc);
}

const SPtr<RasterizerState>& RasterizerState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultRasterizerState();
}
}}
