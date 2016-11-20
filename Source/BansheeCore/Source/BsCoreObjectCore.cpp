//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreObjectCore.h"
#include "BsCoreThread.h"

namespace bs
{
	Signal CoreObjectCore::mCoreGpuObjectLoadedCondition;
	Mutex CoreObjectCore::mCoreGpuObjectLoadedMutex;

	CoreObjectCore::CoreObjectCore()
		:mFlags(0)
	{ }

	CoreObjectCore::~CoreObjectCore()
	{ 
		THROW_IF_NOT_CORE_THREAD;
	}

	void CoreObjectCore::initialize()
	{
		{
			Lock lock(mCoreGpuObjectLoadedMutex);
			setIsInitialized(true);
		}

		setScheduledToBeInitialized(false);

		mCoreGpuObjectLoadedCondition.notify_all();
	}

	void CoreObjectCore::synchronize()
	{
		if (!isInitialized())
		{
#if BS_DEBUG_MODE
			if (BS_THREAD_CURRENT_ID == CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You cannot call this method on the core thread. It will cause a deadlock!");
#endif

			Lock lock(mCoreGpuObjectLoadedMutex);
			while (!isInitialized())
			{
				if (!isScheduledToBeInitialized())
					BS_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

				mCoreGpuObjectLoadedCondition.wait(lock);
			}
		}
	}

	void CoreObjectCore::_setThisPtr(SPtr<CoreObjectCore> ptrThis)
	{
		mThis = ptrThis;
	}
}