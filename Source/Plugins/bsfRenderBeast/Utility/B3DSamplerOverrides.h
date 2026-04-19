//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"

namespace b3d
{
	namespace render
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
			SamplerOverrideKey(const SPtr<Material>& material, u32 variationIndex)
				: Material(material), VariationIndex(variationIndex)
			{}

			bool operator==(const SamplerOverrideKey& rhs) const
			{
				return Material == rhs.Material && VariationIndex == rhs.VariationIndex;
			}

			bool operator!=(const SamplerOverrideKey& rhs) const
			{
				return !(*this == rhs);
			}

			SPtr<Material> Material;
			u32 VariationIndex;
		};

		/**	Helper class for generating sampler overrides. */
		class SamplerOverrideUtility
		{
		public:
			/**
			 * Generates a set of sampler overrides for the specified set of GPU program parameters. Overrides are generates
			 * according to the provided render options.
			 */
			static MaterialSamplerOverrides* GenerateSamplerOverrides(GpuDevice& gpuDevice, const SPtr<Shader>& shader, const SPtr<MaterialParameters>& params, const SPtr<MaterialParameterAdapter>& materialParameterAdapter, const SPtr<RenderBeastOptions>& options);

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
			static SPtr<SamplerState> GenerateSamplerOverride(GpuDevice& gpuDevice, const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options);
		};

		/** @} */
	} // namespace render
} // namespace b3d

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for SamplerOverrideKey. */
	template <>
	struct hash<b3d::render::SamplerOverrideKey>
	{
		size_t operator()(const b3d::render::SamplerOverrideKey& key) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, key.Material);
			b3d::B3DCombineHash(hash, key.VariationIndex);

			return hash;
		}
	};
} // namespace std

/** @endcond */
