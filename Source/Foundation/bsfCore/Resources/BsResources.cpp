//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResources.h"

#include "BsApplication.h"
#include "Resources/BsResource.h"
#include "Resources/BsResourceManifest.h"
#include "Error/BsException.h"
#include "Serialization/BsFileSerializer.h"
#include "FileSystem/BsFileSystem.h"
#include "Utility/BsUUID.h"
#include "Debug/BsDebug.h"
#include "Utility/BsUtility.h"
#include "Resources/BsSavedResourceData.h"
#include "Managers/BsResourceListenerManager.h"
#include "Utility/BsCompression.h"
#include "FileSystem/BsDataStream.h"
#include "Serialization/BsBinarySerializer.h"
#include "Reflection/BsRTTIType.h"
#include "BsCoreApplication.h"
#include "Threading/BsScheduler.h"

using namespace bs;

const ResourceLoadOptions ResourceLoadOptions::kDefault;

Resources::Resources()
{
	{
		Lock lock(mDefaultManifestMutex);
		mDefaultResourceManifest = ResourceManifest::Create("Default");
		mResourceManifests.push_back(mDefaultResourceManifest);
	}
}

Resources::~Resources()
{
	UnloadAll();
}

HResource Resources::Load(const Path& filePath, ResourceLoadFlags loadFlags)
{
	UUID uuid;
	Path physicalFilePath = EnsurePhysicalPath(filePath);

	if(!FileSystem::IsFile(physicalFilePath))
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. Specified file: {0} doesn't exist.", filePath);
		return HResource();
	}

	bool foundUUID = GetUUIDFromFilePath(physicalFilePath, uuid);

	if(!foundUUID)
		uuid = UUIDGenerator::GenerateRandom();

	return LoadInternal(uuid, physicalFilePath, true, loadFlags).Resource;
}

HResource Resources::Load(const Path& resourcePath, const ResourceLoadOptions& loadOptions)
{
	PackageManager& packageManager = GetPackageManager();

	Optional<ResourcePackagePath> maybeResourcePackagePath = packageManager.TryResolvePhysicalResourcePath(resourcePath);
	if(!maybeResourcePackagePath.has_value()) // Maybe it's a virtual path
		maybeResourcePackagePath = packageManager.TryResolveVirtualResourcePath(resourcePath);

	if(!maybeResourcePackagePath.has_value())
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. File at path '{0}' doesn't exist.", resourcePath);
		return nullptr;
	}

	const ResourcePackagePath& resourcePackagePath = *maybeResourcePackagePath;

	AcquirePackageReadLockOptions readLockOptions(true, true, "Load resource");
	UPtr<PackageReadLock> packageReadLock;
	const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(resourcePackagePath.PhysicalPackagePath, readLockOptions, packageReadLock);
	if(!B3D_ENSURE(lockResult == AcquirePackageLockResult::Acquired && packageReadLock == nullptr))
		return nullptr;

	const SPtr<Package>& package = packageReadLock->GetPackage();
	if(!B3D_ENSURE(package != nullptr))
		return nullptr;

	const SPtr<const PackageResourceMetaData>& resourceMetaData = package->GetResourceMetaData(resourcePackagePath.ResourcePathWithinPackage);
	if(resourceMetaData == nullptr)
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. Resource with path '{0}' cannot be found in package '{1}'.", resourcePackagePath.ResourcePathWithinPackage, resourcePackagePath.PhysicalPackagePath);
	}

	const UUID& resourceId = resourceMetaData->Id;

	return Load(std::move(packageReadLock), resourceId, loadOptions);
}

HResource Resources::Load(const UUID& resourceId, const ResourceLoadOptions& loadOptions)
{
	PackageManager& packageManager = GetPackageManager();

	const Optional<Path> maybePackagePath = packageManager.TryGetPackagePathForResource(resourceId);
	if(!maybePackagePath.has_value())
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. Resource with ID '{0}' doesn't exist.", resourceId);
		return nullptr;
	}

	const Path& packagePath = *maybePackagePath;

	AcquirePackageReadLockOptions readLockOptions(true, true, "Load resource");
	UPtr<PackageReadLock> packageReadLock;
	const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(packagePath, readLockOptions, packageReadLock);
	if(!B3D_ENSURE(lockResult == AcquirePackageLockResult::Acquired && packageReadLock == nullptr))
		return nullptr;

	return Load(std::move(packageReadLock), resourceId, loadOptions);
}

HResource Resources::Load(const TWeakResourceHandle<Resource>& handle, ResourceLoadFlags loadFlags)
{
	if(handle.mData == nullptr)
		return HResource();

	UUID uuid = handle.GetId();
	return LoadFromUuid(uuid, false, loadFlags);
}

HResource Resources::LoadAsync(const Path& filePath, ResourceLoadFlags loadFlags)
{
	UUID uuid;
	Path physicalFilePath = EnsurePhysicalPath(filePath);

	if(!FileSystem::IsFile(physicalFilePath))
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. Specified file: {0} doesn't exist.", physicalFilePath);
		return HResource();
	}

	bool foundUUID = GetUUIDFromFilePath(physicalFilePath, uuid);

	if(!foundUUID)
		uuid = UUIDGenerator::GenerateRandom();

	return LoadInternal(uuid, physicalFilePath, false, loadFlags).Resource;
}

HResource Resources::LoadFromUuid(const UUID& uuid, bool async, ResourceLoadFlags loadFlags)
{
	Path filePath;
	GetFilePathFromUuid(uuid, filePath);

	return LoadInternal(uuid, filePath, !async, loadFlags).Resource;
}

Resources::LoadInfo Resources::LoadInternal(const UUID& uuid, const Path& filePath, bool synchronous, ResourceLoadFlags loadFlags)
{
	LoadInfo output;

	// Retrieve/create resource handle, and register with the system
	bool loadInProgress = false;
	bool loadFailed = false;
	bool initiateLoad = false;
	Vector<UUID> dependenciesToLoad;
	{
		bool alreadyLoading = false;

		// Check if the resource is being loaded on a worker thread
		Lock inProgressLock(mInProgressResourcesMutex);
		Lock loadedLock(mLoadedResourceMutex);

		auto iterFind2 = mInProgressResources.find(uuid);
		if(iterFind2 != mInProgressResources.end())
		{
			LoadedResourceData& resData = iterFind2->second->ResData;
			output.Resource = resData.Resource.Lock();
			output.State = LoadInfo::AlreadyInProgress;
			output.Size = resData.Size;

			// Increase ref. count
			if(loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
			{
				resData.NumInternalRefs++;
				output.Resource.IncrementInternalReferenceCount();
			}

			loadInProgress = true;
			alreadyLoading = true;
		}

		// Check if the resource is already loaded
		auto iterFind = mLoadedResources.find(uuid);
		if(iterFind != mLoadedResources.end())
		{
			LoadedResourceData& resData = iterFind->second;
			output.Resource = resData.Resource.Lock();
			output.State = LoadInfo::AlreadyLoaded;
			output.Size = resData.Size;

			// Increase ref. count
			if(loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
			{
				resData.NumInternalRefs++;
				output.Resource.IncrementInternalReferenceCount();
			}

			alreadyLoading = true;
		}

		// Not loaded and not in progress, register a new handle or find a pre-registered one
		if(!alreadyLoading)
		{
			output.State = LoadInfo::Loading;
			output.Size = 0;

			auto iterFind = mHandles.find(uuid);
			if(iterFind != mHandles.end())
				output.Resource = iterFind->second.Lock();
			else
			{
				output.Resource = HResource(uuid);
				mHandles[uuid] = output.Resource.GetWeak();
			}
		}

		// If we have nowhere to load from, warn and complete load if a file path was provided, otherwise pass through
		// as we might just want to complete a previously queued load
		if(filePath.IsEmpty())
		{
			if(!alreadyLoading)
			{
				B3D_LOG(Verbose, Resources, "Cannot load resource. Resource with UUID '{0}' doesn't exist.", uuid);
				loadFailed = true;
			}
		}
		else if(!FileSystem::IsFile(filePath))
		{
			B3D_LOG(Verbose, Resources, "Cannot load resource. Specified file: '{0}' doesn't exist.", filePath);
			loadFailed = true;
		}

		bool loadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
		if(!loadFailed)
		{
			// Load dependency data if a file path is provided
			SPtr<SavedResourceData> savedResourceData;
			if(!filePath.IsEmpty())
			{
				// Note: Ideally this data gets cached eventually (e.g. as part of the manifest). When loading objects
				// with a lot of dependencies (e.g. scenes) this will get called for every dependency, synchronously,
				// which might take a while. It would be nice to just read it from a single location. Another option is
				// to make this whole block asynchronous so every dependency does it on its own thread.
				FileDecoder fs(filePath);
				savedResourceData = std::static_pointer_cast<SavedResourceData>(fs.Decode());
				output.Size = fs.GetSize();
			}

			// Register an in-progress load unless there is an existing load operation, or the resource is already
			// loaded
			if(!alreadyLoading)
			{
				ResourceLoadData* loadData = B3DNew<ResourceLoadData>(output.Resource.GetWeak(), 0, output.Size);
				mInProgressResources[uuid] = loadData;

				if(loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
				{
					loadData->ResData.NumInternalRefs++;
					output.Resource.IncrementInternalReferenceCount();
				}

				loadData->RemainingDependencies = 1; // Self
				loadData->Progress.store(0.0f, std::memory_order_relaxed);

				// Make resource listener trigger before exit if loading synchronously on the main thread
				loadData->NotifyImmediately = synchronous && B3D_CURRENT_THREAD_ID == GetCoreApplication().GetMainThreadId();

				// Register dependencies and count them so we know when the resource is fully loaded
				if(loadDependencies && savedResourceData != nullptr)
				{
					for(auto& dependency : savedResourceData->GetDependencies())
					{
						if(dependency != uuid)
						{
							mDependantLoads[dependency].push_back(loadData);
							loadData->RemainingDependencies++;
							dependenciesToLoad.push_back(dependency);
						}
					}
				}
			}
			// The resource is already being loaded, or is loaded, but we might still need to load some dependencies
			else if(loadDependencies && savedResourceData != nullptr)
			{
				const Vector<UUID>& dependencies = savedResourceData->GetDependencies();
				if(!dependencies.empty())
				{
					ResourceLoadData* loadData = nullptr;

					// If load not in progress, register the resource for load
					if(!loadInProgress)
					{
						loadData = B3DNew<ResourceLoadData>(output.Resource.GetWeak(), 0, output.Size);
						loadData->RemainingDependencies = 0;
						loadData->Progress.store(0.0f, std::memory_order_relaxed);

						// Make resource listener trigger before exit if loading synchronously
						loadData->NotifyImmediately = synchronous && B3D_CURRENT_THREAD_ID == GetCoreApplication().GetMainThreadId();
					}
					else
						loadData = mInProgressResources[uuid];

					// Find dependencies that aren't already loaded or queued for loading
					for(auto& dependency : dependencies)
					{
						if(dependency != uuid)
						{
							bool registerDependency = false;

							auto iterFind3 = mLoadedResources.find(dependency);
							if(iterFind3 == mLoadedResources.end())
							{
								registerDependency = true;

								auto iterFind2 = mDependantLoads.find(dependency);
								if(iterFind2 != mDependantLoads.end())
								{
									Vector<ResourceLoadData*>& dependantData = iterFind2->second;
									auto iterFind3 = std::find_if(dependantData.begin(), dependantData.end(), [&](ResourceLoadData* x)
																  { return x->ResData.Resource.GetId() == output.Resource.GetId(); });

									registerDependency = iterFind3 == dependantData.end();
								}
							}

							if(registerDependency)
							{
								mDependantLoads[dependency].push_back(loadData);
								loadData->RemainingDependencies++;
								dependenciesToLoad.push_back(dependency);
							}
						}
					}

					if(!loadInProgress)
					{
						if(!dependenciesToLoad.empty())
							mInProgressResources[uuid] = loadData;
						else
							B3DDelete(loadData);
					}
				}
			}

			initiateLoad = !alreadyLoading && !filePath.IsEmpty();

			if(savedResourceData != nullptr)
				synchronous = synchronous || !savedResourceData->AllowAsyncLoading();
		}
	}

	// Something went wrong, clean up and exit
	if(loadFailed)
	{
		output.State = LoadInfo::Failed;
		output.Size = 0;

		// Clean up in-progress state
		LoadComplete(output.Resource, true);
		return output;
	}

	// Load dependencies (before the main resource)
	const auto numDependencies = (u32)dependenciesToLoad.size();
	if(numDependencies > 0)
	{
		ResourceLoadFlags depLoadFlags = ResourceLoadFlag::LoadDependencies;
		if(loadFlags.IsSet(ResourceLoadFlag::KeepSourceData))
			depLoadFlags |= ResourceLoadFlag::KeepSourceData;

		Vector<HResource> dependencies(numDependencies);
		u32 dependencySize = 0;
		for(u32 i = 0; i < numDependencies; i++)
		{
			const UUID& depUUID = dependenciesToLoad[i];

			Path depFilePath;
			GetFilePathFromUuid(depUUID, depFilePath);

			LoadInfo loadInfo = LoadInternal(depUUID, depFilePath, synchronous, depLoadFlags);
			dependencies[i] = loadInfo.Resource;

			// Calculate the size of dependencies that still need to be loaded, for progress reporting
			if(loadInfo.State == LoadInfo::Loading || loadInfo.State == LoadInfo::AlreadyInProgress)
			{
				// Note: Technically, since we're queuing the dependency load with no locking, the load could complete
				// before the size of the dependency has been registered, which means getLoadProgress() method would
				// incorrectly report the progress to be higher than it should be. If that becomes an issue then this
				// operation is better to be moved to the child loadInternal() call.
				dependencySize += loadInfo.Size;
			}
		}

		// Keep dependencies alive until the parent is done loading, and record total size of dependencies to load
		{
			Lock inProgressLock(mInProgressResourcesMutex);

			// If we're doing a dependency-only load (main resource itself was previously loaded), then the in-progress
			// operation could have already finished when the last dependency was loaded (this will always be true for
			// synchronous loads), and no need to register dependencies.
			const auto iterFind = mInProgressResources.find(uuid);
			if(iterFind != mInProgressResources.end())
			{
				iterFind->second->DependencySize = dependencySize;
				iterFind->second->Dependencies = dependencies;
			}
		}
	}

	// Check if resource load already started on another thread (in case it was already being loaded), in which case
	// we want to wait
	bool waitOnLoadInProgress = false;
	SPtr<SignalEvent> loadingEvent;
	{
		Lock inProgressLock(mInProgressResourcesMutex);

		const auto iterFind = mInProgressResources.find(uuid);
		if(iterFind != mInProgressResources.end())
		{
			if(iterFind->second->LoadStarted)
			{
				waitOnLoadInProgress = true;
				loadingEvent = iterFind->second->LoadingEvent;
			}
			else
				iterFind->second->LoadStarted = true;
		}
	}

	// Previously being loaded as async but now we want it synced, so we wait
	if(loadInProgress && synchronous && waitOnLoadInProgress)
	{
		if(loadingEvent)
			loadingEvent->Wait();

		output.Resource.BlockUntilLoaded(false);
	}

	// Actually start the file read operation if not already loaded or in progress
	if(initiateLoad)
	{
		// Synchronous or the resource doesn't support async, read the file immediately
		if(synchronous)
		{
			LoadCallback(filePath, output.Resource, loadFlags.IsSet(ResourceLoadFlag::KeepSourceData));
		}
		else // Asynchronous, read the file on a worker thread
		{
			String fileName = filePath.GetFilename();
			String taskName = "Resource load: " + fileName;

			bool keepSourceData = loadFlags.IsSet(ResourceLoadFlag::KeepSourceData);

			// Register the task
			{
				Lock inProgressLock(mInProgressResourcesMutex);

				SPtr<SignalEvent> loadingEvent = B3DMakeShared<SignalEvent>();

				const auto iterFind = mInProgressResources.find(uuid);
				if (iterFind != mInProgressResources.end())
					iterFind->second->LoadingEvent = loadingEvent;

				GetCoreApplication().GetTaskScheduler().Post(SchedulerTask([this, loadingEvent, filePath, resource = output.Resource, keepSourceData]() mutable
				{
					LoadCallback(filePath, resource, keepSourceData);
					loadingEvent->Signal();
				}, "ResourceLoad", SchedulerTaskFlag::None, fileName));
			}
		}
	}
	else
	{
		if(!loadInProgress)
		{
			// Already loaded, decrement dependency count
			LoadComplete(output.Resource, false);
		}
	}

	output.State = LoadInfo::Loading;
	return output;
}

HResource Resources::Load(UPtr<PackageReadLock> packageReadLock, const UUID& resourceId, const ResourceLoadOptions& loadOptions)
{
	const SPtr<Package>& package = packageReadLock->GetPackage();
	const SPtr<const PackageResourceMetaData>& metaData = package->GetResourceMetaData(resourceId);

	if(metaData == nullptr)
	{
		B3D_LOG(Warning, Resources, "Cannot load resource. Resource with ID: {0} cannot be found in the package.", resourceId);
		return HResource();
	}

	HResource resourceHandle = GetOrCreateResourceHandle(resourceId);

	SPtr<InProgressLoadInformation> inProgressLoadInformation;
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
					loadedResourceInformation->ResourceHandle.IncrementInternalReferenceCount();
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
		const SPtr<InProgressLoadInformation> inProgressLoadInformation = inProgressLoadInformationWeak.lock();
		if(!B3D_ENSURE(inProgressLoadInformation != nullptr))
			return;

		const SPtr<Resource> resource = package->DeserializeResource(resourceId);

		{
			Lock lock(mLoadedResourceMutex);
			inProgressLoadInformation->ResourceHandle.SetHandleData(resource, resourceId);

			if(B3D_ENSURE(inProgressLoadInformation->RemainingResourcesToLoadCount > 0))
				inProgressLoadInformation->RemainingResourcesToLoadCount--;

			if(resource != nullptr)
				resource->SetHandle(inProgressLoadInformation->ResourceHandle.GetWeak());
		}

		TryFinalizeLoad(inProgressLoadInformation);
		inProgressLoadInformation->LoadingEvent.Signal();
	};

	bool asyncLoad = loadOptions.AsynchronousLoad;
	//if(!Resources::SupportsAsyncLoad(metaData->TypeId)) // TODO - Implement this by reading the value from a RTTI default object
	//	asyncLoad = false;

	if(asyncLoad)
	{
		const String resourceFileName = metaData->Path.GetFilename();
		GetCoreApplication().GetTaskScheduler().Post(SchedulerTask(fnLoadFromPackageAndFinalize, "Load resource", SchedulerTaskFlag::None, resourceFileName));
	}
	else 
	{
		fnLoadFromPackageAndFinalize();
	}

	return resourceHandle;
}

void Resources::TryFinalizeLoad(const SPtr<InProgressLoadInformation>& inProgressLoadInformation)
{
	const UUID& resourceId = inProgressLoadInformation->ResourceHandle.GetId();

	TInlineArray<SPtr<InProgressLoadInformation>, 4> dependantLoads;
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
			TInlineArray<SPtr<InProgressLoadInformation>, 1>& loadsPerResource = found->second;
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
			loadedResourceInformation->ResourceHandle.IncrementInternalReferenceCount();
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

	if(!inProgressLoadInformation->LoadOptions.AsynchronousLoad && GetCoreApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		ResourceListenerManager::Instance().NotifyListeners(resourceId);

	// See if any dependant load's remaining resource count reached 0, and try to finalize them
	for(const auto& dependantLoad : dependantLoads)
		TryFinalizeLoad(dependantLoad);
}

SPtr<Resource> Resources::LoadFromDiskAndDeserialize(const Path& filePath, bool loadWithSaveData, std::atomic<float>& progress)
{
	SPtr<DataStream> stream = FileSystem::OpenFile(filePath, true);
	if(stream == nullptr)
		return nullptr;

	if(stream->Size() > std::numeric_limits<u32>::max())
	{
		B3D_EXCEPT(InternalErrorException, "File size is larger that u32 can hold. Ask a programmer to use a bigger data type.");
	}

	RTTIOperationEngineContext rttiOperationContext;
	rttiOperationContext.Flags = loadWithSaveData ? SF_KeepResourceSourceData : 0;

	// Read meta-data
	SPtr<SavedResourceData> metaData;
	{
		if(!stream->Eof())
		{
			u32 objectSize = 0;
			stream->Read(&objectSize, sizeof(objectSize));

			BinarySerializer bs;
			metaData = std::static_pointer_cast<SavedResourceData>(bs.Decode(stream, objectSize, rttiOperationContext));
		}
	}

	// Read resource data
	SPtr<IReflectable> loadedData;
	{
		if(metaData && !stream->Eof())
		{
			u32 objectSize = 0;
			stream->Read(&objectSize, sizeof(objectSize));

			if(metaData->GetCompressionMethod() != 0)
			{
				SPtr<MemoryDataStream> decompressedDataStream = B3DMakeShared<MemoryDataStream>();
				Compression::Decompress(*stream, *decompressedDataStream, 0, CompressionType::Default, [&progress](float val) { progress.exchange(val * 0.9f, std::memory_order_relaxed); });

				stream = decompressedDataStream;
				stream->Seek(0);

				BinarySerializer bs;
				loadedData = bs.Decode(stream, objectSize, rttiOperationContext, BinarySerializerFlag::None, [&progress](float val)
									   { progress.exchange(0.9f + val * 0.1f, std::memory_order_relaxed); });
			}
			else
			{
				BinarySerializer bs;
				loadedData = bs.Decode(stream, objectSize, rttiOperationContext, BinarySerializerFlag::None, [&progress](float val)
									   { progress.exchange(val, std::memory_order_relaxed); });
			}
		}
	}

	if(loadedData == nullptr)
		B3D_LOG(Error, Resources, "Unable to load resource at path \"{0}\"", filePath);
	else
	{
		if(!loadedData->IsDerivedFrom(Resource::GetRttiStatic()))
			B3D_EXCEPT(InternalErrorException, "Loaded class doesn't derive from Resource.");
	}

	SPtr<Resource> resource = std::static_pointer_cast<Resource>(loadedData);
	return resource;
}

void Resources::ReleaseInternalReference(ResourceHandle& resource)
{
	const UUID& uuid = resource.GetId();

	{
		bool loadInProgress = false;

		{
			Lock inProgressLock(mInProgressResourcesMutex);
			auto iterFind2 = mInProgressResources.find(uuid);
			if(iterFind2 != mInProgressResources.end())
				loadInProgress = true;
		}

		// Technically we should be able to just cancel a load in progress instead of blocking until it finishes.
		// However that would mean the last reference could get lost on whatever thread did the loading, which
		// isn't something that's supported. If this ends up being a problem either make handle counting atomic
		// or add a separate queue for objects destroyed from the load threads.
		if(loadInProgress)
			resource.BlockUntilLoaded();

		bool lostLastRef = false;
		{
			Lock loadedLock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if(iterFind != mLoadedResources.end())
			{
				LoadedResourceData& resData = iterFind->second;

				B3D_ASSERT(resData.NumInternalRefs > 0);
				resData.NumInternalRefs--;
				resource.DecrementInternalReferenceCount();

				std::uint32_t refCount = resource.GetHandleData()->ReferenceCount.load(std::memory_order_relaxed);
				lostLastRef = refCount == 0;
			}
		}

		if(lostLastRef)
			Destroy(resource);
	}
}

void Resources::UnloadAllUnused()
{
	Vector<HResource> resourcesToUnload;

	{
		Lock lock(mLoadedResourceMutex);
		for(auto iter = mLoadedResources.begin(); iter != mLoadedResources.end(); ++iter)
		{
			const LoadedResourceData& resData = iter->second;

			std::uint32_t refCount = resData.Resource.mData->ReferenceCount.load(std::memory_order_relaxed);
			B3D_ASSERT(refCount > 0); // No references but kept in mLoadedResources list?

			if(refCount == resData.NumInternalRefs) // Only internal references exist, free it
				resourcesToUnload.push_back(resData.Resource.Lock());
		}
	}

	// Note: When unloading multiple resources it's possible that unloading one will also unload
	// another resource in "resourcesToUnload". This is fine because "unload" deals with invalid
	// handles gracefully.
	for(auto iter = resourcesToUnload.begin(); iter != resourcesToUnload.end(); ++iter)
	{
		ReleaseInternalReference(*iter);
	}
}

void Resources::UnloadAll()
{
	// Unload and invalidate all resources
	UnorderedMap<UUID, LoadedResourceData> loadedResourcesCopy;

	{
		Lock lock(mLoadedResourceMutex);
		loadedResourcesCopy = mLoadedResources;
	}

	for(auto& loadedResourcePair : loadedResourcesCopy)
		Destroy(loadedResourcePair.second.Resource);
}

void Resources::Destroy(ResourceHandle& resource)
{
	if(resource.mData == nullptr)
		return;

	// TODO - Begin deprecated
	RecursiveLock lock(mDestroyMutex);

	// If load in progress, first wait until it completes
	const UUID& resourceId = resource.GetId();
	if(!resource.IsLoaded(false))
	{
		bool loadInProgress = false;
		{
			Lock lock(mInProgressResourcesMutex);
			auto iterFind2 = mInProgressResources.find(resourceId);
			if(iterFind2 != mInProgressResources.end())
				loadInProgress = true;
		}

		if(loadInProgress) // If it's still loading wait until that finishes
			resource.BlockUntilLoaded();
		else
			return; // Already unloaded
	}

	// At this point resource is guaranteed to be loaded and this state cannot change by some other thread because of
	// the mDestroyMutex lock
	// TODO - End deprecated

	// Notify external systems before we actually destroy it
	OnResourceDestroyed(resourceId);

	{
		Lock lock(mLoadedResourceMutex);
		auto iterFind = mLoadedResources.find(resourceId);
		if(iterFind != mLoadedResources.end())
		{
			LoadedResourceData& resData = iterFind->second;
			while(resData.NumInternalRefs > 0)
			{
				resData.NumInternalRefs--;
				resData.Resource.DecrementInternalReferenceCount();
			}

			mLoadedResources.erase(iterFind);
		}
		else
		{
			B3D_ASSERT(false); // This should never happen but in case it does fail silently in release mode
		}

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
				loadedResourceInformation->ResourceHandle.DecrementInternalReferenceCount();
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

			const SPtr<Package>& package = packageReadLock->GetPackage();
			if(!B3D_ENSURE(package != nullptr))
				return;

			package->UnloadResource(resourceId);
		}
	}

	resource.mData->Object->Destroy();
	resource.ClearHandleData();
}

void Resources::Save(const HResource& resource, const Path& filePath, bool overwrite, bool compress)
{
	if(resource == nullptr)
		return;

	if(!resource.IsLoaded(false))
	{
		bool loadInProgress = false;
		{
			Lock lock(mInProgressResourcesMutex);
			auto iterFind2 = mInProgressResources.find(resource.GetId());
			if(iterFind2 != mInProgressResources.end())
				loadInProgress = true;
		}

		if(loadInProgress) // If it's still loading wait until that finishes
			resource.BlockUntilLoaded();
		else
			return; // Nothing to save
	}

	const bool fileExists = FileSystem::IsFile(filePath);
	if(fileExists && !overwrite)
	{
		B3D_LOG(Error, Resources, "Another file exists at the specified location. Not saving.");
		return;
	}

	{
		Lock lock(mDefaultManifestMutex);
		mDefaultResourceManifest->RegisterResource(resource.GetId(), filePath);
	}

	SaveInternal(resource.GetShared(), filePath, compress);
}

void Resources::Save(const HResource& resource, bool compress)
{
	if(resource == nullptr)
		return;

	Path path;
	if(GetFilePathFromUuid(resource.GetId(), path))
		Save(resource, path, true, compress);
}

void Resources::SaveAsSinglePackage(const HResource& resource, const Path& folder, const String& name, const ResourceSaveOptions& saveOptions)
{
	if(folder.IsEmpty())
	{
		B3D_LOG(Warning, Resources, "Cannot save resource. Provided folder is empty.");
		return;
	}

	if(name.empty())
	{
		B3D_LOG(Warning, Resources, "Cannot save resource. Provided name is empty.");
		return;
	}

	if(!folder.IsAbsolute())
	{
		B3D_LOG(Warning, Resources, "Cannot save resource. Provided folder is not absolute.");
		return;
	}

	if(!resource.IsLoaded(false))
	{
		B3D_LOG(Warning, Resources, "Cannot save resource at path {0}/{1}.b3d. Provided resource is null or not loaded.", folder, name);
		return;
	}

	const String& packageFilename = name + Package::kPackageExtension;
	const Path packagePath = Path::Combine(folder, packageFilename);

	const SPtr<Package> package = Package::Create(name);
	package->AddResource(name, resource);

	SavePackageOptions packageSaveOptions;
	packageSaveOptions.Compress = saveOptions.Compress;
	packageSaveOptions.Overwrite = saveOptions.Overwrite;
	packageSaveOptions.VirtualPathPrefix = saveOptions.VirtualPathPrefix;

	PackageManager& packageManager = GetPackageManager();
	packageManager.SavePackage(package, packagePath, packageSaveOptions);
}

void Resources::SaveInternal(const SPtr<Resource>& resource, const Path& filePath, bool compress)
{
	if(!resource->mKeepSourceData)
	{
		B3D_LOG(Warning, Resources, "Saving a resource that was created/loaded without KeepSourceData flag."
								   "Some data might not be available for saving. File path: {0}",
			   filePath);
	}

	Vector<ResourceDependency> dependencyList = Utility::FindResourceDependencies(*resource);
	Vector<UUID> dependencyUUIDs(dependencyList.size());
	for(u32 i = 0; i < (u32)dependencyList.size(); i++)
		dependencyUUIDs[i] = dependencyList[i].Resource.GetId();

	u32 compressionMethod = (compress && resource->IsCompressible()) ? 1 : 0;
	SPtr<SavedResourceData> resourceData = B3DMakeShared<SavedResourceData>(dependencyUUIDs, resource->AllowAsyncLoading(), compressionMethod);

	Path parentDir = filePath.GetDirectory();
	if(!FileSystem::Exists(parentDir))
		FileSystem::CreateDir(parentDir);

	Path savePath;
	const bool fileExists = FileSystem::IsFile(filePath);
	if(fileExists)
	{
		// If a file exists, save to a temporary location, then copy over only after a save was successful. This guards
		// against data loss in case the save process fails.

		// TODO: Temp directory should always be on this drive, as files moved from one drive to another will in fact
		// be copied
		savePath = FileSystem::GetTempDirectoryPath();
		savePath.SetFilename(UUIDGenerator::GenerateRandom().ToString());

		u32 safetyCounter = 0;
		while(FileSystem::Exists(savePath))
		{
			if(safetyCounter > 10)
			{
				B3D_LOG(Error, Resources, "Internal error. Unable to save resource due to not being able to find a unique filename.");
				return;
			}

			savePath.SetFilename(UUIDGenerator::GenerateRandom().ToString());
			safetyCounter++;
		}
	}
	else
		savePath = filePath;

	SPtr<DataStream> stream = FileSystem::CreateAndOpenFile(savePath);

	// Write meta-data
	{
		size_t sizePos = stream->Tell();
		stream->Skip(sizeof(u32));

		BinarySerializer bs;
		bs.Encode(resourceData.get(), stream);

		size_t curPos = stream->Tell();
		stream->Seek(sizePos);

		u32 size = (u32)(curPos - sizePos - sizeof(u32));
		stream->Write(&size, sizeof(size));
		stream->Seek(curPos);
	}

	// Write object data
	{
		size_t sizePos = stream->Tell();
		stream->Skip(sizeof(u32));

		BinarySerializer bs;
		uint32_t size = 0;
		if(compressionMethod != 0)
		{
			SPtr<MemoryDataStream> tempStream = B3DMakeShared<MemoryDataStream>();
			bs.Encode(resource.get(), tempStream);

			size = (uint32_t)tempStream->Size();
			tempStream->Seek(0);

			// Note: We should refactor Compression::compress() so it can write straight to the file stream
			SPtr<DataStream> srcStream = std::static_pointer_cast<DataStream>(tempStream);
			SPtr<MemoryDataStream> compressedStream = B3DMakeShared<MemoryDataStream>();

			Compression::Compress(*srcStream, *compressedStream);

			stream->Write(compressedStream->Data(), compressedStream->Size());
		}
		else
		{
			bs.Encode(resource.get(), stream);
			size = (uint32_t)(stream->Tell() - sizePos - sizeof(u32));
		}

		size_t curPos = stream->Tell();
		stream->Seek(sizePos);
		stream->Write(&size, sizeof(size));
		stream->Seek(curPos);
	}

	stream->Close();
	stream = nullptr;

	if(fileExists)
	{
		FileSystem::Remove(filePath);
		FileSystem::Move(savePath, filePath);
	}
}

void Resources::Update(HResource& handle, const SPtr<Resource>& resource)
{
	const UUID& uuid = handle.GetId();
	handle.SetHandleData(resource, uuid);
	handle.NotifyLoadComplete();

	if(resource)
	{
		Lock lock(mLoadedResourceMutex);
		auto iterFind = mLoadedResources.find(uuid);
		if(iterFind == mLoadedResources.end())
		{
			LoadedResourceData& resData = mLoadedResources[uuid];
			resData.Resource = handle.GetWeak();
		}
	}

	OnResourceModified(handle);

	// Notify listeners immediately if on main thread
	if(GetCoreApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		ResourceListenerManager::Instance().NotifyListeners(uuid);
}

void Resources::UpdateHandle(HResource& handle, const SPtr<Resource>& resource)
{
	const UUID& uuid = handle.GetId();
	handle.SetHandleData(resource, uuid);
	handle.NotifyLoadComplete();

	OnResourceModified(handle);

	// Notify listeners immediately if on main thread
	if(GetCoreApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		ResourceListenerManager::Instance().NotifyListeners(uuid);
}

void Resources::UpdateResourcesFromPackage(const UPtr<PackageWriteLock>& packageWriteLock)
{
	if(!B3D_ENSURE(packageWriteLock != nullptr))
		return;

	const SPtr<Package>& package = packageWriteLock->GetPackage();
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
#if B3D_BUILD_TYPE == B3D_BUILD_TYPE_DEVELOPMENT
			if(package->GetResourceLoadState(resourceId) != PackageResourceLoadState::Unloaded)
			{
				B3D_LOG(Warning, Resources, "Updating resource system with a package that contains an loaded resource that's not registered as "
								"loaded in the resource system. This resource might leak unless explicitly freed from the package.");
			}
#endif

			continue;
		}

		const SPtr<Resource> resource = package->LoadResource(resourceId);
		if(resource == nullptr)
		{
			B3D_LOG(Warning, Resources, "Failed to update resource '{0}' with new data from the package. Unknown error.", resourceId);
			continue;
		}

		if(resourceHandle.GetShared() != resource)
			UpdateHandle(resourceHandle, resource);
	}
}

Vector<UUID> Resources::GetDependencies(const Path& filePath)
{
	SPtr<SavedResourceData> savedResourceData;
	if(!filePath.IsEmpty())
	{
		FileDecoder fs(filePath);
		savedResourceData = std::static_pointer_cast<SavedResourceData>(fs.Decode());
	}

	return savedResourceData->GetDependencies();
}

void Resources::RegisterResourceManifest(const SPtr<ResourceManifest>& manifest)
{
	auto findIter = std::find(mResourceManifests.begin(), mResourceManifests.end(), manifest);
	if(findIter == mResourceManifests.end())
		mResourceManifests.push_back(manifest);
	else
		*findIter = manifest;
}

void Resources::UnregisterResourceManifest(const SPtr<ResourceManifest>& manifest)
{
	if(manifest->GetName() == "Default")
		return;

	auto findIter = std::find(mResourceManifests.begin(), mResourceManifests.end(), manifest);
	if(findIter != mResourceManifests.end())
		mResourceManifests.erase(findIter);
}

SPtr<ResourceManifest> Resources::GetResourceManifest(const String& name) const
{
	for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter)
	{
		if(name == (*iter)->GetName())
			return (*iter);
	}

	return nullptr;
}

bool Resources::IsLoaded(const UUID& uuid, bool checkInProgress)
{
	if(checkInProgress)
	{
		Lock inProgressLock(mInProgressResourcesMutex);
		auto iterFind2 = mInProgressResources.find(uuid);
		if(iterFind2 != mInProgressResources.end())
		{
			return true;
		}
	}

	{
		Lock loadedLock(mLoadedResourceMutex);
		auto iterFind = mLoadedResources.find(uuid);
		if(iterFind != mLoadedResources.end())
		{
			return true;
		}
	}

	return false;
}

float Resources::GetLoadProgress(const HResource& resource, bool includeDependencies)
{
	const UUID& uuid = resource.GetId();
	if(uuid.Empty())
		return 0.0f;

	Lock inProgressLock(mInProgressResourcesMutex);
	Lock loadedLock(mLoadedResourceMutex);

	// Fully loaded
	auto iterFind = mLoadedResources.find(uuid);
	if(iterFind != mLoadedResources.end())
		return 1.0f;

	// Not loaded nor being loaded
	auto iterFind2 = mInProgressResources.find(uuid);
	if(iterFind2 == mInProgressResources.end())
		return 0.0f;

	ResourceLoadData* loadData = iterFind2->second;

	// Don't care about dependencies, just report own progress directly
	if(!includeDependencies)
		return loadData->Progress.load(std::memory_order_relaxed);

	// Dependencies that are already fully loaded will just have their loaded sizes in 'dependencyLoadedAmount', while
	// for those still in progress we need to check their load data
	float totalBytesLoaded = (float)loadData->DependencyLoadedAmount;
	for(auto& entry : loadData->Dependencies)
	{
		auto iterFind3 = mInProgressResources.find(entry.GetId());
		if(iterFind3 == mInProgressResources.end())
			continue;

		ResourceLoadData* dependencyLoadData = iterFind3->second;
		totalBytesLoaded += dependencyLoadData->ResData.Size * dependencyLoadData->Progress.load(std::memory_order_relaxed);
	}

	totalBytesLoaded += loadData->ResData.Size * loadData->Progress.load(std::memory_order_relaxed);

	float totalBytesToLoad = (float)(loadData->DependencySize + loadData->ResData.Size);
	B3D_ASSERT(totalBytesLoaded <= totalBytesToLoad);

	return std::min(1.0f, totalBytesLoaded / totalBytesToLoad);
}

float Resources::GetLoadProgress2(const HResource& resource)
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

		Optional<Path> maybePackagePath = packageManager.TryGetPackagePathForResource(resourceId);
		if(!maybePackagePath.has_value())
			return LoadProgress();

		const Path& packagePath = *maybePackagePath;

		AcquirePackageReadLockOptions readLockOptions(false, true, "GetLoadProgress");
		UPtr<PackageReadLock> packageReadLock;
		const AcquirePackageLockResult lockResult = packageManager.AcquireReadLock(packagePath, readLockOptions, packageReadLock);
		if(lockResult != AcquirePackageLockResult::Acquired || packageReadLock == nullptr)
			return LoadProgress();

		const SPtr<Package>& package = packageReadLock->GetPackage();
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
			const TInlineArray<SPtr<InProgressLoadInformation>, 1>& inProgressLoadsPerResource = found->second;
			for(const SPtr<InProgressLoadInformation>& entry : inProgressLoadsPerResource)
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

HResource Resources::CreateResourceHandle(const SPtr<Resource>& resource)
{
	if(resource == nullptr)
		return nullptr;

	const UUID& uuid = resource->GetId().Empty() ? UUIDGenerator::GenerateRandom() : resource->GetId();
	return CreateResourceHandle(resource, uuid);
}

HResource Resources::CreateResourceHandle(const SPtr<Resource>& resource, const UUID& resourceId)
{
	HResource newHandle(resource, resourceId);

	{
		Lock lock(mLoadedResourceMutex);

		if(resource)
		{
			resource->SetHandle(newHandle.GetWeak());

			LoadedResourceData& resData = mLoadedResources[resourceId];
			resData.Resource = newHandle.GetWeak();
		}

		mHandles[resourceId] = newHandle.GetWeak();
	}

	return newHandle;
}

HResource Resources::GetOrCreateResourceHandle(const UUID& resourceId)
{
	Lock lock(mLoadedResourceMutex);
	if(auto found = mHandles.find(resourceId); found != mHandles.end()) // Not loaded, but handle does exist
		return found->second.Lock();

	// Create new handle
	HResource handle(resourceId);
	mHandles[resourceId] = handle.GetWeak();

	return handle;
}

bool Resources::GetFilePathFromUuid(const UUID& uuid, Path& filePath) const
{
	// Default manifest is at 0th index but all other take priority since Default manifest could
	// contain obsolete data.
	for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter)
	{
		if((*iter)->UUIDToPhysicalFilePath(uuid, filePath))
			return true;
	}

	return false;
}

bool Resources::GetUUIDFromFilePath(const Path& path, UUID& outUUID) const
{
	Path absolutePhysicalPath = path;
	if(!absolutePhysicalPath.IsAbsolute())
		absolutePhysicalPath.MakeAbsolute(FileSystem::GetWorkingDirectoryPath());

	for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter)
	{
		const ResourceManifest& manifest = *iter->get();

		if(manifest.PhysicalFilePathToUUID(path, outUUID))
			return true;
	}

	return false;
}

Path Resources::EnsurePhysicalPath(const Path& path) const
{
	// Check if a virtual file path first
	for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter)
	{
		const ResourceManifest& manifest = *iter->get();

		Path physicalPath;
		if(manifest.VirtualToPhysicalPath(path, physicalPath))
			return physicalPath;
	}

	// Not a virtual path? Returns the path as-is.
	return path;
}

void Resources::LoadComplete(HResource& resource, bool notifyProgress) // TODO - Deprecated
{
	UUID uuid = resource.GetId();

	ResourceLoadData* myLoadData = nullptr;
	bool finishLoad = true;
	Vector<ResourceLoadData*> dependantLoads;
	{
		Lock inProgresslock(mInProgressResourcesMutex);

		auto iterFind = mInProgressResources.find(uuid);
		if(iterFind != mInProgressResources.end())
		{
			myLoadData = iterFind->second;
			finishLoad = myLoadData->RemainingDependencies == 0;

			if(finishLoad)
				mInProgressResources.erase(iterFind);
		}

		auto iterFind2 = mDependantLoads.find(uuid);

		if(iterFind2 != mDependantLoads.end())
			dependantLoads = iterFind2->second;

		if(finishLoad)
		{
			mDependantLoads.erase(uuid);

			// If loadedData is null then we're probably completing load on an already loaded resource, triggered
			// by its dependencies.
			if(myLoadData != nullptr && myLoadData->LoadedData != nullptr)
			{
				Lock loadedLock(mLoadedResourceMutex);

				mLoadedResources[uuid] = myLoadData->ResData;
				resource.SetHandleData(myLoadData->LoadedData, uuid);
				myLoadData->LoadedData->SetHandle(resource.GetWeak());
			}

			resource.NotifyLoadComplete();

			for(auto& dependantLoad : dependantLoads)
				dependantLoad->RemainingDependencies--;
		}
	}

	for(auto& dependantLoad : dependantLoads)
	{
		if(notifyProgress && myLoadData)
			dependantLoad->DependencyLoadedAmount += myLoadData->ResData.Size;

		HResource dependant = dependantLoad->ResData.Resource.Lock();
		LoadComplete(dependant, false);
	}

	if(finishLoad && myLoadData != nullptr)
	{
		OnResourceLoaded(resource);

		// Notify listeners immediately if on main thread during synchronous load
		if(myLoadData->NotifyImmediately)
			ResourceListenerManager::Instance().NotifyListeners(uuid);

		B3DDelete(myLoadData);
	}
}

void Resources::LoadCallback(const Path& filePath, HResource& resource, bool loadWithSaveData) // TODO - Deprecated
{
	ResourceLoadData* myLoadData;
	{
		Lock lock(mInProgressResourcesMutex);
		myLoadData = mInProgressResources[resource.GetId()];
	}

	SPtr<Resource> rawResource = LoadFromDiskAndDeserialize(filePath, loadWithSaveData, myLoadData->Progress);

	{
		Lock lock(mInProgressResourcesMutex);

		myLoadData->LoadedData = rawResource;
		myLoadData->RemainingDependencies--;
		myLoadData->Progress.exchange(1.0f, std::memory_order_relaxed);
	}

	LoadComplete(resource, true);
}

namespace bs
{
B3D_CORE_EXPORT Resources& GetResources()
{
	return Resources::Instance();
}
} // namespace bs
