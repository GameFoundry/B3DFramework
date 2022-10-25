//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		/**	Contains data about an overridden sampler states for a single pass. */
		struct PassSamplerOverrides
		{
			u32** StateOverrides;
			u32 NumSets;
		};

		/** Contains data about a single overriden sampler state. */
		struct SamplerOverride
		{
			u32 ParamIdx;
			u64 OriginalStateHash;
			SPtr<SamplerState> State;
			u32 Set;
			u32 Slot;
		};

		/**	Contains data about an overridden sampler states in the entire material. */
		struct MaterialSamplerOverrides
		{
			PassSamplerOverrides* Passes;
			SamplerOverride* Overrides;
			u32 NumPasses;
			u32 NumOverrides;
			u32 RefCount;
			bool IsDirty;
		};

		/** Key used for uniquely identifying a sampler override entry. */
		struct SamplerOverrideKey
		{
			SamplerOverrideKey(const SPtr<Material>& material, u32 techniqueIdx)
				: Material(material), TechniqueIdx(techniqueIdx)
			{}

			bool operator==(const SamplerOverrideKey& rhs) const
			{
				return Material == rhs.Material && TechniqueIdx == rhs.TechniqueIdx;
			}

			bool operator!=(const SamplerOverrideKey& rhs) const
			{
				return !(*this == rhs);
			}

			SPtr<Material> Material;
			u32 TechniqueIdx;
		};

		/**	Helper class for generating sampler overrides. */
		class SamplerOverrideUtility
		{
		public:
			/**
			 * Generates a set of sampler overrides for the specified set of GPU program parameters. Overrides are generates
			 * according to the provided render options.
			 */
			static MaterialSamplerOverrides* GenerateSamplerOverrides(const SPtr<Shader>& shader, const SPtr<MaterialParams>& params, const SPtr<GpuParamsSet>& paramsSet, const SPtr<RenderBeastOptions>& options);

			/**	Destroys sampler overrides previously generated with generateSamplerOverrides(). */
			static void DestroySamplerOverrides(MaterialSamplerOverrides* overrides);

			/**
			 * Checks if the provided sampler state requires an override, in case the render options have requirements not
			 * fulfilled by current sampler state (for example filtering type).
			 */
			static bool CheckNeedsOverride(const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options);

			/**
			 * Generates a new sampler state override using the provided state as the basis. Overridden properties are taken
			 * from the provided render options.
			 */
			static SPtr<SamplerState> GenerateSamplerOverride(const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options);
		};

		/** @} */
	} // namespace ct
} // namespace bs

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for SamplerOverrideKey. */
	template <>
	struct hash<bs::ct::SamplerOverrideKey>
	{
		size_t operator()(const bs::ct::SamplerOverrideKey& key) const
		{
			size_t hash = 0;
			bs::bs_hash_combine(hash, key.Material);
			bs::bs_hash_combine(hash, key.TechniqueIdx);

			return hash;
		}
	};
} // namespace std

/** @endcond */
