//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"

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
		SAMPLER_STATE_DESC(){};

		bool operator==(const SAMPLER_STATE_DESC& rhs) const;

		/** Determines how are texture coordinates outside of [0, 1] range handled. */
		UVWAddressingMode AddressMode;

		/** Filtering used when texture is displayed as smaller than its original size. */
		FilterOptions MinFilter = FO_LINEAR;

		/** Filtering used when texture is displayed as larger than its original size. */
		FilterOptions MagFilter = FO_LINEAR;

		/** Filtering used to blend between the different mip levels. */
		FilterOptions MipFilter = FO_LINEAR;

		/** Maximum number of samples if anisotropic filtering is enabled. Max is 16. */
		u32 MaxAniso = 0;

		/**
		 * Mipmap bias allows you to adjust the mipmap selection calculation. Negative values  force a larger mipmap to be
		 * used, and positive values smaller. Units are in values of mip levels, so -1 means use a mipmap one level higher
		 * than default.
		 */
		float MipmapBias = 0;

		/** Minimum mip-map level that is allowed to be displayed. */
		float MipMin = -FLT_MAX;

		/** Maximum mip-map level that is allowed to be displayed. Set to FLT_MAX for no limit. */
		float MipMax = FLT_MAX;

		/** Border color to use when using border addressing mode as specified by @p addressMode. */
		Color BorderColor = Color::kWhite;

		/** Function that compares sampled data with existing sampled data. */
		CompareFunction ComparisonFunc = CMPF_ALWAYS_PASS;
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
		const UVWAddressingMode& GetTextureAddressingMode() const { return mData.AddressMode; }

		/** Gets the filtering used when sampling from a texture. */
		FilterOptions GetTextureFiltering(FilterType ftpye) const;

		/**
		 * Gets the anisotropy level. Higher anisotropy means better filtering for textures displayed on an angled slope
		 * relative to the viewer.
		 */
		unsigned int GetTextureAnisotropy() const { return mData.MaxAniso; }

		/** Gets a function that compares sampled data with existing sampled data. */
		CompareFunction GetComparisonFunction() const { return mData.ComparisonFunc; }

		/**
		 * Mipmap bias allows you to adjust the mipmap selection calculation. Negative values  force a larger mipmap to be
		 * used, and positive values smaller. Units are in values of mip levels, so -1 means use a mipmap one level higher
		 * than default.
		 */
		float GetTextureMipmapBias() const { return mData.MipmapBias; }

		/** Returns the minimum mip map level. */
		float GetMinimumMip() const { return mData.MipMin; }

		/** Returns the maximum mip map level. */
		float GetMaximumMip() const { return mData.MipMax; }

		/**
		 * Gets the border color that will be used when border texture addressing is used and texture address is outside of
		 * the valid range.
		 */
		const Color& GetBorderColor() const;

		/**	Returns the hash value generated from the sampler state properties. */
		u64 GetHash() const { return mHash; }

		/**	Returns the descriptor originally used for creating the sampler state. */
		SAMPLER_STATE_DESC GetDesc() const { return mData; }

	protected:
		friend class SamplerState;
		friend class ct::SamplerState;
		friend class SamplerStateRTTI;

		SAMPLER_STATE_DESC mData;
		u64 mHash;
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
		virtual ~SamplerState() = default;

		/**	Returns information about the sampler state. */
		const SamplerProperties& GetProperties() const;

		/**	Retrieves a core implementation of the sampler state usable only from the core thread. */
		SPtr<ct::SamplerState> GetCore() const;

		/**	Creates a new sampler state using the provided descriptor structure. */
		static SPtr<SamplerState> Create(const SAMPLER_STATE_DESC& desc);

		/**	Returns the default sampler state. */
		static const SPtr<SamplerState>& GetDefault();

		/**	Generates a hash value from a sampler state descriptor. */
		static u64 GenerateHash(const SAMPLER_STATE_DESC& desc);

	protected:
		SamplerState(const SAMPLER_STATE_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;

		SamplerProperties mProperties;

		friend class RenderStateManager;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SamplerStateRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Core thread version of bs::SamplerState.
		 *
		 * @note	Core thread.
		 */
		class BS_CORE_EXPORT SamplerState : public CoreObject
		{
		public:
			virtual ~SamplerState();

			/**	Returns information about the sampler state. */
			const SamplerProperties& GetProperties() const;

			/**	@copydoc RenderStateManager::CreateSamplerState */
			static SPtr<SamplerState> Create(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**	Returns the default sampler state. */
			static const SPtr<SamplerState>& GetDefault();

		protected:
			friend class RenderStateManager;

			SamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

			/** @copydoc CoreObject::Initialize */
			void Initialize() override;

			/**	Creates any API-specific state objects. */
			virtual void CreateInternal() {}

			SamplerProperties mProperties;
		};

		/** @} */
	} // namespace ct
} // namespace bs

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
	/**	Hash value generator for SAMPLER_STATE_DESC. */
	template <>
	struct hash<bs::SAMPLER_STATE_DESC>
	{
		size_t operator()(const bs::SAMPLER_STATE_DESC& value) const
		{
			return (size_t)bs::SamplerState::GenerateHash(value);
		}
	};
} // namespace std

/** @} */
/** @endcond */
