//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSamplerState.h"
#include "BsSamplerStateRTTI.h"
#include "BsRenderStateManager.h"

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

	SamplerState::~SamplerState()
	{

	}

	SPtr<ct::SamplerStateCore> SamplerState::getCore() const
	{
		return std::static_pointer_cast<ct::SamplerStateCore>(mCoreSpecific);
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

	SamplerStateCore::SamplerStateCore(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:mProperties(desc)
	{

	}

	SamplerStateCore::~SamplerStateCore()
	{
		RenderStateManager::instance().notifySamplerStateDestroyed(mProperties.mData);
	}

	void SamplerStateCore::initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObject::initialize();
	}

	const SamplerProperties& SamplerStateCore::getProperties() const
	{
		return mProperties;
	}

	SPtr<SamplerStateCore> SamplerStateCore::create(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::instance().createSamplerState(desc, deviceMask);
	}

	const SPtr<SamplerStateCore>& SamplerStateCore::getDefault()
	{
		return RenderStateManager::instance().getDefaultSamplerState();
	}

	}
}
