---
title: Persistent Cache
---

@b3d::PersistentCache is a system for storing data on disk that persists between application runs. It manages cache size limits, automatically evicts old entries when space is needed, and provides thread-safe access to cached data.

# Creating and initializing

## Creating a cache

Use @b3d::PersistentCache::Create to create a new cache instance:

~~~~~~~~~~~~~{.cpp}
TShared<PersistentCache> cache = PersistentCache::Create();
~~~~~~~~~~~~~

## Initializing the cache

Use @b3d::PersistentCache::Initialize to set the cache folder location:

~~~~~~~~~~~~~{.cpp}
Path cacheFolder = FileSystem::GetWorkingDirectoryPath() + "Cache/";
cache->Initialize(cacheFolder);
~~~~~~~~~~~~~

The cache folder will be created if it doesn't exist. All cache data will be stored within this folder.

## Setting cache size limit

Use @b3d::PersistentCache::SetMaximumCacheSize to configure the maximum cache size in megabytes:

~~~~~~~~~~~~~{.cpp}
// Set cache limit to 1024 MB (1 GB)
cache->SetMaximumCacheSize(1024);
~~~~~~~~~~~~~

The cache will automatically evict entries when it exceeds this limit, starting with the lowest priority entries that were least recently used.

# Adding entries

## Basic usage

Use @b3d::PersistentCache::SetEntry to add or update cache entries:

~~~~~~~~~~~~~{.cpp}
TShared<MyData> data = B3DMakeShared<MyData>();
data->Value = 42;

Path entryPath = "MyCache/ComputedResult.dat";
cache->SetEntry(entryPath, data);
~~~~~~~~~~~~~

The path is relative to the cache folder. Subdirectories will be created automatically.

## Priority levels

Entries can be assigned different priorities using @b3d::PersistentCachePriority:

~~~~~~~~~~~~~{.cpp}
// Low priority - evicted first
cache->SetEntry("Temp/PreviewData.dat", previewData, PersistentCachePriority::Low);

// Normal priority - default
cache->SetEntry("Cache/ProcessedTexture.dat", textureData, PersistentCachePriority::Normal);

// High priority - evicted after normal and low
cache->SetEntry("Cache/ImportantData.dat", importantData, PersistentCachePriority::High);

// Critical priority - evicted last
cache->SetEntry("Cache/EssentialConfig.dat", configData, PersistentCachePriority::Critical);
~~~~~~~~~~~~~

When the cache reaches its size limit, entries with lower priority are evicted before entries with higher priority.

## Non-blocking writes

By default, SetEntry blocks until any existing operations on that entry complete. You can disable blocking:

~~~~~~~~~~~~~{.cpp}
bool success = cache->SetEntry("Cache/Data.dat", data, PersistentCachePriority::Normal, false);

if(!success)
{
	B3D_LOG(Warning, LogGeneric, "Entry is currently in use, cannot update");
}
~~~~~~~~~~~~~

## Clearing entries

Pass `nullptr` as the data to remove an entry:

~~~~~~~~~~~~~{.cpp}
cache->SetEntry("Cache/OldData.dat", nullptr);
~~~~~~~~~~~~~

# Retrieving entries

## Basic retrieval

Use @b3d::PersistentCache::TryGetEntry to retrieve cached data:

~~~~~~~~~~~~~{.cpp}
Path entryPath = "MyCache/ComputedResult.dat";
TShared<IReflectable> data = cache->TryGetEntry(entryPath);

if(data)
{
	B3D_LOG(Info, LogGeneric, "Cache hit");
	ProcessData(data);
}
else
{
	B3D_LOG(Info, LogGeneric, "Cache miss");
	// Compute and cache the data
}
~~~~~~~~~~~~~

Returns `nullptr` if the entry doesn't exist in the cache.

## Type-safe retrieval

Use the template version to automatically cast to a specific type:

~~~~~~~~~~~~~{.cpp}
TShared<MyData> data = cache->TryGetEntry<MyData>("MyCache/ComputedResult.dat");

if(data)
{
	B3D_LOG(Info, LogGeneric, "Value: {0}", data->Value);
}
~~~~~~~~~~~~~

# Cache management

## Manual eviction

Force eviction to a target size with @b3d::PersistentCache::RunEviction:

~~~~~~~~~~~~~{.cpp}
// Evict entries until cache is below 512 MB
cache->RunEviction(512);
~~~~~~~~~~~~~

## Automatic eviction

The cache automatically runs eviction when needed. Use @b3d::PersistentCache::RunEvictionIfRequired to check and evict:

~~~~~~~~~~~~~{.cpp}
cache->RunEvictionIfRequired();
~~~~~~~~~~~~~

## Updating the cache

Call @b3d::PersistentCache::Update every frame to process pending operations:

~~~~~~~~~~~~~{.cpp}
void GameLoop()
{
	while(IsRunning())
	{
		cache->Update();

		// Rest of game loop
	}
}
~~~~~~~~~~~~~