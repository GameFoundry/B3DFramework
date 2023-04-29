//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsSamplerState.h"
#include "Private/RTTI/BsSamplerStateRTTI.h"
#include "Managers/BsRenderStateManager.h"

using namespace bs;

bool SamplerStateInformation::operator==(const SamplerStateInformation& rhs) const
{
	return AddressMode == rhs.AddressMode &&
		MinFilter == rhs.MinFilter &&
		MagFilter == rhs.MagFilter &&
		MipFilter == rhs.MipFilter &&
		MaxAniso == rhs.MaxAniso &&
		MipmapBias == rhs.MipmapBias &&
		MipMin == rhs.MipMin &&
		MipMax == rhs.MipMax &&
		BorderColor == rhs.BorderColor &&
		ComparisonFunc == rhs.ComparisonFunc;
}

SamplerProperties::SamplerProperties(const SamplerStateInformation& desc)
	: mData(desc), mHash(SamplerState::GenerateHash(desc))
{}

FilterOptions SamplerProperties::GetTextureFiltering(FilterType ft) const
{
	switch(ft)
	{
	case FT_MIN:
		return mData.MinFilter;
	case FT_MAG:
		return mData.MagFilter;
	case FT_MIP:
		return mData.MipFilter;
	}

	return mData.MinFilter;
}

const Color& SamplerProperties::GetBorderColor() const
{
	return mData.BorderColor;
}

SamplerState::SamplerState(const SamplerStateCreateInformation& desc)
	: mProperties(desc)
{
}

SPtr<ct::SamplerState> SamplerState::GetCore() const
{
	return std::static_pointer_cast<ct::SamplerState>(mCoreSpecific);
}

SPtr<ct::CoreObject> SamplerState::CreateCore() const
{
	return ct::RenderStateManager::Instance().CreateSamplerStateInternal(mProperties.mData);
}

SPtr<SamplerState> SamplerState::Create(const SamplerStateCreateInformation& desc)
{
	return RenderStateManager::Instance().CreateSamplerState(desc);
}

const SPtr<SamplerState>& SamplerState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultSamplerState();
}

u64 SamplerState::GenerateHash(const SamplerStateInformation& desc)
{
	size_t hash = 0;
	B3DCombineHash(hash, (u32)desc.AddressMode.U);
	B3DCombineHash(hash, (u32)desc.AddressMode.V);
	B3DCombineHash(hash, (u32)desc.AddressMode.W);
	B3DCombineHash(hash, (u32)desc.MinFilter);
	B3DCombineHash(hash, (u32)desc.MagFilter);
	B3DCombineHash(hash, (u32)desc.MipFilter);
	B3DCombineHash(hash, desc.MaxAniso);
	B3DCombineHash(hash, desc.MipmapBias);
	B3DCombineHash(hash, desc.MipMin);
	B3DCombineHash(hash, desc.MipMax);
	B3DCombineHash(hash, desc.BorderColor);
	B3DCombineHash(hash, (u32)desc.ComparisonFunc);

	return (u64)hash;
}

const SamplerProperties& SamplerState::GetProperties() const
{
	return mProperties;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTITypeBase* SamplerState::GetRttiStatic()
{
	return SamplerStateRTTI::Instance();
}

RTTITypeBase* SamplerState::GetRtti() const
{
	return SamplerState::GetRttiStatic();
}

namespace bs { namespace ct
{

SamplerState::SamplerState(const SamplerStateCreateInformation& desc)
	: mProperties(desc)
{
}

SamplerState::~SamplerState()
{
	RenderStateManager::Instance().NotifySamplerStateDestroyed(mProperties.mData);
}

void SamplerState::Initialize()
{
	// Since we cache states it's possible this object was already initialized
	// (i.e. multiple sim-states can share a single core-state)
	if(IsInitialized())
		return;

	CreateInternal();
	CoreObject::Initialize();
}

const SamplerProperties& SamplerState::GetProperties() const
{
	return mProperties;
}

SPtr<SamplerState> SamplerState::Create(const SamplerStateCreateInformation& desc)
{
	return RenderStateManager::Instance().CreateSamplerState(desc);
}

const SPtr<SamplerState>& SamplerState::GetDefault()
{
	return RenderStateManager::Instance().GetDefaultSamplerState();
}

}}
