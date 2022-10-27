//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsThreadPool.h"
#include "Threading/BsTaskScheduler.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

using namespace bs;

CoreThread::QueueData CoreThread::mPerThreadQueue;
BS_THREADLOCAL CoreThread::ThreadQueueContainer* CoreThread::QueueData::current = nullptr;

#if BS_CORE_THREAD_IS_MAIN
bool CoreThread::sAppStarted = false;
Mutex CoreThread::sAppStartedMutex;
Signal CoreThread::sAppStartedCondition;
#endif

void CoreThread::OnStartUp()
{
	for(u32 i = 0; i < kNumSyncBuffers; i++)
	{
		mFrameAllocs[i] = bs_new<FrameAlloc>();
		mFrameAllocs[i]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
	}

	mSimThreadId = BS_THREAD_CURRENT_ID;
	mCoreThreadId = mSimThreadId; // For now
	mCommandQueue = bs_new<CommandQueue<CommandQueueSync>>(BS_THREAD_CURRENT_ID);

	InitCoreThread();
}

CoreThread::~CoreThread()
{
	// TODO - What if something gets queued between the queued call to destroy_internal and this!?
	ShutdownCoreThread();

	{
		Lock lock(mSubmitMutex);

		for(auto& queue : mAllQueues)
			bs_delete(queue);

		mAllQueues.clear();
	}

	if(mCommandQueue != nullptr)
	{
		bs_delete(mCommandQueue);
		mCommandQueue = nullptr;
	}

	for(u32 i = 0; i < kNumSyncBuffers; i++)
	{
		mFrameAllocs[i]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
		bs_delete(mFrameAllocs[i]);
	}
}

void CoreThread::InitCoreThread()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
#	if !BS_CORE_THREAD_IS_MAIN
	mCoreThread = ThreadPool::Instance().Run("Core", std::bind(&::bs::CoreThread::RunCoreThread, this));
#	else
	{
		Lock lock(sAppStartedMutex);
		sAppStarted = true;
	}

	sAppStartedCondition.notify_one();
#	endif

	// Need to wait to unsure thread ID is correctly set before continuing
	Lock lock(mThreadStartedMutex);

	while(!mCoreThreadStarted)
		mCoreThreadStartedCondition.wait(lock);
#endif
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

	ThreadDefaultPolicy::onThreadStarted("Core");
	instance().runCoreThread();
	ThreadDefaultPolicy::onThreadEnded("Core");
}
#endif

void CoreThread::RunCoreThread()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	TaskScheduler::Instance().RemoveWorker(); // One less worker because we are reserving one core for this thread

	{
		Lock lock(mThreadStartedMutex);

		mCoreThreadStarted = true;
		mCoreThreadId = BS_THREAD_CURRENT_ID;
	}

	mCoreThreadStartedCondition.notify_one();

	while(true)
	{
		// Wait until we get some ready commands
		Queue<QueuedCommand>* commands = nullptr;
		{
			Lock lock(mCommandQueueMutex);

			while(mCommandQueue->IsEmpty())
			{
				if(mCoreThreadShutdown)
				{
					TaskScheduler::Instance().AddWorker();
					return;
				}

				TaskScheduler::Instance().AddWorker(); // Do something else while we wait, otherwise this core will be unused
				mCommandReadyCondition.wait(lock);
				TaskScheduler::Instance().RemoveWorker();
			}

			commands = mCommandQueue->Flush();
		}

		// Play commands
		mCommandQueue->PlaybackWithNotify(commands, std::bind(&::bs::CoreThread::CommandCompletedNotify, this, _1));
	}
#endif
}

void CoreThread::ShutdownCoreThread()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING

	{
		Lock lock(mCommandQueueMutex);
		mCoreThreadShutdown = true;
	}

	// Wake all threads. They will quit after they see the shutdown flag
	mCommandReadyCondition.notify_all();

	mCoreThreadId = BS_THREAD_CURRENT_ID;

#	if !BS_CORE_THREAD_IS_MAIN
	mCoreThread.BlockUntilComplete();
#	endif
#endif
}

SPtr<CommandQueue<CommandQueueSync>> CoreThread::GetQueue()
{
	if(mPerThreadQueue.current == nullptr)
	{
		SPtr<CommandQueue<CommandQueueSync>> newQueue = bs_shared_ptr_new<CommandQueue<CommandQueueSync>>(BS_THREAD_CURRENT_ID);
		mPerThreadQueue.current = bs_new<ThreadQueueContainer>();
		mPerThreadQueue.current->Queue = newQueue;
		mPerThreadQueue.current->IsMain = BS_THREAD_CURRENT_ID == mSimThreadId;

		Lock lock(mSubmitMutex);
		mAllQueues.push_back(mPerThreadQueue.current);
	}

	return mPerThreadQueue.current->Queue;
}

void CoreThread::SubmitCommandQueue(CommandQueue<CommandQueueSync>& queue, bool blockUntilComplete)
{
	Queue<QueuedCommand>* commands = queue.Flush();

	CoreThreadQueueFlags flags = CTQF_InternalQueue;

	if(blockUntilComplete)
		flags |= CTQF_BlockUntilComplete;

	QueueCommand(std::bind(&::bs::CommandQueueBase::Playback, &queue, commands), flags);
}

void CoreThread::SubmitAll(bool blockUntilComplete)
{
	u32 blockCommandId = (u32)-1;

	{
		// This lock is needed mainly because of blocking. Without it another submitting thread might flush a command
		// we want to wait on.
		Lock lock(mSubmitMutex);

		// Submit workers first
		ThreadQueueContainer* mainQueue = nullptr;
		for(auto& queue : mAllQueues)
		{
			if(!queue->IsMain)
				SubmitCommandQueue(*queue->Queue, false);
			else
				mainQueue = queue;
		}

		// Then main
		if(mainQueue != nullptr)
			SubmitCommandQueue(*mainQueue->Queue, false);

		if(blockUntilComplete)
		{
			Lock lock2(mCommandQueueMutex);

			blockCommandId = mMaxCommandNotifyId++;
			mCommandQueue->Queue([]() {}, true, blockCommandId);
		}
	}

	if(blockUntilComplete)
	{
		mCommandReadyCondition.notify_all();
		BlockUntilCommandCompleted(blockCommandId);
	}
}

void CoreThread::Submit(bool blockUntilComplete)
{
	Lock lock(mSubmitMutex);

	CommandQueue<CommandQueueSync>& queue = *GetQueue();
	Queue<QueuedCommand>* commands = queue.Flush();

	u32 commandId = -1;
	{
		Lock lock2(mCommandQueueMutex);

		if(blockUntilComplete)
		{
			commandId = mMaxCommandNotifyId++;

			mCommandQueue->Queue([commands, &queue]()
								 { queue.Playback(commands); },
								 true, commandId);
		}
		else
			mCommandQueue->Queue([commands, &queue]()
								 { queue.Playback(commands); });
	}

	mCommandReadyCondition.notify_all();

	if(blockUntilComplete)
		BlockUntilCommandCompleted(commandId);
}

AsyncOp CoreThread::QueueReturnCommand(std::function<void(AsyncOp&)> commandCallback, CoreThreadQueueFlags flags)
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	assert(BS_THREAD_CURRENT_ID != GetCoreThreadId() && "Cannot queue commands on the core thread for the core thread");
#endif

	if(!flags.IsSet(CTQF_InternalQueue))
		return GetQueue()->QueueReturn(commandCallback);
	else
	{
		bool blockUntilComplete = flags.IsSet(CTQF_BlockUntilComplete);

		AsyncOp op;
		u32 commandId = -1;
		{
			Lock lock(mCommandQueueMutex);

			if(blockUntilComplete)
			{
				commandId = mMaxCommandNotifyId++;
				op = mCommandQueue->QueueReturn(commandCallback, true, commandId);
			}
			else
				op = mCommandQueue->QueueReturn(commandCallback);
		}

		mCommandReadyCondition.notify_all();

		if(blockUntilComplete)
			BlockUntilCommandCompleted(commandId);

		return op;
	}
}

void CoreThread::QueueCommand(std::function<void()> commandCallback, CoreThreadQueueFlags flags)
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	assert(BS_THREAD_CURRENT_ID != GetCoreThreadId() && "Cannot queue commands on the core thread for the core thread");
#endif

	if(!flags.IsSet(CTQF_InternalQueue))
		GetQueue()->Queue(commandCallback);
	else
	{
		bool blockUntilComplete = flags.IsSet(CTQF_BlockUntilComplete);

		u32 commandId = -1;
		{
			Lock lock(mCommandQueueMutex);

			if(blockUntilComplete)
			{
				commandId = mMaxCommandNotifyId++;
				mCommandQueue->Queue(commandCallback, true, commandId);
			}
			else
				mCommandQueue->Queue(commandCallback);
		}

		mCommandReadyCondition.notify_all();

		if(blockUntilComplete)
			BlockUntilCommandCompleted(commandId);
	}
}

void CoreThread::Update()
{
	for(u32 i = 0; i < kNumSyncBuffers; i++)
		mFrameAllocs[i]->SetOwnerThread(mCoreThreadId);

	mActiveFrameAlloc = (mActiveFrameAlloc + 1) % 2;
	mFrameAllocs[mActiveFrameAlloc]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
	mFrameAllocs[mActiveFrameAlloc]->Clear();
}

FrameAlloc* CoreThread::GetFrameAlloc() const
{
	return mFrameAllocs[mActiveFrameAlloc];
}

void CoreThread::BlockUntilCommandCompleted(u32 commandId)
{
#if !BS_FORCE_SINGLETHREADED_RENDERING

	while(true)
	{
		Lock lock(mCommandNotifyMutex);

		// Check if our command id is in the completed list
		auto iter = mCommandsCompleted.begin();
		for(; iter != mCommandsCompleted.end(); ++iter)
		{
			if(*iter == commandId)
			{
				mCommandsCompleted.erase(iter);
				return;
			}
		}

		mCommandCompleteCondition.wait(lock);
	}
#endif
}

void CoreThread::CommandCompletedNotify(u32 commandId)
{
	{
		Lock lock(mCommandNotifyMutex);
		mCommandsCompleted.push_back(commandId);
	}

	mCommandCompleteCondition.notify_all();
}

namespace bs
{
CoreThread& gCoreThread()
{
	return CoreThread::Instance();
}

void throwIfNotCoreThread()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	if(BS_THREAD_CURRENT_ID != CoreThread::Instance().GetCoreThreadId())
		BS_EXCEPT(InternalErrorException, "This method can only be accessed from the core thread.");
#endif
}

void throwIfCoreThread()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	if(BS_THREAD_CURRENT_ID == CoreThread::Instance().GetCoreThreadId())
		BS_EXCEPT(InternalErrorException, "This method cannot be accessed from the core thread.");
#endif
}
} // namespace bs
