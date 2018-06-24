//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsResourceListenerManager.h"
#include "Resources/BsResources.h"
#include "Resources/BsIResourceListener.h"
#include "CoreThread/BsCoreThread.h"

using namespace std::placeholders;

namespace bs
{
	ResourceListenerManager::ResourceListenerManager()
	{
		mResourceLoadedConn = gResources().onResourceLoaded.connect(std::bind(&ResourceListenerManager::onResourceLoaded, this, _1));
		mResourceModifiedConn = gResources().onResourceModified.connect(std::bind(&ResourceListenerManager::onResourceModified, this, _1));
	}

	ResourceListenerManager::~ResourceListenerManager()
	{
		assert(mResourceToListenerMap.empty() && "Not all resource listeners had their resources unregistered properly.");

		mResourceLoadedConn.disconnect();
		mResourceModifiedConn.disconnect();
	}

	void ResourceListenerManager::registerListener(IResourceListener* listener)
	{
#if BS_DEBUG_MODE
		RecursiveLock lock(mMutex);
		mActiveListeners.insert(listener);
#endif
	}

	void ResourceListenerManager::unregisterListener(IResourceListener* listener)
	{
#if BS_DEBUG_MODE
		{
			RecursiveLock lock(mMutex);
			mActiveListeners.erase(listener);
		}
#endif
		
		{
			RecursiveLock lock(mMutex);
			mDirtyListeners.erase(listener);
		}

		clearDependencies(listener);
	}

	void ResourceListenerManager::markListenerDirty(IResourceListener* listener)
	{
		RecursiveLock lock(mMutex);
		mDirtyListeners.insert(listener);
	}

	void ResourceListenerManager::update()
	{
		updateListeners();

		{
			RecursiveLock lock(mMutex);

			for (auto& entry : mLoadedResources)
				sendResourceLoaded(entry.second);

			for (auto& entry : mModifiedResources)
				sendResourceModified(entry.second);

			mLoadedResources.clear();
			mModifiedResources.clear();
		}
	}

	void ResourceListenerManager::updateListeners()
	{
		{
			RecursiveLock lock(mMutex);

			for (auto& listener : mDirtyListeners)
				mTempListenerBuffer.push_back(listener);

			mDirtyListeners.clear();
		}

		for (auto& listener : mTempListenerBuffer)
		{
			clearDependencies(listener);
			addDependencies(listener);
		}

		mTempListenerBuffer.clear();

	}

	void ResourceListenerManager::notifyListeners(const UUID& resourceUUID)
	{
		updateListeners();

		HResource loadedResource;
		{
			RecursiveLock lock(mMutex);

			const auto iterFind = mLoadedResources.find(resourceUUID);
			if (iterFind != mLoadedResources.end())
			{
				loadedResource = std::move(iterFind->second);
				mLoadedResources.erase(iterFind);
			}
		}

		if(loadedResource)
			sendResourceLoaded(loadedResource);

		HResource modifiedResource;
		{
			RecursiveLock lock(mMutex);

			const auto iterFind = mModifiedResources.find(resourceUUID);
			if (iterFind != mModifiedResources.end())
			{
				modifiedResource = std::move(iterFind->second);
				mModifiedResources.erase(iterFind);
			}
		}

		if(modifiedResource)
			sendResourceModified(modifiedResource);
	}

	void ResourceListenerManager::onResourceLoaded(const HResource& resource)
	{
		RecursiveLock lock(mMutex);

		mLoadedResources[resource.getUUID()] = resource;
	}

	void ResourceListenerManager::onResourceModified(const HResource& resource)
	{
		RecursiveLock lock(mMutex);

		mModifiedResources[resource.getUUID()] = resource;
	}

	void ResourceListenerManager::sendResourceLoaded(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.getHandleData().get();

		auto iterFind = mResourceToListenerMap.find(handleId);
		if (iterFind == mResourceToListenerMap.end())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->notifyResourceLoaded(resource);
		}
	}

	void ResourceListenerManager::sendResourceModified(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.getHandleData().get();

		auto iterFind = mResourceToListenerMap.find(handleId);
		if (iterFind == mResourceToListenerMap.end())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->notifyResourceChanged(resource);
		}
	}

	void ResourceListenerManager::clearDependencies(IResourceListener* listener)
	{
		auto iterFind = mListenerToResourceMap.find(listener);
		if (iterFind == mListenerToResourceMap.end())
			return;

		const Vector<UINT64>& dependantResources = iterFind->second;
		for (auto& resourceHandleId : dependantResources)
		{
			auto iterFind2 = mResourceToListenerMap.find(resourceHandleId);
			if (iterFind2 != mResourceToListenerMap.end())
			{
				Vector<IResourceListener*>& listeners = iterFind2->second;
				auto iterFind3 = std::find(listeners.begin(), listeners.end(), listener);

				if (iterFind3 != listeners.end())
					listeners.erase(iterFind3);

				if (listeners.size() == 0)
					mResourceToListenerMap.erase(iterFind2);
			}
		}

		mListenerToResourceMap.erase(iterFind);
	}

	void ResourceListenerManager::addDependencies(IResourceListener* listener)
	{
		listener->getListenerResources(mTempResourceBuffer);

		if (mTempResourceBuffer.size() > 0)
		{
			Vector<UINT64> resourceHandleIds(mTempResourceBuffer.size());
			UINT32 idx = 0;
			for (auto& resource : mTempResourceBuffer)
			{
				UINT64 handleId = (UINT64)resource.getHandleData().get();
				resourceHandleIds[idx] = handleId;
				mResourceToListenerMap[handleId].push_back(listener);

				idx++;
			}

			mListenerToResourceMap[listener] = resourceHandleIds;
		}

		mTempResourceBuffer.clear();
	}
}