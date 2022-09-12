//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsThreadPool.h"
#include "Threading/BsTaskScheduler.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace bs
{
	CoreThread::QueueData CoreThread::mPerThreadQueue;
	BS_THREADLOCAL CoreThread::ThreadQueueContainer* CoreThread::QueueData::current = nullptr;

#if BS_CORE_THREAD_IS_MAIN
	bool CoreThread::sAppStarted = false;
	Mutex CoreThread::sAppStartedMutex;
	Signal CoreThread::sAppStartedCondition;
#endif

	void CoreThread::OnStartUp()
	{
		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
		{
			mFrameAllocs[i] = bs_new<FrameAlloc>();
			mFrameAllocs[i]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
		}

		mSimThreadId = BS_THREAD_CURRENT_ID;
		mCoreThreadId = mSimThreadId; // For now
		mCommandQueue = bs_new<CommandQueue<CommandQueueSync>>(BS_THREAD_CURRENT_ID);

		initCoreThread();
	}

	CoreThread::~CoreThread()
	{
		// TODO - What if something gets queued between the queued call to destroy_internal and this!?
		shutdownCoreThread();

		{
			Lock Lock(mSubmitMutex);

			for(auto& queue : mAllQueues)
				bs_delete(queue);

			mAllQueues.Clear();
		}

		if(mCommandQueue != nullptr)
		{
			bs_delete(mCommandQueue);
			mCommandQueue = nullptr;
		}

		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
		{
			mFrameAllocs[i]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
			bs_delete(mFrameAllocs[i]);
		}
	}

	void CoreThread::InitCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
#if !BS_CORE_THREAD_IS_MAIN
		mCoreThread = ThreadPool::instance().Run("Core", std::bind(&CoreThread::runCoreThread, this));
#else
		{
			Lock Lock(sAppStartedMutex);
			sAppStarted = true;
		}

		sAppStartedCondition.notify_one();
#endif
		
		// Need to wait to unsure thread ID is correctly set before continuing
		Lock Lock(mThreadStartedMutex);

		while (!mCoreThreadStarted)
			mCoreThreadStartedCondition.Wait(lock);
#endif
	}

#if BS_CORE_THREAD_IS_MAIN
	void CoreThread::_run()
	{
		// Wait for the application to reach a point where core thread can be safely started
		{
			Lock Lock(sAppStartedMutex);

			while (!sAppStarted)
				sAppStartedCondition.Wait(lock);
		}

		ThreadDefaultPolicy::onThreadStarted("Core");
		instance().RunCoreThread();
		ThreadDefaultPolicy::onThreadEnded("Core");
	}
#endif

	void CoreThread::RunCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		TaskScheduler::instance().RemoveWorker(); // One less worker because we are reserving one core for this thread

		{
			Lock Lock(mThreadStartedMutex);

			mCoreThreadStarted = true;
			mCoreThreadId = BS_THREAD_CURRENT_ID;
		}

		mCoreThreadStartedCondition.notify_one();

		while(true)
		{
			// Wait until we get some ready commands
			Queue<QueuedCommand>* commands = nullptr;
			{
				Lock Lock(mCommandQueueMutex);

				while(mCommandQueue->IsEmpty())
				{
					if(mCoreThreadShutdown)
					{
						TaskScheduler::instance().AddWorker();
						return;
					}

					TaskScheduler::instance().AddWorker(); // Do something else while we wait, otherwise this core will be unused
					mCommandReadyCondition.Wait(lock);
					TaskScheduler::instance().RemoveWorker();
				}

				commands = mCommandQueue->Flush();
			}

			// Play commands
			mCommandQueue->PlaybackWithNotify(commands, std::bind(&CoreThread::commandCompletedNotify, this, _1));
		}
#endif
	}

	void CoreThread::ShutdownCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING

		{
			Lock Lock(mCommandQueueMutex);
			mCoreThreadShutdown = true;
		}

		// Wake all threads. They will quit after they see the shutdown flag
		mCommandReadyCondition.notify_all();

		mCoreThreadId = BS_THREAD_CURRENT_ID;

#if !BS_CORE_THREAD_IS_MAIN
		mCoreThread.BlockUntilComplete();
#endif
#endif
	}

	SPtr<CommandQueue<CommandQueueSync>> CoreThread::GetQueue()
	{
		if(mPerThreadQueue.current == nullptr)
		{
			SPtr<CommandQueue<CommandQueueSync>> newQueue = bs_shared_ptr_new<CommandQueue<CommandQueueSync>>(BS_THREAD_CURRENT_ID);
			mPerThreadQueue.current = bs_new<ThreadQueueContainer>();
			mPerThreadQueue.current->queue = newQueue;
			mPerThreadQueue.current->isMain = BS_THREAD_CURRENT_ID == mSimThreadId;

			Lock Lock(mSubmitMutex);
			mAllQueues.push_back(mPerThreadQueue.current);
		}

		return mPerThreadQueue.current->queue;
	}

	void CoreThread::SubmitCommandQueue(CommandQueue<CommandQueueSync>& queue, bool blockUntilComplete)
	{
		Queue<QueuedCommand>* commands = queue.Flush();

		CoreThreadQueueFlags flags = CTQF_InternalQueue;

		if(blockUntilComplete)
			flags |= CTQF_BlockUntilComplete;

		queueCommand(std::bind(&CommandQueueBase::playback, &queue, commands), flags);
	}

	void CoreThread::SubmitAll(bool blockUntilComplete)
	{
		UINT32 blockCommandId = (UINT32)-1;

		{
			// This lock is needed mainly because of blocking. Without it another submitting thread might flush a command
			// we want to wait on.
			Lock Lock(mSubmitMutex);

			// Submit workers first
			ThreadQueueContainer* mainQueue = nullptr;
			for (auto& queue : mAllQueues)
			{
				if (!queue->isMain)
					submitCommandQueue(*queue->queue, false);
				else
					mainQueue = queue;
			}

			// Then main
			if (mainQueue != nullptr)
				submitCommandQueue(*mainQueue->queue, false);

			if(blockUntilComplete)
			{
				Lock Lock2(mCommandQueueMutex);

				blockCommandId = mMaxCommandNotifyId++;
				mCommandQueue->Queue([](){}, true, blockCommandId);
			}
		}

		if(blockUntilComplete)
		{
			mCommandReadyCondition.notify_all();
			blockUntilCommandCompleted(blockCommandId);
		}
	}

	void CoreThread::Submit(bool blockUntilComplete)
	{
		Lock Lock(mSubmitMutex);

		CommandQueue<CommandQueueSync>& queue = *getQueue();
		Queue<QueuedCommand>* commands = queue.Flush();

		UINT32 commandId = -1;
		{
			Lock Lock2(mCommandQueueMutex);

			if (blockUntilComplete)
			{
				commandId = mMaxCommandNotifyId++;

				mCommandQueue->Queue([commands, &queue]() { queue.Playback(commands); }, true, commandId);
			}
			else
				mCommandQueue->Queue([commands, &queue]() { queue.Playback(commands); });
		}

		mCommandReadyCondition.notify_all();

		if (blockUntilComplete)
			blockUntilCommandCompleted(commandId);
	}

	AsyncOp CoreThread::QueueReturnCommand(std::function<void(AsyncOp&)> commandCallback, CoreThreadQueueFlags flags)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");
#endif

		if (!flags.IsSet(CTQF_InternalQueue))
			return GetQueue()->QueueReturn(commandCallback);
		else
		{
			bool blockUntilComplete = flags.IsSet(CTQF_BlockUntilComplete);

			AsyncOp op;
			UINT32 commandId = -1;
			{
				Lock Lock(mCommandQueueMutex);

				if (blockUntilComplete)
				{
					commandId = mMaxCommandNotifyId++;
					op = mCommandQueue->QueueReturn(commandCallback, true, commandId);
				}
				else
					op = mCommandQueue->QueueReturn(commandCallback);
			}

			mCommandReadyCondition.notify_all();

			if (blockUntilComplete)
				blockUntilCommandCompleted(commandId);

			return op;
		}
	}

	void CoreThread::QueueCommand(std::function<void()> commandCallback, CoreThreadQueueFlags flags)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");
#endif

		if (!flags.IsSet(CTQF_InternalQueue))
			getQueue()->Queue(commandCallback);
		else
		{
			bool blockUntilComplete = flags.IsSet(CTQF_BlockUntilComplete);

			UINT32 commandId = -1;
			{
				Lock Lock(mCommandQueueMutex);

				if (blockUntilComplete)
				{
					commandId = mMaxCommandNotifyId++;
					mCommandQueue->Queue(commandCallback, true, commandId);
				}
				else
					mCommandQueue->Queue(commandCallback);
			}

			mCommandReadyCondition.notify_all();

			if (blockUntilComplete)
				blockUntilCommandCompleted(commandId);
		}
	}

	void CoreThread::Update()
	{
		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
			mFrameAllocs[i]->SetOwnerThread(mCoreThreadId);

		mActiveFrameAlloc = (mActiveFrameAlloc + 1) % 2;
		mFrameAllocs[mActiveFrameAlloc]->SetOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
		mFrameAllocs[mActiveFrameAlloc]->Clear();
	}

	FrameAlloc* CoreThread::getFrameAlloc() const
	{
		return mFrameAllocs[mActiveFrameAlloc];
	}

	void CoreThread::BlockUntilCommandCompleted(UINT32 commandId)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING

		while(true)
		{
			Lock Lock(mCommandNotifyMutex);

			// Check if our command id is in the completed list
			auto iter = mCommandsCompleted.Begin();
			for(; iter != mCommandsCompleted.End(); ++iter)
			{
				if(*iter == commandId)
				{
					mCommandsCompleted.Erase(iter);
					return;
				}
			}

			mCommandCompleteCondition.Wait(lock);
		}
#endif
	}

	void CoreThread::CommandCompletedNotify(UINT32 commandId)
	{
		{
			Lock Lock(mCommandNotifyMutex);
			mCommandsCompleted.push_back(commandId);
		}

		mCommandCompleteCondition.notify_all();
	}

	CoreThread& GCoreThread()
	{
		return CoreThread::Instance();
	}

	void ThrowIfNotCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(BS_THREAD_CURRENT_ID != CoreThread::instance().GetCoreThreadId())
			BS_EXCEPT(InternalErrorException, "This method can only be accessed from the core thread.");
#endif
	}

	void ThrowIfCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(BS_THREAD_CURRENT_ID == CoreThread::instance().GetCoreThreadId())
			BS_EXCEPT(InternalErrorException, "This method cannot be accessed from the core thread.");
#endif
	}
}
