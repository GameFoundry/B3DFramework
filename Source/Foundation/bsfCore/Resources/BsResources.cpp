//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResources.h"
#include "Resources/BsResource.h"
#include "Resources/BsResourceManifest.h"
#include "Error/BsException.h"
#include "Serialization/BsFileSerializer.h"
#include "FileSystem/BsFileSystem.h"
#include "Threading/BsTaskScheduler.h"
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

namespace bs
{
	Resources::Resources()
	{
		{
			Lock Lock(mDefaultManifestMutex);
			mDefaultResourceManifest = ResourceManifest::create("Default");
			mResourceManifests.push_back(mDefaultResourceManifest);
		}
	}

	Resources::~Resources()
	{
		unloadAll();
	}

	HResource Resources::Load(const Path& filePath, ResourceLoadFlags loadFlags)
	{
		if (!FileSystem::isFile(filePath))
		{
			BS_LOG(Warning, Resources, "Cannot load resource. Specified file: {0} doesn't exist.", filePath);
			return HResource();
		}

		UUID uuid;
		bool foundUUID = getUUIDFromFilePath(filePath, uuid);

		if (!foundUUID)
			uuid = UUIDGenerator::generateRandom();

		return LoadInternal(uuid, filePath, true, loadFlags).resource;
	}

	HResource Resources::Load(const WeakResourceHandle<Resource>& handle, ResourceLoadFlags loadFlags)
	{
		if (handle.mData == nullptr)
			return HResource();

		UUID uuid = handle.GetUUID();
		return LoadFromUUID(uuid, false, loadFlags);
	}

	HResource Resources::LoadAsync(const Path& filePath, ResourceLoadFlags loadFlags)
	{
		if (!FileSystem::isFile(filePath))
		{
			BS_LOG(Warning, Resources, "Cannot load resource. Specified file: '{0}' doesn't exist.", filePath);
			return HResource();
		}

		UUID uuid;
		bool foundUUID = getUUIDFromFilePath(filePath, uuid);

		if (!foundUUID)
			uuid = UUIDGenerator::generateRandom();

		return LoadInternal(uuid, filePath, false, loadFlags).resource;
	}

	HResource Resources::LoadFromUUID(const UUID& uuid, bool async, ResourceLoadFlags loadFlags)
	{
		Path filePath;
		getFilePathFromUUID(uuid, filePath);

		return LoadInternal(uuid, filePath, !async, loadFlags).resource;
	}

	Resources::LoadInfo Resources::loadInternal(const UUID& uuid, const Path& filePath, bool synchronous,
		ResourceLoadFlags loadFlags)
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
			Lock InProgressLock(mInProgressResourcesMutex);
			Lock LoadedLock(mLoadedResourceMutex);

			auto iterFind2 = mInProgressResources.find(uuid);
			if (iterFind2 != mInProgressResources.end())
			{
				LoadedResourceData& resData = iterFind2->second->resData;
				output.resource = resData.resource.Lock();
				output.state = LoadInfo::AlreadyInProgress;
				output.size = resData.size;

				// Increase ref. count
				if (loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
				{
					resData.numInternalRefs++;
					output.resource.AddInternalRef();
				}

				loadInProgress = true;
				alreadyLoading = true;
			}

			// Check if the resource is already loaded
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				LoadedResourceData& resData = iterFind->second;
				output.resource = resData.resource.Lock();
				output.state = LoadInfo::AlreadyLoaded;
				output.size = resData.size;

				// Increase ref. count
				if (loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
				{
					resData.numInternalRefs++;
					output.resource.AddInternalRef();
				}

				alreadyLoading = true;
			}

			// Not loaded and not in progress, register a new handle or find a pre-registered one
			if(!alreadyLoading)
			{
				output.state = LoadInfo::Loading;
				output.size = 0;

				auto iterFind = mHandles.find(uuid);
				if (iterFind != mHandles.end())
					output.resource = iterFind->second.Lock();
				else
				{
					output.resource = HResource(uuid);
					mHandles[uuid] = output.resource.GetWeak();
				}
			}

			// If we have nowhere to load from, warn and complete load if a file path was provided, otherwise pass through
			// as we might just want to complete a previously queued load
			if (filePath.IsEmpty())
			{
				if (!alreadyLoading)
				{
					BS_LOG(Verbose, Resources, "Cannot load resource. Resource with UUID '{0}' doesn't exist.", uuid);
					loadFailed = true;
				}
			}
			else if (!FileSystem::isFile(filePath))
			{
				BS_LOG(Verbose, Resources, "Cannot load resource. Specified file: '{0}' doesn't exist.", filePath);
				loadFailed = true;
			}

			bool loadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
			if(!loadFailed)
			{
				// Load dependency data if a file path is provided
				SPtr<SavedResourceData> savedResourceData;
				if (!filePath.IsEmpty())
				{
					// Note: Ideally this data gets cached eventually (e.g. as part of the manifest). When loading objects
					// with a lot of dependencies (e.g. scenes) this will get called for every dependency, synchronously,
					// which might take a while. It would be nice to just read it from a single location. Another option is
					// to make this whole block asynchronous so every dependency does it on its own thread.
					FileDecoder Fs(filePath);
					savedResourceData = std::static_pointer_cast<SavedResourceData>(fs.Decode());
					output.size = fs.GetSize();
				}

				// Register an in-progress load unless there is an existing load operation, or the resource is already
				// loaded
				if(!alreadyLoading)
				{
					ResourceLoadData* loadData = bs_new<ResourceLoadData>(output.resource.GetWeak(), 0, output.size);
					mInProgressResources[uuid] = loadData;

					if (loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef))
					{
						loadData->resData.numInternalRefs++;
						output.resource.AddInternalRef();
					}

					loadData->remainingDependencies = 1; // Self
					loadData->progress.Store(0.0f, std::memory_order_relaxed);

					// Make resource listener trigger before exit if loading synchronously on the main thread
					loadData->notifyImmediately = synchronous && BS_THREAD_CURRENT_ID == gCoreApplication().GetSimThreadId();

					// Register dependencies and count them so we know when the resource is fully loaded
					if (loadDependencies && savedResourceData != nullptr)
					{
						for (auto& dependency : savedResourceData->GetDependencies())
						{
							if (dependency != uuid)
							{
								mDependantLoads[dependency].push_back(loadData);
								loadData->remainingDependencies++;
								dependenciesToLoad.push_back(dependency);
							}
						}
					}
				}
				// The resource is already being loaded, or is loaded, but we might still need to load some dependencies
				else if(loadDependencies && savedResourceData != nullptr)
				{
					const Vector<UUID>& dependencies = savedResourceData->GetDependencies();
					if (!dependencies.empty())
					{
						ResourceLoadData* loadData = nullptr;

						// If load not in progress, register the resource for load
						if (!loadInProgress)
						{
							loadData = bs_new<ResourceLoadData>(output.resource.GetWeak(), 0, output.size);
							loadData->remainingDependencies = 0;
							loadData->progress.Store(0.0f, std::memory_order_relaxed);

							// Make resource listener trigger before exit if loading synchronously
							loadData->notifyImmediately = synchronous && BS_THREAD_CURRENT_ID == gCoreApplication().GetSimThreadId();
						}
						else
							loadData = mInProgressResources[uuid];

						// Find dependencies that aren't already loaded or queued for loading
						for (auto& dependency : dependencies)
						{
							if (dependency != uuid)
							{
								bool registerDependency = false;

								auto iterFind3 = mLoadedResources.find(dependency);
								if(iterFind3 == mLoadedResources.end())
								{
									registerDependency = true;

									auto iterFind2 = mDependantLoads.find(dependency);
									if (iterFind2 != mDependantLoads.end())
									{
										Vector<ResourceLoadData*>& dependantData = iterFind2->second;
										auto iterFind3 = std::find_if(dependantData.begin(), dependantData.end(),
											[&](ResourceLoadData* x)
										{
											return x->resData.resource.GetUUID() == output.resource.getUUID();
										});

										registerDependency = iterFind3 == dependantData.end();
									}
								}

								if (registerDependency)
								{
									mDependantLoads[dependency].push_back(loadData);
									loadData->remainingDependencies++;
									dependenciesToLoad.push_back(dependency);
								}
							}
						}

						if(!loadInProgress)
						{
							if(!dependenciesToLoad.empty())
								mInProgressResources[uuid] = loadData;
							else
								bs_delete(loadData);
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
			output.state = LoadInfo::Failed;
			output.size = 0;

			// Clean up in-progress state
			loadComplete(output.resource, true);
			return output;
		}

		// Load dependencies (before the main resource)
		const auto numDependencies = (UINT32)dependenciesToLoad.size();
		if(numDependencies > 0)
		{
			ResourceLoadFlags depLoadFlags = ResourceLoadFlag::LoadDependencies;
			if (loadFlags.IsSet(ResourceLoadFlag::KeepSourceData))
				depLoadFlags |= ResourceLoadFlag::KeepSourceData;

			Vector<HResource> Dependencies(numDependencies);
			UINT32 dependencySize = 0;
			for (UINT32 i = 0; i < numDependencies; i++)
			{
				const UUID& depUUID = dependenciesToLoad[i];

				Path depFilePath;
				getFilePathFromUUID(depUUID, depFilePath);

				LoadInfo loadInfo = loadInternal(depUUID, depFilePath, synchronous, depLoadFlags);
				dependencies[i] = loadInfo.resource;

				// Calculate the size of dependencies that still need to be loaded, for progress reporting
				if(loadInfo.state == LoadInfo::Loading || loadInfo.state == LoadInfo::AlreadyInProgress)
				{
					// Note: Technically, since we're queuing the dependency load with no locking, the load could complete
					// before the size of the dependency has been registered, which means getLoadProgress() method would
					// incorrectly report the progress to be higher than it should be. If that becomes an issue then this
					// operation is better to be moved to the child loadInternal() call.
					dependencySize += loadInfo.size;
				}
			}

			// Keep dependencies alive until the parent is done loading, and record total size of dependencies to load
			{
				Lock InProgressLock(mInProgressResourcesMutex);

				// If we're doing a dependency-only load (main resource itself was previously loaded), then the in-progress
				// operation could have already finished when the last dependency was loaded (this will always be true for
				// synchronous loads), and no need to register dependencies.
				const auto iterFind = mInProgressResources.find(uuid);
				if(iterFind != mInProgressResources.end())
				{
					iterFind->second->dependencySize = dependencySize;
					iterFind->second->dependencies = dependencies;
				}
			}
		}

		// Check if resource load already started on another thread (in case it was already being loaded), in which case
		// we want to wait
		bool waitOnLoadInProgress = false;
		SPtr<Task> loadTask;
		{
			Lock InProgressLock(mInProgressResourcesMutex);

			const auto iterFind = mInProgressResources.find(uuid);
			if (iterFind != mInProgressResources.end())
			{
				if (iterFind->second->loadStarted)
				{
					waitOnLoadInProgress = true;
					loadTask = iterFind->second->task;
				}
				else
					iterFind->second->loadStarted = true;
			}
		}

		// Previously being loaded as async but now we want it synced, so we wait
		if (loadInProgress && synchronous && waitOnLoadInProgress)
		{
			if(loadTask)
				loadTask->Wait();

			output.resource.BlockUntilLoaded(false);
		}

		// Actually start the file read operation if not already loaded or in progress
		if (initiateLoad)
		{
			// Synchronous or the resource doesn't support async, read the file immediately
			if (synchronous)
			{
				loadCallback(filePath, output.resource, loadFlags.IsSet(ResourceLoadFlag::KeepSourceData));
			}
			else // Asynchronous, read the file on a worker thread
			{
				String fileName = filePath.GetFilename();
				String taskName = "Resource load: " + fileName;

				bool keepSourceData = loadFlags.IsSet(ResourceLoadFlag::KeepSourceData);
				SPtr<Task> task = Task::create(taskName,
					std::bind(&Resources::loadCallback, this, filePath, output.resource, keepSourceData));

				// Register the task
				{
					Lock InProgressLock(mInProgressResourcesMutex);

					const auto iterFind = mInProgressResources.find(uuid);
					if (iterFind != mInProgressResources.end())
						iterFind->second->task = task;

					TaskScheduler::instance().AddTask(task);
				}
			}
		}
		else
		{
			if(!loadInProgress)
			{
				// Already loaded, decrement dependency count
				loadComplete(output.resource, false);
			}
		}

		output.state = LoadInfo::Loading;
		return output;
	}

	SPtr<Resource> Resources::loadFromDiskAndDeserialize(const Path& filePath, bool loadWithSaveData,
		std::atomic<float>& progress)
	{
		Lock fileLock = FileScheduler::getLock(filePath);

		SPtr<DataStream> stream = FileSystem::openFile(filePath, true);
		if (stream == nullptr)
			return nullptr;

		if (stream->Size() > std::numeric_limits<UINT32>::max())
		{
			BS_EXCEPT(InternalErrorException,
				"File size is larger that UINT32 can hold. Ask a programmer to use a bigger data type.");
		}

		CoreSerializationContext serzContext;
		serzContext.flags = loadWithSaveData ? SF_KeepResourceSourceData : 0;

		// Read meta-data
		SPtr<SavedResourceData> metaData;
		{
			if (!stream->Eof())
			{
				UINT32 objectSize = 0;
				stream->Read(&objectSize, sizeof(objectSize));

				BinarySerializer bs;
				metaData = std::static_pointer_cast<SavedResourceData>(bs.Decode(stream, objectSize, BinarySerializerFlag::None, &serzContext));
			}
		}

		// Read resource data
		SPtr<IReflectable> loadedData;
		{
			if(metaData && !stream->Eof())
			{
				UINT32 objectSize = 0;
				stream->Read(&objectSize, sizeof(objectSize));

				if (metaData->GetCompressionMethod() != 0)
				{
					stream = Compression::decompress(stream, [&progress](float val)
					{
						progress.Exchange(val * 0.9f, std::memory_order_relaxed);
					});

					BinarySerializer bs;
					loadedData = bs.decode(stream, objectSize, BinarySerializerFlag::None, &serzContext,
						[&progress](float val)
					{
						progress.Exchange(0.9f + val * 0.1f, std::memory_order_relaxed);
					});
				}
				else
				{
					BinarySerializer bs;
					loadedData = bs.decode(stream, objectSize, BinarySerializerFlag::None, &serzContext,
						[&progress](float val)
					{
						progress.Exchange(val, std::memory_order_relaxed);
					});
				}
			}
		}

		if (loadedData == nullptr)
			BS_LOG(Error, Resources, "Unable to load resource at path \"{0}\"", filePath);
		else
		{
			if (!loadedData->IsDerivedFrom(Resource::getRTTIStatic()))
				BS_EXCEPT(InternalErrorException, "Loaded class doesn't derive from Resource.");
		}

		SPtr<Resource> resource = std::static_pointer_cast<Resource>(loadedData);
		return resource;
	}

	void Resources::Release(ResourceHandleBase& resource)
	{
		const UUID& uuid = resource.GetUUID();

		{
			bool loadInProgress = false;

			{
				Lock InProgressLock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(uuid);
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			// Technically we should be able to just cancel a load in progress instead of blocking until it finishes.
			// However that would mean the last reference could get lost on whatever thread did the loading, which
			// isn't something that's supported. If this ends up being a problem either make handle counting atomic
			// or add a separate queue for objects destroyed from the load threads.
			if (loadInProgress)
				resource.BlockUntilLoaded();

			bool lostLastRef = false;
			{
				Lock LoadedLock(mLoadedResourceMutex);
				auto iterFind = mLoadedResources.find(uuid);
				if (iterFind != mLoadedResources.end())
				{
					LoadedResourceData& resData = iterFind->second;

					assert(resData.numInternalRefs > 0);
					resData.numInternalRefs--;
					resource.RemoveInternalRef();
					
					std::uint32_t refCount = resource.GetHandleData()->mRefCount.load(std::memory_order_relaxed);
					lostLastRef = refCount == 0;
				}
			}

			if(lostLastRef)
				destroy(resource);
		}
	}

	void Resources::UnloadAllUnused()
	{
		Vector<HResource> resourcesToUnload;

		{
			Lock Lock(mLoadedResourceMutex);
			for(auto iter = mLoadedResources.begin(); iter != mLoadedResources.end(); ++iter)
			{
				const LoadedResourceData& resData = iter->second;

				std::uint32_t refCount = resData.resource.mData->mRefCount.Load(std::memory_order_relaxed);
				assert(refCount > 0); // No references but kept in mLoadedResources list?

				if (refCount == resData.numInternalRefs) // Only internal references exist, free it
					resourcesToUnload.push_back(resData.resource.Lock());
			}
		}

		// Note: When unloading multiple resources it's possible that unloading one will also unload
		// another resource in "resourcesToUnload". This is fine because "unload" deals with invalid
		// handles gracefully.
		for(auto iter = resourcesToUnload.begin(); iter != resourcesToUnload.end(); ++iter)
		{
			release(*iter);
		}
	}

	void Resources::UnloadAll()
	{
		// Unload and invalidate all resources
		UnorderedMap<UUID, LoadedResourceData> loadedResourcesCopy;
		
		{
			Lock Lock(mLoadedResourceMutex);
			loadedResourcesCopy = mLoadedResources;
		}

		for (auto& loadedResourcePair : loadedResourcesCopy)
			destroy(loadedResourcePair.second.resource);
	}

	void Resources::Destroy(ResourceHandleBase& resource)
	{
		if (resource.mData == nullptr)
			return;

		RecursiveLock Lock(mDestroyMutex);

		// If load in progress, first wait until it completes
		const UUID& uuid = resource.GetUUID();
		if (!resource.IsLoaded(false))
		{
			bool loadInProgress = false;
			{
				Lock Lock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(uuid);
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			if (loadInProgress) // If it's still loading wait until that finishes
				resource.BlockUntilLoaded();
			else
				return; // Already unloaded
		}

		// At this point resource is guaranteed to be loaded and this state cannot change by some other thread because of
		// the mDestroyMutex lock

		// Notify external systems before we actually destroy it
		onResourceDestroyed(uuid);
		resource.mData->mPtr->Destroy();

		{
			Lock Lock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				LoadedResourceData& resData = iterFind->second;
				while (resData.numInternalRefs > 0)
				{
					resData.numInternalRefs--;
					resData.resource.RemoveInternalRef();
				}

				mLoadedResources.erase(iterFind);
			}
			else
			{
				assert(false); // This should never happen but in case it does fail silently in release mode
			}
		}

		resource.ClearHandleData();
	}

	void Resources::Save(const HResource& resource, const Path& filePath, bool overwrite, bool compress)
	{
		if (resource == nullptr)
			return;

		if (!resource.IsLoaded(false))
		{
			bool loadInProgress = false;
			{
				Lock Lock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(resource.getUUID());
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			if (loadInProgress) // If it's still loading wait until that finishes
				resource.BlockUntilLoaded();
			else
				return; // Nothing to save
		}

		const bool fileExists = FileSystem::isFile(filePath);
		if(fileExists && !overwrite)
		{
			BS_LOG(Error, Resources, "Another file exists at the specified location. Not saving.");
			return;
		}

		{
			Lock Lock(mDefaultManifestMutex);
			mDefaultResourceManifest->RegisterResource(resource.GetUUID(), filePath);
		}

		_save(resource.GetInternalPtr(), filePath, compress);
	}

	void Resources::Save(const HResource& resource, bool compress)
	{
		if (resource == nullptr)
			return;

		Path path;
		if (getFilePathFromUUID(resource.GetUUID(), path))
			save(resource, path, true, compress);
	}

	void Resources::_save(const SPtr<Resource>& resource, const Path& filePath, bool compress)
	{
		if (!resource->mKeepSourceData)
		{
			BS_LOG(Warning, Resources, "Saving a resource that was created/loaded without KeepSourceData flag."
				"Some data might not be available for saving. File path: {0}", filePath);
		}

		Vector<ResourceDependency> dependencyList = Utility::findResourceDependencies(*resource);
		Vector<UUID> DependencyUUIDs(dependencyList.size());
		for (UINT32 i = 0; i < (UINT32)dependencyList.size(); i++)
			dependencyUUIDs[i] = dependencyList[i].resource.GetUUID();

		UINT32 compressionMethod = (compress && resource->IsCompressible()) ? 1 : 0;
		SPtr<SavedResourceData> resourceData = bs_shared_ptr_new<SavedResourceData>(dependencyUUIDs,
			resource->AllowAsyncLoading(), compressionMethod);

		Path parentDir = filePath.GetDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);

		Path savePath;
		const bool fileExists = FileSystem::isFile(filePath);
		if(fileExists)
		{
			// If a file exists, save to a temporary location, then copy over only after a save was successful. This guards
			// against data loss in case the save process fails.

			// TODO: Temp directory should always be on this drive, as files moved from one drive to another will in fact
			// be copied
			savePath = FileSystem::getTempDirectoryPath();
			savePath.SetFilename(UUIDGenerator::generateRandom().ToString());

			UINT32 safetyCounter = 0;
			while(FileSystem::exists(savePath))
			{
				if(safetyCounter > 10)
				{
					BS_LOG(Error, Resources,
						"Internal error. Unable to save resource due to not being able to find a unique filename.");
					return;
				}

				savePath.SetFilename(UUIDGenerator::generateRandom().ToString());
				safetyCounter++;
			}

		}
		else
			savePath = filePath;
		
		Lock fileLock = FileScheduler::getLock(filePath);

		SPtr<DataStream> stream = FileSystem::createAndOpenFile(savePath);
	
		// Write meta-data
		{
			size_t sizePos = stream->Tell();
			stream->Skip(sizeof(UINT32));

			BinarySerializer bs;
			bs.Encode(resourceData.get(), stream);
			
			size_t curPos = stream->Tell();
			stream->Seek(sizePos);

			UINT32 size = (UINT32)(curPos - sizePos - sizeof(UINT32));
			stream->Write(&size, sizeof(size));
			stream->Seek(curPos);
		}

		// Write object data
		{
			size_t sizePos = stream->Tell();
			stream->Skip(sizeof(UINT32));

			BinarySerializer bs;
			uint32_t size = 0;
			if (compressionMethod != 0)
			{
				SPtr<MemoryDataStream> tempStream = bs_shared_ptr_new<MemoryDataStream>();
				bs.Encode(resource.get(), tempStream);

				size = (uint32_t)tempStream->Size();
				tempStream->Seek(0);
				
				// Note: We should refactor Compression::compress() so it can write straight to the file stream
				SPtr<DataStream> srcStream = std::static_pointer_cast<DataStream>(tempStream);
				SPtr<MemoryDataStream> compressedStream = Compression::compress(srcStream);

				stream->Write(compressedStream->data(), compressedStream->size());
			}
			else
			{
				bs.Encode(resource.get(), stream);
				size = (uint32_t)(stream->Tell() - sizePos - sizeof(UINT32));
			}
			
			size_t curPos = stream->Tell();
			stream->Seek(sizePos);
			stream->Write(&size, sizeof(size));
			stream->Seek(curPos);
		}

		stream->Close();
		stream = nullptr;

		if (fileExists)
		{
			FileSystem::remove(filePath);
			FileSystem::move(savePath, filePath);
		}
	}

	void Resources::Update(HResource& handle, const SPtr<Resource>& resource)
	{
		const UUID& uuid = handle.GetUUID();
		handle.SetHandleData(resource, uuid);
		handle.NotifyLoadComplete();

		if(resource)
		{
			Lock Lock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind == mLoadedResources.end())
			{
				LoadedResourceData& resData = mLoadedResources[uuid];
				resData.resource = handle.GetWeak();
			}
		}

		onResourceModified(handle);

		// This method is not thread safe due to this call (callable from main thread only)
		ResourceListenerManager::instance().NotifyListeners(uuid);
	}

	Vector<UUID> Resources::GetDependencies(const Path& filePath)
	{
		SPtr<SavedResourceData> savedResourceData;
		if (!filePath.IsEmpty())
		{
			FileDecoder Fs(filePath);
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
		if (manifest->GetName() == "Default")
			return;

		auto findIter = std::find(mResourceManifests.begin(), mResourceManifests.end(), manifest);
		if (findIter != mResourceManifests.end())
			mResourceManifests.erase(findIter);
	}

	SPtr<ResourceManifest> Resources::GetResourceManifest(const String& name) const
	{
		for(auto iter = mResourceManifests.Rbegin(); iter != mResourceManifests.rend(); ++iter)
		{
			if(name == (*iter)->GetName())
				return (*iter);
		}

		return nullptr;
	}

	bool Resources::IsLoaded(const UUID& uuid, bool checkInProgress)
	{
		if (checkInProgress)
		{
			Lock InProgressLock(mInProgressResourcesMutex);
			auto iterFind2 = mInProgressResources.find(uuid);
			if (iterFind2 != mInProgressResources.end())
			{
				return true;
			}
		}

		{
			Lock LoadedLock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				return true;
			}
		}

		return false;
	}

	float Resources::GetLoadProgress(const HResource& resource, bool includeDependencies)
	{
		const UUID& uuid = resource.GetUUID();
		if(uuid.empty())
			return 0.0f;

		Lock InProgressLock(mInProgressResourcesMutex);
		Lock LoadedLock(mLoadedResourceMutex);

		// Fully loaded
		auto iterFind = mLoadedResources.find(uuid);
		if (iterFind != mLoadedResources.end())
			return 1.0f;

		// Not loaded nor being loaded
		auto iterFind2 = mInProgressResources.find(uuid);
		if (iterFind2 == mInProgressResources.end())
			return 0.0f;

		ResourceLoadData* loadData = iterFind2->second;

		// Don't care about dependencies, just report own progress directly
		if(!includeDependencies)
			return loadData->progress.Load(std::memory_order_relaxed);

		// Dependencies that are already fully loaded will just have their loaded sizes in 'dependencyLoadedAmount', while
		// for those still in progress we need to check their load data
		float totalBytesLoaded = (float)loadData->dependencyLoadedAmount;
		for(auto& entry : loadData->dependencies)
		{
			auto iterFind3 = mInProgressResources.find(entry.getUUID());
			if (iterFind3 == mInProgressResources.end())
				continue;

			ResourceLoadData* dependencyLoadData = iterFind3->second;
			totalBytesLoaded += dependencyLoadData->resData.size * dependencyLoadData->progress.Load(std::memory_order_relaxed);
		}

		totalBytesLoaded += loadData->resData.size * loadData->progress.Load(std::memory_order_relaxed);

		float totalBytesToLoad = (float)(loadData->dependencySize + loadData->resData.size);
		assert(totalBytesLoaded <= totalBytesToLoad);

		return std::Min(1.0f, totalBytesLoaded / totalBytesToLoad);
	}

	HResource Resources::_createResourceHandle(const SPtr<Resource>& obj)
	{
		UUID uuid = UUIDGenerator::generateRandom();
		return _createResourceHandle(obj, uuid);
	}

	HResource Resources::_createResourceHandle(const SPtr<Resource>& obj, const UUID& UUID)
	{
		HResource NewHandle(obj, UUID);

		{
			Lock Lock(mLoadedResourceMutex);

			if(obj)
			{
				LoadedResourceData& resData = mLoadedResources[UUID];
				resData.resource = newHandle.GetWeak();
			}

			mHandles[UUID] = newHandle.GetWeak();
		}

		return newHandle;
	}

	HResource Resources::_getResourceHandle(const UUID& uuid)
	{
		Lock Lock(mLoadedResourceMutex);
		auto iterFind3 = mHandles.find(uuid);
		if (iterFind3 != mHandles.end()) // Not loaded, but handle does exist
		{
			return iterFind3->second.Lock();
		}

		// Create new handle
		HResource Handle(uuid);
		mHandles[uuid] = handle.GetWeak();

		return handle;
	}

	bool Resources::GetFilePathFromUUID(const UUID& uuid, Path& filePath) const
	{
		// Default manifest is at 0th index but all other take priority since Default manifest could
		// contain obsolete data.
		for(auto iter = mResourceManifests.Rbegin(); iter != mResourceManifests.rend(); ++iter)
		{
			if((*iter)->UuidToFilePath(uuid, filePath))
				return true;
		}

		return false;
	}

	bool Resources::GetUUIDFromFilePath(const Path& path, UUID& uuid) const
	{
		Path manifestPath = path;
		if (!manifestPath.IsAbsolute())
			manifestPath.MakeAbsolute(FileSystem::getWorkingDirectoryPath());

		for(auto iter = mResourceManifests.Rbegin(); iter != mResourceManifests.rend(); ++iter)
		{
			if ((*iter)->FilePathToUUID(manifestPath, uuid))
				return true;
		}

		return false;
	}

	void Resources::LoadComplete(HResource& resource, bool notifyProgress)
	{
		UUID uuid = resource.GetUUID();

		ResourceLoadData* myLoadData = nullptr;
		bool finishLoad = true;
		Vector<ResourceLoadData*> dependantLoads;
		{
			Lock InProgresslock(mInProgressResourcesMutex);

			auto iterFind = mInProgressResources.find(uuid);
			if (iterFind != mInProgressResources.end())
			{
				myLoadData = iterFind->second;
				finishLoad = myLoadData->remainingDependencies == 0;

				if (finishLoad)
					mInProgressResources.erase(iterFind);
			}

			auto iterFind2 = mDependantLoads.find(uuid);

			if (iterFind2 != mDependantLoads.end())
				dependantLoads = iterFind2->second;

			if (finishLoad)
			{
				mDependantLoads.erase(uuid);

				// If loadedData is null then we're probably completing load on an already loaded resource, triggered
				// by its dependencies.
				if (myLoadData != nullptr && myLoadData->loadedData != nullptr)
				{
					Lock LoadedLock(mLoadedResourceMutex);

					mLoadedResources[uuid] = myLoadData->resData;
					resource.SetHandleData(myLoadData->loadedData, uuid);
				}

				resource.NotifyLoadComplete();

				for (auto& dependantLoad : dependantLoads)
					dependantLoad->remainingDependencies--;
			}
		}

		for (auto& dependantLoad : dependantLoads)
		{
			if(notifyProgress && myLoadData)
				dependantLoad->dependencyLoadedAmount += myLoadData->resData.size;

			HResource dependant = dependantLoad->resData.resource.Lock();
			loadComplete(dependant, false);
		}

		if (finishLoad && myLoadData != nullptr)
		{
			onResourceLoaded(resource);

			// This should only ever be true on the main thread
			if (myLoadData->notifyImmediately)
				ResourceListenerManager::instance().NotifyListeners(uuid);

			bs_delete(myLoadData);
		}
	}

	void Resources::LoadCallback(const Path& filePath, HResource& resource, bool loadWithSaveData)
	{
		ResourceLoadData* myLoadData;
		{
			Lock Lock(mInProgressResourcesMutex);
			myLoadData = mInProgressResources[resource.GetUUID()];
		}

		SPtr<Resource> rawResource = loadFromDiskAndDeserialize(filePath, loadWithSaveData, myLoadData->progress);

		{
			Lock Lock(mInProgressResourcesMutex);

			myLoadData->loadedData = rawResource;
			myLoadData->remainingDependencies--;
			myLoadData->progress.Exchange(1.0f, std::memory_order_relaxed);
		}

		loadComplete(resource, true);
	}

	BS_CORE_EXPORT Resources& GResources()
	{
		return Resources::Instance();
	}
}
