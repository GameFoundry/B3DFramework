//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	ResourceHandleBase::ResourceHandleBase()
	{
		mData = nullptr;
	}

	ResourceHandleBase::~ResourceHandleBase() 
	{ 

	}

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

	void ResourceHandleBase::addInternalRef()
	{
		mData->mRefCount++;
	}

	void ResourceHandleBase::removeInternalRef()
	{
		mData->mRefCount--;
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