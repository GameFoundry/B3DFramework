//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Resources/BsResourceHandle.h"
#include "Resources/BsResource.h"
#include "Private/RTTI/BsResourceHandleRTTI.h"
#include "Resources/BsResources.h"
#include "Managers/BsResourceListenerManager.h"
#include "BsCoreApplication.h"

using namespace bs;

Signal ResourceHandle::mResourceCreatedCondition;
Mutex ResourceHandle::mResourceCreatedMutex;

bool ResourceHandle::IsLoaded(bool checkDependencies) const
{
	bool isLoaded = (mData != nullptr && mData->IsCreated && mData->Object != nullptr);

	if(checkDependencies && isLoaded)
		isLoaded = mData->Object->AreDependenciesLoaded();

	return isLoaded;
}

void ResourceHandle::BlockUntilLoaded(bool waitForDependencies) const
{
	if(mData == nullptr)
		return;

	if(!mData->IsCreated)
	{
		Lock lock(mResourceCreatedMutex);
		mResourceCreatedCondition.Wait(lock, [this] { return mData->IsCreated; });

		// Send out ResourceListener events right away, as whatever called this method probably also expects the
		// listener events to trigger immediately as well
		if(B3D_CURRENT_THREAD_ID == GetCoreApplication().GetMainThreadId())
			ResourceListenerManager::Instance().NotifyListeners(mData->Id);
	}

	if(waitForDependencies)
	{
		B3DMarkAllocatorFrame();

		{
			FrameVector<HResource> dependencies;
			mData->Object->GetResourceDependencies(dependencies);

			for(auto& dependency : dependencies)
				dependency.BlockUntilLoaded(waitForDependencies);
		}

		B3DClearAllocatorFrame();
	}
}

void ResourceHandle::ReleaseInternalReference()
{
	GetResources().ReleaseInternalReference(*this);
}

void ResourceHandle::Destroy()
{
	if(mData->Object)
		GetResources().Destroy(*this);
}

void ResourceHandle::SetHandleData(const SPtr<Resource>& ptr, const UUID& uuid)
{
	mData->Object = ptr;

	if(mData->Object)
		mData->Id = uuid;
}

void ResourceHandle::NotifyLoadComplete()
{
	if(!mData->IsCreated)
	{
		Lock lock(mResourceCreatedMutex);
		{
			mData->IsCreated = true;
		}

		mResourceCreatedCondition.NotifyAll();
	}
}

void ResourceHandle::ClearHandleData()
{
	mData->Object = nullptr;

	Lock lock(mResourceCreatedMutex);
	mData->IsCreated = false;
}

void ResourceHandle::IncrementInternalReferenceCount()
{
	mData->ReferenceCount.fetch_add(1, std::memory_order_relaxed);
}

void ResourceHandle::DecrementInternalReferenceCount()
{
	mData->ReferenceCount.fetch_sub(1, std::memory_order_relaxed);
}

void ResourceHandle::ThrowIfNotLoaded() const
{
#if B3D_DEBUG
	if(!IsLoaded(false))
	{
		B3D_EXCEPT(InternalErrorException, "Trying to access a resource that hasn't been loaded yet.");
	}
#endif
}

RTTIType* WeakResourceHandle::GetRttiStatic()
{
	return WeakResourceHandleRTTI::Instance();
}

RTTIType* WeakResourceHandle::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* StrongResourceHandle::GetRttiStatic()
{
	return StrongResourceHandleRTTI::Instance();
}

RTTIType* StrongResourceHandle::GetRtti() const
{
	return GetRttiStatic();
}
