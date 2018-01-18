//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Flags that can be used to control resource loading. */
	enum class ResourceLoadFlag
	{
		/** No flags. */
		None = 0,
		/** If enabled all resources referenced by the root resource will be loaded as well. */
		LoadDependencies = 1 << 0, 
		/**
		 * If enabled the resource system will keep an internal reference to the resource so it doesn't get destroyed when
		 * it goes out of scope. You can call Resources::release() to release the internal reference. Each call to load will
		 * create a new internal reference and therefore must be followed by the same number of release calls. If
		 * dependencies are being loaded, they will not have internal references created regardless of this parameter.
		 */
		KeepInternalRef = 1 << 1,
		/**
		 * Determines if the loaded resource keeps original data loaded. Sometime resources will process loaded data
		 * and discard the original (e.g. uncompressing audio on load). This flag can prevent the resource from discarding
		 * the original data. The original data might be required for saving the resource (via Resources::save), but will
		 * use up extra memory. Normally you want to keep this enabled if you plan on saving the resource to disk.
		 */
		KeepSourceData = 1 << 2,
		/** Default set of flags used for resource loading. */
		Default = LoadDependencies | KeepInternalRef
	};

	typedef Flags<ResourceLoadFlag> ResourceLoadFlags;
	BS_FLAGS_OPERATORS(ResourceLoadFlag);

	/**
	 * Manager for dealing with all engine resources. It allows you to save new resources and load existing ones.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT Resources : public Module<Resources>
	{
		/** Information about a loaded resource. */
		struct LoadedResourceData
		{
			LoadedResourceData()
				:numInternalRefs(0)
			{ }

			LoadedResourceData(const WeakResourceHandle<Resource>& resource)
				:resource(resource), numInternalRefs(0)
			{ }

			WeakResourceHandle<Resource> resource;
			UINT32 numInternalRefs;
		};

		/** Information about a resource that's currently being loaded. */
		struct ResourceLoadData
		{
			ResourceLoadData(const WeakResourceHandle<Resource>& resource, UINT32 numDependencies)
				:resData(resource), remainingDependencies(numDependencies)
			{ }

			LoadedResourceData resData;
			SPtr<Resource> loadedData;
			UINT32 remainingDependencies;
			Vector<HResource> dependencies;
			bool notifyImmediately;
		};

	public:
		Resources();
		~Resources();

		/**
		 * Loads the resource from a given path. Returns an empty handle if resource can't be loaded. Resource is loaded 
		 * synchronously.
		 *			
		 * @param[in]	filePath	File path to the resource to load. This can be absolute or relative to the working 
		 *							folder.
		 * @param[in]	loadFlags	Flags used to control the load process.
		 *			
		 * @see		release(ResourceHandleBase&), unloadAllUnused()
		 */
		HResource load(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc load(const Path&, ResourceLoadFlags) */
		template <class T>
		ResourceHandle<T> load(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return static_resource_cast<T>(load(filePath, loadFlags));
		}

		/**
		 * Loads the resource for the provided weak resource handle, or returns a loaded resource if already loaded.
		 * 			
		 * @see		load(const Path&, ResourceLoadFlags)
		 */
		HResource load(const WeakResourceHandle<Resource>& handle, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc load(const WeakResourceHandle<Resource>&, ResourceLoadFlags) */
		template <class T>
		ResourceHandle<T> load(const WeakResourceHandle<T>& handle, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return static_resource_cast<T>(load((const WeakResourceHandle<Resource>&)handle, loadFlags));
		}

		/**
		 * Loads the resource asynchronously. Initially returned resource handle will be invalid until resource loading is 
		 * done. Use ResourceHandle<T>::isLoaded to check if resource has been loaded, or 
		 * ResourceHandle<T>::blockUntilLoaded to wait until load completes.
		 *
		 * @param[in]	filePath	Full pathname of the file.
		 * @param[in]	loadFlags	Flags used to control the load process.
		 *			
		 * @see		load(const Path&, ResourceLoadFlags)
		 */
		HResource loadAsync(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc loadAsync */
		template <class T>
		ResourceHandle<T> loadAsync(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return static_resource_cast<T>(loadAsync(filePath, loadFlags));
		}

		/**
		 * Loads the resource with the given UUID. Returns an empty handle if resource can't be loaded.
		 *
		 * @param[in]	uuid		UUID of the resource to load.
		 * @param[in]	async		If true resource will be loaded asynchronously. Handle to non-loaded resource will be
		 *							returned immediately while loading will continue in the background.		
		 * @param[in]	loadFlags	Flags used to control the load process.
		 *													
		 * @see		load(const Path&, bool)
		 */
		HResource loadFromUUID(const UUID& uuid, bool async = false, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/**
		 * Releases an internal reference to the resource held by the resources system. This allows the resource to be 
		 * unloaded when it goes out of scope, if the resource was loaded with @p keepInternalReference parameter.
		 *
		 * Alternatively you can also skip manually calling release() and call unloadAllUnused() which will unload all 
		 * resources that do not have any external references, but you lose the fine grained control of what will be 
		 * unloaded.
		 *			
		 * @param[in]	resource	Handle of the resource to release.
		 */
		void release(ResourceHandleBase& resource);

		/**
		 * Finds all resources that aren't being referenced outside of the resources system and unloads them.
		 * 			
		 * @see		release(ResourceHandleBase&)
		 */
		void unloadAllUnused();

		/** Forces unload of all resources, whether they are being used or not. */
		void unloadAll();

		/**
		 * Saves the resource at the specified location.
		 *
		 * @param[in]	resource 	Handle to the resource.
		 * @param[in]	filePath 	Full pathname of the file to save as.
		 * @param[in]	overwrite	If true, any existing resource at the specified location will be overwritten.
		 * @param[in]	compress	Should the resource be compressed before saving. Some resource have data that is already
		 *							compressed and this option will be ignored for such resources.
		 * 			
		 * @note
		 * If the resource is a GpuResource and you are in some way modifying it from the core thread, make sure all those
		 * commands are submitted before you call this method. Otherwise an obsolete version of the resource might get saved.
		 * @note
		 * If saving a core thread resource this is a potentially very slow operation as we must wait on the core thread 
		 * and the GPU in order to read the resource.
		 */
		void save(const HResource& resource, const Path& filePath, bool overwrite, bool compress = false);

		/**
		 * Saves an existing resource to its previous location.
		 *
		 * @param[in]	resource 	Handle to the resource.
		 * @param[in]	compress	Should the resource be compressed before saving. Some resource have data that is already
		 *							compressed and this option will be ignored for such resources.
		 *
		 * @note	
		 * If the resource is a GpuResource and you are in some way modifying it from the Core thread, make sure all those
		 * commands are submitted before you call this method. Otherwise an obsolete version of the resource might get saved.
		 * @note
		 * If saving a core thread resource this is a potentially very slow operation as we must wait on the core thread 
		 * and the GPU in order to read the resource.
		 */
		void save(const HResource& resource, bool compress = false);

		/**
		 * Updates an existing resource handle with a new resource. Caller must ensure that new resource type matches the 
		 * original resource type.
		 */
		void update(HResource& handle, const SPtr<Resource>& resource);

		/**
		 * Returns a list of dependencies from the resources at the specified path. Resource will not be loaded or parsed, 
		 * but instead the saved list of dependencies will be read from the file and returned.
		 *
		 * @param[in]	filePath	Full path to the resource to get dependencies for.
		 * @return					List of dependencies represented as UUIDs.
		 */
		Vector<UUID> getDependencies(const Path& filePath);

		/**
		 * Checks is the resource with the specified UUID loaded.
		 *
		 * @param[in]	uuid			UUID of the resource to check.
		 * @param[in]	checkInProgress	Should this method also check resources that are in progress of being 
		 *								asynchronously loaded.					
		 * @return						True if loaded or loading in progress, false otherwise.
		 */
		bool isLoaded(const UUID& uuid, bool checkInProgress = true);

		/**
		 *Allows you to set a resource manifest containing UUID <-> file path mapping that is used when resolving 
		 * resource references.
		 *
		 * @note	
		 * If you want objects that reference resources (using ResourceHandles) to be able to find that resource even after
		 * application restart, then you must save the resource manifest before closing the application and restore it 
		 * upon startup. Otherwise resources will be assigned brand new UUIDs and references will be broken.
		 */
		void registerResourceManifest(const SPtr<ResourceManifest>& manifest);

		/**	Unregisters a resource manifest previously registered with registerResourceManifest(). */
		void unregisterResourceManifest(const SPtr<ResourceManifest>& manifest);

		/**
		 * Allows you to retrieve resource manifest containing UUID <-> file path mapping that is used when resolving 
		 * resource references.
		 * 			
		 * @note	
		 * Resources module internally holds a "Default" manifest that it automatically updated whenever a resource is saved.
		 *
		 * @see		registerResourceManifest
		 */
		SPtr<ResourceManifest> getResourceManifest(const String& name) const;

		/** Attempts to retrieve file path from the provided UUID. Returns true if successful, false otherwise. */
		bool getFilePathFromUUID(const UUID& uuid, Path& filePath) const;

		/** Attempts to retrieve UUID from the provided file path. Returns true if successful, false otherwise. */
		bool getUUIDFromFilePath(const Path& path, UUID& uuid) const;

		/**
		 * Called when the resource has been successfully loaded. 
		 *
		 * @note	
		 * It is undefined from which thread this will get called from. Most definitely not the sim thread if resource was
		 * being loaded asynchronously.
		 */
		Event<void(const HResource&)> onResourceLoaded;

		/**
		 * Called when the resource has been destroyed. Provides UUID of the destroyed resource.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		Event<void(const UUID&)> onResourceDestroyed;

		/**
		 * Called when the internal resource the handle is pointing to has changed.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		Event<void(const HResource&)> onResourceModified;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new resource handle from a resource pointer. 
		 *
		 * @note	Internal method used primarily be resource factory methods.
		 */
		HResource _createResourceHandle(const SPtr<Resource>& obj);

		/**
		 * Creates a new resource handle from a resource pointer, with a user defined UUID. 
		 *
		 * @note	Internal method used primarily be resource factory methods.
		 */
		HResource _createResourceHandle(const SPtr<Resource>& obj, const UUID& UUID);

		/** Returns an existing handle for the specified UUID if one exists, or creates a new one. */
		HResource _getResourceHandle(const UUID& uuid);

		/** @} */
	private:
		friend class ResourceHandleBase;

		/**
		 * Starts resource loading or returns an already loaded resource. Both UUID and filePath must match the	same 
		 * resource, although you may provide an empty path in which case the resource will be retrieved from memory if its
		 * currently loaded.
		 */
		HResource loadInternal(const UUID& UUID, const Path& filePath, bool synchronous, ResourceLoadFlags loadFlags);

		/** Performs actually reading and deserializing of the resource file. Called from various worker threads. */
		SPtr<Resource> loadFromDiskAndDeserialize(const Path& filePath, bool loadWithSaveData);

		/**	Triggered when individual resource has finished loading. */
		void loadComplete(HResource& resource);

		/**	Callback triggered when the task manager is ready to process the loading task. */
		void loadCallback(const Path& filePath, HResource& resource, bool loadWithSaveData);

		/**	Destroys a resource, freeing its memory. */
		void destroy(ResourceHandleBase& resource);

	private:
		Vector<SPtr<ResourceManifest>> mResourceManifests;
		SPtr<ResourceManifest> mDefaultResourceManifest;

		Mutex mInProgressResourcesMutex;
		Mutex mLoadedResourceMutex;

		UnorderedMap<UUID, WeakResourceHandle<Resource>> mHandles;
		UnorderedMap<UUID, LoadedResourceData> mLoadedResources;
		UnorderedMap<UUID, ResourceLoadData*> mInProgressResources; // Resources that are being asynchronously loaded
		UnorderedMap<UUID, Vector<ResourceLoadData*>> mDependantLoads; // Allows dependency to be notified when a dependant is loaded
	};

	/** Provides easier access to Resources manager. */
	BS_CORE_EXPORT Resources& gResources();

	/** @} */
}