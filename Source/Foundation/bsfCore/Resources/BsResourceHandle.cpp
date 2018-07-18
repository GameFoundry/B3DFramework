//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Resources/BsResourceHandle.h"
#include "Resources/BsResource.h"
#include "Private/RTTI/BsResourceHandleRTTI.h"
#include "Resources/BsResources.h"
#include "Managers/BsResourceListenerManager.h"

namespace bs
{
	Signal ResourceHandleBase::mResourceCreatedCondition;
	Mutex ResourceHandleBase::mResourceCreatedMutex;

	bool ResourceHandleBase::isLoaded(bool checkDependencies) const
	{
		bool isLoaded = (mData != nullptr && mData->mIsCreated && mData->mPtr != nullptr);

		if (checkDependencies && isLoaded)
			isLoaded = mData->mPtr->areDependenciesLoaded();

		return isLoaded;
	}

	void ResourceHandleBase::blockUntilLoaded(bool waitForDependencies) const
	{
		if(mData == nullptr)
			return;

		if (!mData->mIsCreated)
		{
			Lock lock(mResourceCreatedMutex);
			while (!mData->mIsCreated)
			{
				mResourceCreatedCondition.wait(lock);
			}

			// Send out ResourceListener events right away, as whatever called this method
			// probably also expects the listener events to trigger immediately as well
			ResourceListenerManager::instance().notifyListeners(mData->mUUID);
		}

		if (waitForDependencies)
		{
			bs_frame_mark();

			{
				FrameVector<HResource> dependencies;
				mData->mPtr->getResourceDependencies(dependencies);

				for (auto& dependency : dependencies)
					dependency.blockUntilLoaded(waitForDependencies);
			}

			bs_frame_clear();
		}
	}

	void ResourceHandleBase::release()
	{
		gResources().release(*this);
	}

	void ResourceHandleBase::destroy()
	{
		if(mData->mPtr)
			gResources().destroy(*this);
	}

	void ResourceHandleBase::setHandleData(const SPtr<Resource>& ptr, const UUID& uuid)
	{
		mData->mPtr = ptr;

		if(mData->mPtr)
		{
			mData->mUUID = uuid;

			if(!mData->mIsCreated)
			{
				Lock lock(mResourceCreatedMutex);
				{
					mData->mIsCreated = true;
				}

				mResourceCreatedCondition.notify_all();
			}
		}
	}

	void ResourceHandleBase::clearHandleData()
	{
		mData->mPtr = nullptr;

		Lock lock(mResourceCreatedMutex);
		mData->mIsCreated = false;
	}

	void ResourceHandleBase::addInternalRef()
	{
		mData->mRefCount.fetch_add(1, std::memory_order_relaxed);
	}

	void ResourceHandleBase::removeInternalRef()
	{
		mData->mRefCount.fetch_sub(1, std::memory_order_relaxed);
	}

	void ResourceHandleBase::throwIfNotLoaded() const
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
		return WeakResourceHandleRTTI::instance();
	}

	RTTITypeBase* TResourceHandleBase<true>::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* TResourceHandleBase<false>::getRTTIStatic()
	{
		return ResourceHandleRTTI::instance();
	}

	RTTITypeBase* TResourceHandleBase<false>::getRTTI() const
	{
		return getRTTIStatic();
	}
}
