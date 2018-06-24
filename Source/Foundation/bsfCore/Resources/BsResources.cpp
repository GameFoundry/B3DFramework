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
#include "Serialization/BsMemorySerializer.h"
#include "Utility/BsCompression.h"
#include "FileSystem/BsDataStream.h"
#include "Serialization/BsBinarySerializer.h"

namespace bs
{
	Resources::Resources()
	{
		{
			Lock lock(mDefaultManifestMutex);
			mDefaultResourceManifest = ResourceManifest::create("Default");
			mResourceManifests.push_back(mDefaultResourceManifest);
		}
	}

	Resources::~Resources()
	{
		unloadAll();
	}

	HResource Resources::load(const Path& filePath, ResourceLoadFlags loadFlags)
	{
		if (!FileSystem::isFile(filePath))
		{
			LOGWRN("Cannot load resource. Specified file: " + filePath.toString() + " doesn't exist.");

			return HResource();
		}

		UUID uuid;
		bool foundUUID = getUUIDFromFilePath(filePath, uuid);

		if (!foundUUID)
			uuid = UUIDGenerator::generateRandom();

		return loadInternal(uuid, filePath, true, loadFlags);
	}

	HResource Resources::load(const WeakResourceHandle<Resource>& handle, ResourceLoadFlags loadFlags)
	{
		if (handle.mData == nullptr)
			return HResource();

		UUID uuid = handle.getUUID();
		return loadFromUUID(uuid, false, loadFlags);
	}

	HResource Resources::loadAsync(const Path& filePath, ResourceLoadFlags loadFlags)
	{
		if (!FileSystem::isFile(filePath))
		{
			LOGWRN("Cannot load resource. Specified file: " + filePath.toString() + " doesn't exist.");

			return HResource();
		}

		UUID uuid;
		bool foundUUID = getUUIDFromFilePath(filePath, uuid);

		if (!foundUUID)
			uuid = UUIDGenerator::generateRandom();

		return loadInternal(uuid, filePath, false, loadFlags);
	}

	HResource Resources::loadFromUUID(const UUID& uuid, bool async, ResourceLoadFlags loadFlags)
	{
		Path filePath;

		// Default manifest is at 0th index but all other take priority since Default manifest could
		// contain obsolete data. 
		for (auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter)
		{
			if ((*iter)->uuidToFilePath(uuid, filePath))
				break;
		}

		return loadInternal(uuid, filePath, !async, loadFlags);
	}

	HResource Resources::loadInternal(const UUID& uuid, const Path& filePath, bool synchronous, ResourceLoadFlags loadFlags)
	{
		HResource outputResource;

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
			if (iterFind2 != mInProgressResources.end())
			{
				LoadedResourceData& resData = iterFind2->second->resData;
				outputResource = resData.resource.lock();

				// Increase ref. count
				if (loadFlags.isSet(ResourceLoadFlag::KeepInternalRef))
				{
					resData.numInternalRefs++;
					outputResource.addInternalRef();
				}

				loadInProgress = true;
				alreadyLoading = true;
			}

			// Check if the resource is already loaded
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				LoadedResourceData& resData = iterFind->second;
				outputResource = resData.resource.lock();

				// Increase ref. count
				if (loadFlags.isSet(ResourceLoadFlag::KeepInternalRef))
				{
					resData.numInternalRefs++;
					outputResource.addInternalRef();
				}

				alreadyLoading = true;
			}

			// Not loaded and not in progress, register a new handle or find a pre-registered one
			if(!alreadyLoading)
			{
				auto iterFind = mHandles.find(uuid);
				if (iterFind != mHandles.end())
					outputResource = iterFind->second.lock();
				else
				{
					outputResource = HResource(uuid);
					mHandles[uuid] = outputResource.getWeak();
				}
			}

			// If we have nowhere to load from, warn and complete load if a file path was provided, otherwise pass through
			// as we might just want to complete a previously queued load 
			if (filePath.isEmpty())
			{
				if (!alreadyLoading)
				{
					LOGWRN_VERBOSE("Cannot load resource. Resource with UUID '" + UUID + "' doesn't exist.");
					loadFailed = true;
				}
			}
			else if (!FileSystem::isFile(filePath))
			{
				LOGWRN_VERBOSE("Cannot load resource. Specified file: " + filePath.toString() + " doesn't exist.");
				loadFailed = true;
			}

			if(!loadFailed)
			{
				// Load dependency data if a file path is provided
				SPtr<SavedResourceData> savedResourceData;
				if (!filePath.isEmpty())
				{
					FileDecoder fs(filePath);
					savedResourceData = std::static_pointer_cast<SavedResourceData>(fs.decode());
				}

				// Register an in-progress load unless there is an existing load operation, or the resource is already
				// loaded
				if(!alreadyLoading)
				{
					ResourceLoadData* loadData = bs_new<ResourceLoadData>(outputResource.getWeak(), 0);
					mInProgressResources[uuid] = loadData;
					loadData->resData = outputResource.getWeak();

					if (loadFlags.isSet(ResourceLoadFlag::KeepInternalRef))
					{
						loadData->resData.numInternalRefs++;
						outputResource.addInternalRef();
					}

					loadData->remainingDependencies = 1; // Self

					// Make resource listener trigger before exit if loading synchronously
					loadData->notifyImmediately = synchronous; 

					// Register dependencies and count them so we know when the resource is fully loaded
					if (loadFlags.isSet(ResourceLoadFlag::LoadDependencies) && savedResourceData != nullptr)
					{
						for (auto& dependency : savedResourceData->getDependencies())
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
				else if(savedResourceData != nullptr)
				{
					const Vector<UUID>& dependencies = savedResourceData->getDependencies();
					if (!dependencies.empty())
					{
						ResourceLoadData* loadData = nullptr;

						// If load not in progress, register the resource for load
						if (!loadInProgress)
						{
							loadData = bs_new<ResourceLoadData>(outputResource.getWeak(), 0);
							loadData->resData = outputResource.getWeak();

							loadData->remainingDependencies = 0;

							// Make resource listener trigger before exit if loading synchronously
							loadData->notifyImmediately = synchronous;
						}
						else
							loadData = mInProgressResources[uuid];

						// Find dependencies that aren't already queued for loading
						for (auto& dependency : dependencies)
						{
							if (dependency != uuid)
							{
								bool registerDependency = true;

								auto iterFind2 = mDependantLoads.find(dependency);
								if (iterFind2 != mDependantLoads.end())
								{
									Vector<ResourceLoadData*>& dependantData = iterFind2->second;
									auto iterFind3 = std::find_if(dependantData.begin(), dependantData.end(),
										[&](ResourceLoadData* x)
									{
										return x->resData.resource.getUUID() == outputResource.getUUID();
									});

									registerDependency = iterFind3 == dependantData.end();
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

				initiateLoad = !alreadyLoading && !filePath.isEmpty();

				if(savedResourceData != nullptr)
					synchronous = synchronous & savedResourceData->allowAsyncLoading();
			}
		}

		// Previously being loaded as async but now we want it synced, so we wait
		if (loadInProgress && synchronous)
			outputResource.blockUntilLoaded();

		// Something went wrong, clean up and exit
		if(loadFailed)
		{
			// Clean up in-progress state
			loadComplete(outputResource);
			return outputResource;
		}

		// Load dependencies (before the main resource)
		const auto numDependencies = (UINT32)dependenciesToLoad.size();
		if(numDependencies > 0)
		{
			ResourceLoadFlags depLoadFlags = ResourceLoadFlag::LoadDependencies;
			if (loadFlags.isSet(ResourceLoadFlag::KeepSourceData))
				depLoadFlags |= ResourceLoadFlag::KeepSourceData;

			Vector<HResource> dependencies(numDependencies);
			for (UINT32 i = 0; i < numDependencies; i++)
				dependencies[i] = loadFromUUID(dependenciesToLoad[i], !synchronous, depLoadFlags);

			// Keep dependencies alive until the parent is done loading
			{
				Lock inProgressLock(mInProgressResourcesMutex);

				// If we're doing a dependency-only load (main resource itself was previously loaded), then the in-progress
				// operation could have already finished when the last dependency was loaded (this will always be true for
				// synchronous loads), and no need to register dependencies.
				const auto iterFind = mInProgressResources.find(uuid);
				if(iterFind != mInProgressResources.end())
					iterFind->second->dependencies = dependencies;
			}
		}

		// Actually start the file read operation if not already loaded or in progress
		if (initiateLoad)
		{
			// Synchronous or the resource doesn't support async, read the file immediately
			if (synchronous)
			{
				loadCallback(filePath, outputResource, loadFlags.isSet(ResourceLoadFlag::KeepSourceData));
			}
			else // Asynchronous, read the file on a worker thread
			{
				String fileName = filePath.getFilename();
				String taskName = "Resource load: " + fileName;

				bool keepSourceData = loadFlags.isSet(ResourceLoadFlag::KeepSourceData);
				SPtr<Task> task = Task::create(taskName, 
					std::bind(&Resources::loadCallback, this, filePath, outputResource, keepSourceData));
				TaskScheduler::instance().addTask(task);
			}
		}
		else
		{
			if(!loadInProgress)
			{
				// Already loaded, decrement dependency count
				loadComplete(outputResource);
			}
		}

		return outputResource;
	}

	SPtr<Resource> Resources::loadFromDiskAndDeserialize(const Path& filePath, bool loadWithSaveData)
	{
		Lock fileLock = FileScheduler::getLock(filePath);

		SPtr<DataStream> stream = FileSystem::openFile(filePath, true);
		if (stream == nullptr)
			return nullptr;

		if (stream->size() > std::numeric_limits<UINT32>::max())
		{
			BS_EXCEPT(InternalErrorException,
				"File size is larger that UINT32 can hold. Ask a programmer to use a bigger data type.");
		}

		UnorderedMap<String, UINT64> params;
		if(loadWithSaveData)
			params["keepSourceData"] = 1;

		// Read meta-data
		SPtr<SavedResourceData> metaData;
		{
			if (!stream->eof())
			{
				UINT32 objectSize = 0;
				stream->read(&objectSize, sizeof(objectSize));

				BinarySerializer bs;
				metaData = std::static_pointer_cast<SavedResourceData>(bs.decode(stream, objectSize, params));
			}
		}

		// Read resource data
		SPtr<IReflectable> loadedData;
		{
			if(metaData && !stream->eof())
			{
				UINT32 objectSize = 0;
				stream->read(&objectSize, sizeof(objectSize));

				if (metaData->getCompressionMethod() != 0)
					stream = Compression::decompress(stream);

				BinarySerializer bs;
				loadedData = std::static_pointer_cast<SavedResourceData>(bs.decode(stream, objectSize, params));
			}
		}

		if (loadedData == nullptr)
		{
			LOGERR("Unable to load resource at path \"" + filePath.toString() + "\"");
		}
		else
		{
			if (!loadedData->isDerivedFrom(Resource::getRTTIStatic()))
				BS_EXCEPT(InternalErrorException, "Loaded class doesn't derive from Resource.");
		}

		SPtr<Resource> resource = std::static_pointer_cast<Resource>(loadedData);
		return resource;
	}

	void Resources::release(ResourceHandleBase& resource)
	{
		const UUID& uuid = resource.getUUID();

		{
			bool loadInProgress = false;

			{
				Lock inProgressLock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(uuid);
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			// Technically we should be able to just cancel a load in progress instead of blocking until it finishes.
			// However that would mean the last reference could get lost on whatever thread did the loading, which
			// isn't something that's supported. If this ends up being a problem either make handle counting atomic
			// or add a separate queue for objects destroyed from the load threads.
			if (loadInProgress)
				resource.blockUntilLoaded();

			bool lostLastRef = false;
			{
				Lock loadedLock(mLoadedResourceMutex);
				auto iterFind = mLoadedResources.find(uuid);
				if (iterFind != mLoadedResources.end())
				{
					LoadedResourceData& resData = iterFind->second;

					assert(resData.numInternalRefs > 0);
					resData.numInternalRefs--;
					resource.removeInternalRef();
					
					std::uint32_t refCount = resource.getHandleData()->mRefCount.load(std::memory_order_relaxed);
					lostLastRef = refCount == 0;
				}
			}

			if(lostLastRef)
				destroy(resource);
		}
	}

	void Resources::unloadAllUnused()
	{
		Vector<HResource> resourcesToUnload;

		{
			Lock lock(mLoadedResourceMutex);
			for(auto iter = mLoadedResources.begin(); iter != mLoadedResources.end(); ++iter)
			{
				const LoadedResourceData& resData = iter->second;

				std::uint32_t refCount = resData.resource.mData->mRefCount.load(std::memory_order_relaxed);
				assert(refCount > 0); // No references but kept in mLoadedResources list?

				if (refCount == resData.numInternalRefs) // Only internal references exist, free it
					resourcesToUnload.push_back(resData.resource.lock());
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

	void Resources::unloadAll()
	{
		// Unload and invalidate all resources
		UnorderedMap<UUID, LoadedResourceData> loadedResourcesCopy;
		
		{
			Lock lock(mLoadedResourceMutex);
			loadedResourcesCopy = mLoadedResources;
		}

		for (auto& loadedResourcePair : loadedResourcesCopy)
			destroy(loadedResourcePair.second.resource);
	}

	void Resources::destroy(ResourceHandleBase& resource)
	{
		if (resource.mData == nullptr)
			return;

		RecursiveLock lock(mDestroyMutex);

		// If load in progress, first wait until it completes
		const UUID& uuid = resource.getUUID();
		if (!resource.isLoaded(false))
		{
			bool loadInProgress = false;
			{
				Lock lock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(uuid);
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			if (loadInProgress) // If it's still loading wait until that finishes
				resource.blockUntilLoaded();
			else
				return; // Already unloaded
		}

		// At this point resource is guaranteed to be loaded and this state cannot change by some other thread because of
		// the mDestroyMutex lock

		// Notify external systems before we actually destroy it
		onResourceDestroyed(uuid);
		resource.mData->mPtr->destroy();

		{
			Lock lock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				LoadedResourceData& resData = iterFind->second;
				while (resData.numInternalRefs > 0)
				{
					resData.numInternalRefs--;
					resData.resource.removeInternalRef();
				}

				mLoadedResources.erase(iterFind);
			}
			else
			{
				assert(false); // This should never happen but in case it does fail silently in release mode
			}
		}

		resource.clearHandleData();
	}

	void Resources::save(const HResource& resource, const Path& filePath, bool overwrite, bool compress)
	{
		if (resource == nullptr)
			return;

		if (!resource.isLoaded(false))
		{
			bool loadInProgress = false;
			{
				Lock lock(mInProgressResourcesMutex);
				auto iterFind2 = mInProgressResources.find(resource.getUUID());
				if (iterFind2 != mInProgressResources.end())
					loadInProgress = true;
			}

			if (loadInProgress) // If it's still loading wait until that finishes
				resource.blockUntilLoaded();
			else
				return; // Nothing to save
		}

		const bool fileExists = FileSystem::isFile(filePath);
		if(fileExists && !overwrite)
		{
			LOGERR("Another file exists at the specified location. Not saving.");
			return;
		}

		{
			Lock lock(mDefaultManifestMutex);
			mDefaultResourceManifest->registerResource(resource.getUUID(), filePath);
		}

		_save(resource.getInternalPtr(), filePath, compress);
	}

	void Resources::save(const HResource& resource, bool compress)
	{
		if (resource == nullptr)
			return;

		Path path;
		if (getFilePathFromUUID(resource.getUUID(), path))
			save(resource, path, true, compress);
	}

	void Resources::_save(const SPtr<Resource>& resource, const Path& filePath, bool compress)
	{
		if (!resource->mKeepSourceData)
		{
			LOGWRN("Saving a resource that was created/loaded without ResourceLoadFlag::KeepSourceData. Some data might "
				"not be available for saving. File path: " + filePath.toString());
		}

		Vector<ResourceDependency> dependencyList = Utility::findResourceDependencies(*resource);
		Vector<UUID> dependencyUUIDs(dependencyList.size());
		for (UINT32 i = 0; i < (UINT32)dependencyList.size(); i++)
			dependencyUUIDs[i] = dependencyList[i].resource.getUUID();

		UINT32 compressionMethod = (compress && resource->isCompressible()) ? 1 : 0;
		SPtr<SavedResourceData> resourceData = bs_shared_ptr_new<SavedResourceData>(dependencyUUIDs, 
			resource->allowAsyncLoading(), compressionMethod);

		Path parentDir = filePath.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);

		Path savePath;
		const bool fileExists = FileSystem::isFile(filePath);
		if(fileExists)
		{
			// If a file exists, save to a temporary location, then copy over only after a save was successful. This guards
			// against data loss in case the save process fails.

			savePath = FileSystem::getTempDirectoryPath();
			savePath.setFilename(UUIDGenerator::generateRandom().toString());

			UINT32 safetyCounter = 0;
			while(FileSystem::exists(savePath))
			{
				if(safetyCounter > 10)
				{
					LOGERR("Internal error. Unable to save resource due to not being able to find a unique filename.");
					return;
				}

				savePath.setFilename(UUIDGenerator::generateRandom().toString());
				safetyCounter++;
			}

		}
		else
			savePath = filePath;
		
		Lock fileLock = FileScheduler::getLock(filePath);

		std::ofstream stream;
		stream.open(savePath.toPlatformString().c_str(), std::ios::out | std::ios::binary);
		if (stream.fail())
			LOGWRN("Failed to save file: \"" + filePath.toString() + "\". Error: " + strerror(errno) + ".");
	
		// Write meta-data
		{
			MemorySerializer ms;
			UINT32 numBytes = 0;
			UINT8* bytes = ms.encode(resourceData.get(), numBytes);
			
			stream.write((char*)&numBytes, sizeof(numBytes));
			stream.write((char*)bytes, numBytes);
			
			bs_free(bytes);
		}

		// Write object data
		{
			MemorySerializer ms;
			UINT32 numBytes = 0;
			UINT8* bytes = ms.encode(resource.get(), numBytes);

			SPtr<MemoryDataStream> objStream = bs_shared_ptr_new<MemoryDataStream>(bytes, numBytes);
			if (compressionMethod != 0)
			{
				SPtr<DataStream> srcStream = std::static_pointer_cast<DataStream>(objStream);
				objStream = Compression::compress(srcStream);
			}

			stream.write((char*)&numBytes, sizeof(numBytes));
			stream.write((char*)objStream->getPtr(), objStream->size());
		}

		stream.close();
		stream.clear();

		if (fileExists)
		{
			FileSystem::remove(filePath);
			FileSystem::move(savePath, filePath);
		}
	}

	void Resources::update(HResource& handle, const SPtr<Resource>& resource)
	{
		const UUID& uuid = handle.getUUID();
		handle.setHandleData(resource, uuid);

		{
			Lock lock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind == mLoadedResources.end())
			{
				LoadedResourceData& resData = mLoadedResources[uuid];
				resData.resource = handle.getWeak();
			}
		}

		onResourceModified(handle);

		// This method is not thread safe due to this call (callable from main thread only)
		ResourceListenerManager::instance().notifyListeners(uuid);
	}

	Vector<UUID> Resources::getDependencies(const Path& filePath)
	{
		SPtr<SavedResourceData> savedResourceData;
		if (!filePath.isEmpty())
		{
			FileDecoder fs(filePath);
			savedResourceData = std::static_pointer_cast<SavedResourceData>(fs.decode());
		}

		return savedResourceData->getDependencies();
	}

	void Resources::registerResourceManifest(const SPtr<ResourceManifest>& manifest)
	{
		auto findIter = std::find(mResourceManifests.begin(), mResourceManifests.end(), manifest);
		if(findIter == mResourceManifests.end())
			mResourceManifests.push_back(manifest);
		else
			*findIter = manifest;
	}

	void Resources::unregisterResourceManifest(const SPtr<ResourceManifest>& manifest)
	{
		if (manifest->getName() == "Default")
			return;

		auto findIter = std::find(mResourceManifests.begin(), mResourceManifests.end(), manifest);
		if (findIter != mResourceManifests.end())
			mResourceManifests.erase(findIter);
	}

	SPtr<ResourceManifest> Resources::getResourceManifest(const String& name) const
	{
		for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter) 
		{
			if(name == (*iter)->getName())
				return (*iter);
		}

		return nullptr;
	}

	bool Resources::isLoaded(const UUID& uuid, bool checkInProgress)
	{
		if (checkInProgress)
		{
			Lock inProgressLock(mInProgressResourcesMutex);
			auto iterFind2 = mInProgressResources.find(uuid);
			if (iterFind2 != mInProgressResources.end())
			{
				return true;
			}
		}

		{
			Lock loadedLock(mLoadedResourceMutex);
			auto iterFind = mLoadedResources.find(uuid);
			if (iterFind != mLoadedResources.end())
			{
				return true;
			}
		}

		return false;
	}

	HResource Resources::_createResourceHandle(const SPtr<Resource>& obj)
	{
		UUID uuid = UUIDGenerator::generateRandom();
		return _createResourceHandle(obj, uuid);
	}

	HResource Resources::_createResourceHandle(const SPtr<Resource>& obj, const UUID& UUID)
	{
		HResource newHandle(obj, UUID);

		{
			Lock lock(mLoadedResourceMutex);

			LoadedResourceData& resData = mLoadedResources[UUID];
			resData.resource = newHandle.getWeak();

#if BS_DEBUG_MODE
			const auto iterFind = mHandles.find(UUID);
			assert(iterFind == mHandles.end() && "Overwriting an existing handle!");
#endif

			mHandles[UUID] = newHandle.getWeak();
		}

		return newHandle;
	}

	HResource Resources::_getResourceHandle(const UUID& uuid)
	{
		Lock lock(mLoadedResourceMutex);
		auto iterFind3 = mHandles.find(uuid);
		if (iterFind3 != mHandles.end()) // Not loaded, but handle does exist
		{
			return iterFind3->second.lock();
		}

		// Create new handle
		HResource handle(uuid);
		mHandles[uuid] = handle.getWeak();

		return handle;
	}

	bool Resources::getFilePathFromUUID(const UUID& uuid, Path& filePath) const
	{
		for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter) 
		{
			if((*iter)->uuidToFilePath(uuid, filePath))
				return true;
		}

		return false;
	}

	bool Resources::getUUIDFromFilePath(const Path& path, UUID& uuid) const
	{
		Path manifestPath = path;
		if (!manifestPath.isAbsolute())
			manifestPath.makeAbsolute(FileSystem::getWorkingDirectoryPath());

		for(auto iter = mResourceManifests.rbegin(); iter != mResourceManifests.rend(); ++iter) 
		{
			if ((*iter)->filePathToUUID(manifestPath, uuid))
				return true;
		}

		return false;
	}

	void Resources::loadComplete(HResource& resource)
	{
		UUID uuid = resource.getUUID();

		ResourceLoadData* myLoadData = nullptr;
		bool finishLoad = true;
		Vector<ResourceLoadData*> dependantLoads;
		{
			Lock inProgresslock(mInProgressResourcesMutex);

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
					Lock loadedLock(mLoadedResourceMutex);

					mLoadedResources[uuid] = myLoadData->resData;
					resource.setHandleData(myLoadData->loadedData, uuid);
				}

				for (auto& dependantLoad : dependantLoads)
					dependantLoad->remainingDependencies--;
			}
		}

		for (auto& dependantLoad : dependantLoads)
		{
			HResource dependant = dependantLoad->resData.resource.lock();
			loadComplete(dependant);
		}

		if (finishLoad && myLoadData != nullptr)
		{
			onResourceLoaded(resource);

			// This should only ever be true on the main thread
			if (myLoadData->notifyImmediately)
				ResourceListenerManager::instance().notifyListeners(uuid);

			bs_delete(myLoadData);
		}
	}

	void Resources::loadCallback(const Path& filePath, HResource& resource, bool loadWithSaveData)
	{
		SPtr<Resource> rawResource = loadFromDiskAndDeserialize(filePath, loadWithSaveData);

		{
			Lock lock(mInProgressResourcesMutex);

			// Check if all my dependencies are loaded
			ResourceLoadData* myLoadData = mInProgressResources[resource.getUUID()];
			myLoadData->loadedData = rawResource;
			myLoadData->remainingDependencies--;
		}

		loadComplete(resource);
	}

	BS_CORE_EXPORT Resources& gResources()
	{
		return Resources::instance();
	}
}
