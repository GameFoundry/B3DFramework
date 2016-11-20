//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/** 
	 * Provides a way to map one resource path to another path. Useful if the resources are being referenced using a path
	 * that is not the path to their physical location.
	 */
	class BS_EXPORT ResourceMapping : public IReflectable
	{
	public:
		/** Returns the resource path map. */
		const UnorderedMap<Path, Path>& getMap() const { return mMapping; }

		/** Adds a new entry to the resource map. Translated from path @p from to path @p to. */
		void add(const Path& from, const Path& to);

		/** Creates a new empty resource mapping. */
		static SPtr<ResourceMapping> create();
	private:
		UnorderedMap<Path, Path> mMapping;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ResourceMappingRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Interface that can be implemented by the resource loaders required by GameResourceManager. */
	class BS_EXPORT IGameResourceLoader
	{
	public:
		virtual ~IGameResourceLoader() { }

		/**	Loads the resource at the specified path. */
		virtual HResource load(const Path& path, bool keepLoaded) const = 0;

		/** @copydoc GameResourceManager::setMapping */
		virtual void setMapping(const SPtr<ResourceMapping>& mapping) { }
	};

	/**	Handles loading of game resources when the standalone game is running. */
	class BS_EXPORT StandaloneResourceLoader : public IGameResourceLoader
	{
	public:
		/** @copydoc IGameResourceLoader::load */
		HResource load(const Path& path, bool keepLoaded) const override;

		/** @copydoc IGameResourceLoader::setMapping */
		void setMapping(const SPtr<ResourceMapping>& mapping) override;

	private:
		UnorderedMap<Path, Path> mMapping;
	};

	/**
	 * Keeps track of resources that can be dynamically loaded during runtime. These resources will be packed with the game
	 * build so that they're available on demand.
	 *
	 * Internal resource handle can be overridden so that editor or other systems can handle resource loading more directly.
	 */
	class BS_EXPORT GameResourceManager : public Module<GameResourceManager>
	{
	public:
		GameResourceManager();

		/**
		 * Loads the resource at the specified path.
		 * 			
		 * @see	Resources::load
		 */
		HResource load(const Path& path, bool keepLoaded) const;	

		/** @copydoc load */
		template <class T>
		ResourceHandle<T> load(const Path& filePath, bool keepLoaded)
		{
			return static_resource_cast<T>(load(filePath, keepLoaded));
		}

		/** 
		 * Sets an optional mapping that be applied to any path provided to load(). This allows you to reference files
		 * using different names and/or folder structure than they are actually in.
		 *
		 * For example normally in script code you would reference resources based on their path relative to the project
		 * resoruces folder, but in standalone there is no such folder and we need to map the values.
		 *
		 * Provided paths should be relative to the working directory.
		 */
		void setMapping(const SPtr<ResourceMapping>& mapping);

		/** Sets the resource loader implementation that determines how are the paths provided to load() loaded. */
		void setLoader(const SPtr<IGameResourceLoader>& loader);

	private:
		SPtr<IGameResourceLoader> mLoader;
	};

	/** @} */
}