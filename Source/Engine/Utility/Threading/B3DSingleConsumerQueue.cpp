//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSingleConsumerQueue.h"

#include "Debug/B3DDebug.h"
#include "Threading/B3DScheduler.h"

using namespace b3d;

SingleConsumerQueue::SingleConsumerQueue()
	:mCommandCompletedSignalEvent(SignalEvent::Mode::AutomaticallyReset)
{
	mCommandQueue = B3DNew<Queue<QueuedCommand>>();
}

SingleConsumerQueue::~SingleConsumerQueue()
{
	Lock lock(mCommandQueueMutex);
	B3D_ENSURE(mCommandQueue == nullptr || mCommandQueue->empty());

	if(mCommandQueue != nullptr)
		B3DDelete(mCommandQueue);

	while(!mEmptyCommandQueues.empty())
	{
		B3DDelete(mEmptyCommandQueues.top());
		mEmptyCommandQueues.pop();
	}
}

void SingleConsumerQueue::RunUntilShutdown()
{
	mThreadId = Thread::GetCurrentThreadId();
	mSchedulerThread = SchedulerThread::Get();

	while (true)
	{
		RunUntilIdle(Clock::now());

		if (mIsShutdownRequested)
			break;

		auto fnIsNotEmpty = [this]() { return mCommandQueue != nullptr && !mCommandQueue->empty(); };

		Lock lock(mCommandQueueMutex);
		mCommandAddedSignal.Wait(lock, fnIsNotEmpty);
	}
}

void SingleConsumerQueue::ScheduleRunUntilShutdown(Scheduler& scheduler, bool runOnCallingThread, Milliseconds yieldInterval, bool blockUntilDone)
{
	TShared<SignalEvent> isDone = B3DMakeShared<SignalEvent>();

	auto fnRun = [this, yieldInterval, &scheduler, isDone](const auto& run)
	{
		mThreadId = Thread::GetCurrentThreadId();
		mSchedulerThread = SchedulerThread::Get();

		TimePoint startTime = Clock::now();

		while (true)
		{
			const bool isTimeoutReached = RunUntilIdle(startTime, yieldInterval);

			if (mIsShutdownRequested)
			{
				isDone->Signal();
				return;
			}

			// If timeout reached, re-schedule itself. This lets other tasks on the scheduler thread to have a go, as the new task will be put at the back of the queue.
			if (isTimeoutReached)
			{
				scheduler.Post(SchedulerTask([run]() { run(run); }, "SingleConsumerQueue re-schedule due timeout", SchedulerTaskFlag::SameThread));
				return;
			}

			auto fnIsNotEmpty = [this]() { return mCommandQueue != nullptr && !mCommandQueue->empty(); };

			Lock lock(mCommandQueueMutex);
			mCommandAddedSignal.Wait(lock, fnIsNotEmpty);
		}
	};

	if(runOnCallingThread)
		scheduler.Post(SchedulerTask([fnRun] { fnRun(fnRun); }, "SingleConsumerQueue run on same thread", SchedulerTaskFlag::SameThread));
	else
		scheduler.Post(SchedulerTask([fnRun] { fnRun(fnRun); }, "SingleConsumerQueue run"));

	if(blockUntilDone)
		isDone->Wait();
}

void SingleConsumerQueue::PostRequestShutdownCommand(bool waitUntilComplete)
{
	PostCommand([this]() { mIsShutdownRequested = true; }, "Request shutdown", waitUntilComplete);
}

void SingleConsumerQueue::PostCommand(Function<void()>&& callback, const char* debugName, bool waitUntilComplete, const String& extraInformation)
{
	if (waitUntilComplete)
	{
		Mutex completionMutex;
		Signal completionSignal;
		bool isCompleted = false;

		auto fnRunBlocking = [&completionMutex, &completionSignal, &isCompleted, function = std::move(callback)]()
		{
			function();

			{
				Lock lock(completionMutex);
				isCompleted = true;
			}

			completionSignal.NotifyOne();
		};

		QueuedCommand newCommand(std::move(fnRunBlocking), debugName, extraInformation);

		{
			Lock lock(mCommandQueueMutex);
			mCommandQueue->push(newCommand);

			mCommandAddedSignal.NotifyAll();
		}

		{
			Lock lock(completionMutex);
			completionSignal.Wait(lock, [&isCompleted] { return isCompleted; });
		}
	}
	else
	{
		QueuedCommand newCommand(std::move(callback), debugName, extraInformation);

		{
			Lock lock(mCommandQueueMutex);
			mCommandQueue->push(newCommand);

			mCommandAddedSignal.NotifyAll();
		}
	}
}

bool SingleConsumerQueue::RunUntilIdle(TimePoint startTime, Milliseconds timeout)
{
	if (!B3D_ENSURE_LOG(mThreadId == Thread::GetCurrentThreadId(), "Called from incorrect fiber."))
		return false;

	Queue<QueuedCommand>* commandsToProcess = nullptr;

	{
		Lock lock(mCommandQueueMutex);
		commandsToProcess = mCommandQueue;

		if (!mEmptyCommandQueues.empty())
		{
			mCommandQueue = mEmptyCommandQueues.top();
			mEmptyCommandQueues.pop();
		}
		else
		{
			mCommandQueue = B3DNew<Queue<QueuedCommand>>();
		}
	}

	if(commandsToProcess == nullptr)
		return false;

	while(!commandsToProcess->empty())
	{
		QueuedCommand& command = commandsToProcess->front();

		if (command.Callback != nullptr)
			command.Callback();

		commandsToProcess->pop();

		TimePoint currentTime = Clock::now();
		if(timeout != 0ms && (currentTime - startTime) > timeout)
		{
			if(!commandsToProcess->empty())
			{
				Lock lock(mCommandQueueMutex);

				if (mCommandQueue != nullptr)
				{
					while (!mCommandQueue->empty())
					{
						commandsToProcess->push(mCommandQueue->front());
						mCommandQueue->pop();
					}

					mEmptyCommandQueues.push(mCommandQueue);
				}
				
				mCommandQueue = commandsToProcess;
			}

			return true;
		}
	}

	Lock lock(mCommandQueueMutex);
	mEmptyCommandQueues.push(commandsToProcess);

	return false;
}

void SingleConsumerQueue::CancelAll()
{
	Lock lock(mCommandQueueMutex);

	while(!mCommandQueue->empty())
		mCommandQueue->pop();
}

bool SingleConsumerQueue::IsEmpty()
{
	Lock lock(mCommandQueueMutex);

	return mCommandQueue == nullptr || mCommandQueue->empty();
}
