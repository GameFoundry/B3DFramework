//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreObject/BsRenderThread.h"
#include "Threading/BsThreadPool.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

using namespace bs;

#if BS_CORE_THREAD_IS_MAIN
bool CoreThread::sAppStarted = false;
Mutex CoreThread::sAppStartedMutex;
Signal CoreThread::sAppStartedCondition;
#endif

CoreThread::CoreThread()
	:mScheduler(SchedulerCreateInformation())
{
	
}

void CoreThread::OnStartUp()
{
	mCoreThreadId = B3D_CURRENT_THREAD_ID;

#if !BS_CORE_THREAD_IS_MAIN
	auto fnRunThread = [this]()
	{
		{
			Lock lock(mThreadStartedMutex);

			mCoreThreadStarted = true;
			mCoreThreadId = B3D_CURRENT_THREAD_ID;
		}

		Thread::SetName("Render Thread");
		mCoreThreadStartedCondition.notify_one();

		mScheduler.BindToCurrentThread();
		mCommandQueue.ScheduleRunUntilShutdown(mScheduler, true, 10ms, true);
		Scheduler::UnbindFromCurrentThread();
	};

	mCoreThread = ThreadPool::Instance().Run("Core", fnRunThread);

	// Need to wait to unsure thread ID is correctly set before continuing
	Lock lock(mThreadStartedMutex);

	while(!mCoreThreadStarted)
		mCoreThreadStartedCondition.wait(lock);
#else
	{
		Lock lock(sAppStartedMutex);
		sAppStarted = true;
	}

	sAppStartedCondition.notify_one();
#endif
}

CoreThread::~CoreThread()
{
	mCommandQueue.PostRequestShutdownCommand(true);
}

#if BS_CORE_THREAD_IS_MAIN
void CoreThread::RunInternal()
{
	// Wait for the application to reach a point where core thread can be safely started
	{
		Lock lock(sAppStartedMutex);

		while(!sAppStarted)
			sAppStartedCondition.wait(lock);
	}

	mScheduler.BindToCurrentThread();
	mCommandQueue.RunUntilShutdown();
	mScheduler.UnbindFromCurrentThread();
}
#endif

void CoreThread::PostCommand(std::function<void()>&& commandCallback, bool waitUntilComplete)
{
	mCommandQueue.PostCommand(std::move(commandCallback), "CoreThread Command", waitUntilComplete);
}

namespace bs
{
CoreThread& GetCoreThread()
{
	return CoreThread::Instance();
}

void AssertIfNotRenderThread()
{
	if(B3D_CURRENT_THREAD_ID != CoreThread::Instance().GetCoreThreadId())
		B3D_EXCEPT(InternalErrorException, "This method can only be accessed from the core thread.");
}

void AssertIfRenderThread()
{
	if(B3D_CURRENT_THREAD_ID == CoreThread::Instance().GetCoreThreadId())
		B3D_EXCEPT(InternalErrorException, "This method cannot be accessed from the core thread.");
}
} // namespace bs
