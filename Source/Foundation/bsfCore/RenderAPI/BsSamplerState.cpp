//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsSamplerState.h"
#include "Private/RTTI/BsSamplerStateRTTI.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	bool SAMPLER_STATE_DESC::operator == (const SAMPLER_STATE_DESC& rhs) const
	{
		return addressMode == rhs.addressMode &&
			minFilter == rhs.minFilter &&
			magFilter == rhs.magFilter &&
			mipFilter == rhs.mipFilter &&
			maxAniso == rhs.maxAniso &&
			mipmapBias == rhs.mipmapBias &&
			mipMin == rhs.mipMin &&
			mipMax == rhs.mipMax &&
			borderColor == rhs.borderColor &&
			comparisonFunc == rhs.comparisonFunc;
	}

	SamplerProperties::SamplerProperties(const SAMPLER_STATE_DESC& desc)
		:mData(desc), mHash(SamplerState::generateHash(desc))
	{ }

	FilterOptions SamplerProperties::GetTextureFiltering(FilterType ft) const
	{
		switch (ft)
		{
		case FT_MIN:
			return mData.minFilter;
		case FT_MAG:
			return mData.magFilter;
		case FT_MIP:
			return mData.mipFilter;
		}

		return mData.minFilter;
	}

	const Color& SamplerProperties::GetBorderColor() const
	{
		return mData.borderColor;
	}

	SamplerState::SamplerState(const SAMPLER_STATE_DESC& desc)
		:mProperties(desc)
	{

	}

	SPtr<ct::SamplerState> SamplerState::GetCore() const
	{
		return std::static_pointer_cast<ct::SamplerState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> SamplerState::CreateCore() const
	{
		return ct::RenderStateManager::instance()._createSamplerState(mProperties.mData);
	}

	SPtr<SamplerState> SamplerState::Create(const SAMPLER_STATE_DESC& desc)
	{
		return RenderStateManager::Instance().createSamplerState(desc);
	}

	const SPtr<SamplerState>& SamplerState::GetDefault()
	{
		return RenderStateManager::Instance().getDefaultSamplerState();
	}

	UINT64 SamplerState::GenerateHash(const SAMPLER_STATE_DESC& desc)
	{
		size_t hash = 0;
		bs_hash_combine(hash, (UINT32)desc.addressMode.u);
		bs_hash_combine(hash, (UINT32)desc.addressMode.v);
		bs_hash_combine(hash, (UINT32)desc.addressMode.w);
		bs_hash_combine(hash, (UINT32)desc.minFilter);
		bs_hash_combine(hash, (UINT32)desc.magFilter);
		bs_hash_combine(hash, (UINT32)desc.mipFilter);
		bs_hash_combine(hash, desc.maxAniso);
		bs_hash_combine(hash, desc.mipmapBias);
		bs_hash_combine(hash, desc.mipMin);
		bs_hash_combine(hash, desc.mipMax);
		bs_hash_combine(hash, desc.borderColor);
		bs_hash_combine(hash, (UINT32)desc.comparisonFunc);

		return (UINT64)hash;
	}

	const SamplerProperties& SamplerState::GetProperties() const
	{
		return mProperties;
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* SamplerState::getRTTIStatic()
	{
		return SamplerStateRTTI::Instance();
	}

	RTTITypeBase* SamplerState::getRTTI() const
	{
		return SamplerState::GetRTTIStatic();
	}

	namespace ct
	{

	SamplerState::SamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:mProperties(desc)
	{

	}

	SamplerState::~SamplerState()
	{
		RenderStateManager::instance().notifySamplerStateDestroyed(mProperties.mData);
	}

	void SamplerState::Initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObject::initialize();
	}

	const SamplerProperties& SamplerState::GetProperties() const
	{
		return mProperties;
	}

	SPtr<SamplerState> SamplerState::Create(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::Instance().createSamplerState(desc, deviceMask);
	}

	const SPtr<SamplerState>& SamplerState::GetDefault()
	{
		return RenderStateManager::Instance().getDefaultSamplerState();
	}

	}
}
