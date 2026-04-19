//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DResourceListenerManager.h"

#include "B3DApplication.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DIResourceListener.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

static void AssertIfNotMainThread()
{
	B3D_ASSERT(B3D_CURRENT_THREAD_ID == Application::Instance().GetMainThreadId() && "This method can only be accessed from the simulation thread.");
}

ResourceListenerManager::ResourceListenerManager()
{
	auto fnOnResourceLoaded = [this](const HResource& resource)
	{
		OnResourceLoaded(resource);
	};
	mResourceLoadedConn = GetResources().OnResourceLoaded.Connect(fnOnResourceLoaded);

	auto fnOnResourceModified = [this](const HResource& resource)
	{
		OnResourceModified(resource);
	};
	mResourceModifiedConn = GetResources().OnResourceModified.Connect(fnOnResourceModified);
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
	AssertIfNotMainThread();	
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
	AssertIfNotMainThread();	
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

	mLoadedResources[resource.GetId()] = resource;
}

void ResourceListenerManager::OnResourceModified(const HResource& resource)
{
	RecursiveLock lock(mMutex);

	mModifiedResources[resource.GetId()] = resource;
}

void ResourceListenerManager::SendResourceLoaded(const HResource& resource)
{
	u64 handleId = (u64)resource.GetHandleData();

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
	u64 handleId = (u64)resource.GetHandleData();

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
		u32 resourceIndex = 0;
		for(auto& resource : mTempResourceBuffer)
		{
			u64 handleId = (u64)resource.GetHandleData();
			resourceHandleIds[resourceIndex] = handleId;
			mResourceToListenerMap[handleId].push_back(listener);

			resourceIndex++;
		}

		mListenerToResourceMap[listener] = resourceHandleIds;
	}

	mTempResourceBuffer.clear();
}
