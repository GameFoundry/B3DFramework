//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPrerequisites.h"
#include "Resources/B3DResourceHandle.h"
#include "Resources/B3DResource.h"
#include "RTTI/B3DResourceHandleRTTI.h"
#include "Resources/B3DResources.h"
#include "Managers/B3DResourceListenerManager.h"
#include "B3DApplication.h"

using namespace b3d;

void ResourceHandleData::DestroyManagedResource()
{
	if(!IsCreated)
		return; // Already destroyed or never created

	GetResources().Destroy(*this);
}

void ResourceHandleData::DestroySelf()
{
	// If Resources module is destroyed we're shutting down and no need to clear handle data
	if(!Resources::IsDestroyed())
		GetResources().DestroyHandleData(*this);

	B3DDelete(this);
}

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
		if(B3D_CURRENT_THREAD_ID == GetApplication().GetMainThreadId())
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

void ResourceHandle::Destroy() const
{
	if(mData == nullptr || !mData->IsCreated)
		return;

	mData->DestroyManagedResource();
}

void ResourceHandle::ReleaseInternalReference()
{
	GetResources().ReleaseInternalReference(*this);
}

void ResourceHandle::AssociateResourceWithHandle(const SPtr<Resource>& resource, const UUID& resourceId)
{
	mData->Object = resource;

	if(mData->Object)
		mData->Id = resourceId;
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

void ResourceHandle::ReportIfNotLoaded() const
{
	B3D_ENSURE_LOG(IsLoaded(false), "Trying to access a resource that hasn't been loaded yet.");
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
