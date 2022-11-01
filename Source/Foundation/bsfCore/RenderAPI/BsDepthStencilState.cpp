//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsDepthStencilState.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsDepthStencilStateRTTI.h"
#include "Error/BsException.h"
#include "Resources/BsResources.h"

using namespace bs;

bool DEPTH_STENCIL_STATE_DESC::operator==(const DEPTH_STENCIL_STATE_DESC& rhs) const
{
	return DepthReadEnable == rhs.DepthReadEnable &&
		DepthWriteEnable == rhs.DepthWriteEnable &&
		DepthComparisonFunc == rhs.DepthComparisonFunc &&
		StencilEnable == rhs.StencilEnable &&
		StencilReadMask == rhs.StencilReadMask &&
		StencilWriteMask == rhs.StencilWriteMask &&
		FrontStencilFailOp == rhs.FrontStencilFailOp &&
		FrontStencilZFailOp == rhs.FrontStencilZFailOp &&
		FrontStencilPassOp == rhs.FrontStencilPassOp &&
		FrontStencilComparisonFunc == rhs.FrontStencilComparisonFunc &&
		BackStencilFailOp == rhs.BackStencilFailOp &&
		BackStencilZFailOp == rhs.BackStencilZFailOp &&
		BackStencilPassOp == rhs.BackStencilPassOp &&
		BackStencilComparisonFunc == rhs.BackStencilComparisonFunc;
}

DepthStencilProperties::DepthStencilProperties(const DEPTH_STENCIL_STATE_DESC& desc)
	: mData(desc), mHash(DepthStencilState::GenerateHash(desc))
{
}

DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc)
	: mProperties(desc), mId(0)
{
}

SPtr<ct::DepthStencilState> DepthStencilState::GetCore() const
{
	return std::static_pointer_cast<ct::DepthStencilState>(mCoreSpecific);
}

SPtr<ct::CoreObject> DepthStencilState::CreateCore() const
{
	SPtr<ct::DepthStencilState> core = ct::RenderStateManager::Instance().CreateDepthStencilStateInternal(mProperties.mData);
	mId = core->GetId(); // Accessing core from sim thread is okay here since core ID is immutable

	return core;
}

const SPtr<DepthStencilState>& DepthStencilState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultDepthStencilState();
}

const DepthStencilProperties& DepthStencilState::GetProperties() const
{
	return mProperties;
}

SPtr<DepthStencilState> DepthStencilState::Create(const DEPTH_STENCIL_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateDepthStencilState(desc);
}

u64 DepthStencilState::GenerateHash(const DEPTH_STENCIL_STATE_DESC& desc)
{
	size_t hash = 0;
	B3DCombineHash(hash, desc.DepthReadEnable);
	B3DCombineHash(hash, desc.DepthWriteEnable);
	B3DCombineHash(hash, (u32)desc.DepthComparisonFunc);
	B3DCombineHash(hash, desc.StencilEnable);
	B3DCombineHash(hash, desc.StencilReadMask);
	B3DCombineHash(hash, desc.StencilWriteMask);
	B3DCombineHash(hash, (u32)desc.FrontStencilFailOp);
	B3DCombineHash(hash, (u32)desc.FrontStencilZFailOp);
	B3DCombineHash(hash, (u32)desc.FrontStencilPassOp);
	B3DCombineHash(hash, (u32)desc.FrontStencilComparisonFunc);
	B3DCombineHash(hash, (u32)desc.BackStencilFailOp);
	B3DCombineHash(hash, (u32)desc.BackStencilZFailOp);
	B3DCombineHash(hash, (u32)desc.BackStencilPassOp);
	B3DCombineHash(hash, (u32)desc.BackStencilComparisonFunc);

	return (u64)hash;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTITypeBase* DepthStencilState::GetRttiStatic()
{
	return DepthStencilStateRTTI::Instance();
}

RTTITypeBase* DepthStencilState::GetRtti() const
{
	return DepthStencilState::GetRttiStatic();
}

namespace bs { namespace ct
{
DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, u32 id)
	: mProperties(desc), mId(id)
{
}

DepthStencilState::~DepthStencilState()
{
}

void DepthStencilState::Initialize()
{
	// Since we cache states it's possible this object was already initialized
	// (i.e. multiple sim-states can share a single core-state)
	if(IsInitialized())
		return;

	CreateInternal();
	CoreObject::Initialize();
}

const DepthStencilProperties& DepthStencilState::GetProperties() const
{
	return mProperties;
}

SPtr<DepthStencilState> DepthStencilState::Create(const DEPTH_STENCIL_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateDepthStencilState(desc);
}

const SPtr<DepthStencilState>& DepthStencilState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultDepthStencilState();
}
}}
