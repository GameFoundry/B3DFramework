//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsPackageManager.h"
#include "Threading/BsSignalEvent.h"
#include "Utility/BsModule.h"

namespace bs
{
	struct PackageReadLock;
}

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Flags that can be used to control resource loading. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Resources)) ResourceLoadFlag
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
	B3D_FLAGS_OPERATORS(ResourceLoadFlag);

	/** Options that may be used to customize resource load operation. */
	struct ResourceLoadOptions
	{
		ResourceLoadOptions()
			: LoadDependencies(true), KeepInternalReference(true), AsynchronousLoad(true)
		{ }

		u8 LoadDependencies : 1;
		u8 KeepInternalReference : 1;
		u8 AsynchronousLoad : 1;
	};

	/** Options that control resource save operation. */
	struct ResourceSaveOptions
	{
		bool Overwrite = true; /**< If set, save operation will overwrite any existing resource at the provided path. */
		bool Compress = true; /**< If set, compression will be used on resource data when saving. */
		/**
		 * If non-empty, this path can be used for loading the resource. (e.g. "/Game/Textures/" prefix path will allow loading of a
		 * resource named "BrickAlbedo" via "/Game/Textures/BrickAlbedo" path). 
		 */
		Path VirtualPathPrefix;
	};

	/**
	 * Manager for dealing with all engine resources. It allows you to save new resources and load existing ones.
	 *
	 * @note	Main thread only.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Resources), API(Framework)) Resources : public Module<Resources>
	{
		/** Information about a loaded resource. */
		struct LoadedResourceData // TODO - Deprecated
		{
			LoadedResourceData() = default;

			LoadedResourceData(const TWeakResourceHandle<Resource>& resource, u32 size)
				: Resource(resource), Size(size)
			{}

			TWeakResourceHandle<Resource> Resource;
			u32 NumInternalRefs = 0;
			u32 Size = 0;
		};

		/** Information about a resource that's currently being loaded. */
		struct ResourceLoadData // TODO - Deprecated
		{
			ResourceLoadData(const TWeakResourceHandle<Resource>& resource, u32 numDependencies, u32 size)
				: ResData(resource, size), RemainingDependencies(numDependencies)
			{}

			LoadedResourceData ResData;
			SPtr<Resource> LoadedData;
			u32 RemainingDependencies;
			Vector<HResource> Dependencies;
			bool NotifyImmediately;
			bool LoadStarted = false;
			SPtr<SignalEvent> LoadingEvent;

			// Progress reporting
			u32 DependencySize = 0;
			u32 DependencyLoadedAmount = 0;
			std::atomic<float> Progress;
		};

		/** Information about a loaded resource. */
		struct LoadedResourceInformation
		{
			TWeakResourceHandle<Resource> ResourceHandle;
			u32 InternalReferenceCount = 0;
			bool DependenciesLoaded = false;
		};

		/** Information about an in-progress resource load. */
		struct InProgressLoadInformation
		{
			InProgressLoadInformation()
				:LoadFinished(false)
			{ }

			ResourceLoadOptions LoadOptions;
			UPtr<PackageReadLock> PackageReadLock;

			HResource ResourceHandle;
			Vector<HResource> DependencyResourceHandles;

			u32 RemainingResourcesToLoadCount = 0;
			u8 LoadFinished : 1;
			SignalEvent LoadingEvent;
		};

		/** Information about an issued resource load. */
		struct LoadInfo // TODO - Deprecated
		{
			enum State
			{
				Loading,
				Failed,
				AlreadyInProgress,
				AlreadyLoaded
			};

			HResource Resource;
			u32 Size;
			State State;
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
		 * @see		ReleaseInternalReference(ResourceHandle&), UnloadAllUnused()
		 */
		B3D_SCRIPT_EXPORT()
		B3D_NO_RREF HResource Load(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc Load(const Path&, ResourceLoadFlags) */
		template <class T>
		TResourceHandle<T> Load(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return B3DStaticResourceCast<T>(Load(filePath, loadFlags));
		}

		/**
		 * Loads the resource for the provided weak resource handle, or returns a loaded resource if already loaded.
		 *
		 * @see		Load(const Path&, ResourceLoadFlags)
		 */
		HResource Load(const TWeakResourceHandle<Resource>& handle, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc Load(const WeakResourceHandle<Resource>&, ResourceLoadFlags) */
		template <class T>
		TResourceHandle<T> Load(const TWeakResourceHandle<T>& handle, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return static_resource_cast<T>(Load((const TWeakResourceHandle<Resource>&)handle, loadFlags));
		}

		/**
		 * Loads the resource asynchronously. Initially returned resource handle will be invalid until resource loading is
		 * done. Use ResourceHandle<T>::isLoaded to check if resource has been loaded, or
		 * ResourceHandle<T>::blockUntilLoaded to wait until load completes.
		 *
		 * @param[in]	filePath	Full pathname of the file.
		 * @param[in]	loadFlags	Flags used to control the load process.
		 *
		 * @see		Load(const Path&, ResourceLoadFlags)
		 */
		B3D_SCRIPT_EXPORT()
		HResource LoadAsync(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		/** @copydoc LoadAsync */
		template <class T>
		TResourceHandle<T> LoadAsync(const Path& filePath, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default)
		{
			return static_resource_cast<T>(LoadAsync(filePath, loadFlags));
		}

		/**
		 * Loads the resource with the given UUID. Returns an empty handle if resource can't be loaded.
		 *
		 * @param[in]	uuid		UUID of the resource to load.
		 * @param[in]	async		If true resource will be loaded asynchronously. Handle to non-loaded resource will be
		 *							returned immediately while loading will continue in the background.
		 * @param[in]	loadFlags	Flags used to control the load process.
		 *
		 * @see		Load(const Path&, bool)
		 */
		B3D_SCRIPT_EXPORT()
		HResource LoadFromUuid(const UUID& uuid, bool async = false, ResourceLoadFlags loadFlags = ResourceLoadFlag::Default);

		// TODO - Doc
		HResource Load(const Path& resourcePath, const ResourceLoadOptions& loadOptions);
		HResource Load(const UUID& resourceId, const ResourceLoadOptions& loadOptions);

		/**
		 * Releases an internal reference to the resource held by the resources system. This allows the resource to be
		 * unloaded when it goes out of scope, if the resource was loaded with @p KeepInternalReference option.
		 *
		 * Alternatively you can also skip manually calling ReleaseInternalReference() and call UnloadAllUnused() which will unload all
		 * resources that do not have any external references, but you lose the fine grained control of what will be
		 * unloaded.
		 */
		B3D_SCRIPT_EXPORT()
		void ReleaseInternalReference(const HResource& resource) { ReleaseInternalReference((ResourceHandle&)resource); }

		/** @copydoc ReleaseInternalReference(const HResource&) */
		void ReleaseInternalReference(ResourceHandle& resource);

		/**
		 * Finds all resources that aren't being referenced outside of the resources system and unloads them.
		 *
		 * @see		ReleaseInternalReference(const HResource&)
		 */
		B3D_SCRIPT_EXPORT()
		void UnloadAllUnused();

		/** Forces unload of all resources, whether they are being used or not. */
		B3D_SCRIPT_EXPORT()
		void UnloadAll();

		/**
		 * Saves the resource at the specified location.
		 *
		 * @param[in]	resource 	Handle to the resource.
		 * @param[in]	filePath 	Full pathname of the file to save as.
		 * @param[in]	overwrite	If true, any existing resource at the specified location will be overwritten.
		 * @param[in]	compress	Should the resource be compressed before saving. Some resources have data that is
		 *							already	compressed and this option will be ignored for such resources.
		 *
		 * @note
		 * If the resource is used on the GPU and you are in some way modifying it from the render thread, make sure all
		 * render thread commands are submitted and executed before you call this method. Otherwise an obsolete version of
		 * the resource might get saved.
		 * @note
		 * If saving a render thread resource this is a potentially very slow operation as we must wait on the render thread
		 * and the GPU in order to read the resource.
		 * @note
		 * Thread safe if you guarantee the resource isn't being written to from another thread.
		 */
		B3D_SCRIPT_EXPORT()
		void Save(B3D_NO_RREF const HResource& resource, const Path& filePath, bool overwrite, bool compress = false);

		/**
		 * Saves an existing resource to its previous location.
		 *
		 * @param[in]	resource 	Handle to the resource.
		 * @param[in]	compress	Should the resource be compressed before saving. Some resources have data that is
		 *							already compressed and this option will be ignored for such resources.
		 *
		 * @note
		 * If the resource is used on the GPU and you are in some way modifying it from the render thread, make sure all
		 * render thread commands are submitted and executed before you call this method. Otherwise an obsolete version of
		 * the resource might get saved.
		 * @note
		 * If saving a render thread resource this is a potentially very slow operation as we must wait on the render thread
		 * and the GPU in order to read the resource.
		 * @note
		 * Thread safe if you guarantee the resource isn't being written to from another thread.
		 */
		B3D_SCRIPT_EXPORT()
		void Save(B3D_NO_RREF const HResource& resource, bool compress = false);

		/**
		 * Saves a resource into its own package. The package will be created in @p folder, with @p name as the package name. There will
		 * be a single resource in the package, also named @p name.
		 *
		 * @param	resource		Resource to save.
		 * @param	folder			Absolute path to a folder in which to create the package.
		 * @param	name			Name of the package to create, as well as the name of the resource within the package.
		 * @param	saveOptions		Options to control the save operation.
		 */
		//B3D_SCRIPT_EXPORT()
		void SaveAsSinglePackage(const HResource& resource, const Path& folder, const String& name, const ResourceSaveOptions& saveOptions);

		/**
		 * Updates an existing resource handle with a new resource. Caller must ensure that new resource type matches the
		 * original resource type.
		 */
		void Update(HResource& handle, const SPtr<Resource>& resource);

		/**
		 * Returns a list of dependencies from the resources at the specified path. Resource will not be loaded or parsed,
		 * but instead the saved list of dependencies will be read from the file and returned.
		 *
		 * @param[in]	filePath	Full path to the resource to get dependencies for.
		 * @return					List of dependencies represented as UUIDs.
		 */
		B3D_SCRIPT_EXPORT()
		Vector<UUID> GetDependencies(const Path& filePath);

		/**
		 * Checks is the resource with the specified UUID loaded.
		 *
		 * @param[in]	uuid			UUID of the resource to check.
		 * @param[in]	checkInProgress	Should this method also check resources that are in progress of being
		 *								asynchronously loaded.
		 * @return						True if loaded or loading in progress, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsLoaded(const UUID& uuid, bool checkInProgress = true);

		/**
		 * Returns the loading progress of a resource that's being asynchronously loaded.
		 *
		 * @param[in]	resource				Resource whose load progress to check.
		 * @param[in]	includeDependencies		If false the progress will reflect the load progress only for this
		 *										inidividual resource. If true the progress will reflect load progress
		 *										of this resource and all of its dependencies.
		 * @return								Load progress in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT()
		float GetLoadProgress(const HResource& resource, bool includeDependencies = true); // TODO - Deprecated
		
		/**
		 * Returns the loading progress of a resource that's being loaded
		 *
		 * @param	resource	Resource whose load progress to check.
		 * @return				Load progress in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT()
		float GetLoadProgress2(const HResource& resource);

		struct LoadProgress
		{
			LoadProgress(u64 totalSize = 0, float progress = 0.0f)
				:TotalSize(totalSize), Progress(progress)
			{ }

			u64 TotalSize;
			float Progress;
		};
		void GetLoadProgressRecursive(const HResource& resource, UnorderedMap<UUID, LoadProgress>& loadProgressMap);

		/**
		 *Allows you to set a resource manifest containing UUID <-> file path mapping that is used when resolving
		 * resource references.
		 *
		 * @note
		 * If you want objects that reference resources (using ResourceHandles) to be able to find that resource even after
		 * application restart, then you must save the resource manifest before closing the application and restore it
		 * upon startup. Otherwise resources will be assigned brand new UUIDs and references will be broken.
		 */
		B3D_SCRIPT_EXPORT()
		void RegisterResourceManifest(const SPtr<ResourceManifest>& manifest); // TODO - Deprecated

		/**	Unregisters a resource manifest previously registered with registerResourceManifest(). */
		B3D_SCRIPT_EXPORT()
		void UnregisterResourceManifest(const SPtr<ResourceManifest>& manifest); // TODO - Deprecated

		/**
		 * Allows you to retrieve resource manifest containing UUID <-> file path mapping that is used when resolving
		 * resource references.
		 *
		 * @note
		 * Resources module internally holds a "Default" manifest that it automatically updated whenever a resource is saved.
		 *
		 * @see		registerResourceManifest
		 */
		B3D_SCRIPT_EXPORT()
		SPtr<ResourceManifest> GetResourceManifest(const String& name) const; // TODO - Deprecated

		/** Attempts to retrieve file path from the provided UUID. Returns true if successful, false otherwise. */
		B3D_SCRIPT_EXPORT()
		bool GetFilePathFromUuid(const UUID& uuid, Path& filePath) const;

		/** Attempts to retrieve UUID from the provided physical file path. Returns true if successful, false otherwise. */
		B3D_SCRIPT_EXPORT()
		bool GetUUIDFromFilePath(const Path& path, UUID& outUUID) const;

		/** Converts a potentially virtual path into a physical one. If the path is not virtual, returns the path as-is. */
		B3D_SCRIPT_EXPORT()
		Path EnsurePhysicalPath(const Path& path) const;

		/**
		 * Called when the resource has been successfully loaded.
		 *
		 * @note
		 * It is undefined from which thread this will get called from. Most definitely not the main thread if resource was
		 * being loaded asynchronously.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(B3D_NO_RREF const HResource&)> OnResourceLoaded;

		/**
		 * Called when the resource has been destroyed. Provides UUID of the destroyed resource.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(const UUID&)> OnResourceDestroyed;

		/**
		 * Called when the internal resource the handle is pointing to has changed.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(B3D_NO_RREF const HResource&)> OnResourceModified;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new resource handle from a resource pointer.
		 *
		 * @note	Internal method used primarily be resource factory methods.
		 */
		HResource CreateResourceHandle(const SPtr<Resource>& resource);

		/**
		 * Creates a new resource handle from a resource pointer, with a user defined UUID.
		 *
		 * @note	Internal method used primarily be resource factory methods.
		 */
		HResource CreateResourceHandle(const SPtr<Resource>& resource, const UUID& resourceId);

		/** Returns an existing handle for the specified UUID if one exists, or creates a new one. */
		HResource GetOrCreateResourceHandle(const UUID& resourceId);

		/**
		 * Same as save() except it saves the resource without registering it in the default manifest, requiring a handle,
		 * or checking for overwrite.
		 */
		void SaveInternal(const SPtr<Resource>& resource, const Path& filePath, bool compress);

		/**
		 * Updates all resources from the resource data in the package locked by the provided write lock. This means if a resource is already loaded by the resource system, the
		 * resource will be retrieved from the package and handle to the resource updated with the new resource. This may involve loading the resource, if the new package doesn't
		 * have the resource loaded.
		 */
		void UpdateResourcesFromPackage(const UPtr<PackageWriteLock>& packageWriteLock);

		/** Updates an existing resource handle with a new resource. Caller must ensure that new resource type matches the original resource type. */
		void UpdateHandle(HResource& handle, const SPtr<Resource>& resource);

		/** @} */
	private:
		friend class ResourceHandle;

		/**
		 * Starts resource loading or returns an already loaded resource. Both UUID and filePath must match the	same
		 * resource, although you may provide an empty path in which case the resource will be retrieved from memory if its
		 * currently loaded.
		 */
		LoadInfo LoadInternal(const UUID& UUID, const Path& filePath, bool synchronous, ResourceLoadFlags loadFlags); // TODO - Deprecated

		// TODO
		HResource Load(UPtr<PackageReadLock> packageReadLock, const UUID& resourceId, const ResourceLoadOptions& loadOptions);

		/** Performs actually reading and deserializing of the resource file. Called from various worker threads. */
		SPtr<Resource> LoadFromDiskAndDeserialize(const Path& filePath, bool loadWithSaveData, std::atomic<float>& progress); // TODO - Deprecated

		/**	Triggered when individual resource has finished loading. */
		void LoadComplete(HResource& resource, bool notifyProgress); // TODO - Deprecated

		/**	Callback triggered when the task manager is ready to process the loading task. */
		void LoadCallback(const Path& filePath, HResource& resource, bool loadWithSaveData); // TODO - Deprecated

		/**
		 * Checks if the provided in-progress load has completed any finalizes the operation. Operation is deemed complete once its primary resource and
		 * all dependencies (and their dependencies) have finished loading. At that point we will clear the in-progress load map and add the resource
		 * into the loaded resource map. External code will be notified that load completed, and if any other resource is waiting on this resource to
		 * finish loading, they will be notified so they may try to finalize their operations as well.
		 */
		void TryFinalizeLoad(const SPtr<InProgressLoadInformation>& inProgressLoadInformation);

		/**	Destroys a resource, freeing its memory. */
		void Destroy(ResourceHandle& resource);

	private:
		Vector<SPtr<ResourceManifest>> mResourceManifests; // TODO - Deprecated
		SPtr<ResourceManifest> mDefaultResourceManifest; // TODO - Deprecated

		Mutex mInProgressResourcesMutex; // TODO - Deprecated
		Mutex mLoadedResourceMutex; // TODO - Deprecated
		Mutex mDefaultManifestMutex; // TODO - Deprecated
		RecursiveMutex mDestroyMutex;

		UnorderedMap<UUID, TWeakResourceHandle<Resource>> mHandles;
		UnorderedMap<UUID, LoadedResourceData> mLoadedResources;
		UnorderedMap<UUID, ResourceLoadData*> mInProgressResources; // Resources that are being asynchronously loaded // TODO - Deprecated
		UnorderedMap<UUID, Vector<ResourceLoadData*>> mDependantLoads; // Allows dependency to be notified when a dependant is loaded // TODO - Deprecated

		// New package based code
		UnorderedMap<UUID, UPtr<LoadedResourceInformation>> mLoadedResourceInformation;
		UnorderedMap<UUID, TInlineArray<SPtr<InProgressLoadInformation>, 1>> mInProgressLoadInformation;
		UnorderedMap<UUID, TInlineArray<SPtr<InProgressLoadInformation>, 4>> mDependantResourceLoads;
	};

	/** Provides easier access to Resources manager. */
	B3D_CORE_EXPORT Resources& GetResources();

	/** @} */
} // namespace bs
