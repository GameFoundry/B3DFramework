//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObjectCore.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

namespace bs { namespace ct
{
Signal CoreObject::mCoreGpuObjectLoadedCondition;
Mutex CoreObject::mCoreGpuObjectLoadedMutex;

CoreObject::CoreObject()
	: mFlags(0)
{}

CoreObject::~CoreObject()
{
	THROW_IF_NOT_CORE_THREAD;
}

void CoreObject::Initialize()
{
	{
		Lock lock(mCoreGpuObjectLoadedMutex);
		SetIsInitialized(true);
	}

	SetScheduledToBeInitialized(false);

	mCoreGpuObjectLoadedCondition.notify_all();
}

void CoreObject::Synchronize()
{
	if(!IsInitialized())
	{
#if B3D_DEBUG
		if(BS_THREAD_CURRENT_ID == CoreThread::Instance().GetCoreThreadId())
			B3D_EXCEPT(InternalErrorException, "You cannot call this method on the core thread. It will cause a deadlock!");
#endif

		GetCoreThread().SubmitAll(true);

		Lock lock(mCoreGpuObjectLoadedMutex);
		while(!IsInitialized())
		{
			if(!IsScheduledToBeInitialized())
				B3D_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

			mCoreGpuObjectLoadedCondition.wait(lock);
		}
	}
}

void CoreObject::SetThisPtrInternal(SPtr<CoreObject> ptrThis)
{
	mThis = ptrThis;
}
}}
