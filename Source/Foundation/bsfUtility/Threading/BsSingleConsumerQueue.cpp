//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSingleConsumerQueue.h"

#include "Debug/BsDebug.h"
#include "Threading/BsScheduler.h"

using namespace bs;

SingleConsumerQueue::SingleConsumerQueue()
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

	while (true)
	{
		RunUntilIdle();

		if (mIsShutdownRequested)
			break;

		auto fnIsNotEmpty = [this]() { return mCommandQueue != nullptr && !mCommandQueue->empty(); };

		Lock lock(mCommandQueueMutex);
		mSignal.Wait(lock, fnIsNotEmpty);
	}
}

void SingleConsumerQueue::PostRequestShutdownCommand(bool waitUntilComplete)
{
	PostCommand([this]() { mIsShutdownRequested = true; }, "Request shutdown", waitUntilComplete);
}

void SingleConsumerQueue::PostCommand(Function<void()>&& callback, const char* debugName, bool waitUntilComplete)
{
	if (waitUntilComplete)
	{
		Mutex completionMutex;
		FiberSignal completionSignal;
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

		QueuedCommand newCommand(std::move(fnRunBlocking), debugName);

		{
			Lock lock(mCommandQueueMutex);
			mCommandQueue->push(newCommand);

			mSignal.NotifyAll();
		}

		{
			Lock lock(completionMutex);
			completionSignal.Wait(lock, [&isCompleted] { return isCompleted; });
		}
	}
	else
	{
		QueuedCommand newCommand(std::move(callback), debugName);

		{
			Lock lock(mCommandQueueMutex);
			mCommandQueue->push(newCommand);

			mSignal.NotifyAll();
		}
	}
}

void SingleConsumerQueue::RunUntilIdle()
{
	if (!B3D_ENSURE_LOG(mThreadId == Thread::GetCurrentThreadId(), "FiberQueue::ProcessCommands called from incorrect fiber."))
		return;

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
		return;

	while(!commandsToProcess->empty())
	{
		QueuedCommand& command = commandsToProcess->front();

		if(command.Callback != nullptr)
			command.Callback();

		commandsToProcess->pop();
	}

	Lock lock(mCommandQueueMutex);
	mEmptyCommandQueues.push(commandsToProcess);
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
