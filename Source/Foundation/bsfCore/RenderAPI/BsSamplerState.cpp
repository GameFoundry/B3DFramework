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

	FilterOptions SamplerProperties::getTextureFiltering(FilterType ft) const
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

	const Color& SamplerProperties::getBorderColor() const
	{
		return mData.borderColor;
	}

	SamplerState::SamplerState(const SAMPLER_STATE_DESC& desc)
		:mProperties(desc)
	{

	}

	SPtr<ct::SamplerState> SamplerState::getCore() const
	{
		return std::static_pointer_cast<ct::SamplerState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> SamplerState::createCore() const
	{
		return ct::RenderStateManager::instance()._createSamplerState(mProperties.mData);
	}

	SPtr<SamplerState> SamplerState::create(const SAMPLER_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createSamplerState(desc);
	}

	const SPtr<SamplerState>& SamplerState::getDefault()
	{
		return RenderStateManager::instance().getDefaultSamplerState();
	}

	UINT64 SamplerState::generateHash(const SAMPLER_STATE_DESC& desc)
	{
		size_t hash = 0;
		hash_combine(hash, (UINT32)desc.addressMode.u);
		hash_combine(hash, (UINT32)desc.addressMode.v);
		hash_combine(hash, (UINT32)desc.addressMode.w);
		hash_combine(hash, (UINT32)desc.minFilter);
		hash_combine(hash, (UINT32)desc.magFilter);
		hash_combine(hash, (UINT32)desc.mipFilter);
		hash_combine(hash, desc.maxAniso);
		hash_combine(hash, desc.mipmapBias);
		hash_combine(hash, desc.mipMin);
		hash_combine(hash, desc.mipMax);
		hash_combine(hash, desc.borderColor);
		hash_combine(hash, (UINT32)desc.comparisonFunc);

		return (UINT64)hash;
	}

	const SamplerProperties& SamplerState::getProperties() const
	{
		return mProperties;
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* SamplerState::getRTTIStatic()
	{
		return SamplerStateRTTI::instance();
	}

	RTTITypeBase* SamplerState::getRTTI() const
	{
		return SamplerState::getRTTIStatic();
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

	void SamplerState::initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObject::initialize();
	}

	const SamplerProperties& SamplerState::getProperties() const
	{
		return mProperties;
	}

	SPtr<SamplerState> SamplerState::create(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::instance().createSamplerState(desc, deviceMask);
	}

	const SPtr<SamplerState>& SamplerState::getDefault()
	{
		return RenderStateManager::instance().getDefaultSamplerState();
	}

	}
}
