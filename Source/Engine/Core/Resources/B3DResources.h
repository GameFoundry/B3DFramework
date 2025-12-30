//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DPackageManager.h"
#include "Threading/B3DSignalEvent.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	struct PackageReadLock;
}

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Options that may be used to customize resource load operation. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), API(Engine)) ResourceLoadOptions
	{
		ResourceLoadOptions(bool asynchronousLoad = true, bool loadDependencies = true, bool keepInternalReference = true)
			: AsynchronousLoad(asynchronousLoad), LoadDependencies(loadDependencies), KeepInternalReference(keepInternalReference)
		{ }

		bool AsynchronousLoad;
		bool LoadDependencies;
		bool KeepInternalReference;

		static const ResourceLoadOptions kDefault;
	};

	/** Options that control resource save operation. */
	struct ResourceSaveOptions
	{
		ResourceSaveOptions(bool overwrite = true, bool compress = true, const Path& virtualPathPrefix = Path::kBlank)
			:Overwrite(overwrite), Compress(compress), VirtualPathPrefix(virtualPathPrefix)
		{ }

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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Resources), API(Engine)) Resources : public Module<Resources>
	{
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

	public:
		Resources() = default;
		~Resources();

		/**
		 * Loads a resource at the specified path. Resources are searched in all currently loaded packages within the PackageManager,
		 * as well as any in-memory resources registered with the Resources manager.
		 *
		 * @param resourcePath			Path to the resource. This may be a virtual or physical path. e.g.:
		 *									Virtual path: '/game/textures/path/to/resource'
		 *									Physical path: 'D:/path/to/package.b3d/path/to/resource'
		 * @param loadOptions			Options to control the loading process.
		 * @return						Handle to the resource. Note if performing async loading this method will return immediately, but
		 *								the resource may not yet be loaded. Returns null if resource cannot be loaded, and logs why it failed.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		HResource Load(const Path& resourcePath, const ResourceLoadOptions& loadOptions);

		/**
		 * Loads a resource with the specified ID. Resources are searched in all currently loaded packages within the PackageManager,
		 * as well as any in-memory resources registered with the Resources manager.
		 *
		 * @param resourceId			ID of the resource.
		 * @param loadOptions			Options to control the loading process.
		 * @return						Handle to the resource. Note if performing async loading this method will return immediately, but
		 *								the resource may not yet be loaded. Returns null if resource cannot be loaded, and logs why it failed.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		HResource Load(const UUID& resourceId, const ResourceLoadOptions& loadOptions);

		/** @copydoc LoadFromPackage(const Path&, const ResourceLoadOptions&) */
		template <class T>
		TResourceHandle<T> Load(const Path& resourcePath, const ResourceLoadOptions& loadOptions)
		{
			return B3DStaticResourceCast<T>(Load(resourcePath, loadOptions));
		}

		/** @copydoc LoadFromPackage(const UUID&, const ResourceLoadOptions&) */
		template <class T>
		TResourceHandle<T> Load(const UUID& resourceId, const ResourceLoadOptions& loadOptions)
		{
			return B3DStaticResourceCast<T>(Load(resourceId, loadOptions));
		}

		/**
		 * Checks if the resource at the provided path exists. 
		 *
		 * @param resourcePath			Path to the resource. This may be a virtual or physical path. e.g.:
		 *									Virtual path: '/game/textures/path/to/resource'
		 *									Physical path: 'D:/path/to/package.b3d/path/to/resource'
		 * @return						True if the resource can be located, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool Exists(const Path& resourcePath) const;

		/**
		 * Checks if the resource with the provided ID exists. 
		 *
		 * @param resourceId			ID of the resource.
		 * @return						True if the resource can be located, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool Exists(const UUID& resourceId) const;

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
		 * Saves a resource into its own package. The package will be created in @p folder, with @p name as the package name. There will
		 * be a single resource in the package, also named @p name.
		 *
		 * @param	resource		Resource to save.
		 * @param	folder			Absolute path to a folder in which to create the package.
		 * @param	name			Name of the package to create, as well as the name of the resource within the package.
		 * @param	saveOptions		Options to control the save operation.
		 */
		//B3D_SCRIPT_EXPORT()
		void SaveAsSinglePackage(const HResource& resource, const Path& folder, const String& name, const ResourceSaveOptions& saveOptions = ResourceSaveOptions());

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
		 * Returns the loading progress of a resource that's being loaded
		 *
		 * @param	resource	Resource whose load progress to check.
		 * @return				Load progress in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT()
		float GetLoadProgress(const HResource& resource);

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
		 * Updates all resources from the resource data in the package locked by the provided write lock. This means if a resource is already loaded by the resource system, the
		 * resource will be retrieved from the package and handle to the resource updated with the new resource. This may involve loading the resource, if the new package doesn't
		 * have the resource loaded. If the resource is loaded in the package, but not marked as loaded by the resource system, it will be unloaded from the package.
		 */
		void UpdateResourcesFromPackage(const UPtr<PackageWriteLock>& packageWriteLock);

		/** Updates an existing resource handle with a new resource. Caller must ensure that new resource type matches the original resource type. */
		void UpdateHandle(HResource& handle, const SPtr<Resource>& resource);

		/** Triggered when the last resource handle for a particular resource goes out of scope. */
		void DestroyHandleData(ResourceHandleData& handleData);

		/** @} */
	private:
		friend class ResourceHandle;
		friend struct ResourceHandleData;

		/** Stores load progress for a single resource. */
		struct LoadProgress
		{
			LoadProgress(u64 totalSize = 0, float progress = 0.0f)
				:TotalSize(totalSize), Progress(progress)
			{ }

			u64 TotalSize; /**< Total size of the resource, in bytes. Excludes dependencies (only self size). */
			float Progress; /**< Current loading progress, in range [0, 1]. */
		};

		/**
		 * Calculates the load progress of the provided resource and appends the progress information to the @p loadProgressMap.
		 * Then calls this method recursively for any in-progress dependency loads. If the resource is already fully loaded, or
		 * not being loaded at all the LoadProgress structure will not contain a valid size value.
		 */
		void GetLoadProgressRecursive(const HResource& resource, UnorderedMap<UUID, LoadProgress>& loadProgressMap);

		/**
		 * Performs a load of the provided resource from the package locked by the write lock. If requested this will also trigger a load
		 * of all dependencies of the resource. Triggers TryFinalizeLoad() when a resource or any of its dependencies complete loading.
		 *
		 * This is an internal method to be shared by public Load() overloads.
		 */
		HResource LoadFromPackage(UPtr<PackageReadLock> packageReadLock, const UUID& resourceId, const ResourceLoadOptions& loadOptions);

		/**
		 * Checks if the provided in-progress load has completed any finalizes the operation. Operation is deemed complete once its primary resource and
		 * all dependencies (and their dependencies) have finished loading. At that point we will clear the in-progress load map and add the resource
		 * into the loaded resource map. External code will be notified that load completed, and if any other resource is waiting on this resource to
		 * finish loading, they will be notified so they may try to finalize their operations as well.
		 */
		void TryFinalizeLoad(const SPtr<InProgressLoadInformation>& inProgressLoadInformation);

		/**	Destroys a resource, freeing its memory. */
		void Destroy(ResourceHandleData& handleData);

	private:
		mutable Mutex mLoadedResourceMutex;
		Mutex mResourceHandleMutex;

		UnorderedMap<UUID, ResourceHandleData*> mHandles;
		UnorderedMap<UUID, UPtr<LoadedResourceInformation>> mLoadedResourceInformation;
		UnorderedMap<UUID, TInlineArray<SPtr<InProgressLoadInformation>, 1>> mInProgressLoadInformation;
		UnorderedMap<UUID, TInlineArray<SPtr<InProgressLoadInformation>, 4>> mDependantResourceLoads;
	};

	/** Provides easier access to Resources manager. */
	B3D_EXPORT Resources& GetResources();

	/** @} */
} // namespace b3d
