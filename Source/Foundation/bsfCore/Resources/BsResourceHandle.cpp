//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Resources/BsResourceHandle.h"
#include "Resources/BsResource.h"
#include "Private/RTTI/BsResourceHandleRTTI.h"
#include "Resources/BsResources.h"
#include "Managers/BsResourceListenerManager.h"
#include "BsCoreApplication.h"

namespace bs
{
	Signal ResourceHandleBase::mResourceCreatedCondition;
	Mutex ResourceHandleBase::mResourceCreatedMutex;

	bool ResourceHandleBase::IsLoaded(bool checkDependencies) const
	{
		bool isLoaded = (mData != nullptr && mData->mIsCreated && mData->mPtr != nullptr);

		if (checkDependencies && isLoaded)
			isLoaded = mData->mPtr->AreDependenciesLoaded();

		return isLoaded;
	}

	void ResourceHandleBase::BlockUntilLoaded(bool waitForDependencies) const
	{
		if(mData == nullptr)
			return;

		if (!mData->mIsCreated)
		{
			Lock lock(mResourceCreatedMutex);
			while (!mData->mIsCreated)
			{
				mResourceCreatedCondition.Wait(lock);
			}

			// Send out ResourceListener events right away, as whatever called this method probably also expects the
			// listener events to trigger immediately as well
			if(BS_THREAD_CURRENT_ID == gCoreApplication().GetSimThreadId())
				ResourceListenerManager::instance().NotifyListeners(mData->mUUID);
		}

		if (waitForDependencies)
		{
			bs_frame_mark();

			{
				FrameVector<HResource> dependencies;
				mData->mPtr->GetResourceDependencies(dependencies);

				for (auto& dependency : dependencies)
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
		if(mData->mPtr)
			gResources().Destroy(*this);
	}

	void ResourceHandleBase::SetHandleData(const SPtr<Resource>& ptr, const UUID& uuid)
	{
		mData->mPtr = ptr;

		if(mData->mPtr)
			mData->mUUID = uuid;
	}

	void ResourceHandleBase::NotifyLoadComplete()
	{
		if (!mData->mIsCreated)
		{
			Lock lock(mResourceCreatedMutex);
			{
				mData->mIsCreated = true;
			}

			mResourceCreatedCondition.notify_all();
		}
	}

	void ResourceHandleBase::ClearHandleData()
	{
		mData->mPtr = nullptr;

		Lock lock(mResourceCreatedMutex);
		mData->mIsCreated = false;
	}

	void ResourceHandleBase::AddInternalRef()
	{
		mData->mRefCount.fetch_add(1, std::memory_order_relaxed);
	}

	void ResourceHandleBase::RemoveInternalRef()
	{
		mData->mRefCount.fetch_sub(1, std::memory_order_relaxed);
	}

	void ResourceHandleBase::ThrowIfNotLoaded() const
	{
#if BS_DEBUG_MODE
		if (!isLoaded(false))
		{
			BS_EXCEPT(InternalErrorException, "Trying to access a resource that hasn't been loaded yet.");
		}
#endif
	}

	RTTITypeBase* TResourceHandleBase<true>::getRTTIStatic()
	{
		return WeakResourceHandleRTTI::Instance();
	}

	RTTITypeBase* TResourceHandleBase<true>::getRTTI() const
	{
		return GetRTTIStatic();
	}

	RTTITypeBase* TResourceHandleBase<false>::getRTTIStatic()
	{
		return ResourceHandleRTTI::Instance();
	}

	RTTITypeBase* TResourceHandleBase<false>::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
