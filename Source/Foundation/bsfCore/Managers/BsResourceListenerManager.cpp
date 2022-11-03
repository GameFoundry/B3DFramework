//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsResourceListenerManager.h"
#include "Resources/BsResources.h"
#include "Resources/BsIResourceListener.h"
#include "CoreThread/BsCoreThread.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

using namespace bs;

#if B3D_DEBUG
void throwIfNotSimThread()
{
	if(BS_THREAD_CURRENT_ID != CoreApplication::Instance().GetSimThreadId())
		B3D_EXCEPT(InternalErrorException, "This method can only be accessed from the simulation thread.");
}

#	define THROW_IF_NOT_SIM_THREAD throwIfNotSimThread();
#else
#	define THROW_IF_NOT_SIM_THREAD
#endif

ResourceListenerManager::ResourceListenerManager()
{
	mResourceLoadedConn = GetResources().OnResourceLoaded.Connect(std::bind(&::bs::ResourceListenerManager::OnResourceLoaded, this, _1));
	mResourceModifiedConn = GetResources().OnResourceModified.Connect(std::bind(&::bs::ResourceListenerManager::OnResourceModified, this, _1));
}

ResourceListenerManager::~ResourceListenerManager()
{
	B3D_ASSERT(mResourceToListenerMap.empty() && "Not all resource listeners had their resources unregistered properly.");

	mResourceLoadedConn.Disconnect();
	mResourceModifiedConn.Disconnect();
}

void ResourceListenerManager::RegisterListener(IResourceListener* listener)
{
#if B3D_DEBUG
	RecursiveLock lock(mMutex);
	mActiveListeners.insert(listener);
#endif
}

void ResourceListenerManager::UnregisterListener(IResourceListener* listener)
{
#if B3D_DEBUG
	{
		RecursiveLock lock(mMutex);
		mActiveListeners.erase(listener);
	}
#endif

	{
		RecursiveLock lock(mMutex);
		mDirtyListeners.erase(listener);
	}

	ClearDependencies(listener);
}

void ResourceListenerManager::MarkListenerDirty(IResourceListener* listener)
{
	RecursiveLock lock(mMutex);
	mDirtyListeners.insert(listener);
}

void ResourceListenerManager::Update()
{
	THROW_IF_NOT_SIM_THREAD
	UpdateListeners();

	{
		RecursiveLock lock(mMutex);

		for(auto& entry : mLoadedResources)
			SendResourceLoaded(entry.second);

		for(auto& entry : mModifiedResources)
			SendResourceModified(entry.second);

		mLoadedResources.clear();
		mModifiedResources.clear();
	}
}

void ResourceListenerManager::UpdateListeners()
{
	{
		RecursiveLock lock(mMutex);

		for(auto& listener : mDirtyListeners)
			mTempListenerBuffer.push_back(listener);

		mDirtyListeners.clear();
	}

	for(auto& listener : mTempListenerBuffer)
	{
		ClearDependencies(listener);
		AddDependencies(listener);
	}

	mTempListenerBuffer.clear();
}

void ResourceListenerManager::NotifyListeners(const UUID& resourceUUID)
{
	THROW_IF_NOT_SIM_THREAD
	UpdateListeners();

	HResource loadedResource;
	{
		RecursiveLock lock(mMutex);

		const auto iterFind = mLoadedResources.find(resourceUUID);
		if(iterFind != mLoadedResources.end())
		{
			loadedResource = std::move(iterFind->second);
			mLoadedResources.erase(iterFind);
		}
	}

	if(loadedResource)
		SendResourceLoaded(loadedResource);

	HResource modifiedResource;
	{
		RecursiveLock lock(mMutex);

		const auto iterFind = mModifiedResources.find(resourceUUID);
		if(iterFind != mModifiedResources.end())
		{
			modifiedResource = std::move(iterFind->second);
			mModifiedResources.erase(iterFind);
		}
	}

	if(modifiedResource)
		SendResourceModified(modifiedResource);
}

void ResourceListenerManager::OnResourceLoaded(const HResource& resource)
{
	RecursiveLock lock(mMutex);

	mLoadedResources[resource.GetUuid()] = resource;
}

void ResourceListenerManager::OnResourceModified(const HResource& resource)
{
	RecursiveLock lock(mMutex);

	mModifiedResources[resource.GetUuid()] = resource;
}

void ResourceListenerManager::SendResourceLoaded(const HResource& resource)
{
	u64 handleId = (u64)resource.GetHandleData().get();

	auto iterFind = mResourceToListenerMap.find(handleId);
	if(iterFind == mResourceToListenerMap.end())
		return;

	const Vector<IResourceListener*> relevantListeners = iterFind->second;
	for(auto& listener : relevantListeners)
	{
#if B3D_DEBUG
		B3D_ASSERT(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

		listener->NotifyResourceLoaded(resource);
	}
}

void ResourceListenerManager::SendResourceModified(const HResource& resource)
{
	u64 handleId = (u64)resource.GetHandleData().get();

	auto iterFind = mResourceToListenerMap.find(handleId);
	if(iterFind == mResourceToListenerMap.end())
		return;

	const Vector<IResourceListener*> relevantListeners = iterFind->second;
	for(auto& listener : relevantListeners)
	{
#if B3D_DEBUG
		B3D_ASSERT(mActiveListeners.find(listener) != mActiveListeners.end() && "Attempting to notify a destroyed IResourceListener");
#endif

		listener->NotifyResourceChanged(resource);
	}
}

void ResourceListenerManager::ClearDependencies(IResourceListener* listener)
{
	auto iterFind = mListenerToResourceMap.find(listener);
	if(iterFind == mListenerToResourceMap.end())
		return;

	const Vector<u64>& dependantResources = iterFind->second;
	for(auto& resourceHandleId : dependantResources)
	{
		auto iterFind2 = mResourceToListenerMap.find(resourceHandleId);
		if(iterFind2 != mResourceToListenerMap.end())
		{
			Vector<IResourceListener*>& listeners = iterFind2->second;
			auto iterFind3 = std::find(listeners.begin(), listeners.end(), listener);

			if(iterFind3 != listeners.end())
				listeners.erase(iterFind3);

			if(listeners.size() == 0)
				mResourceToListenerMap.erase(iterFind2);
		}
	}

	mListenerToResourceMap.erase(iterFind);
}

void ResourceListenerManager::AddDependencies(IResourceListener* listener)
{
	listener->GetListenerResources(mTempResourceBuffer);

	if(mTempResourceBuffer.size() > 0)
	{
		Vector<u64> resourceHandleIds(mTempResourceBuffer.size());
		u32 idx = 0;
		for(auto& resource : mTempResourceBuffer)
		{
			u64 handleId = (u64)resource.GetHandleData().get();
			resourceHandleIds[idx] = handleId;
			mResourceToListenerMap[handleId].push_back(listener);

			idx++;
		}

		mListenerToResourceMap[listener] = resourceHandleIds;
	}

	mTempResourceBuffer.clear();
}
