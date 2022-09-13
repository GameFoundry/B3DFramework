//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsResourceListenerManager.h"
#include "Resources/BsResources.h"
#include "Resources/BsIResourceListener.h"
#include "CoreThread/BsCoreThread.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace bs
{
#if BS_DEBUG_MODE
	void ThrowIfNotSimThread()
	{
		if(BS_THREAD_CURRENT_ID != CoreApplication::instance().GetSimThreadId())
			BS_EXCEPT(InternalErrorException, "This method can only be accessed from the simulation thread.");
	}

#define THROW_IF_NOT_SIM_THREAD throwIfNotSimThread();
#else
#define THROW_IF_NOT_SIM_THREAD
#endif

	ResourceListenerManager::ResourceListenerManager()
	{
		mResourceLoadedConn = gResources().onResourceLoaded.Connect(std::bind(&ResourceListenerManager::onResourceLoaded, this, _1));
		mResourceModifiedConn = gResources().onResourceModified.Connect(std::bind(&ResourceListenerManager::onResourceModified, this, _1));
	}

	ResourceListenerManager::~ResourceListenerManager()
	{
		assert(mResourceToListenerMap.empty() && "Not all resource listeners had their resources unregistered properly.");

		mResourceLoadedConn.Disconnect();
		mResourceModifiedConn.Disconnect();
	}

	void ResourceListenerManager::RegisterListener(IResourceListener* listener)
	{
#if BS_DEBUG_MODE
		RecursiveLock lock(mMutex);
		mActiveListeners.Insert(listener);
#endif
	}

	void ResourceListenerManager::UnregisterListener(IResourceListener* listener)
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

	void ResourceListenerManager::MarkListenerDirty(IResourceListener* listener)
	{
		RecursiveLock lock(mMutex);
		mDirtyListeners.Insert(listener);
	}

	void ResourceListenerManager::Update()
	{
		THROW_IF_NOT_SIM_THREAD
		updateListeners();

		{
			RecursiveLock lock(mMutex);

			for (auto& entry : mLoadedResources)
				sendResourceLoaded(entry.second);

			for (auto& entry : mModifiedResources)
				sendResourceModified(entry.second);

			mLoadedResources.Clear();
			mModifiedResources.Clear();
		}
	}

	void ResourceListenerManager::UpdateListeners()
	{
		{
			RecursiveLock lock(mMutex);

			for (auto& listener : mDirtyListeners)
				mTempListenerBuffer.push_back(listener);

			mDirtyListeners.Clear();
		}

		for (auto& listener : mTempListenerBuffer)
		{
			clearDependencies(listener);
			addDependencies(listener);
		}

		mTempListenerBuffer.Clear();

	}

	void ResourceListenerManager::NotifyListeners(const UUID& resourceUUID)
	{
		THROW_IF_NOT_SIM_THREAD
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

	void ResourceListenerManager::OnResourceLoaded(const HResource& resource)
	{
		RecursiveLock lock(mMutex);

		mLoadedResources[resource.GetUUID()] = resource;
	}

	void ResourceListenerManager::OnResourceModified(const HResource& resource)
	{
		RecursiveLock lock(mMutex);

		mModifiedResources[resource.GetUUID()] = resource;
	}

	void ResourceListenerManager::SendResourceLoaded(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.GetHandleData().get();

		auto iterFind = mResourceToListenerMap.find(handleId);
		if (iterFind == mResourceToListenerMap.end())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->NotifyResourceLoaded(resource);
		}
	}

	void ResourceListenerManager::SendResourceModified(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.GetHandleData().get();

		auto iterFind = mResourceToListenerMap.find(handleId);
		if (iterFind == mResourceToListenerMap.end())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->NotifyResourceChanged(resource);
		}
	}

	void ResourceListenerManager::ClearDependencies(IResourceListener* listener)
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

	void ResourceListenerManager::AddDependencies(IResourceListener* listener)
	{
		listener->GetListenerResources(mTempResourceBuffer);

		if (mTempResourceBuffer.size() > 0)
		{
			Vector<UINT64> ResourceHandleIds(mTempResourceBuffer.size());
			UINT32 idx = 0;
			for (auto& resource : mTempResourceBuffer)
			{
				UINT64 handleId = (UINT64)resource.GetHandleData().get();
				resourceHandleIds[idx] = handleId;
				mResourceToListenerMap[handleId].push_back(listener);

				idx++;
			}

			mListenerToResourceMap[listener] = resourceHandleIds;
		}

		mTempResourceBuffer.Clear();
	}
}
