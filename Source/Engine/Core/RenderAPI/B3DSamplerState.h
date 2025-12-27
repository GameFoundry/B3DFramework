//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Reflection/B3DIReflectable.h"
#include "CoreObject/B3DCoreObject.h"

namespace b3d
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Information describing a SamplerState. */
	struct B3D_EXPORT SamplerStateInformation
	{
		SamplerStateInformation() = default;

		bool operator==(const SamplerStateInformation& rhs) const;
		bool operator!=(const SamplerStateInformation& rhs) const { return !this->operator==(rhs); }

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

	/** Descriptor structure used for initialization of a SamplerState. */
	struct B3D_EXPORT SamplerStateCreateInformation : SamplerStateInformation
	{
		SamplerStateCreateInformation() = default;

		SamplerStateCreateInformation(const SamplerStateInformation& other)
			: SamplerStateInformation(other)
		{ }
	};

	/** Defines the behaviour of a texture sampler on the GPU. */
	class B3D_EXPORT SamplerState : public IReflectable
	{
	public:
		virtual ~SamplerState() = default;

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() {}

		/** Returns information describing the sampler state. */
		const SamplerStateInformation& GetInformation() const { return mInformation; }

		/**	Generates a hash value from a sampler state descriptor. */
		static u64 GenerateHash(const SamplerStateInformation& information);
	protected:
		SamplerState(const SamplerStateCreateInformation& createInformation);

		SamplerStateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SamplerStateRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
	/**	Hash value generator for SamplerStateInformation. */
	template <>
	struct hash<b3d::SamplerStateInformation>
	{
		size_t operator()(const b3d::SamplerStateInformation& value) const
		{
			return (size_t)b3d::SamplerState::GenerateHash(value);
		}
	};

	/**	Hash value generator for SamplerStateCreateInformation. */
	template <>
	struct hash<b3d::SamplerStateCreateInformation>
	{
		size_t operator()(const b3d::SamplerStateCreateInformation& value) const
		{
			return (size_t)b3d::SamplerState::GenerateHash(value);
		}
	};
} // namespace std

/** @} */
/** @endcond */
