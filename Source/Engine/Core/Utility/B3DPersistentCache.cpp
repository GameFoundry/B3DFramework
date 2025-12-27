//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DPersistentCache.h"
#include "RTTI/B3DPersistentCacheRTTI.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Resources/B3DPackage.h"
#include "Resources/B3DResources.h"
#include "Utility/B3DScopeGuard.h"

using namespace b3d;

B3D_LOG_CATEGORY_STATIC(PersistentCache, Log)

SPtr<PersistentCacheObject> PersistentCacheObject::Create(const SPtr<IReflectable>& object)
{
	SPtr<PersistentCacheObject> cacheObject = B3DMakeSharedFromExisting<PersistentCacheObject>(new (B3DAllocate<PersistentCacheObject>()) PersistentCacheObject(object));
	cacheObject->SetId(UUIDGenerator::GenerateRandom());
	cacheObject->SetShared(cacheObject);
	cacheObject->Initialize();

	return cacheObject;
}

SPtr<PersistentCacheObject> PersistentCacheObject::Create(const TInlineArray<SPtr<IReflectable>, 1>& objects)
{
	SPtr<PersistentCacheObject> cacheObject = B3DMakeSharedFromExisting<PersistentCacheObject>(new (B3DAllocate<PersistentCacheObject>()) PersistentCacheObject(objects));
	cacheObject->SetId(UUIDGenerator::GenerateRandom());
	cacheObject->SetShared(cacheObject);
	cacheObject->Initialize();

	return cacheObject;
}

RTTIType* PersistentCacheObject::GetRttiStatic()
{
	return PersistentCacheObjectRTTI::Instance();
}

RTTIType* PersistentCacheObject::GetRtti() const
{
	return PersistentCacheObject::GetRttiStatic();
}

RTTIType* PersistentCacheMetaData::GetRttiStatic()
{
	return PersistentCacheMetaDataRTTI::Instance();
}

RTTIType* PersistentCacheMetaData::GetRtti() const
{
	return PersistentCacheMetaData::GetRttiStatic();
}

PersistentCache::CacheOperation::CacheOperation(const WeakSPtr<PersistentCache>& cache, const Path& entryPath, CacheOperationType type)
	: Cache(cache), EntryPath(entryPath), Type(type)
{
	const SPtr<PersistentCache> cacheShared = cache.lock();

	if(!B3D_ENSURE(cacheShared))
		return;

	cacheShared->NotifyOperationDidStart(entryPath, type);
}

PersistentCache::CacheOperation::~CacheOperation()
{
	const SPtr<PersistentCache> cacheShared = Cache.lock();

	if(!cacheShared)
		return;

	cacheShared->NotifyOperationWillEnd(EntryPath, Type);
}

PersistentCache::PersistentCache(PrivatelyConstruct)
{ }

void PersistentCache::Initialize(const Path& cacheFolder)
{
	{
		{
			Lock lock(mMutex);

			// Make sure all existing operations complete, and data is saved
			WaitForAllOperationsToComplete(lock);
		}

		WriteDirtyMetaData();

		Lock lock(mMutex);
		mUsedCacheSizeInBytes = 0;
		mIsAnyEntryMetaDataDirty = false;
		mEntries.clear();

		B3D_ASSERT(mTotalActiveReadOperationCount == 0);
		B3D_ASSERT(mTotalActiveWriteOperationCount == 0);

		if (!cacheFolder.IsEmpty())
			mCacheFolder = cacheFolder + kCacheRootFolderName;

		if (mCacheFolder.IsEmpty())
			return;

		// If the cache folder already exists, load all the data from it
		if (FileSystem::Exists(mCacheFolder))
		{
			const Path& stagingFolder = mCacheFolder + kCacheStagingDirectory;
			auto fnOnFileFound = [this, &stagingFolder](const Path& path) -> bool {
				if(stagingFolder.Includes(path))
				{
					FileSystem::Remove(path);
					return true;
				}

				SPtr<Package> package = Package::Load(path);
				if(!B3D_ENSURE(package != nullptr))
					return true;

				bool isPackageOutOfDate = false;
				for(const auto& resourceUUID : package->CreateResourceIdList())
				{
					const SPtr<const PackageResourceMetaData> resourceMetaData = package->GetResourceMetaData(resourceUUID);
					if(!B3D_ENSURE(resourceMetaData != nullptr))
						continue;

					const SPtr<PersistentCacheMetaData> cacheObjectMetaData = B3DRTTICast<PersistentCacheMetaData>(resourceMetaData->AdditionalMetaData);
					if(!B3D_ENSURE(cacheObjectMetaData != nullptr))
						continue;

					if(cacheObjectMetaData->CacheVersion < kVersion)
					{
						isPackageOutOfDate = true;
						break;
					}

					CacheEntry cacheEntry;
					cacheEntry.Priority = cacheObjectMetaData->Priority;
					cacheEntry.LastUsedTimestamp = cacheObjectMetaData->LastUsedTimestamp;
					cacheEntry.SizeInBytes = FileSystem::GetFileSize(path);

					Path relativePath = path;
					relativePath.MakeRelative(mCacheFolder);

					mEntries[relativePath] = cacheEntry;
					mUsedCacheSizeInBytes += cacheEntry.SizeInBytes;
				}

				if(isPackageOutOfDate)
				{
					package = nullptr;
					B3D_ENSURE(FileSystem::Remove(path));

					return true;
				}

				mUsedCacheSizeInBytes += (u64)FileSystem::GetFileSize(path);

				return true;
			};

			FileSystem::Iterate(mCacheFolder, fnOnFileFound, nullptr, true);

			// Note: Could also clean up empty folders here
		}
		else
		{
			FileSystem::CreateDir(mCacheFolder);
		}
	}

	RunEvictionIfRequired();
}

SPtr<PersistentCache> PersistentCache::Create()
{
	return B3DMakeShared<PersistentCache>(PrivatelyConstruct());
}

void PersistentCache::SharedDeleter(PersistentCache* cache)
{
	{
		Lock lock(mMutex);
		WaitForAllOperationsToComplete(lock);
	}

	WriteDirtyMetaData();
}

void PersistentCache::RunEvictionIfRequired()
{
	u64 sizeLimitInBytes;
	u64 currentCacheSize;
	{
		Lock lock(mMutex);

		sizeLimitInBytes = mSizeLimitInBytes;
		currentCacheSize = mUsedCacheSizeInBytes;
	}

	if(currentCacheSize <= sizeLimitInBytes)
		return;

	const u64 reserveSize = (u64)((float)sizeLimitInBytes * kEvictPercent) * (u64)Bitwise::kBytesInMegabyte;
	RunEviction(reserveSize);
}

void PersistentCache::RunEviction(u64 targetSizeInMb)
{
	struct SortedCacheEntry
	{
		SortedCacheEntry() = default;

		SortedCacheEntry(const Path& path, const CacheEntry& entry)
			: Path(path), Priority(entry.Priority), LastUsedTimestamp(entry.LastUsedTimestamp)
		{
		}

		Path Path;
		PersistentCachePriority Priority;
		u64 LastUsedTimestamp;
	};

	const FrameAllocatorScope frameScope;
	FrameVector<SortedCacheEntry> sortedEntries;

	{
		Lock lock(mMutex);

		for(const auto& entryPair : mEntries)
			sortedEntries.push_back(SortedCacheEntry(entryPair.first, entryPair.second));
	}

	// Sort by priority, then by timestamp. Lower priority first, oldest timestamp first.
	std::sort(sortedEntries.begin(), sortedEntries.end(), [](const SortedCacheEntry& lhs, const SortedCacheEntry& rhs)
	{
		if (lhs.Priority == rhs.Priority)
		{
			if (lhs.LastUsedTimestamp == rhs.LastUsedTimestamp)
				return (u64)&lhs > (u64)&rhs;

			return lhs.LastUsedTimestamp > rhs.LastUsedTimestamp;
		}

		return lhs.Priority > rhs.Priority; });

	for(const auto& entry : sortedEntries)
	{
		{
			Lock lock(mMutex);

			const u64 targetSizeInBytes = targetSizeInMb * (u64)Bitwise::kBytesInMegabyte;
			if(mUsedCacheSizeInBytes <= targetSizeInBytes)
				return;
		}

		TOptional<CacheOperation> writeOperation = AcquireWriteOperation(entry.Path, false, PersistentCachePriority::Normal, false);

		// We skip entries that are currently open for operations, otherwise eviction would need to block
		if(!writeOperation.has_value())
			continue;

		DeleteEntry(writeOperation.value());
	}
}

void PersistentCache::WriteDirtyMetaData()
{
	const FrameAllocatorScope frameScope;
	FrameVector<Path> entriesToUpdate;

	// Get a list of dirty entries
	{
		Lock lock(mMutex);
		if(!mIsAnyEntryMetaDataDirty)
			return;

		for(const auto& entryPair : mEntries)
		{
			if(entryPair.second.IsMetaDataOutOfDate)
				entriesToUpdate.push_back(entryPair.first);
		}
	}

	// Write package meta-data for dirty entries
	for(const auto& entryPath : entriesToUpdate)
	{
		TOptional<CacheOperation> writeOperation = AcquireWriteOperation(entryPath, false, PersistentCachePriority::Normal, false);

		// We skip entries that are currently open for operations, otherwise eviction would need to block
		if(!writeOperation.has_value())
			continue;

		const Path pathToPackage = GetPackagePathForEntry(entryPath);

		const SPtr<DataStream> packageDataStream = FileSystem::OpenFile(pathToPackage, false);
		if(!packageDataStream)
			continue;

		const SPtr<Package> package = Package::Load(pathToPackage);

		if(!B3D_ENSURE(package != nullptr))
			continue;

		for(const auto& resourceUUID : package->CreateResourceIdList())
		{
			const SPtr<const PackageResourceMetaData> resourceMetaData = package->GetResourceMetaData(resourceUUID);
			if(!B3D_ENSURE(resourceMetaData != nullptr))
				continue;

			SPtr<PersistentCacheMetaData> cacheObjectMetaData = B3DRTTICast<PersistentCacheMetaData>(resourceMetaData->AdditionalMetaData);
			if(cacheObjectMetaData == nullptr)
				cacheObjectMetaData = B3DMakeShared<PersistentCacheMetaData>();

			{
				Lock lock(mMutex);

				auto found = mEntries.find(entryPath);
				if(found == mEntries.end())
					continue;

				cacheObjectMetaData->LastUsedTimestamp = found->second.LastUsedTimestamp;
				found->second.IsMetaDataOutOfDate = false;
			}

			package->SetResourceMetaData(resourceUUID, cacheObjectMetaData);
		}

		SavePackageOptions savePackageOptions;
		savePackageOptions.CompressResources = true;
		savePackageOptions.SaveMetaDataOnly = true;

		if(!package->Save(packageDataStream, savePackageOptions))
		{
			// Try re-saving everything
			savePackageOptions.SaveMetaDataOnly = false;
			B3D_ENSURE(package->Save(packageDataStream, savePackageOptions));	
		}

		B3D_ENSURE(packageDataStream->Close());
	}

	// Determine if there are more remaining dirty entries
	{
		Lock lock(mMutex);

		mIsAnyEntryMetaDataDirty = false;
		for(const auto& entryPair : mEntries)
		{
			if(entryPair.second.IsMetaDataOutOfDate)
			{
				mIsAnyEntryMetaDataDirty = true;
				break;
			}
		}
	}
}

void PersistentCache::Update()
{
	RunEvictionIfRequired();
	WriteDirtyMetaData();
}

SPtr<IReflectable> PersistentCache::TryGetEntry(const Path& path)
{
	TOptional<CacheOperation> operation = AcquireReadOperation(path);
	if (!operation.has_value())
		return nullptr;

	const SPtr<Package> package = GetPackageForEntry(operation.value());
	if (package == nullptr)
		return nullptr;

	const SPtr<Resource> resource = package->LoadResource(path);
	if (resource == nullptr)
	{
		B3D_LOG(Error, PersistentCache, "Cannot retrieve cache entry at path '{0}'. Failed deserializing entry from the package.", path);
		return nullptr;
	}

	const SPtr<PersistentCacheObject> persistentCacheObject = B3DRTTICast<PersistentCacheObject>(resource);
	if (persistentCacheObject == nullptr)
	{
		B3D_LOG(Error, PersistentCache, "Cannot retrieve cache entry at path '{0}. Package object is not of a valid type", path);
		return nullptr;
	}

	const TInlineArray<SPtr<IReflectable>, 1>& cachedObjects = persistentCacheObject->GetObjects();
	if (cachedObjects.Empty())
		return nullptr;

	return cachedObjects[0];
}

bool PersistentCache::SetEntry(const Path& path, const SPtr<IReflectable>& data, PersistentCachePriority priority, bool blocking)
{
	if (path.IsEmpty())
	{
		B3D_LOG(Error, PersistentCache, "Cannot add entry to cache. Provided path is empty.");
		return false;
	}

	if (path.IsAbsolute())
	{
		B3D_LOG(Error, PersistentCache, "Cannot add entry to cache. Path must be relative. Path provided: {0}", path);
		return false;
	}

	if (data == nullptr)
	{
		// Clear the entry
		TOptional<CacheOperation> operation = AcquireWriteOperation(path, false, priority, blocking);
		if (operation.has_value())
			DeleteEntry(*operation);

		return true;
	}

	const SPtr<PersistentCacheObject> persistentCacheObject = PersistentCacheObject::Create(data);
	B3D_ASSERT(persistentCacheObject != nullptr);

	const SPtr<Package> package = Package::Create(path.GetFilename(), UUIDGenerator::GenerateRandom());
	B3D_ASSERT(package != nullptr);

	package->AddResource(path,  persistentCacheObject);

	const SPtr<PersistentCacheMetaData> cacheMetaData = B3DMakeShared<PersistentCacheMetaData>();
	cacheMetaData->Priority = priority;
	cacheMetaData->LastUsedTimestamp = (u64)std::time(nullptr);
	cacheMetaData->CacheVersion = kVersion;

	package->SetResourceMetaData(path, cacheMetaData);

	TOptional<CacheOperation> operation = AcquireWriteOperation(path, true, priority);
	if (!operation.has_value())
		return false;

	return SetPackageForEntry(operation.value(), package);
}

void PersistentCache::DeleteEntry(const CacheOperation& operation)
{
	if(!B3D_ENSURE(operation.Type == CacheOperationType::Write))
		return;

	Lock lock(mMutex);

	auto found = mEntries.find(operation.EntryPath);
	if(!B3D_ENSURE(found != mEntries.end()))
	{
		B3D_LOG(Error, PersistentCache, "Cannot delete entry from cache. Entry at path {0} is not registered as part of the cache. ", operation.EntryPath);
		return;
	}

	const Path pathToPackage = GetPackagePathForEntry(operation.EntryPath);
	if (FileSystem::Exists(pathToPackage))
	{
		const u64 fileSize = found->second.SizeInBytes;
		FileSystem::Remove(pathToPackage, false);

		if(B3D_ENSURE(mUsedCacheSizeInBytes >= fileSize))
		{
			mUsedCacheSizeInBytes -= std::min(fileSize, mUsedCacheSizeInBytes);
		}
	}

	mEntries.erase(found);
}

void PersistentCache::NotifyOperationDidStart(const Path& path, CacheOperationType type)
{
	// Cache mutex must be held externally when this is called

	const auto found = mEntries.find(path);
	B3D_ASSERT(found != mEntries.end());

	if (type == CacheOperationType::Read)
	{
		found->second.ActiveReadOperationCount++;
		mTotalActiveReadOperationCount++;
	}
	else
	{
		B3D_ASSERT(!found->second.IsWriteOperationActive);

		found->second.IsWriteOperationActive = true;
		mTotalActiveWriteOperationCount++;
	}
}

void PersistentCache::NotifyOperationWillEnd(const Path& path, CacheOperationType type)
{
	Lock lock(mMutex);

	const auto found = mEntries.find(path);
	if (type == CacheOperationType::Read)
	{
		if(B3D_ENSURE(found != mEntries.end()))
		{
			B3D_ASSERT(found->second.ActiveReadOperationCount > 0);
			found->second.ActiveReadOperationCount--;
		}

		B3D_ASSERT(mTotalActiveReadOperationCount > 0);
		mTotalActiveReadOperationCount--;
	}
	else
	{
		// It won't be part of entries if this is a write operation that failed
		if(found != mEntries.end())
		{
			B3D_ASSERT(found->second.IsWriteOperationActive);
			found->second.IsWriteOperationActive = false;
		}

		B3D_ASSERT(mTotalActiveWriteOperationCount > 0);
		mTotalActiveWriteOperationCount--;
	}

	mOperationCompletedSignal.NotifyAll();
}

TOptional<PersistentCache::CacheOperation> PersistentCache::AcquireReadOperation(const Path& path)
{
	Lock lock(mMutex);

	if(mCacheFolder.IsEmpty())
		return {};

	mOperationCompletedSignal.Wait(lock, [this, &path]
	{
		auto found = mEntries.find(path);
		if (found == mEntries.end())
			return true;

		return !found->second.IsWriteOperationActive;
	});

	// Entry could have been deleted
	auto found = mEntries.find(path);
	if (found == mEntries.end())
		return {};

	found->second.LastUsedTimestamp = (u64)std::time(nullptr);
	found->second.IsMetaDataOutOfDate = true;
	mIsAnyEntryMetaDataDirty = true;

	return CacheOperation(shared_from_this(), path, CacheOperationType::Read);
}

TOptional<PersistentCache::CacheOperation> PersistentCache::AcquireWriteOperation(const Path& path, bool createNewIfMissing, PersistentCachePriority priority, bool blocking)
{
	Lock lock(mMutex);

	if (mCacheFolder.IsEmpty())
		return {};

	auto fnCreateNewEntry = [this](const Path& path, PersistentCachePriority priority) -> TOptional<CacheOperation> {
		CacheEntry cacheEntry;
		cacheEntry.Priority = priority;
		cacheEntry.LastUsedTimestamp = (u64)std::time(nullptr);

		mEntries[path] = cacheEntry;
		return CacheOperation(shared_from_this(), path, CacheOperationType::Write);
	};

	// If there are any existing read or write operation, they need to complete first
	mOperationCompletedSignal.Wait(lock, [this, &path]
	{
		auto found = mEntries.find(path);
		if (found == mEntries.end())
			return true;

		return !found->second.IsWriteOperationActive && found->second.ActiveReadOperationCount == 0;
	});

	auto found = mEntries.find(path);
	if(found == mEntries.end())
	{
		if(!createNewIfMissing)
			return {};

		return fnCreateNewEntry(path, priority);
	}

	found->second.LastUsedTimestamp = (u64)std::time(nullptr);
	found->second.IsMetaDataOutOfDate = false;

	return CacheOperation(shared_from_this(), path, CacheOperationType::Write);
}

SPtr<Package> PersistentCache::GetPackageForEntry(const CacheOperation& operation) const
{
	if(!B3D_ENSURE(operation.Type == CacheOperationType::Read))
		return nullptr;

	const Path pathToPackage = GetPackagePathForEntry(operation.EntryPath);
	if (!FileSystem::Exists(pathToPackage))
		return nullptr;

	SPtr<Package> package = Package::Load(pathToPackage);
	if (package == nullptr)
	{
		B3D_LOG(Error, PersistentCache, "Failed retrieving cache entry. Cache package at path '{0}' failed to deserialize.", operation.EntryPath);
		return nullptr;
	}

	return package;
}

bool PersistentCache::SetPackageForEntry(const CacheOperation& operation, const SPtr<Package>& package)
{
	if(package == nullptr)
		return false;

	const Path pathToPackage = GetPackagePathForEntry(operation.EntryPath);

	bool hasCompletedSuccesfully = false;
	ScopeGuard removeEntryOnFailure([this, &hasCompletedSuccesfully, &operation]()
	{
		if(!hasCompletedSuccesfully)
			DeleteEntry(operation);
	});

	// Delete existing file if it exists
	{
		Lock lock(mMutex);

		auto found = mEntries.find(operation.EntryPath);
		if(found != mEntries.end())
		{
			const u64 fileSize = found->second.SizeInBytes;
			found->second.SizeInBytes = 0;

			B3D_ENSURE(FileSystem::Remove(pathToPackage, false));

			if(B3D_ENSURE(mUsedCacheSizeInBytes >= fileSize))
				mUsedCacheSizeInBytes -= std::min(fileSize, mUsedCacheSizeInBytes);
		}
	}

	// Create folder hierarchy and check for clashing file names
	const u32 pathEntryCount = operation.EntryPath.GetDirectoryCount() + (operation.EntryPath.IsFile() ? 1 : 0);
	if (pathEntryCount > 1)
	{
		for (u32 pathEntryIndex = 1; pathEntryIndex < pathEntryCount; ++pathEntryIndex)
		{
			const Path fullPathToDirectory = mCacheFolder + operation.EntryPath.GetSubPath(pathEntryIndex);
			if (FileSystem::Exists(fullPathToDirectory))
			{
				if (FileSystem::IsFile(fullPathToDirectory))
				{
					B3D_LOG(Error, PersistentCache, "Cannot add entry '{0}' to cache. Directory in the path '{1}' already exists as a file with the same name.", operation.EntryPath, fullPathToDirectory);
					return false;
				}

				continue;
			}

			if(!FileSystem::CreateDir(fullPathToDirectory))
			{
				B3D_LOG(Error, PersistentCache, "Cannot add entry '{0}' to cache. Failed to create directory hierarchy to path '{1}'.", operation.EntryPath, fullPathToDirectory);
				return false;
			}
		}
	}

	// First write to a temporary file
	// Note: This means we might temporarily exceed cache capacity, but that's fine as alternatives are too complex to implement.
	const Path temporaryPackageDirectory = mCacheFolder + kCacheStagingDirectory;
	FileSystem::CreateDir(temporaryPackageDirectory);

	Path temporaryPackagePath = temporaryPackageDirectory;
	temporaryPackagePath.Append(package->GetPackageId().ToString() + Package::kPackageExtension);

	SPtr<DataStream> temporaryFileStream = FileSystem::CreateAndOpenFile(temporaryPackagePath);
	if(temporaryFileStream == nullptr)
	{
		B3D_LOG(Error, PersistentCache, "Cannot add entry '{0}' to cache. Unable to open a temporary file for writing at path '{1}'.", operation.EntryPath, temporaryPackagePath);
		return false;
	}

	SavePackageOptions savePackageOptions;
	savePackageOptions.CompressResources = true;

	package->Save(temporaryFileStream, savePackageOptions);

	bool isTemporaryFileMoved = false;
	ScopeGuard removeTemporaryFileOnFailure([this, &isTemporaryFileMoved, temporaryPackagePath]()
	{
		if(!isTemporaryFileMoved)
			FileSystem::Remove(temporaryPackagePath);
	});

	if(!temporaryFileStream->Close())
	{
		B3D_LOG(Error, PersistentCache, "Cannot add entry '{0}' to cache. Failed to write to the data stream at path '{1}'.", operation.EntryPath, temporaryPackagePath);
		return false;
	}

	// Move the file to it's final location
	if(!FileSystem::Move(temporaryPackagePath, pathToPackage, false))
	{
		B3D_LOG(Error, PersistentCache, "Cannot add entry '{0}' to cache. Failed to copy from staging '{1}' to destination '{2}'.", operation.EntryPath, temporaryPackagePath, pathToPackage);
		return false;
	}

	isTemporaryFileMoved = false;
	ScopeGuard removePackageFileOnFailure([this, &hasCompletedSuccesfully, pathToPackage]()
	{
		if(!hasCompletedSuccesfully)
			FileSystem::Remove(pathToPackage);
	});

	// Update entry size
	bool isEntryUpdated = false;
	{
		Lock lock(mMutex);

		auto found = mEntries.find(operation.EntryPath);
		if(B3D_ENSURE(found != mEntries.end()))
		{
			found->second.SizeInBytes = FileSystem::GetFileSize(pathToPackage);
			mUsedCacheSizeInBytes += found->second.SizeInBytes;
			isEntryUpdated = true;
		}
	}

	if(!isEntryUpdated)
		return false;

	// See if we're over the size limit and evict excess if required
	const u64 fileSize = FileSystem::GetFileSize(pathToPackage);

	// Entry can never fit
	if(fileSize > mSizeLimitInBytes)
		return false;

	bool isEvictionRequired = false;
	u64 evictTargetSizeInMb = 0;
	{
		Lock lock(mMutex);

		const u64 newSize = mUsedCacheSizeInBytes + fileSize;
		isEvictionRequired = newSize > mSizeLimitInBytes;

		if(isEvictionRequired)
		{
			static constexpr float kFileSizeBuffer = 1.5f; // Determines how much extra file-size to try to reserve when writing the file, in percent.

			// Evict using two parameters, whichever one is higher:
			//  - Certain percentage of the total cache size (based on kEvictPercent)
			//  - Size of the file we're writing (+ some buffer based on kFileSizeBuffer)

			const u64 reserveSize = (u64)((float)mSizeLimitInBytes * kEvictPercent);
			const u64 fileSizeWithBuffer = (u64)((float)fileSize * kFileSizeBuffer);

			evictTargetSizeInMb = Math::DivideAndRoundUp(Math::Max(reserveSize, fileSizeWithBuffer), (u64)Bitwise::kBytesInMegabyte); // Round up to megabyte
		}
	}

	if(isEvictionRequired)
	{
		RunEviction(evictTargetSizeInMb);

		// Check if there's enough space now
		{
			Lock lock(mMutex);

			const u64 newSize = mSizeLimitInBytes + fileSize;
			if(newSize > mSizeLimitInBytes)
				return false; // We weren't able to evict enough, likely too many things in the cache are being currently used and/or current file is large
		}
	}

	hasCompletedSuccesfully = true;
	return true;
}

Path PersistentCache::GetPackagePathForEntry(const Path& path) const
{
	Path pathToPackage = mCacheFolder + path;
	if(!pathToPackage.IsFile())
	{
		const String& filename = pathToPackage.GetTail();
		pathToPackage.MakeParent();
		pathToPackage.SetFilename(filename);
	}

	return pathToPackage;
}

void PersistentCache::WaitForAllOperationsToComplete(Lock& lock)
{
	mOperationCompletedSignal.Wait(lock, [this] { return mTotalActiveWriteOperationCount == 0 && mTotalActiveReadOperationCount == 0; });
}

void PersistentCache::SetMaximumCacheSize(u64 sizeLimitInMb)
{
	{
		Lock lock(mMutex);
		mSizeLimitInBytes = sizeLimitInMb * Bitwise::kBytesInMegabyte;
	}

	RunEvictionIfRequired();
}
