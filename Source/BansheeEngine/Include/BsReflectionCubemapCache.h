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
	 * Keeps track of all reflection probes and maintains a cache of their textures so they may be generated during
	 * development time, and then just loaded during runtime.
	 */
	class BS_EXPORT ReflectionCubemapCache : public Module<ReflectionCubemapCache>
	{
	public:
		~ReflectionCubemapCache();

		/** Initializes the cache with a path at which it can find the saved textures. */
		void initCache(const Path& path);

		/** Notifies the manager that the probe with the provided UUID is dirty and needs to be re-created. */
		void notifyDirty(const String& uuid);

		/** Checks if the texture with the specified UUID is marked as dirty, or if it hasn't been cached yet at all. */
		bool isDirty(const String& uuid) const;

		/** Sets a cached texture and associates it with the provided UUID. */
		void setCachedTexture(const String& uuid, const SPtr<Texture>& texture);

		/** Returns a texture that was assigned to the specified UUID when createCachedTexture() was called. */
		SPtr<Texture> getCachedTexture(const String& uuid) const;

		/** Unloads in-memory data for a texture mapped with the specified UUID. */
		void unloadCachedTexture(const String& uuid);

		/** Saves all cached textures in a folder at the provided path. */
		void saveCache(const Path& path);

	private:
		/** Information about a single loaded texture. */
		struct TextureInfo
		{
			SPtr<Texture> texture;
			bool dirty;
			bool needsSaving;
		};

		Path mDataPath;
		mutable UnorderedMap<String, TextureInfo> mTextureInfos;
	};

	/** @} */
}}
