//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsColor.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

#include <cfloat>

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Structure used for initializing a SamplerState.
	 *
	 * @see		SamplerState
	 */
	struct BS_CORE_EXPORT SAMPLER_STATE_DESC
	{
		SAMPLER_STATE_DESC()
			: minFilter(FO_LINEAR), magFilter(FO_LINEAR), mipFilter(FO_POINT), 
			maxAniso(0), mipmapBias(0), mipMin(-FLT_MAX), mipMax(FLT_MAX),
			borderColor(Color::White), comparisonFunc(CMPF_ALWAYS_PASS)
		{ }

		bool operator==(const SAMPLER_STATE_DESC& rhs) const;

		/** Determines how are texture coordinates outside of [0, 1] range handled. */
		UVWAddressingMode addressMode;
		
		/** Filtering used when texture is displayed as smaller than its original size. */
		FilterOptions minFilter;
		
		/** Filtering used when texture is displayed as larger than its original size. */
		FilterOptions magFilter;
		
		/** Filtering used to blend between the different mip levels. */
		FilterOptions mipFilter; 

		/** Maximum number of samples if anisotropic filtering is enabled. Max is 16. */
		UINT32 maxAniso;

		/**
		 * Mipmap bias allows you to adjust the mipmap selection calculation. Negative values  force a larger mipmap to be
		 * used, and positive values smaller. Units are in values of mip levels, so -1 means use a mipmap one level higher 
		 * than default.
		 */
		float mipmapBias;

		/** Minimum mip-map level that is allowed to be displayed. */
		float mipMin;

		/** Maximum mip-map level that is allowed to be displayed. Set to FLT_MAX for no limit. */
		float mipMax;

		/** Border color to use when using border addressing mode as specified by @p addressMode. */
		Color borderColor;

		/** Function that compares sampled data with existing sampled data. */
		CompareFunction comparisonFunc;
	};

	/** Properties of SamplerState. Shared between sim and core thread versions of SamplerState. */
	class BS_CORE_EXPORT SamplerProperties
	{
	public:
		SamplerProperties(const SAMPLER_STATE_DESC& desc);

		/**
		 * Returns texture addressing mode for each possible texture coordinate. Addressing modes determine how are texture
		 * coordinates outside of [0, 1] range handled.
		 */
		const UVWAddressingMode& getTextureAddressingMode() const { return mData.addressMode; }

		/** Gets the filtering used when sampling from a texture. */
        FilterOptions getTextureFiltering(FilterType ftpye) const;

		/**
		 * Gets the anisotropy level. Higher anisotropy means better filtering for textures displayed on an angled slope 
		 * relative to the viewer.
		 */
		unsigned int getTextureAnisotropy() const { return mData.maxAniso; }

		/** Gets a function that compares sampled data with existing sampled data. */
		CompareFunction getComparisonFunction() const { return mData.comparisonFunc; }

		/**
		 * Mipmap bias allows you to adjust the mipmap selection calculation. Negative values  force a larger mipmap to be
		 * used, and positive values smaller. Units are in values of mip levels, so -1 means use a mipmap one level higher 
		 * than default.
		 */
		float getTextureMipmapBias() const { return mData.mipmapBias; }

		/** Returns the minimum mip map level. */
		float getMinimumMip() const { return mData.mipMin; }

		/** Returns the maximum mip map level. */
		float getMaximumMip() const { return mData.mipMax; }

		/**
		 * Gets the border color that will be used when border texture addressing is used and texture address is outside of
		 * the valid range.
		 */
		const Color& getBorderColor() const;

		/**	Returns the hash value generated from the sampler state properties. */
		UINT64 getHash() const { return mHash; }

		/**	Returns the descriptor originally used for creating the sampler state. */
		SAMPLER_STATE_DESC getDesc() const { return mData; }

	protected:
		friend class SamplerState;
		friend class ct::SamplerStateCore;
		friend class SamplerStateRTTI;

		SAMPLER_STATE_DESC mData;
		UINT64 mHash;
	};

	/**
	 * Class representing the state of a texture sampler.
	 *	
	 * @note	
	 * Sampler units are used for retrieving and filtering data from textures set in a GPU program. Sampler states are 
	 * immutable.
	 * @note
	 * Sim thread.
	 */
	class BS_CORE_EXPORT SamplerState : public IReflectable, public CoreObject
    {
    public:
		virtual ~SamplerState();

		/**	Returns information about the sampler state. */
		const SamplerProperties& getProperties() const;

		/**	Retrieves a core implementation of the sampler state usable only from the core thread. */
		SPtr<ct::SamplerStateCore> getCore() const;

		/**	Creates a new sampler state using the provided descriptor structure. */
		static SPtr<SamplerState> create(const SAMPLER_STATE_DESC& desc);

		/**	Returns the default sampler state. */
		static const SPtr<SamplerState>& getDefault();

		/**	Generates a hash value from a sampler state descriptor. */
		static UINT64 generateHash(const SAMPLER_STATE_DESC& desc);

	protected:
		SamplerState(const SAMPLER_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObjectCore> createCore() const override;

		SamplerProperties mProperties;

		friend class RenderStateManager;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SamplerStateRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
    };

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of SamplerState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT SamplerStateCore : public CoreObjectCore
	{
	public:
		virtual ~SamplerStateCore();

		/**	Returns information about the sampler state. */
		const SamplerProperties& getProperties() const;

		/**	@copydoc RenderStateManager::createSamplerState */
		static SPtr<SamplerStateCore> create(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/**	Returns the default sampler state. */
		static const SPtr<SamplerStateCore>& getDefault();

	protected:
		friend class RenderStateManager;

		SamplerStateCore(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

		SamplerProperties mProperties;
	};
	
	/** @} */
	}
}

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
/**	Hash value generator for SAMPLER_STATE_DESC. */
template<>
struct hash<bs::SAMPLER_STATE_DESC>
{
	size_t operator()(const bs::SAMPLER_STATE_DESC& value) const
	{
		return (size_t)bs::SamplerState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
