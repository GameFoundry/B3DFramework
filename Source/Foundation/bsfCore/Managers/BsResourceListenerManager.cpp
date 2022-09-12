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
		assert(mResourceToListenerMap.Empty() && "Not all resource listeners had their resources unregistered properly.");

		mResourceLoadedConn.Disconnect();
		mResourceModifiedConn.Disconnect();
	}

	void ResourceListenerManager::RegisterListener(IResourceListener* listener)
	{
#if BS_DEBUG_MODE
		RecursiveLock Lock(mMutex);
		mActiveListeners.Insert(listener);
#endif
	}

	void ResourceListenerManager::UnregisterListener(IResourceListener* listener)
	{
#if BS_DEBUG_MODE
		{
			RecursiveLock Lock(mMutex);
			mActiveListeners.Erase(listener);
		}
#endif
		
		{
			RecursiveLock Lock(mMutex);
			mDirtyListeners.Erase(listener);
		}

		clearDependencies(listener);
	}

	void ResourceListenerManager::MarkListenerDirty(IResourceListener* listener)
	{
		RecursiveLock Lock(mMutex);
		mDirtyListeners.Insert(listener);
	}

	void ResourceListenerManager::Update()
	{
		THROW_IF_NOT_SIM_THREAD
		updateListeners();

		{
			RecursiveLock Lock(mMutex);

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
			RecursiveLock Lock(mMutex);

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
			RecursiveLock Lock(mMutex);

			const auto iterFind = mLoadedResources.Find(resourceUUID);
			if (iterFind != mLoadedResources.End())
			{
				loadedResource = std::move(iterFind->second);
				mLoadedResources.Erase(iterFind);
			}
		}

		if(loadedResource)
			sendResourceLoaded(loadedResource);

		HResource modifiedResource;
		{
			RecursiveLock Lock(mMutex);

			const auto iterFind = mModifiedResources.Find(resourceUUID);
			if (iterFind != mModifiedResources.End())
			{
				modifiedResource = std::move(iterFind->second);
				mModifiedResources.Erase(iterFind);
			}
		}

		if(modifiedResource)
			sendResourceModified(modifiedResource);
	}

	void ResourceListenerManager::OnResourceLoaded(const HResource& resource)
	{
		RecursiveLock Lock(mMutex);

		mLoadedResources[resource.GetUUID()] = resource;
	}

	void ResourceListenerManager::OnResourceModified(const HResource& resource)
	{
		RecursiveLock Lock(mMutex);

		mModifiedResources[resource.GetUUID()] = resource;
	}

	void ResourceListenerManager::SendResourceLoaded(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.GetHandleData().get();

		auto iterFind = mResourceToListenerMap.Find(handleId);
		if (iterFind == mResourceToListenerMap.End())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.Find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->NotifyResourceLoaded(resource);
		}
	}

	void ResourceListenerManager::SendResourceModified(const HResource& resource)
	{
		UINT64 handleId = (UINT64)resource.GetHandleData().get();

		auto iterFind = mResourceToListenerMap.Find(handleId);
		if (iterFind == mResourceToListenerMap.End())
			return;

		const Vector<IResourceListener*> relevantListeners = iterFind->second;
		for (auto& listener : relevantListeners)
		{
#if BS_DEBUG_MODE
			assert(mActiveListeners.Find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

			listener->NotifyResourceChanged(resource);
		}
	}

	void ResourceListenerManager::ClearDependencies(IResourceListener* listener)
	{
		auto iterFind = mListenerToResourceMap.Find(listener);
		if (iterFind == mListenerToResourceMap.End())
			return;

		const Vector<UINT64>& dependantResources = iterFind->second;
		for (auto& resourceHandleId : dependantResources)
		{
			auto iterFind2 = mResourceToListenerMap.Find(resourceHandleId);
			if (iterFind2 != mResourceToListenerMap.End())
			{
				Vector<IResourceListener*>& listeners = iterFind2->second;
				auto iterFind3 = std::find(listeners.Begin(), listeners.end(), listener);

				if (iterFind3 != listeners.End())
					listeners.Erase(iterFind3);

				if (listeners.Size() == 0)
					mResourceToListenerMap.Erase(iterFind2);
			}
		}

		mListenerToResourceMap.Erase(iterFind);
	}

	void ResourceListenerManager::AddDependencies(IResourceListener* listener)
	{
		listener->GetListenerResources(mTempResourceBuffer);

		if (mTempResourceBuffer.Size() > 0)
		{
			Vector<UINT64> ResourceHandleIds(mTempResourceBuffer.Size());
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
