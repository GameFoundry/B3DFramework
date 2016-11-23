//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreThread.h"
#include "BsThreadPool.h"
#include "BsTaskScheduler.h"
#include "BsFrameAlloc.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace bs
{
	CoreThread::QueueData CoreThread::mPerThreadQueue;
	BS_THREADLOCAL CoreThread::ThreadQueueContainer* CoreThread::QueueData::current = nullptr;

	CoreThread::CoreThread()
		: mActiveFrameAlloc(0)
		, mCoreThreadShutdown(false)
		, mCoreThreadStarted(false)
		, mCommandQueue(nullptr)
		, mMaxCommandNotifyId(0)
	{
		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
		{
			mFrameAllocs[i] = bs_new<FrameAlloc>();
			mFrameAllocs[i]->setOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
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
			Lock lock(mCoreQueueMutex);

			for(auto& queue : mAllQueues)
				bs_delete(queue);

			mAllQueues.clear();
		}

		if(mCommandQueue != nullptr)
		{
			bs_delete(mCommandQueue);
			mCommandQueue = nullptr;
		}

		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
		{
			mFrameAllocs[i]->setOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
			bs_delete(mFrameAllocs[i]);
		}
	}

	void CoreThread::initCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
#if BS_THREAD_SUPPORT
		mCoreThread = ThreadPool::instance().run("Core", std::bind(&CoreThread::runCoreThread, this));
		
		// Need to wait to unsure thread ID is correctly set before continuing
		Lock lock(mThreadStartedMutex);

		while (!mCoreThreadStarted)
			mCoreThreadStartedCondition.wait(lock);
#else
		BS_EXCEPT(InternalErrorException, "Attempting to start a core thread but application isn't compiled with thread support.");
#endif
#endif
	}

	void CoreThread::runCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		TaskScheduler::instance().removeWorker(); // One less worker because we are reserving one core for this thread

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

				while(mCommandQueue->isEmpty())
				{
					if(mCoreThreadShutdown)
					{
						TaskScheduler::instance().addWorker();
						return;
					}

					TaskScheduler::instance().addWorker(); // Do something else while we wait, otherwise this core will be unused
					mCommandReadyCondition.wait(lock);
					TaskScheduler::instance().removeWorker();
				}

				commands = mCommandQueue->flush();
			}

			// Play commands
			mCommandQueue->playbackWithNotify(commands, std::bind(&CoreThread::commandCompletedNotify, this, _1)); 
		}
#endif
	}

	void CoreThread::shutdownCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING

		{
			Lock lock(mCommandQueueMutex);
			mCoreThreadShutdown = true;
		}

		// Wake all threads. They will quit after they see the shutdown flag
		mCommandReadyCondition.notify_all();

		mCoreThreadId = BS_THREAD_CURRENT_ID;

		mCoreThread.blockUntilComplete();
#endif
	}

	SPtr<TCoreThreadQueue<CommandQueueNoSync>> CoreThread::getQueue()
	{
		if(mPerThreadQueue.current == nullptr)
		{
			SPtr<TCoreThreadQueue<CommandQueueNoSync>> newQueue = bs_shared_ptr_new<TCoreThreadQueue<CommandQueueNoSync>>(BS_THREAD_CURRENT_ID);
			mPerThreadQueue.current = bs_new<ThreadQueueContainer>();
			mPerThreadQueue.current->queue = newQueue;
			mPerThreadQueue.current->isMain = BS_THREAD_CURRENT_ID == mSimThreadId;

			Lock lock(mCoreQueueMutex);
			mAllQueues.push_back(mPerThreadQueue.current);
		}

		return mPerThreadQueue.current->queue;
	}

	void CoreThread::submitAll(bool blockUntilComplete)
	{
		Vector<ThreadQueueContainer*> queueCopies;

		{
			Lock lock(mCoreQueueMutex);

			queueCopies = mAllQueues;
		}

		// Submit workers first
		ThreadQueueContainer* mainQueue = nullptr;
		for (auto& queue : queueCopies)
		{
			if (!queue->isMain)
				queue->queue->submitToCoreThread(blockUntilComplete);
			else
				mainQueue = queue;
		}

		// Then main
		if (mainQueue != nullptr)
			mainQueue->queue->submitToCoreThread(blockUntilComplete);
	}

	void CoreThread::submit(bool blockUntilComplete)
	{
		getQueue()->submitToCoreThread(blockUntilComplete);
	}

	AsyncOp CoreThread::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback, CoreThreadQueueFlags flags)
	{
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");

		if (!flags.isSet(CTQF_InternalQueue))
			return getQueue()->queueReturnCommand(commandCallback);
		else
		{
			bool blockUntilComplete = flags.isSet(CTQF_BlockUntilComplete);

			AsyncOp op;
			UINT32 commandId = -1;
			{
				Lock lock(mCommandQueueMutex);

				if (blockUntilComplete)
				{
					commandId = mMaxCommandNotifyId++;
					op = mCommandQueue->queueReturn(commandCallback, true, commandId);
				}
				else
					op = mCommandQueue->queueReturn(commandCallback);
			}

			mCommandReadyCondition.notify_all();

			if (blockUntilComplete)
				blockUntilCommandCompleted(commandId);

			return op;
		}
	}

	void CoreThread::queueCommand(std::function<void()> commandCallback, CoreThreadQueueFlags flags)
	{
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");

		if (!flags.isSet(CTQF_InternalQueue))
			getQueue()->queueCommand(commandCallback);
		else
		{
			bool blockUntilComplete = flags.isSet(CTQF_BlockUntilComplete);

			UINT32 commandId = -1;
			{
				Lock lock(mCommandQueueMutex);

				if (blockUntilComplete)
				{
					commandId = mMaxCommandNotifyId++;
					mCommandQueue->queue(commandCallback, true, commandId);
				}
				else
					mCommandQueue->queue(commandCallback);
			}

			mCommandReadyCondition.notify_all();

			if (blockUntilComplete)
				blockUntilCommandCompleted(commandId);
		}
	}

	void CoreThread::update()
	{
		for (UINT32 i = 0; i < NUM_SYNC_BUFFERS; i++)
			mFrameAllocs[i]->setOwnerThread(mCoreThreadId);

		mActiveFrameAlloc = (mActiveFrameAlloc + 1) % 2;
		mFrameAllocs[mActiveFrameAlloc]->setOwnerThread(BS_THREAD_CURRENT_ID); // Sim thread
		mFrameAllocs[mActiveFrameAlloc]->clear();
	}

	FrameAlloc* CoreThread::getFrameAlloc() const
	{
		return mFrameAllocs[mActiveFrameAlloc];
	}

	void CoreThread::blockUntilCommandCompleted(UINT32 commandId)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		Lock lock(mCommandNotifyMutex);

		while(true)
		{
			// TODO - This might be causing a deadlock in Release mode. I'm thinking because mCommandsCompleted isn't marked as volatile.

			// Check if our command id is in the completed list
			auto iter = mCommandsCompleted.begin();
			for(; iter != mCommandsCompleted.end(); ++iter)
			{
				if(*iter == commandId)
					break;
			}

			if(iter != mCommandsCompleted.end())
			{
				mCommandsCompleted.erase(iter);
				break;
			}

			mCommandCompleteCondition.wait(lock);
		}
#endif
	}

	void CoreThread::commandCompletedNotify(UINT32 commandId)
	{
		{
			Lock lock(mCommandNotifyMutex);

			mCommandsCompleted.push_back(commandId);
		}

		mCommandCompleteCondition.notify_all();
	}

	CoreThread& gCoreThread()
	{
		return CoreThread::instance();
	}

	void throwIfNotCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
			BS_EXCEPT(InternalErrorException, "This method can only be accessed from the core thread.");
#endif
	}

	void throwIfCoreThread()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(BS_THREAD_CURRENT_ID == CoreThread::instance().getCoreThreadId())
			BS_EXCEPT(InternalErrorException, "This method cannot be accessed from the core thread.");
#endif
	}
}