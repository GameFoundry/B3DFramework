//************************************ bs::framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsWorkerThreadWithCommandQueue.h"
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsTaskScheduler.h"

using namespace bs;

WorkerThreadWithCommandQueue::WorkerThreadWithCommandQueue(const char* threadName)
	: mCommandQueue(B3D_CURRENT_THREAD_ID)
{
	auto fnThreadWorker = [this]()
	{
		mThreadId = B3D_CURRENT_THREAD_ID;
		mCommandQueue.SetThreadId(B3D_CURRENT_THREAD_ID);

		{
			Lock lock(mThreadStartedMutex);
			mIsWorkerStarted = true;
			mThreadStartedSignal.notify_one();
		}

		TaskScheduler::Instance().RemoveWorker();

		while(true)
		{
			bs::Queue<QueuedCommand>* commandToExecute = nullptr;
			{
				Lock lock(mCommandQueuedMutex);

				while(mCommandQueue.IsEmpty())
				{
					if(mIsWorkerShutdownRequested)
					{
						TaskScheduler::Instance().AddWorker();
						return;
					}

					TaskScheduler::Instance().AddWorker();
					mCommandQueuedSignal.wait(lock);
					TaskScheduler::Instance().RemoveWorker();
				}

				commandToExecute = mCommandQueue.Flush();
			}

			mCommandQueue.PlaybackWithNotify(commandToExecute, [this](u32 commandId)
			{
				{
					Lock lock(mCommandCompleteMutex);
					mCompletedCommandIds.push_back(commandId);
				}

				mCommandCompleteSignal.notify_all(); });
			}
	};

	mThread = ThreadPool::Instance().Run(threadName, fnThreadWorker);

	// Wait until thread starts so thread ID is assigned
	Lock lock(mThreadStartedMutex);
	while(!mIsWorkerStarted)
	{
		mThreadStartedSignal.wait(lock);
	}
}

WorkerThreadWithCommandQueue::~WorkerThreadWithCommandQueue()
{
	{
		Lock lock(mCommandQueuedMutex);
		mIsWorkerShutdownRequested = true;
	}

	mCommandQueuedSignal.notify_all();
	mThread.BlockUntilComplete();
}

AsyncOp WorkerThreadWithCommandQueue::QueueCommand(std::function<void(AsyncOp&)>&& commandCallback, const char* name, bool waitUntilComplete)
{
	// TODO - Add and pass name to CommandQueue

	AsyncOp asyncOperation;
	u32 commandId = ~0u;
	{
		Lock lock(mCommandQueuedMutex);

		if(waitUntilComplete)
		{
			commandId = mNextBlockableCommandId++;
			asyncOperation = mCommandQueue.QueueReturn(std::move(commandCallback), true, commandId);
		}
		else
			asyncOperation = mCommandQueue.QueueReturn(std::move(commandCallback));
	}

	mCommandQueuedSignal.notify_all();

	if(waitUntilComplete)
		WaitUntilCommandCompletes(commandId);

	return asyncOperation;
}

void WorkerThreadWithCommandQueue::QueueCommand(std::function<void()>&& commandCallback, const char* name, bool waitUntilComplete)
{
	// TODO - Add and pass name to CommandQueue

	u32 commandId = ~0u;
	{
		Lock lock(mCommandQueuedMutex);

		if(waitUntilComplete)
		{
			commandId = mNextBlockableCommandId++;
			mCommandQueue.Queue(std::move(commandCallback), true, commandId);
		}
		else
			mCommandQueue.Queue(std::move(commandCallback));
	}

	mCommandQueuedSignal.notify_all();

	if(waitUntilComplete)
		WaitUntilCommandCompletes(commandId);
}

void WorkerThreadWithCommandQueue::WaitUntilCommandCompletes(u32 commandId)
{
	while(true)
	{
		Lock lock(mCommandCompleteMutex);

		auto it = mCompletedCommandIds.begin();
		for(; it != mCompletedCommandIds.end(); ++it)
		{
			if(*it == commandId)
			{
				mCompletedCommandIds.erase(it);
				return;
			}
		}

		mCommandCompleteSignal.wait(lock);
	}
}
