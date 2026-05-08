//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DResources.h"

#include "B3DApplication.h"
#include "B3DPackage.h"
#include "Resources/B3DResource.h"
#include "Serialization/B3DFileSerializer.h"
#include "FileSystem/B3DFileSystem.h"
#include "Utility/B3DUUID.h"
#include "Debug/B3DDebug.h"
#include "Utility/B3DUtility.h"
#include "Managers/B3DResourceListenerManager.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Reflection/B3DRTTIType.h"
#include "Threading/B3DScheduler.h"

using namespace b3d;

const ResourceLoadOptions ResourceLoadOptions::kDefault;

/**
 * Attempts to acquire a read lock on the package containing the resource at the provided virtual/physical path. Returns true if the lock
 * was acquired, in which case also outputs the lock and the ID of the locked resource. Also see PackageManager::AcquireReadLock().
 */
static bool TryAcquirePackageLockForResourceLoad(const Path& resourcePath, const char* lockReason, UPtr<PackageReadLock>& outReadLock, UUID& outResourceId)
{
	PackageManager& packageManager = GetPackageManager();

	TOptional<ResourcePackagePath> maybeResourcePackagePath = packageManager.TryResolvePhysicalResourcePath(resourcePath);
	if(!maybeResourcePackagePath.has_value()) // Maybe it's a virtual path
		maybeResourcePackagePath = packageManager.TryResolveVirtualResourcePath(resourcePath);

	if(!maybeResourcePackagePath.has_value())
		return false;

	const ResourcePackagePath& resourcePackagePath = *maybeResourcePackagePath;

	AcquirePackageReadLockOptions readLockOptions(true, true, lockReason);
	const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(resourcePackagePath.PhysicalPackagePath, readLockOptions, outReadLock);
	if(!B3D_ENSURE(lockResult == AcquirePackageLockResult::Acquired && outReadLock != nullptr))
		return false;

	const TShared<Package>& package = outReadLock->GetPackage();
	if(!B3D_ENSURE(package != nullptr))
		return nullptr;

	const TShared<const PackageResourceMetaData>& resourceMetaData = package->GetResourceMetaData(resourcePackagePath.ResourcePathWithinPackage);
	if(resourceMetaData == nullptr)
		return false;

	outResourceId = resourceMetaData->Id;
	return true;
}

/**
 * Attempts to acquire a write lock on the package containing the resource with the provided ID. Returns true if the lock was acquired, in which case also
 * outputs the lock and the physical path of the package containing the resource . Also see PackageManager::AcquireWriteLock().
 */
static bool TryAcquirePackageLockForResourceLoad(const UUID& resourceId, const char* lockReason, UPtr<PackageReadLock>& outReadLock, Path& outPackagePath)
{
	PackageManager& packageManager = GetPackageManager();

	const TOptional<Path> maybePackagePath = packageManager.TryGetPackagePathForResource(resourceId);
	if(!maybePackagePath.has_value())
		return false;

	outPackagePath = *maybePackagePath;

	AcquirePackageReadLockOptions readLockOptions(true, true, lockReason);
	const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(outPackagePath, readLockOptions, outReadLock);
	if(!B3D_ENSURE(lockResult == AcquirePackageLockResult::Acquired && outReadLock != nullptr))
		return false;

	return true;
}

Resources::~Resources()
{
	UnloadAll();
}

HResource Resources::Load(const Path& resourcePath, const ResourceLoadOptions& loadOptions)
{
	UPtr<PackageReadLock> packageReadLock;
	UUID resourceId;
	if(!TryAcquirePackageLockForResourceLoad(resourcePath, "Load resource", packageReadLock, resourceId))
	{
		B3D_LOG(Warning, LogResources, "Cannot load resource. File at path '{0}' doesn't exist.", resourcePath);
		return nullptr;
	}

	return LoadFromPackage(std::move(packageReadLock), resourceId, loadOptions);
}

HResource Resources::Load(const UUID& resourceId, const ResourceLoadOptions& loadOptions)
{
	Function<TOptional<HResource>(bool)> fnTryGetLoadedResource = [this, &resourceId, &loadOptions](bool allowUnloadedDependencies) -> TOptional<HResource>
	{
		Lock lock(mLoadedResourceMutex);
		if(auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
		{
			LoadedResourceInformation* const loadedResourceInformation = found->second.get();
			B3D_ASSERT(loadedResourceInformation != nullptr);

			if(!allowUnloadedDependencies && loadOptions.LoadDependencies && !loadedResourceInformation->DependenciesLoaded)
				return {};

			if(loadOptions.KeepInternalReference)
			{
				loadedResourceInformation->InternalReferenceCount++;
				loadedResourceInformation->ResourceHandle.IncrementStrongReferenceCount();
			}

			return found->second->ResourceHandle.Lock();
		}

		return {};
	};

	// Try the quick path first by checking loaded resources. This will check for any runtime resources (i.e. resources without a package), or previously loaded
	// package resources. But only if we don't need to load dependencies, in which case we fall back to the package load path to ensure dependencies are handled.
	{
		auto result = fnTryGetLoadedResource(false);
		if(result.has_value())
			return *result;
	}

	UPtr<PackageReadLock> packageReadLock;
	Path packagePath;
	if(!TryAcquirePackageLockForResourceLoad(resourceId, "Load resource", packageReadLock, packagePath))
	{
		// Package cannot be found, look up the loaded resources again as this could be a runtime created resource. This time ignore dependency loads.
		auto result = fnTryGetLoadedResource(true);
		if(result.has_value())
			return *result;

		B3D_LOG(Warning, LogResources, "Cannot load resource. Resource with ID '{0}' doesn't exist.", resourceId);
		return nullptr;
	}

	return LoadFromPackage(std::move(packageReadLock), resourceId, loadOptions);
}

bool Resources::Exists(const Path& resourcePath) const
{
	UPtr<PackageReadLock> packageReadLock;
	UUID resourceId;
	return TryAcquirePackageLockForResourceLoad(resourcePath, "Check resource exists", packageReadLock, resourceId);
}

bool Resources::Exists(const UUID& resourceId) const
{
	// Check loaded resources first. This includes any runtime created resources (without a package), and is also a quick
	// way to avoid package lookups for already loaded resources.
	{
		Lock lock(mLoadedResourceMutex);
		if(auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
			return found->second->ResourceHandle.Lock();
	}

	UPtr<PackageReadLock> packageReadLock;
	Path packagePath;
	return TryAcquirePackageLockForResourceLoad(resourceId, "Check resource exists", packageReadLock, packagePath);
}

HResource Resources::LoadFromPackage(UPtr<PackageReadLock> packageReadLock, const UUID& resourceId, const ResourceLoadOptions& loadOptions)
{
	const TShared<Package>& package = packageReadLock->GetPackage();
	const TShared<const PackageResourceMetaData>& metaData = package->GetResourceMetaData(resourceId);

	if(metaData == nullptr)
	{
		B3D_LOG(Warning, LogResources, "Cannot load resource. Resource with ID: {0} cannot be found in the package.", resourceId);
		return HResource();
	}

	HResource resourceHandle = GetOrCreateResourceHandle(resourceId);

	TShared<InProgressLoadInformation> inProgressLoadInformation;
	{
		Lock lock(mLoadedResourceMutex);

		// Is the resource and (optionally) its dependencies already loaded?
		if(const auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
		{
			LoadedResourceInformation* const loadedResourceInformation = found->second.get();
			if(!B3D_ENSURE(loadedResourceInformation != nullptr))
				return HResource();

			if(!loadOptions.LoadDependencies || loadedResourceInformation->DependenciesLoaded)
			{
				if(loadOptions.KeepInternalReference)
				{
					loadedResourceInformation->InternalReferenceCount++;
					loadedResourceInformation->ResourceHandle.IncrementStrongReferenceCount();
				}

				return loadedResourceInformation->ResourceHandle.Lock();
			}
		}

		// If not already loaded, create structure to track in progress load for the resource and all dependencies
		inProgressLoadInformation = B3DMakeShared<InProgressLoadInformation>();
		inProgressLoadInformation->PackageReadLock = std::move(packageReadLock);
		inProgressLoadInformation->ResourceHandle = resourceHandle;
		inProgressLoadInformation->LoadOptions = loadOptions;
		inProgressLoadInformation->RemainingResourcesToLoadCount = 1;

		if(loadOptions.LoadDependencies)
		{
			const u32 dependencyCount = (u32)metaData->Dependencies.size();
			inProgressLoadInformation->DependencyResourceHandles.reserve(dependencyCount);
			
			for(const UUID& dependencyId : metaData->Dependencies)
			{
				if(dependencyId == resourceId)
					continue;

				HResource dependencyResourceHandle = GetOrCreateResourceHandle(dependencyId);
				inProgressLoadInformation->DependencyResourceHandles.push_back(dependencyResourceHandle);

				const bool isDependencyLoaded = mLoadedResourceInformation.find(dependencyId) != mLoadedResourceInformation.end();
				if(isDependencyLoaded)
					continue;

				mDependantResourceLoads[dependencyId].Add(inProgressLoadInformation);
				inProgressLoadInformation->RemainingResourcesToLoadCount++;
			}
		}

		mInProgressLoadInformation[resourceId].Add(inProgressLoadInformation);
	}

	// Issue load request for all dependencies
	if(loadOptions.LoadDependencies)
	{
		ResourceLoadOptions dependencyLoadOptions;
		dependencyLoadOptions.LoadDependencies = false;
		dependencyLoadOptions.AsynchronousLoad = loadOptions.AsynchronousLoad;

		for(const UUID& dependencyId : metaData->Dependencies)
		{
			if(dependencyId == resourceId)
				continue;

			HResource dependency = Load(dependencyId, dependencyLoadOptions);
			(void)dependency;
		}
	}

	auto fnLoadFromPackageAndFinalize = [this, inProgressLoadInformationWeak = WeakSPtr<InProgressLoadInformation>(inProgressLoadInformation), package, resourceId]()
	{
		const TShared<InProgressLoadInformation> inProgressLoadInformation = inProgressLoadInformationWeak.lock();
		if(!B3D_ENSURE(inProgressLoadInformation != nullptr))
			return;

		const TShared<Resource> resource = package->DeserializeResource(resourceId);

		{
			Lock lock(mLoadedResourceMutex);
			inProgressLoadInformation->ResourceHandle.AssociateResourceWithHandle(resource, resourceId);

			if(B3D_ENSURE(inProgressLoadInformation->RemainingResourcesToLoadCount > 0))
				inProgressLoadInformation->RemainingResourcesToLoadCount--;

			if(resource != nullptr)
				resource->SetHandle(inProgressLoadInformation->ResourceHandle.GetWeak());
		}

		TryFinalizeLoad(inProgressLoadInformation);
		inProgressLoadInformation->LoadingEvent.Signal();
	};

	bool asyncLoad = loadOptions.AsynchronousLoad;

	// Disable async load if not supported
	RTTIType* const rttiType = IReflectable::GetRTTITypeFromTypeId(metaData->TypeId);
	if(asyncLoad && rttiType != nullptr)
	{
		if(const Resource* defaultObject = B3DRTTICast<Resource>(rttiType->GetDefaultObject()))
			asyncLoad = defaultObject->AllowAsyncLoading();
	}

	if(asyncLoad)
	{
		const String resourceFileName = metaData->Path.GetFilename();
		GetApplication().GetTaskScheduler().Post(SchedulerTask(fnLoadFromPackageAndFinalize, "Load resource", SchedulerTaskFlag::None, resourceFileName));
	}
	else 
	{
		fnLoadFromPackageAndFinalize();
	}

	return resourceHandle;
}

void Resources::TryFinalizeLoad(const TShared<InProgressLoadInformation>& inProgressLoadInformation)
{
	const UUID& resourceId = inProgressLoadInformation->ResourceHandle.GetId();

	TInlineArray<TShared<InProgressLoadInformation>, 4> dependantLoads;
	{
		Lock lock(mLoadedResourceMutex);

		bool isReadyToFinalizeLoad = inProgressLoadInformation->RemainingResourcesToLoadCount == 0 && !inProgressLoadInformation->LoadFinished;
		if(!isReadyToFinalizeLoad)
			return;

		// Mark the load as complete so we could have multiple in-progress loads for the same resource, so we don't finalize twice
		inProgressLoadInformation->LoadFinished = true;

		// Remove from in-progress map
		if(auto found = mInProgressLoadInformation.find(resourceId); B3D_ENSURE(found != mInProgressLoadInformation.end()))
		{
			TInlineArray<TShared<InProgressLoadInformation>, 1>& loadsPerResource = found->second;
			for(auto it = loadsPerResource.begin(); it != loadsPerResource.end(); ++it)
			{
				if((*it) == inProgressLoadInformation)
				{
					loadsPerResource.erase(it);
					break;
				}
			}

			if(loadsPerResource.Empty())
				mInProgressLoadInformation.erase(found);
		}

		// Add or update the loaded resource map
		LoadedResourceInformation* loadedResourceInformation;
		if(const auto foundLoadedResource = mLoadedResourceInformation.find(resourceId); foundLoadedResource != mLoadedResourceInformation.end())
		{
			loadedResourceInformation = foundLoadedResource->second.get();

			if(loadedResourceInformation->ResourceHandle == nullptr)
				loadedResourceInformation->ResourceHandle = inProgressLoadInformation->ResourceHandle.GetWeak();
		}
		else
		{
			UPtr<LoadedResourceInformation> newLoadedResourceInformation = B3DMakeUnique<LoadedResourceInformation>();
			newLoadedResourceInformation->ResourceHandle = inProgressLoadInformation->ResourceHandle.GetWeak();

			loadedResourceInformation = newLoadedResourceInformation.get();

			mLoadedResourceInformation[resourceId] = std::move(newLoadedResourceInformation);
		}

		if(inProgressLoadInformation->LoadOptions.LoadDependencies)
			loadedResourceInformation->DependenciesLoaded = true;

		if(inProgressLoadInformation->LoadOptions.KeepInternalReference)
		{
			loadedResourceInformation->InternalReferenceCount = 1;
			loadedResourceInformation->ResourceHandle.IncrementStrongReferenceCount();
		}

		inProgressLoadInformation->ResourceHandle.NotifyLoadComplete();

		// Record any dependants we need to notify, and decrement their load counts
		if(const auto found = mDependantResourceLoads.find(resourceId); found != mDependantResourceLoads.end())
		{
			dependantLoads = found->second;
			mDependantResourceLoads.erase(found);
		}

		for(const auto& dependantLoad : dependantLoads)
		{
			if(B3D_ENSURE(dependantLoad->RemainingResourcesToLoadCount > 0))
				dependantLoad->RemainingResourcesToLoadCount--;
		}
	}

	// Notify external code
	if(inProgressLoadInformation->ResourceHandle.IsLoaded(false))
		OnResourceLoaded(inProgressLoadInformation->ResourceHandle);

	if(!inProgressLoadInformation->LoadOptions.AsynchronousLoad && GetApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		ResourceListenerManager::Instance().NotifyListeners(resourceId);

	// See if any dependant load's remaining resource count reached 0, and try to finalize them
	for(const auto& dependantLoad : dependantLoads)
		TryFinalizeLoad(dependantLoad);
}

void Resources::ReleaseInternalReference(ResourceHandle& resource)
{
	const UUID& resourceId = resource.GetId();

	HResource resourceToDestroy;
	{
		Lock lock(mLoadedResourceMutex);
		if(auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
		{
			LoadedResourceInformation& loadedResourceInformation = *found->second;
			resourceToDestroy = loadedResourceInformation.ResourceHandle.Lock(); // Make sure resource goes out of scope outside of the mutex

			B3D_ASSERT(loadedResourceInformation.InternalReferenceCount > 0);
			loadedResourceInformation.InternalReferenceCount--;

			resource.DecrementStrongReferenceCount();
		}
	}

	resourceToDestroy = nullptr;
}

void Resources::UnloadAllUnused()
{
	FrameAllocatorScope frameScope;
	FrameVector<HResource> resourcesToUnload;

	{
		Lock lock(mLoadedResourceMutex);
		for(auto it = mLoadedResourceInformation.begin(); it != mLoadedResourceInformation.end(); ++it)
		{
			const LoadedResourceInformation& loadedResourceInformation = *it->second;

			const std::uint32_t referenceCount = loadedResourceInformation.ResourceHandle.mData->StrongReferenceCount.load(std::memory_order_relaxed);
			B3D_ASSERT(referenceCount > 0); // No references but kept in loaded resource map?

			if(referenceCount == loadedResourceInformation.InternalReferenceCount) // Only internal references exist, free it
				resourcesToUnload.push_back(loadedResourceInformation.ResourceHandle.Lock());
		}
	}

	// Note: When unloading multiple resources it's possible that unloading one will also unload
	// another resource in "resourcesToUnload". This is fine because "unload" deals with invalid
	// handles gracefully.
	for(auto& resource : resourcesToUnload)
	{
		ResourceHandleData* handleData = resource.GetHandleData();
		if(handleData == nullptr)
			continue;

		Destroy(*handleData);
	}
}

void Resources::UnloadAll()
{
	FrameAllocatorScope frameScope;
	FrameVector<HResource> resourcesToUnload;

	// Unload and invalidate all resources
	{
		Lock lock(mLoadedResourceMutex);
		for(auto it = mLoadedResourceInformation.begin(); it != mLoadedResourceInformation.end(); ++it)
		{
			const LoadedResourceInformation& loadedResourceInformation = *it->second;
			resourcesToUnload.push_back(loadedResourceInformation.ResourceHandle.Lock());
		}
	}

	for(auto& resource : resourcesToUnload)
	{
		ResourceHandleData* handleData = resource.GetHandleData();
		if(handleData == nullptr)
			continue;

		Destroy(*handleData);
	}
}

void Resources::Destroy(ResourceHandleData& handleData)
{
	const UUID& resourceId = handleData.Id;

	// Notify external systems before we actually destroy it
	OnResourceDestroyed(resourceId);

	{
		Lock lock(mLoadedResourceMutex);

		// TODO - Do I need to wait for in-progress loads to finish above?
		UPtr<LoadedResourceInformation> loadedResourceInformation;
		if(auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
		{
			loadedResourceInformation = std::move(found->second);
			mLoadedResourceInformation.erase(found);
		}

		if(loadedResourceInformation != nullptr)
		{
			while(loadedResourceInformation->InternalReferenceCount > 0)
			{
				loadedResourceInformation->InternalReferenceCount--;
				loadedResourceInformation->ResourceHandle.DecrementStrongReferenceCount();
			}
		}

		PackageManager& packageManager = GetPackageManager();
		if(const auto& packagePath = packageManager.TryGetPackagePathForResource(resourceId); packagePath.has_value())
		{
			AcquirePackageReadLockOptions readLockOptions(false, true, "Destroy resource");
			UPtr<PackageReadLock> packageReadLock;
			const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(*packagePath, readLockOptions, packageReadLock);
			if(lockResult != AcquirePackageLockResult::Acquired || packageReadLock == nullptr)
				return;

			const TShared<Package>& package = packageReadLock->GetPackage();
			if(!B3D_ENSURE(package != nullptr))
				return;

			package->UnloadResource(resourceId);
		}
	}

	if(handleData.Object != nullptr)
		handleData.Object->Destroy();

	handleData.Object = nullptr;
	handleData.IsCreated = false;
}

void Resources::SaveAsSinglePackage(const HResource& resource, const Path& folder, const String& name, const ResourceSaveOptions& saveOptions)
{
	if(folder.IsEmpty())
	{
		B3D_LOG(Warning, LogResources, "Cannot save resource. Provided folder is empty.");
		return;
	}

	if(name.empty())
	{
		B3D_LOG(Warning, LogResources, "Cannot save resource. Provided name is empty.");
		return;
	}

	if(!folder.IsAbsolute())
	{
		B3D_LOG(Warning, LogResources, "Cannot save resource. Provided folder is not absolute.");
		return;
	}

	if(!resource.IsLoaded(false))
	{
		B3D_LOG(Warning, LogResources, "Cannot save resource at path {0}/{1}.b3d. Provided resource is null or not loaded.", folder, name);
		return;
	}

	const String& packageFilename = name + Package::kPackageExtension;
	const Path packagePath = Path::Combine(folder, packageFilename);

	const TShared<Package> package = Package::Create(name);
	package->AddResource(name, resource);

	PackageManagerSavePackageOptions packageSaveOptions;
	packageSaveOptions.Compress = saveOptions.Compress;
	packageSaveOptions.Overwrite = saveOptions.Overwrite;
	packageSaveOptions.VirtualPathPrefix = saveOptions.VirtualPathPrefix;

	PackageManager& packageManager = GetPackageManager();
	packageManager.SavePackage(package, packagePath, packageSaveOptions);
}

void Resources::UpdateHandle(HResource& handle, const TShared<Resource>& resource)
{
	const UUID& uuid = handle.GetId();
	handle.AssociateResourceWithHandle(resource, uuid);
	handle.NotifyLoadComplete();

	OnResourceModified(handle);

	// Notify listeners immediately if on main thread
	if(GetApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		ResourceListenerManager::Instance().NotifyListeners(uuid);
}

void Resources::DestroyHandleData(ResourceHandleData& handleData)
{
	Lock lock(mResourceHandleMutex);

	if(const auto found = mHandles.find(handleData.Id); found != mHandles.end())
		mHandles.erase(found);
}

void Resources::UpdateResourcesFromPackage(const UPtr<PackageWriteLock>& packageWriteLock)
{
	if(!B3D_ENSURE(packageWriteLock != nullptr))
		return;

	const TShared<Package>& package = packageWriteLock->GetPackage();
	if(package == nullptr)
		return;

	const Vector<UUID> packageResourceIds = package->CreateResourceIdList();
	for(const auto& resourceId : packageResourceIds)
	{
		HResource resourceHandle;
		{
			Lock lock(mLoadedResourceMutex);

			if(const auto found = mLoadedResourceInformation.find(resourceId); found != mLoadedResourceInformation.end())
				resourceHandle = found->second->ResourceHandle.Lock();
		}

		if(resourceHandle == nullptr)
		{
			package->UnloadResource(resourceId);
			continue;
		}

		const TShared<Resource> resource = package->LoadResource(resourceId);
		if(resource == nullptr)
		{
			B3D_LOG(Warning, LogResources, "Failed to update resource '{0}' with new data from the package. Unknown error.", resourceId);
			continue;
		}

		if(resourceHandle.GetShared() != resource)
			UpdateHandle(resourceHandle, resource);
	}
}

bool Resources::IsLoaded(const UUID& uuid, bool checkInProgress)
{
	{
		Lock loadedLock(mLoadedResourceMutex);

		if(auto found = mLoadedResourceInformation.find(uuid); found != mLoadedResourceInformation.end())
			return true;

		if(auto found = mInProgressLoadInformation.find(uuid); checkInProgress && found != mInProgressLoadInformation.end())
			return true;
	}

	return false;
}

float Resources::GetLoadProgress(const HResource& resource)
{
	UnorderedMap<UUID, LoadProgress> loadProgressMap;
	GetLoadProgressRecursive(resource, loadProgressMap);

	u64 totalSize = 0;
	u64 loadedSize = 0;
	for(const auto& entry : loadProgressMap)
	{
		totalSize += entry.second.TotalSize;
		loadedSize += (u64)((double)entry.second.TotalSize * entry.second.Progress);
	}

	return totalSize != 0 ? (float)((double)loadedSize / (double)totalSize) : 0.0f;
}

void Resources::GetLoadProgressRecursive(const HResource& resource, UnorderedMap<UUID, LoadProgress>& loadProgressMap)
{
	const UUID& resourceId = resource.GetId();
	if(resourceId.Empty())
		return;

	auto fnGetLoadProgress = [](const UUID& resourceId) -> LoadProgress
	{
		PackageManager& packageManager = GetPackageManager();

		TOptional<Path> maybePackagePath = packageManager.TryGetPackagePathForResource(resourceId);
		if(!maybePackagePath.has_value())
			return LoadProgress();

		const Path& packagePath = *maybePackagePath;

		AcquirePackageReadLockOptions readLockOptions(false, true, "GetLoadProgress");
		UPtr<PackageReadLock> packageReadLock;
		const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(packagePath, readLockOptions, packageReadLock);
		if(lockResult != AcquirePackageLockResult::Acquired || packageReadLock == nullptr)
			return LoadProgress();

		const TShared<Package>& package = packageReadLock->GetPackage();
		if(!B3D_ENSURE(package != nullptr))
			return LoadProgress();

		const float progress = package->GetResourceLoadProgress(resourceId);
		const u64 size = package->GetResourceSizeInDataStream(resourceId);

		return LoadProgress(size, progress);
	};

	LoadProgress selfLoadProgress = fnGetLoadProgress(resourceId);

	UnorderedMap<UUID, HResource> uniqueDependencies;

	{
		Lock lock(mLoadedResourceMutex);

		if(const auto found = mInProgressLoadInformation.find(resourceId); found != mInProgressLoadInformation.end())
		{
			const TInlineArray<TShared<InProgressLoadInformation>, 1>& inProgressLoadsPerResource = found->second;
			for(const TShared<InProgressLoadInformation>& entry : inProgressLoadsPerResource)
			{
				if(!B3D_ENSURE(entry != nullptr))
					continue;

				for(const auto& dependency : entry->DependencyResourceHandles)
					uniqueDependencies[dependency.GetId()] = dependency;
			}
		}
		else
		{
			// Either fully loaded or not being loaded at all, we don't even care about dependencies
			if(const auto foundLoaded = mLoadedResourceInformation.find(resourceId); foundLoaded != mLoadedResourceInformation.end())
				selfLoadProgress.Progress = 1.0f;
			else
				selfLoadProgress.Progress = 0.0f;
		}
	}

	for(const auto& dependency : uniqueDependencies)
		GetLoadProgressRecursive(dependency.second, loadProgressMap);

	loadProgressMap[resourceId] = selfLoadProgress;
}

HResource Resources::CreateResourceHandle(const TShared<Resource>& resource)
{
	if(resource == nullptr)
		return nullptr;

	const UUID& uuid = resource->GetId().Empty() ? UUIDGenerator::GenerateRandom() : resource->GetId();
	return CreateResourceHandle(resource, uuid);
}

HResource Resources::CreateResourceHandle(const TShared<Resource>& resource, const UUID& resourceId)
{
	HResource newHandle(resource, resourceId);

	{
		Lock handleLock(mResourceHandleMutex);

		if(resource)
		{
			Lock lock(mLoadedResourceMutex);

			// Ensure no handles with the same ID already exist
			auto found = mLoadedResourceInformation.find(resourceId);
			if(!B3D_ENSURE(found == mLoadedResourceInformation.end()))
				return {};

			resource->SetHandle(newHandle.GetWeak());

			UPtr<LoadedResourceInformation> loadedResourceInformation = B3DMakeUnique<LoadedResourceInformation>();
			loadedResourceInformation->ResourceHandle = newHandle.GetWeak();

			mLoadedResourceInformation[resourceId] = std::move(loadedResourceInformation);
		}

		mHandles[resourceId] = newHandle.GetHandleData();
	}

	return newHandle;
}

HResource Resources::GetOrCreateResourceHandle(const UUID& resourceId)
{
	Lock handleLock(mResourceHandleMutex);
	if(auto found = mHandles.find(resourceId); found != mHandles.end()) // Not loaded, but handle does exist
		return TResourceHandle<Resource>(found->second);

	// Create new handle
	HResource handle(resourceId);
	mHandles[resourceId] = handle.GetHandleData();

	return handle;
}

namespace b3d
{
B3D_EXPORT Resources& GetResources()
{
	return Resources::Instance();
}
} // namespace b3d
