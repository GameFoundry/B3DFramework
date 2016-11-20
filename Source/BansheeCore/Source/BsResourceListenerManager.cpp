//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsResourceListenerManager.h"
#include "BsResources.h"
#include "BsIResourceListener.h"

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
		assert(mResourceToListenerMap.size() == 0 && "Not all resource listeners had their resources unregistered properly.");

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
		
		mDirtyListeners.erase(listener);

		clearDependencies(listener);
	}

	void ResourceListenerManager::markListenerDirty(IResourceListener* listener)
	{
		mDirtyListeners.insert(listener);
	}

	void ResourceListenerManager::update()
	{
		for (auto& listener : mDirtyListeners)
		{
			clearDependencies(listener);
			addDependencies(listener);
		}

		mDirtyListeners.clear();

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

	void ResourceListenerManager::notifyListeners(const String& resourceUUID)
	{
		RecursiveLock lock(mMutex);

		auto iterFindLoaded = mLoadedResources.find(resourceUUID);
		if (iterFindLoaded != mLoadedResources.end())
		{
			sendResourceLoaded(iterFindLoaded->second);

			mLoadedResources.erase(iterFindLoaded);
		}

		auto iterFindModified = mModifiedResources.find(resourceUUID);
		if (iterFindModified != mModifiedResources.end())
		{
			sendResourceModified(iterFindModified->second);

			mModifiedResources.erase(iterFindModified);
		}
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