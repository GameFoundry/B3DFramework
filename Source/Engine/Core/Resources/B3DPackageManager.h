//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	class PackageResourceMetaData;
	class Package;

	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/** Information about a package managed by PackageManager. */
	struct RuntimePackageInformation
	{
		SPtr<Package> LoadedPackage;
		Path PhysicalPath;
		Path VirtualPathPrefix;
		bool AcquiredWriteLock = true;
		u32 AcquiredReadLockCount = 0;
		Signal LoadSignal;

#if B3D_BUILD_TYPE_DEVELOPMENT
		TInlineArray<const char*, 2> ReadLockReasons;
		const char* WriteLockReason = nullptr;
#endif
	};

	/** @} */

	/** @addtogroup Resources
	 *  @{
	 */

	/** Options that control how are package read locks acquired. */
	struct AcquirePackageReadLockOptions
	{
		AcquirePackageReadLockOptions(bool loadIfMissing = true, bool blockUntilAcquired = true, const char* lockReason = nullptr)
			: LoadIfMissing(loadIfMissing), BlockUntilAcquired(blockUntilAcquired), LockReason(lockReason)
		{ }

		bool LoadIfMissing; /**< If package cannot be found in the loaded package list, initiate a load operation. */
		bool BlockUntilAcquired; /**< Block the calling thread until the lock can be acquired. If false, locking will fail if someone else is holding a write lock. */
		/**
		 * If non-empty, this path will be added as a prefix to all resources within the package, so they may be looked up using
		 * this path. (e.g. "/Game/Textures/" prefix path will allow loading of a resource within the package named "BrickAlbedo"
		 * via "/Game/Textures/BrickAlbedo" path). Virtual package path prefix is only relevant when a package is being
		 * registered with the package manager.
		 */
		Path VirtualPathPrefix;
		const char* LockReason; /**< Reason for acquiring the lock, for debugging only. */
	};

	/** Options that control how are package write locks acquired. */
	struct AcquirePackageWriteLockOptions
	{
		AcquirePackageWriteLockOptions(bool allowCreateNew = false, bool blockUntilAcquired = true, const char* lockReason = nullptr)
			: AllowCreateNew(allowCreateNew), BlockUntilAcquired(blockUntilAcquired) , LockReason(lockReason)
		{ }

		bool AllowCreateNew; /**< If true, write operation can be acquired for a non-existing package (e.g. for when saving a brand new package). */
		bool BlockUntilAcquired; /**< Block the calling thread until the lock can be acquired. If false, locking will fail if some else is holding a read or write lock. */
		Path VirtualPathPrefix; /**< @see AcquirePackageReadLockOptions::VirtualPathPrefix. */
		const char* LockReason; /**< Reason for acquiring the lock, for debugging only. */
	};

	/** Options that control package save operation. */
	struct PackageManagerSavePackageOptions
	{
		bool Overwrite = true; /**< If set, save operation will overwrite any existing package at the provided path. */
		bool Compress = true; /**< If set, compression will be used on package data when saving. */
		bool CopyLoadStatesOnOverwrite = false; /**< If overwriting a package and this is true, load states will be copied from the original package into the package we're saving. */
		Path VirtualPathPrefix; /**< @see AcquirePackageReadLockOptions::VirtualPathPrefix. */
		u32 MetaDataPaddingByteCount = false; /**< Adds extra free space after package meta-data so we can update package meta-data without having to re-write the whole package (as long as new meta-data fits). */
	};

	/** Potential resulting status codes when attempting to acquire a package lock. */
	enum class AcquirePackageLockResult
	{
		Acquired, /**< Lock was acquired successfully. */
		NotAcquiredWriteLockHeld, /**< Lock was not acquired because someone else is holding a write lock. */
		NotAcquiredReadLockHeld,  /**< Lock was not acquired because someone else is holding a read lock. */
		NotAcquiredPackageNotFound, /**< Lock was not acquired because the package was not found. */
	};

	/** Information about an acquired package read lock. */
	struct B3D_EXPORT PackageReadLock
	{
		PackageReadLock(RuntimePackageInformation& runtimePackageInformation, Mutex& lockMutex, const char* lockReason);
		~PackageReadLock();

		/** Returns the package this lock is acquired for. */
		const SPtr<Package>& GetPackage() const { return RuntimePackageInformation.LoadedPackage; }

		RuntimePackageInformation& RuntimePackageInformation;
		Mutex& LockMutex;

#if B3D_BUILD_TYPE_DEVELOPMENT
		const char* LockReason;
#endif
	};

	/** Information about an acquired package write lock. */
	struct B3D_EXPORT PackageWriteLock
	{
		PackageWriteLock(RuntimePackageInformation* runtimePackageInformation, Mutex& lockMutex, const char* lockReason);
		~PackageWriteLock();

		/** Returns the package this lock is acquired for. */
		const SPtr<Package>& GetPackage() const;

		/** Clears the package information. To be called when a package has been deleted while holding the write lock. */
		void ClearPackageInformation() { RuntimePackageInformation = nullptr; }

		RuntimePackageInformation* RuntimePackageInformation;
		Mutex& LockMutex;
	};

	/** Contains a path to a resource within a package. */
	struct ResourcePackagePath
	{
		Path PhysicalPackagePath; /**< Absolute physical path to the package containing the resource. */
		Path ResourcePathWithinPackage; /**< Path to the resource within the package. */
	};

	/**
	 * Handles loading, unloading and saving of packages.
	 *
	 * @note All methods are thread-safe unless otherwise noted.
	 */
	class B3D_EXPORT PackageManager : public Module<PackageManager>
	{
	public:
		/**
		 * Loads a package from the provided @p packagePhysicalPath on disk. The package will remain loaded until explicitly unloaded via a call to Unload().
		 * If you request a package that was previously loaded, the existing package will be returned.
		 *
		 * @param	packagePhysicalPath		Path on disk from where to load the package.
		 * @param	virtualPathPrefix		If non-empty, this path will be added as a prefix to all resources within the package, so they may be loaded using
		 *									this path. (e.g. "/Game/Textures/" prefix path will allow loading of a resource within the package named "BrickAlbedo"
		 *									via "/Game/Textures/BrickAlbedo" path). Provided virtual prefix path is ignored if the package is previously loaded.
		 * @return							Lock that can be used for accessing the newly or previously loaded package, if successful. Null otherwise. Lock
		 *									must be kept alive as long as you are accessing the underlying package.
		 */
		UPtr<PackageReadLock> LoadOrGetPackage(const Path& packagePhysicalPath, const Path& virtualPathPrefix = Path::kBlank);

		/**
		 * Loads all packages within the folder. All loaded packages will remain loaded until explicitly unloaded via a call to Unload(). If a package is already
		 * loaded, no operation will be performed for that package.
		 *
		 * @param	folderPath					Path to the folder which to search for packages.
		 * @param	recursive					If true, child folders of @p folderPath will also be searched for packages to load.
		 * @param	virtualPathPrefix			If non-empty, this path will be added as a prefix to all resources within the package, so they may be loaded using
		 *										this path. (e.g. "/Game/Textures/" prefix path will allow loading of a resource within the package named "BrickAlbedo"
		 *										via "/Game/Textures/BrickAlbedo" path). Provided virtual prefix path is ignored if the package is previously loaded.
		 * @param	addSubFoldersToVirtualPath	If this is true, then any sub-folders found in @p folderPath will be appended to @p virtualPathPrefix, for any
		 *										resources in those folders. If false, all resources will be registered directly with @p virtualPathPrefix
		 *										virtual path, provided it is not empty.
		 */
		void LoadPackages(const Path& folderPath, bool recursive = true, const Path& virtualPathPrefix = Path::kBlank, bool addSubFoldersToVirtualPath = true);

		/**
		 * Saves a package to a provided location on disk.
		 *
		 * @param package			Package to save. 
		 * @param destinationPath	Path (including file name) to save the package to.
		 * @param options			Options to control the save process.
		 * @return					Write lock on the newly acquired package.
		 *
		 * @note	If the package you are trying to save is managed by the package manager, then a read lock for the package must be acquired.
		 *			If this is a brand new package not yet registered with the package manager, package can be provided as is, as long as you ensure
		 *			no other operations are being performed on the package.
		 *			If a read lock for a package is acquired, the save operation at that same location will result in a deadlock.
		 */
		UPtr<PackageWriteLock> SavePackage(const SPtr<Package>& package, const Path& destinationPath, const PackageManagerSavePackageOptions& options);

		/**
		 * Unloads a package at the specified path.
		 *
		 * @param	packagePath		Physical path to the package to unload.
		 */
		void UnloadPackage(const Path& packagePath);

		/**
		 * Moves or renames an existing package to a new physical location while keeping all the package references intact.
		 *
		 * @param	packageWriteLock	Write lock for the package to update the physical path for.
		 * @param	newPath				New physical path for the package.
		 */
		void ChangePhysicalPackagePath(const PackageWriteLock& packageWriteLock, const Path& newPath);

		/**
		 * Changes the virtual paths for all the resources in the provided package. You should call this if you want to change the virtual path prefix
		 * for a package, but also if any of the resource paths within the package change.
		 *
		 * @param	packageWriteLock		Write lock for the package to update the virtual path for.
		 * @param	newVirtualPathPrefix	New virtual path for the package.
		 */
		void ChangeVirtualPackagePath(const PackageWriteLock& packageWriteLock, const Path& newVirtualPathPrefix);

		/**
		 * Updates meta-data of a previously saved package.
		 * 
		 * @param	packageWriteLock	Write lock for the package to update the meta-data for.
		 * @return						True if no errors occurred during the process. False if errors occurred, or if the meta-data doesn't fit in the package,
		 *								in which case you should re-attempt a full save.
		 */
		bool SavePackageMetaData(const PackageWriteLock& packageWriteLock);

		/**
		 * Attempts to lock a package for reading. Locking the package before performing read operations ensures that the
		 * package is not modified or destroying while a read operation is in progress. 
		 *
		 * @param	physicalPackagePath			Absolute path to the package to lock.
		 * @param	options						Options that control how is the lock acquired.
		 * @param	outLock						Acquired lock, if successful. Package will remain locked until this object goes out of scope.
		 * @return								Information if the lock was acquired or not.
		 */
		AcquirePackageLockResult AcquireReadLock(const Path& physicalPackagePath, const AcquirePackageReadLockOptions& options, UPtr<PackageReadLock>& outLock);

		/**
		 * Attempts to lock a package for writing. Locking the package before performing write operations ensures that any existing read or write operations
		 * are allowed to finish, and that any new read or write operations do not start until the write lock is released.
		 *
		 * @param	physicalPackagePath			Absolute path to the package to lock.
		 * @param	options						Options that control how is the lock acquired.
		 * @param	outLock						Acquired lock, if successful. Package will remain locked until this object goes out of scope.
		 * @return								Information if the lock was acquired or not.
		 */
		AcquirePackageLockResult AcquireWriteLock(const Path& physicalPackagePath, const AcquirePackageWriteLockOptions& options, UPtr<PackageWriteLock>& outLock);

		/**
		 * Resolves a physical path to a resource into a physical path to the package, and a path to the resource within the package.
		 *
		 * @param physicalResourcePath		Physical path to the resource in the package, e.g. 'D:/path/to/package.b3d/path/to/resource'. Must be absolute.
		 * @return							If the provided path is valid, returns a structure with a physical path to the package, and path to the resource
		 *									within that package. Returns null otherwise.
		 */
		TOptional<ResourcePackagePath> TryResolvePhysicalResourcePath(const Path& physicalResourcePath) const;

		/**
		 * Resolves a virtual path to a resource into a physical path to the package, and a path to the resource within the package.
		 *
		 * @param virtualResourcePath		Virtual path to the resource, e.g. '/game/textures/path/to/resource'.
		 * @return							If provided path can be resolved, returns a structure with a physical path to the package, and path to the resource
		 *									within that package. Returns null otherwise.
		 */
		TOptional<ResourcePackagePath> TryResolveVirtualResourcePath(const Path& virtualResourcePath) const;

		/**
		 * Attempts to retrieve a path to the package the resource is located in.
		 *
		 * @param resourceId		ID of the resource.
		 * @return					Path to the package if the resource was located, or null otherwise.
		 */
		TOptional<Path> TryGetPackagePathForResource(const UUID& resourceId);

		/** Retrieves resource meta-data from the associated (previously loaded) package. Returns null if resource cannot be found. */
		SPtr<const PackageResourceMetaData> GetResourceMetaData(const UUID& resourceId);

	private:
		/**
		 * Registers all required resource path/id mappings for all resources in the package. Package read or write lock must be acquired, and
		 * package manager mutex must be locked by the caller.
		 */
		void LoadPackageResourceInformation(Package& package, const Path& physicalPackagePath, const Path& virtualPathPrefix);

		/**
		 * Clears all resource path/id mappings for all resources in the package. Package read or write lock must be acquired, and
		 * package manager mutex must be locked by the caller.
		 */
		void ClearPackageResourceInformation(Package& package, const Path& virtualPathPrefix);

		UnorderedMap<Path, SPtr<RuntimePackageInformation>> mPackagesByPath;
		UnorderedMap<UUID, RuntimePackageInformation*> mPackagesById;

		UnorderedMap<UUID, UUID> mResourceIdToPackageId;
		UnorderedMap<Path, ResourcePackagePath> mVirtualPathToResourcePackagePath;

		mutable Mutex mMutex;
	};

	/** Provides easier access to PackageManager. */
	B3D_EXPORT PackageManager& GetPackageManager();

	/** @} */
} // namespace b3d
