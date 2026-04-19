//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DPackageManager.h"

#include "FileSystem/B3DDataStream.h"
#include "Resources/B3DPackage.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

PackageReadLock::PackageReadLock(struct RuntimePackageInformation& runtimePackageInformation, Mutex& lockMutex, const char* lockReason)
	: RuntimePackageInformation(runtimePackageInformation), LockMutex(lockMutex)
#if B3D_BUILD_TYPE_DEVELOPMENT
	, LockReason(lockReason)
#endif
{
	// NOTE: Mutex must be locked at this point
	runtimePackageInformation.AcquiredReadLockCount++;

#if B3D_BUILD_TYPE_DEVELOPMENT
	runtimePackageInformation.ReadLockReasons.Add(lockReason);
#endif
}

PackageReadLock::~PackageReadLock()
{
	{
		Lock lock(LockMutex);

		if(B3D_ENSURE(RuntimePackageInformation.AcquiredReadLockCount > 0))
		{
			RuntimePackageInformation.AcquiredReadLockCount--;

#if B3D_BUILD_TYPE_DEVELOPMENT
			auto found = std::find(RuntimePackageInformation.ReadLockReasons.begin(), RuntimePackageInformation.ReadLockReasons.end(), LockReason);
			if(found != RuntimePackageInformation.ReadLockReasons.end())
				RuntimePackageInformation.ReadLockReasons.erase(found);
#endif
		}
	}

	RuntimePackageInformation.LoadSignal.NotifyAll();
}
PackageWriteLock::PackageWriteLock(struct RuntimePackageInformation* runtimePackageInformation, Mutex& lockMutex, const char* lockReason)
	: RuntimePackageInformation(runtimePackageInformation), LockMutex(lockMutex)
{
	// NOTE: Mutex must be locked at this point
#if B3D_BUILD_TYPE_DEVELOPMENT
	runtimePackageInformation->WriteLockReason = lockReason;
#endif
}

PackageWriteLock::~PackageWriteLock()
{
	Lock lock(LockMutex);

	if(RuntimePackageInformation == nullptr)
		return;

	B3D_ASSERT(RuntimePackageInformation->LoadedPackage != nullptr);
	B3D_ASSERT(RuntimePackageInformation->AcquiredReadLockCount == 0);

	RuntimePackageInformation->AcquiredWriteLock = false;

#if B3D_BUILD_TYPE_DEVELOPMENT
	RuntimePackageInformation->WriteLockReason = nullptr;
#endif

	RuntimePackageInformation->LoadSignal.NotifyAll();
}

const SPtr<Package>& PackageWriteLock::GetPackage() const
{
	static SPtr<Package> kNullPackage;
	return RuntimePackageInformation != nullptr ? RuntimePackageInformation->LoadedPackage : kNullPackage;
}

UPtr<PackageReadLock> PackageManager::LoadOrGetPackage(const Path& packagePhysicalPath, const Path& virtualPathPrefix)
{
	AcquirePackageReadLockOptions readLockOptions(true, true, "LoadOrGetPackage");
	readLockOptions.VirtualPathPrefix = virtualPathPrefix;

	UPtr<PackageReadLock> readLock;
	const AcquirePackageLockResult lockResult = AcquireReadLock(packagePhysicalPath, readLockOptions, readLock);

	if(lockResult == AcquirePackageLockResult::Acquired && readLock != nullptr)
		return std::move(readLock);

	return nullptr;
}

void PackageManager::LoadPackages(const Path& folderPath, bool recursive, const Path& virtualPathPrefix, bool addSubFoldersToVirtualPath)
{
	auto fnOnFileFound = [this, &folderPath, &virtualPathPrefix, addSubFoldersToVirtualPath](const Path& path) -> bool
	{
		if(path.GetExtension() == Package::kPackageExtension)
		{
			Path virtualPath = virtualPathPrefix;
			if(!virtualPathPrefix.IsEmpty() && addSubFoldersToVirtualPath)
			{
				const Path& relativePath = path.GetRelative(folderPath);

				if(relativePath.GetDirectoryCount() > 0)
					virtualPath = Path::Combine(virtualPathPrefix, relativePath.GetDirectory());
			}

			UPtr<PackageReadLock> readLock = LoadOrGetPackage(path, virtualPath);
			(void)readLock;
		}

		return true;
	};

	FileSystem::Iterate(folderPath, fnOnFileFound, nullptr, recursive);
}

UPtr<PackageWriteLock> PackageManager::SavePackage(const SPtr<Package>& package, const Path& destinationPath, const PackageManagerSavePackageOptions& options)
{
	if(package == nullptr)
	{
		B3D_LOG(Error, LogResources, "Cannot save package. Provided package for path '{0}' is empty.", destinationPath);
		return nullptr;
	}

	if(!destinationPath.IsAbsolute())
	{
		B3D_LOG(Error, LogResources, "Cannot save package. Provided path '{0}' is not absolute.", destinationPath);
		return nullptr;
	}

	const bool destinationFileAlreadyExists = FileSystem::IsFile(destinationPath);
	if(destinationFileAlreadyExists && !options.Overwrite)
	{
		B3D_LOG(Error, LogResources, "Cannot save package. File exists at provided path '{0}'.", destinationPath);
		return nullptr;
	}

	const Path& temporarySavePath = FileSystem::GetUniqueTemporaryFilePath();
	SPtr<DataStream> temporaryPackageStream = FileSystem::CreateAndOpenFile(temporarySavePath);

	SavePackageOptions savePackageOptions;
	savePackageOptions.CompressResources = options.Compress;
	savePackageOptions.MetaDataPaddingByteCount = options.MetaDataPaddingByteCount;

	package->Save(temporaryPackageStream, savePackageOptions);
	temporaryPackageStream->Close();

	AcquirePackageWriteLockOptions writeLockOptions(true);
	writeLockOptions.VirtualPathPrefix = options.VirtualPathPrefix;

	UPtr<PackageWriteLock> packageWriteLock;
	const AcquirePackageLockResult acquireLockResult = AcquireWriteLock(destinationPath, writeLockOptions, packageWriteLock);

	if(!B3D_ENSURE(acquireLockResult == AcquirePackageLockResult::Acquired && packageWriteLock != nullptr))
	{
		FileSystem::Remove(temporarySavePath);
		return nullptr;
	}

	FileSystem::Remove(destinationPath);

	const Path& destinationParentFolderPath = destinationPath.GetParent();
	if(!FileSystem::Exists(destinationParentFolderPath))
		FileSystem::CreateFolder(destinationParentFolderPath);

	FileSystem::Move(temporarySavePath, destinationPath);

	package->AssociateFileWithPackage(destinationPath);

	const SPtr<Package> originalPackage = packageWriteLock->GetPackage();
	{
		Lock lock(mMutex);
		if(originalPackage != nullptr)
		{
			mPackagesById.erase(originalPackage->GetPackageId());
			ClearPackageResourceInformation(*originalPackage, packageWriteLock->RuntimePackageInformation->VirtualPathPrefix);

			if(options.CopyLoadStatesOnOverwrite)
				package->CopyResourceLoadStatesFromClone(*originalPackage);
		}

		mPackagesById[package->GetPackageId()] = packageWriteLock->RuntimePackageInformation;
		LoadPackageResourceInformation(*package, destinationPath, options.VirtualPathPrefix);

		packageWriteLock->RuntimePackageInformation->LoadedPackage = package;
		packageWriteLock->RuntimePackageInformation->VirtualPathPrefix = options.VirtualPathPrefix;
	}

	return packageWriteLock;
}

void PackageManager::UnloadPackage(const Path& packagePath)
{
	if(!packagePath.IsAbsolute())
	{
		B3D_LOG(Warning, LogResources, "Cannot delete package. Provided path '{0}' is not absolute.", packagePath);
		return;
	}

	UPtr<PackageWriteLock> packageWriteLock;
	const AcquirePackageLockResult acquireLockResult = AcquireWriteLock(packagePath, AcquirePackageWriteLockOptions(), packageWriteLock);

	if(acquireLockResult != AcquirePackageLockResult::Acquired)
		return;

	if(!B3D_ENSURE(packageWriteLock != nullptr))
		return;

	const SPtr<Package>& package = packageWriteLock->GetPackage();
	{
		Lock lock(mMutex);

		mPackagesById.erase(package->GetPackageId());
		ClearPackageResourceInformation(*package, packageWriteLock->RuntimePackageInformation->VirtualPathPrefix);

		package->AssociateFileWithPackage(Path::kBlank);
		mPackagesByPath.erase(packagePath);

		packageWriteLock->ClearPackageInformation();
	}
}

void PackageManager::ChangePhysicalPackagePath(const PackageWriteLock& packageWriteLock, const Path& newPath)
{
	if(!newPath.IsAbsolute())
	{
		B3D_LOG(Warning, LogResources, "Cannot change physical package path. Provided path '{0}' is not absolute.", newPath);
		return;
	}

	const Path& physicalPackagePath = packageWriteLock.RuntimePackageInformation->PhysicalPath;
	const SPtr<Package>& package = packageWriteLock.GetPackage();
	{
		Lock lock(mMutex);

		auto foundSourcePackage = mPackagesByPath.find(physicalPackagePath);
		if(!B3D_ENSURE(foundSourcePackage != mPackagesByPath.end()))
			return;

		if(auto foundDestinationPackage = mPackagesByPath.find(newPath); foundDestinationPackage != mPackagesByPath.end())
		{
			B3D_LOG(Warning, LogResources, "Cannot change physical package path. Another package already exists at location '{0}'.", newPath);
			return;
		}

		SPtr<RuntimePackageInformation> runtimePackageInformation = foundSourcePackage->second;
		mPackagesByPath.erase(foundSourcePackage);
		mPackagesByPath.insert(std::make_pair(newPath, runtimePackageInformation));

		runtimePackageInformation->PhysicalPath = newPath;
		package->AssociateFileWithPackage(newPath);
	}
}

void PackageManager::ChangeVirtualPackagePath(const PackageWriteLock& packageWriteLock, const Path& newVirtualPathPrefix)
{
	const SPtr<Package>& package = packageWriteLock.GetPackage();
	const Path& oldVirtualPathPrefix = packageWriteLock.RuntimePackageInformation->VirtualPathPrefix;
	{
		Lock lock(mMutex);

		const Vector<UUID>& resourceIds = package->CreateResourceIdList();
		for(const auto& resourceId : resourceIds)
		{
			const SPtr<const PackageResourceMetaData>& resourceMetaData = package->GetResourceMetaData(resourceId);
			if(!B3D_ENSURE(resourceMetaData))
				continue;

			if(!oldVirtualPathPrefix.IsEmpty())
			{
				const Path& oldVirtualResourcePath = Path::Combine(oldVirtualPathPrefix, resourceMetaData->Path);
				mVirtualPathToResourcePackagePath.erase(oldVirtualResourcePath);
			}

			ResourcePackagePath resourcePackagePath;
			resourcePackagePath.PhysicalPackagePath = packageWriteLock.RuntimePackageInformation->PhysicalPath;
			resourcePackagePath.ResourcePathWithinPackage = resourceMetaData->Path;

			const Path& newVirtualResourcePath = Path::Combine(newVirtualPathPrefix, resourceMetaData->Path);
			mVirtualPathToResourcePackagePath.insert(std::make_pair(newVirtualResourcePath, resourcePackagePath));
		}
	}
}

bool PackageManager::SavePackageMetaData(const PackageWriteLock& packageWriteLock)
{
	const Path& physicalPathToPackage = packageWriteLock.RuntimePackageInformation->PhysicalPath;
	const SPtr<Package>& package = packageWriteLock.GetPackage();

	const SPtr<DataStream> packageDataStream = FileSystem::OpenFile(physicalPathToPackage, false);
	if(!B3D_ENSURE(packageDataStream != nullptr))
		return false;

	if(!B3D_ENSURE(package != nullptr))
		return false;

	SavePackageOptions savePackageOptions;
	savePackageOptions.CompressResources = true;
	savePackageOptions.SaveMetaDataOnly = true;

	bool result = package->Save(packageDataStream, savePackageOptions);
	B3D_ENSURE(packageDataStream->Close());
	
	return result;
}

TOptional<ResourcePackagePath> PackageManager::TryResolvePhysicalResourcePath(const Path& physicalResourcePath) const
{
	if(!physicalResourcePath.IsAbsolute())
		return {};

	ResourcePackagePath resourcePackagePath;
	if(Package::BreakCombinedPackagePath(physicalResourcePath, resourcePackagePath.PhysicalPackagePath, resourcePackagePath.ResourcePathWithinPackage))
		return resourcePackagePath;

	return {};
}

TOptional<ResourcePackagePath> PackageManager::TryResolveVirtualResourcePath(const Path& virtualResourcePath) const
{
	Lock lock(mMutex);
	if(auto found = mVirtualPathToResourcePackagePath.find(virtualResourcePath); found != mVirtualPathToResourcePackagePath.end())
		return found->second;

	return {};
}

TOptional<Path> PackageManager::TryGetPackagePathForResource(const UUID& resourceId)
{
	Lock lock(mMutex);
	if(auto foundResource = mResourceIdToPackageId.find(resourceId); foundResource != mResourceIdToPackageId.end())
	{
		if(auto foundPackage = mPackagesById.find(foundResource->second); foundPackage != mPackagesById.end())
			return foundPackage->second->PhysicalPath;
	}

	return {};
}

SPtr<const PackageResourceMetaData> PackageManager::GetResourceMetaData(const UUID& resourceId)
{
	if(auto packagePath = TryGetPackagePathForResource(resourceId); packagePath.has_value())
	{
		UPtr<PackageReadLock> readLock;
		AcquirePackageReadLockOptions readLockOptions(false, true, "Read meta-data");
		const AcquirePackageLockResult lockResult = AcquireReadLock(*packagePath, readLockOptions, readLock);
		if(!B3D_ENSURE(lockResult == AcquirePackageLockResult::Acquired && readLock != nullptr))
			return nullptr;

		const SPtr<Package>& package = readLock->GetPackage();
		if(!B3D_ENSURE(package != nullptr))
			return nullptr;

		return package->GetResourceMetaData(resourceId);
	}

	return nullptr;
}

AcquirePackageLockResult PackageManager::AcquireReadLock(const Path& physicalPackagePath, const AcquirePackageReadLockOptions& options, UPtr<PackageReadLock>& outLock)
{
	if(!physicalPackagePath.IsAbsolute())
	{
		B3D_LOG(Warning, LogResources, "Fail to acquire package read lock. Provided path \"{0}\" is not absolute.", physicalPackagePath);
		return AcquirePackageLockResult::NotAcquiredPackageNotFound;
	}

	{
		Lock lock(mMutex);

		while(true)
		{
			// Retrieve runtime package information, or create new package information
			const auto found = mPackagesByPath.find(physicalPackagePath);
			if(found == mPackagesByPath.end())
			{
				if(!options.LoadIfMissing) // Not currently loaded, and don't want to load the package
					return AcquirePackageLockResult::NotAcquiredPackageNotFound;

				// Create new runtime package information for package we're about to load.
				// Write lock flag will be set by default, which prevents any other read operation from loading the same package.
				mPackagesByPath[physicalPackagePath] = B3DMakeUnique<RuntimePackageInformation>();
				break;
			}

			const SPtr<RuntimePackageInformation> runtimePackageInformation = found->second;
			if(!runtimePackageInformation->AcquiredWriteLock)
			{
				// No write lock held, we can acquire a read lock
				outLock = B3DMakeUnique<PackageReadLock>(*runtimePackageInformation, mMutex, options.LockReason);
				return AcquirePackageLockResult::Acquired;
			}
			else if(!options.BlockUntilAcquired)
			{
				// Write lock held, but we don't want to wait on it
				return AcquirePackageLockResult::NotAcquiredWriteLockHeld;
			}

			// Write lock held, wait until it is released, then re-do the checks above
			runtimePackageInformation->LoadSignal.Wait(lock, [&runtimePackageInformation, this]() {
				return !runtimePackageInformation->AcquiredWriteLock;
			});
		}
	}

	// Loading a brand new package. We only reach this point if we created a new RuntimePackageInformation above.
	SPtr<Package> package;
	if(FileSystem::Exists(physicalPackagePath))
		package = Package::Load(physicalPackagePath);

	Lock lock(mMutex);
	{
		const auto found = mPackagesByPath.find(physicalPackagePath);
		if(!B3D_ENSURE(found != mPackagesByPath.end()))
			return AcquirePackageLockResult::NotAcquiredPackageNotFound;

		RuntimePackageInformation* const runtimePackageInformation = found->second.get();
		B3D_ENSURE(runtimePackageInformation->AcquiredWriteLock);

		if(package)
		{
			mPackagesById[package->GetPackageId()] = runtimePackageInformation;

			const SPtr<PackageMetaData>& packageMetaData = package->GetPackageMetaData();

			runtimePackageInformation->LoadedPackage = package;
			runtimePackageInformation->PhysicalPath = physicalPackagePath;
			runtimePackageInformation->AcquiredWriteLock = false;

			if(packageMetaData != nullptr && packageMetaData->IncludePackageNameInVirtualPath)
				runtimePackageInformation->VirtualPathPrefix = Path::Combine(options.VirtualPathPrefix, package->GetPackageName());
			else
				runtimePackageInformation->VirtualPathPrefix = options.VirtualPathPrefix;

			LoadPackageResourceInformation(*package, physicalPackagePath, runtimePackageInformation->VirtualPathPrefix);

			runtimePackageInformation->LoadSignal.NotifyAll();

			outLock = B3DMakeUnique<PackageReadLock>(*runtimePackageInformation, mMutex, options.LockReason);
			return AcquirePackageLockResult::Acquired;
		}
		else // Nothing was loaded, clear the runtime information entry we added
		{
			const SPtr<RuntimePackageInformation> runtimePackageInformationCopy = std::move(found->second);
			mPackagesByPath.erase(found);

			runtimePackageInformationCopy->LoadSignal.NotifyAll();
			return AcquirePackageLockResult::NotAcquiredPackageNotFound;
		}
	}
}

AcquirePackageLockResult PackageManager::AcquireWriteLock(const Path& physicalPackagePath, const AcquirePackageWriteLockOptions& options, UPtr<PackageWriteLock>& outLock)
{
	if(!physicalPackagePath.IsAbsolute())
	{
		B3D_LOG(Warning, LogResources, "Fail to acquire package write lock. Provided path \"{0}\" is not absolute.", physicalPackagePath);
		return AcquirePackageLockResult::NotAcquiredPackageNotFound;
	}

	SPtr<RuntimePackageInformation> runtimePackageInformation;
	{
		Lock lock(mMutex);

		while(true)
		{
			const auto found = mPackagesByPath.find(physicalPackagePath);
			if(found == mPackagesByPath.end())
			{
				if(!options.AllowCreateNew) // Package information not found, and caller doesn't want to create one
					return AcquirePackageLockResult::NotAcquiredPackageNotFound;

				// Create new runtime package information, and acquire the lock
				runtimePackageInformation = B3DMakeShared<RuntimePackageInformation>();
				runtimePackageInformation->PhysicalPath = physicalPackagePath;
				runtimePackageInformation->VirtualPathPrefix = options.VirtualPathPrefix;

				mPackagesByPath[physicalPackagePath] = runtimePackageInformation;

				outLock = B3DMakeUnique<PackageWriteLock>(runtimePackageInformation.get(), mMutex, options.LockReason);
				return AcquirePackageLockResult::Acquired;
			}

			runtimePackageInformation = found->second;

			// If write lock is acquired, either wait for it or bail out
			if(runtimePackageInformation->AcquiredWriteLock)
			{
				// If not allowed to block, we cannot acquire the lock as something else is holding a write lock
				if(!options.BlockUntilAcquired)
					return AcquirePackageLockResult::NotAcquiredWriteLockHeld;

				// If we're allowed to block, wait on the read lock to release, then repeat the checks above
				runtimePackageInformation->LoadSignal.Wait(lock, [&runtimePackageInformation]() {
					return !runtimePackageInformation->AcquiredWriteLock;
				});
			}
			else
			{
				// No write-lock held, move along to read-lock checks below
				runtimePackageInformation->AcquiredReadLockCount++; // Increment read lock count so nothing else can acquire the write lock
				break;
			}
		}

		// Wait for all active read locks to be released
		while(runtimePackageInformation->AcquiredReadLockCount > 1)
		{
			if(!options.BlockUntilAcquired) // Not allowed to wait on locks, fail the lock acquire
				return AcquirePackageLockResult::NotAcquiredReadLockHeld;

			runtimePackageInformation->LoadSignal.Wait(lock, [&runtimePackageInformation]() {
				return runtimePackageInformation->AcquiredReadLockCount <= 1;
			});
		}

		// Acquire the write lock, and release the ad-hoc read lock we added above
		runtimePackageInformation->AcquiredWriteLock = true;
		runtimePackageInformation->AcquiredReadLockCount--;
	}

	// Associate path with package
	if(runtimePackageInformation->LoadedPackage != nullptr)
		runtimePackageInformation->LoadedPackage->AssociateFileWithPackage(physicalPackagePath);

	outLock = B3DMakeUnique<PackageWriteLock>(runtimePackageInformation.get(), mMutex, options.LockReason);
	return AcquirePackageLockResult::Acquired;
}

void PackageManager::LoadPackageResourceInformation(Package& package, const Path& physicalPackagePath, const Path& virtualPathPrefix)
{
	const Vector<UUID>& resourceIds = package.CreateResourceIdList();
	for(const auto& resourceId : resourceIds)
	{
		mResourceIdToPackageId[resourceId] = package.GetPackageId();

		if(!virtualPathPrefix.IsEmpty())
		{
			const SPtr<const PackageResourceMetaData>& resourceMetaData = package.GetResourceMetaData(resourceId);
			if(!B3D_ENSURE(resourceMetaData))
				continue;

			ResourcePackagePath resourcePackagePath;
			resourcePackagePath.PhysicalPackagePath = physicalPackagePath;
			resourcePackagePath.ResourcePathWithinPackage = resourceMetaData->Path;

			const Path& virtualResourcePath = Path::Combine(virtualPathPrefix, resourceMetaData->Path);

#if B3D_BUILD_TYPE_DEVELOPMENT
			if(auto foundVirtualPath = mVirtualPathToResourcePackagePath.find(virtualResourcePath); foundVirtualPath != mVirtualPathToResourcePackagePath.end())
			{
				const Path existingPhysicalPath = Path::Combine(foundVirtualPath->second.PhysicalPackagePath, foundVirtualPath->second.ResourcePathWithinPackage);
				const Path newPhysicalPath = Path::Combine(physicalPackagePath, resourceMetaData->Path);

				if(existingPhysicalPath != newPhysicalPath)
				{
					B3D_LOG(Warning, LogResources, "Same virtual path '{0}' used for multiple resources: '{1}' and '{2}'. This will result in undefined behaviour when looking up the resources via virtual path.", virtualResourcePath, existingPhysicalPath, newPhysicalPath);
				}
			}
#endif

			mVirtualPathToResourcePackagePath[virtualResourcePath] = std::move(resourcePackagePath);
		}
	}
}

void PackageManager::ClearPackageResourceInformation(Package& package, const Path& virtualPathPrefix)
{
	const Vector<UUID>& resourceIds = package.CreateResourceIdList();
	for(const auto& resourceId : resourceIds)
	{
		mResourceIdToPackageId.erase(resourceId);

		if(!virtualPathPrefix.IsEmpty())
		{
			const SPtr<const PackageResourceMetaData>& resourceMetaData = package.GetResourceMetaData(resourceId);
			if(!B3D_ENSURE(resourceMetaData))
				continue;

			const Path& virtualResourcePath = Path::Combine(virtualPathPrefix, resourceMetaData->Path);
			mVirtualPathToResourcePackagePath.erase(virtualResourcePath);
		}
	}
}

namespace b3d
{
B3D_EXPORT PackageManager& GetPackageManager()
{
	return PackageManager::Instance();
}
} // namespace b3d
