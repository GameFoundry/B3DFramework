//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Resources/B3DPackage.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Utility
	 *  @{
	 */

	/** Determines when to evict data from PersistentCache. Objects with lower priority will be evicted before objects with higher priority. */
	enum class PersistentCachePriority
	{
		Low = 0,
		Normal = 1,
		High = 2,
		Critical = 3
	};

	class PersistentCacheObject;
	typedef TResourceHandle<PersistentCacheObject> HPersistentCacheObject;

	/** @addtogroup Utility-Internal
	 *  @{
	 */

	/** Resource used for storing data within a PersistentCache. */
	class PersistentCacheObject : public Resource
	{
	public:
		/** Returns all stored objects. */
		const TInlineArray<SPtr<IReflectable>, 1>& GetObjects() const { return mObjects; }

		/** Creates the resource holding a single object. */
		static SPtr<PersistentCacheObject> Create(const SPtr<IReflectable>& object);

		/** Creates the resource holding multiple objects. */
		static SPtr<PersistentCacheObject> Create(const TInlineArray<SPtr<IReflectable>, 1>& objects);

	private:
		PersistentCacheObject(const SPtr<IReflectable>& object)
			: Resource(false), mObjects({ object })
		{ }

		PersistentCacheObject(const TInlineArray<SPtr<IReflectable>, 1>& objects)
			: Resource(false), mObjects(objects)
		{ }

		TInlineArray<SPtr<IReflectable>, 1> mObjects;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PersistentCacheObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Meta-data associated with each cache entry. */
	class PersistentCacheMetaData : public PackageResourceUserMetaData
	{
	public:
		PersistentCachePriority Priority = PersistentCachePriority::Normal;
		u64 LastUsedTimestamp = 0;
		u32 CacheVersion = 0;
		
		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PersistentCacheMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Utility
	 *  @{
	 */

	/** Cache that will persist between application runs. */
	class B3D_EXPORT PersistentCache : public std::enable_shared_from_this<PersistentCache>
	{
		/** Information about a single entry in the cache. */
		struct CacheEntry
		{
			PersistentCachePriority Priority = PersistentCachePriority::Normal;
			u64 LastUsedTimestamp = 0;
			u64 SizeInBytes = 0;
			u32 ActiveReadOperationCount = 0;
			bool IsWriteOperationActive = false;
			bool IsMetaDataOutOfDate = false;
		};

		/** Type of the CacheOperation. */
		enum class CacheOperationType
		{
			Read,
			Write
		};

		/** Represents a read or write operation in a single entry in the cache. */
		struct CacheOperation : INonCopyable
		{
			CacheOperation(const WeakSPtr<PersistentCache>& cache, const Path& entryPath, CacheOperationType type);
			~CacheOperation();

			CacheOperation(CacheOperation&& other)
				: Cache(std::move(other.Cache)), EntryPath(std::exchange(other.EntryPath, Path::kBlank)), Type(other.Type)
			{ }

			CacheOperation& operator=(CacheOperation&& other)
			{
				if(this != &other)
				{
					Cache = std::move(other.Cache);
					EntryPath = std::exchange(other.EntryPath, Path::kBlank);
					Type = other.Type;
				}

				return *this;
			}

			WeakSPtr<PersistentCache> Cache;
			Path EntryPath;
			CacheOperationType Type;
		};

		struct PrivatelyConstruct { };

	public:
		/** Version of the cache. Ticking this value will cause any cached data using the old version to be invalidated. */
		static constexpr u32 kVersion = 1;

		PersistentCache(PrivatelyConstruct);
		~PersistentCache() = default;

		/**
		 * Initializes the cache. Must be called at least once before using the cache. Can be called multiple times in which the cache will be reinitialized
		 * with the provided settings.
		 *
		 * @param cacheFolder			Absolute path to folder in which the cache will store its contents.
		 */
		void Initialize(const Path& cacheFolder);

		/**
		 * Changes the maximum size of the cache. May trigger eviction.
		 *
		 * @param	sizeLimitInMb		Cache size limit in megabytes. The cache will attempt to stay below this number, but may exceed it momentarily if there are a lot of operations in progress and entries cannot be evicted in time.
		 */
		void SetMaximumCacheSize(u64 sizeLimitInMb = 2048);

		/** To be called every frame. */
		void Update();

		/** Evicts cache entries if the cache contains more data than it can hold. */
		void RunEvictionIfRequired();

		/** Evicts files from the cache until adequate amount of space has been acquired. */
		void RunEviction(u64 targetSizeInMb);

		/**
		 * Adds a new entry to the cache, or overrides an existing one.
		 *
		 * @param	path		Path at which to store the data. Relative to the cache folder.
		 * @param	data		Data to store or null to clear an existing entry.
		 * @param	priority	Priority that determines when is the entry evicted. Entries with lower priority will be evicted before ones with higher priority.
		 * @param	blocking	If false, the entry will not be cleared if there are currently any read or write locks acquired for it (i.e. it's used by another thread).
		 *						Otherwise we will block the calling until the locks are released, and then update the entry.
		 * @return				True if the entry was added to the cache, or false if the operation failed. Failure can happen if the entry exceeds the size of the cache.
		 */
		bool SetEntry(const Path& path, const SPtr<IReflectable>& data, PersistentCachePriority priority = PersistentCachePriority::Normal, bool blocking = true);

		/**
		 * Attempts to retrieve an entry at the specified path.
		 *
		 * @param	path		Path of the entry relative to the cache folder.
		 * @return				Data at the provided path if found or null otherwise.
		 */
		SPtr<IReflectable> TryGetEntry(const Path& path);

		/** @copydoc TryGetEntry */
		template<typename T>
		SPtr<T> TryGetEntry(const Path& path)
		{
			return B3DRTTICast<T>(TryGetEntry(path));
		}

		/** Creates a new persistent cache object. */
		static SPtr<PersistentCache> Create();

	private:
		static constexpr const char* kCacheRootFolderName = "PersistentCache";
		static constexpr const char* kCacheStagingDirectory = "Staging";
		static constexpr float kEvictPercent = 0.25f; /**< Determines what percent of the cache total size to try to evict. */

		/** Notifies the cache that a new CacheOperation has started. Caller must be holding the cache mutex lock during the call. */
		void NotifyOperationDidStart(const Path& path, CacheOperationType type);

		/** Notifies the cache that a CacheOperation is about to end. */
		void NotifyOperationWillEnd(const Path& path, CacheOperationType type);

		/**
		 * Attempts to acquire a read operation on the provided cache entry. You should only read from a cache entry if there is an active read operation for the entry.
		 *
		 * @param	path		Path to the entry, relative to the root cache folder.
		 * @return				Operation if successful, or null otherwise.
		 */
		TOptional<CacheOperation> AcquireReadOperation(const Path& path);

		/**
		 * Attempts to acquire a write operation on the provided cache entry. You should only write to a cache entry if there is an active write operation for the entry.
		 *
		 * @param	path				Path to the entry, relative to the root cache folder.
		 * @param	createNewIfMissing	If the entry doesn't yet exist, this will create a new entry. If not enabled a null operation will be returned for non-existing entries.
		 * @param	priority			If creating a new entry, priority to use when creating it.
		 * @param	blocking			If true, the calling thread will block until all other operations on the resource complete. If false, the method with fail to acquire an operation if there are any other operation in progress.
		 * @return						Operation if successful, or null otherwise.
		 */
		TOptional<CacheOperation> AcquireWriteOperation(const Path& path, bool createNewIfMissing, PersistentCachePriority priority, bool blocking = true);

		/** Retrieves a package in which the cache entry is stored. Caller must acquire a read operation for the entry beforehand. */
		SPtr<Package> GetPackageForEntry(const CacheOperation& operation) const;

		/** Adds a new package to the cache. Caller must acquire a write operation for the entry beforehand. Will trigger eviction if the cache is full. Returns false if the entry doesn't fit into the cache. */
		bool SetPackageForEntry(const CacheOperation& operation, const SPtr<Package>& package);

		/** Converts a path of a cache entry (relative to the cache) to an absolute path the package is being stored at in the file system. */
		Path GetPackagePathForEntry(const Path& path) const;

		/** Deletes a cache entry associated with the provided operation. Operation must be have a Write type. */
		void DeleteEntry(const CacheOperation& operation);

		/** Waits for both read and write operation to complete for all cache entries. */
		void WaitForAllOperationsToComplete(Lock& lock);

		/** Iterates over all entries with dirty meta-data, and writes the meta-data into their packages. */
		void WriteDirtyMetaData();

		/** Called when the shared pointer to this goes out of scope. */
		void SharedDeleter(PersistentCache* cache);

		Path mCacheFolder;
		u64 mUsedCacheSizeInBytes = 0;
		u64 mSizeLimitInBytes = (u64)2048 * Bitwise::kBytesInMegabyte;
		UnorderedMap<Path, CacheEntry> mEntries;
		bool mIsAnyEntryMetaDataDirty = false;

		mutable u32 mTotalActiveReadOperationCount = 0;
		mutable u32 mTotalActiveWriteOperationCount = 0;

		mutable Mutex mMutex;
		mutable Signal mOperationCompletedSignal;
	};

	/** @} */
} // namespace b3d
