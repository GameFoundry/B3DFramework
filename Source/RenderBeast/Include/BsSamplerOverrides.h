//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**	Contains data about an overridden sampler states for a single pass. */
	struct PassSamplerOverrides
	{
		UINT32** stateOverrides;
		UINT32 numSets;
	};

	/** Contains data about a single overriden sampler state. */
	struct SamplerOverride
	{
		UINT32 paramIdx;
		UINT64 originalStateHash;
		SPtr<SamplerState> state;
		UINT32 set;
		UINT32 slot;
	};

	/**	Contains data about an overridden sampler states in the entire material. */
	struct MaterialSamplerOverrides
	{
		PassSamplerOverrides* passes;
		SamplerOverride* overrides;
		UINT32 numPasses;
		UINT32 numOverrides;
		UINT32 refCount;
		bool isDirty;
	};

	/** Key used for uniquely identifying a sampler override entry. */
	struct SamplerOverrideKey
	{
		SamplerOverrideKey(const SPtr<Material>& material, UINT32 techniqueIdx)
			:material(material), techniqueIdx(techniqueIdx)
		{ }

		bool operator== (const SamplerOverrideKey& rhs) const
		{ 
			return material == rhs.material && techniqueIdx == rhs.techniqueIdx;
		}

		bool operator!= (const SamplerOverrideKey& rhs) const 
		{ 
			return !(*this == rhs); 
		}

		SPtr<Material> material;
		UINT32 techniqueIdx;
	};

	/**	Helper class for generating sampler overrides. */
	class SamplerOverrideUtility
	{
	public:
		/**
		 * Generates a set of sampler overrides for the specified set of GPU program parameters. Overrides are generates
		 * according to the provided render options. 
		 */
		static MaterialSamplerOverrides* generateSamplerOverrides(const SPtr<Shader>& shader,
			const SPtr<MaterialParams>& params, 
			const SPtr<GpuParamsSet>& paramsSet,
			const SPtr<RenderBeastOptions>& options);

		/**	Destroys sampler overrides previously generated with generateSamplerOverrides(). */
		static void destroySamplerOverrides(MaterialSamplerOverrides* overrides);

		/**
		 * Checks if the provided sampler state requires an override, in case the render options have requirements not
		 * fulfilled by current sampler state (for example filtering type).
		 */
		static bool checkNeedsOverride(const SPtr<SamplerState>& samplerState, 
			const SPtr<RenderBeastOptions>& options);

		/**
		 * Generates a new sampler state override using the provided state as the basis. Overridden properties are taken
		 * from the provided render options.
		 */
		static SPtr<SamplerState> generateSamplerOverride(const SPtr<SamplerState>& samplerState, 
			const SPtr<RenderBeastOptions>& options);
	};

	/** @} */
}}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for SamplerOverrideKey. */
	template<>
	struct hash<bs::ct::SamplerOverrideKey>
	{
		size_t operator()(const bs::ct::SamplerOverrideKey& key) const
		{
			size_t hash = 0;
			bs::hash_combine(hash, key.material);
			bs::hash_combine(hash, key.techniqueIdx);

			return hash;
		}
	};
}

/** @endcond */