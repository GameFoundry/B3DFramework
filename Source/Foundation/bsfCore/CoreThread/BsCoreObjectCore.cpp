//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObjectCore.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	namespace ct 
	{
	Signal CoreObject::mCoreGpuObjectLoadedCondition;
	Mutex CoreObject::mCoreGpuObjectLoadedMutex;

	CoreObject::CoreObject()
		:mFlags(0)
	{ }

	CoreObject::~CoreObject()
	{ 
		THROW_IF_NOT_CORE_THREAD;
	}

	void CoreObject::initialize()
	{
		{
			Lock lock(mCoreGpuObjectLoadedMutex);
			setIsInitialized(true);
		}

		setScheduledToBeInitialized(false);

		mCoreGpuObjectLoadedCondition.notify_all();
	}

	void CoreObject::synchronize()
	{
		if (!isInitialized())
		{
#if BS_DEBUG_MODE
			if (BS_THREAD_CURRENT_ID == CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You cannot call this method on the core thread. It will cause a deadlock!");
#endif

			gCoreThread().submitAll(true);

			Lock lock(mCoreGpuObjectLoadedMutex);
			while (!isInitialized())
			{
				if (!isScheduledToBeInitialized())
					BS_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

				mCoreGpuObjectLoadedCondition.wait(lock);
			}
		}
	}

	void CoreObject::_setThisPtr(SPtr<CoreObject> ptrThis)
	{
		mThis = ptrThis;
	}
	}
}