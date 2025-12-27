//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DThreadPool.h"

using namespace b3d;

#if B3D_SWAP_RENDER_AND_MAIN_THREAD
bool RenderThread::sAppStarted = false;
Mutex RenderThread::sAppStartedMutex;
Signal RenderThread::sAppStartedCondition;
#endif

RenderThread::RenderThread()
	:mScheduler(SchedulerCreateInformation())
{
	
}

void RenderThread::OnStartUp()
{
	mRenderThreadId = B3D_CURRENT_THREAD_ID;

#if !B3D_SWAP_RENDER_AND_MAIN_THREAD
	auto fnRunThread = [this]()
	{
		{
			Lock lock(mThreadStartedMutex);

			mRenderThreadStarted = true;
			mRenderThreadId = B3D_CURRENT_THREAD_ID;
		}

		Thread::SetName("Render Thread");
		mThreadStartedCondition.notify_one();

		mScheduler.BindToCurrentThread();
		mCommandQueue.ScheduleRunUntilShutdown(mScheduler, true, 10ms, true);
		Scheduler::UnbindFromCurrentThread();
	};

	mRenderThread = ThreadPool::Instance().Run("Render", fnRunThread);

	// Need to wait to unsure thread ID is correctly set before continuing
	Lock lock(mThreadStartedMutex);

	while(!mRenderThreadStarted)
		mThreadStartedCondition.wait(lock);
#else
	{
		Lock lock(sAppStartedMutex);
		sAppStarted = true;
	}

	sAppStartedCondition.notify_one();
#endif
}

RenderThread::~RenderThread()
{
	mCommandQueue.PostRequestShutdownCommand(true);
}

#if B3D_SWAP_RENDER_AND_MAIN_THREAD
void RenderThread::RunInternal()
{
	// Wait for the application to reach a point where render thread can be safely started
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

void RenderThread::PostCommand(std::function<void()>&& commandCallback, const char* debugName, bool waitUntilComplete, const String& extraDebugInformation)
{
	mCommandQueue.PostCommand(std::move(commandCallback), debugName, waitUntilComplete, extraDebugInformation);
}

namespace b3d
{
RenderThread& GetRenderThread()
{
	return RenderThread::Instance();
}

void AssertIfNotRenderThread()
{
	if(B3D_CURRENT_THREAD_ID != RenderThread::Instance().GetThreadId())
		B3D_EXCEPT(InternalErrorException, "This method can only be accessed from the render thread.");
}

void AssertIfRenderThread()
{
	if(B3D_CURRENT_THREAD_ID == RenderThread::Instance().GetThreadId())
		B3D_EXCEPT(InternalErrorException, "This method cannot be accessed from the render thread.");
}
} // namespace b3d
