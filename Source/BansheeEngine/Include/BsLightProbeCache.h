//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsParamBlocks.h"
#include "BsRendererMaterial.h"
#include "BsModule.h"

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** 
	 * Keeps a cache of all textures used by light probes so they may be generated during development time, and then just
	 * loaded during runtime.
	 */
	class BS_EXPORT LightProbeCache : public Module<LightProbeCache>
	{
	public:
		~LightProbeCache();

		/** Initializes the cache with a path at which it can find the saved textures. */
		void initCache(const Path& path);

		/** Notifies the manager that the probe with the provided UUID is dirty and needs to be re-created. */
		void notifyDirty(const String& uuid);

		/** 
		 * Checks if the radiance texture for the probe with the specified UUID is marked as dirty, or if it hasn't been
		 * cached yet at all. 
		 */
		bool isRadianceDirty(const String& uuid) const;

		/** 
		 * Checks if the radiance texture for the probe with the specified UUID is marked as dirty, or if it hasn't been
		 * cached yet at all. 
		 */
		bool isIrradianceDirty(const String& uuid) const;

		/** Sets a cached radiance texture and associates it with the provided UUID. */
		void setCachedRadianceTexture(const String& uuid, const SPtr<Texture>& texture);

		/** Sets a cached irradiance texture and associates it with the provided UUID. */
		void setCachedIrradianceTexture(const String& uuid, const SPtr<Texture>& texture);

		/** 
		 * Returns an radiance texture that was assigned to the specified UUID with setCachedRadianceTexture() was
		 * called. 
		 */
		SPtr<Texture> getCachedRadianceTexture(const String& uuid) const;

		/** 
		 * Returns an irradiance texture that was assigned to the specified UUID with setCachedIrradianceTexture() was
		 * called. 
		 */
		SPtr<Texture> getCachedIrradianceTexture(const String& uuid) const;

		/** Unloads in-memory data for a probe with the specified UUID. */
		void unloadCachedTexture(const String& uuid);

		/** Saves all cached textures in a folder at the provided path. */
		void saveCache(const Path& path);

	private:
		/** Information about a single light probe texture. */
		struct TextureInfo
		{
			SPtr<Texture> texture = nullptr;
			bool dirty = false;
			bool needsSaving = false;
		};

		/** Information about textures for a single light probe. */
		struct ProbeInfo
		{
			TextureInfo radiance;
			TextureInfo irradiance;
		};

		/** Loads a saved cached texture at the specified path. */
		SPtr<Texture> loadCachedTexture(const Path& path) const;

		/** Saves cached texture data at the specified path. */
		void saveCachedTexture(const TextureInfo& texInfo, const Path& path);

		Path mDataPath;
		mutable UnorderedMap<String, ProbeInfo> mProbeInfos;
	};

	/** @} */
}}
