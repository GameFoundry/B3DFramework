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
		mResourceCreatedCondition.Wait(lock, [this] { return mData->MIsCreated; });

		// Send out ResourceListener events right away, as whatever called this method probably also expects the
		// listener events to trigger immediately as well
		if(B3D_CURRENT_THREAD_ID == GetCoreApplication().GetMainThreadId())
			ResourceListenerManager::Instance().NotifyListeners(mData->MUuid);
	}

	if(waitForDependencies)
	{
		B3DMarkAllocatorFrame();

		{
			FrameVector<HResource> dependencies;
			mData->MPtr->GetResourceDependencies(dependencies);

			for(auto& dependency : dependencies)
				dependency.BlockUntilLoaded(waitForDependencies);
		}

		B3DClearAllocatorFrame();
	}
}

void ResourceHandleBase::Release()
{
	GetResources().ReleaseInternalReference(*this);
}

void ResourceHandleBase::Destroy()
{
	if(mData->MPtr)
		GetResources().Destroy(*this);
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

		mResourceCreatedCondition.NotifyAll();
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
#if B3D_DEBUG
	if(!IsLoaded(false))
	{
		B3D_EXCEPT(InternalErrorException, "Trying to access a resource that hasn't been loaded yet.");
	}
#endif
}

RTTIType* TResourceHandleBase<true>::GetRttiStatic()
{
	return WeakResourceHandleRTTI::Instance();
}

RTTIType* TResourceHandleBase<true>::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* TResourceHandleBase<false>::GetRttiStatic()
{
	return ResourceHandleRTTI::Instance();
}

RTTIType* TResourceHandleBase<false>::GetRtti() const
{
	return GetRttiStatic();
}
