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

Signal ResourceHandleBase::mResourceCreatedCondition;
Mutex ResourceHandleBase::mResourceCreatedMutex;

bool ResourceHandleBase::IsLoaded(bool checkDependencies) const
{
	bool isLoaded = (mData != nullptr && mData->MIsCreated && mData->MPtr != nullptr);

	if(checkDependencies && isLoaded)
		isLoaded = mData->MPtr->AreDependenciesLoaded();

	return isLoaded;
}

void ResourceHandleBase::BlockUntilLoaded(bool waitForDependencies) const
{
	if(mData == nullptr)
		return;

	if(!mData->MIsCreated)
	{
		Lock lock(mResourceCreatedMutex);
		while(!mData->MIsCreated)
		{
			mResourceCreatedCondition.wait(lock);
		}

		// Send out ResourceListener events right away, as whatever called this method probably also expects the
		// listener events to trigger immediately as well
		if(BS_THREAD_CURRENT_ID == gCoreApplication().GetSimThreadId())
			ResourceListenerManager::Instance().NotifyListeners(mData->MUuid);
	}

	if(waitForDependencies)
	{
		bs_frame_mark();

		{
			FrameVector<HResource> dependencies;
			mData->MPtr->GetResourceDependencies(dependencies);

			for(auto& dependency : dependencies)
				dependency.BlockUntilLoaded(waitForDependencies);
		}

		bs_frame_clear();
	}
}

void ResourceHandleBase::Release()
{
	gResources().Release(*this);
}

void ResourceHandleBase::Destroy()
{
	if(mData->MPtr)
		gResources().Destroy(*this);
}

void ResourceHandleBase::SetHandleData(const SPtr<Resource>& ptr, const UUID& uuid)
{
	mData->MPtr = ptr;

	if(mData->MPtr)
		mData->MUuid = uuid;
}

void ResourceHandleBase::NotifyLoadComplete()
{
	if(!mData->MIsCreated)
	{
		Lock lock(mResourceCreatedMutex);
		{
			mData->MIsCreated = true;
		}

		mResourceCreatedCondition.notify_all();
	}
}

void ResourceHandleBase::ClearHandleData()
{
	mData->MPtr = nullptr;

	Lock lock(mResourceCreatedMutex);
	mData->MIsCreated = false;
}

void ResourceHandleBase::AddInternalRef()
{
	mData->MRefCount.fetch_add(1, std::memory_order_relaxed);
}

void ResourceHandleBase::RemoveInternalRef()
{
	mData->MRefCount.fetch_sub(1, std::memory_order_relaxed);
}

void ResourceHandleBase::ThrowIfNotLoaded() const
{
#if BS_DEBUG_MODE
	if(!IsLoaded(false))
	{
		BS_EXCEPT(InternalErrorException, "Trying to access a resource that hasn't been loaded yet.");
	}
#endif
}

RTTITypeBase* TResourceHandleBase<true>::GetRttiStatic()
{
	return WeakResourceHandleRTTI::Instance();
}

RTTITypeBase* TResourceHandleBase<true>::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* TResourceHandleBase<false>::GetRttiStatic()
{
	return ResourceHandleRTTI::Instance();
}

RTTITypeBase* TResourceHandleBase<false>::GetRtti() const
{
	return GetRttiStatic();
}
