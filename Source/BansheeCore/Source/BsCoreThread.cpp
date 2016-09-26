//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreThread.h"
#include "BsThreadPool.h"
#include "BsTaskScheduler.h"
#include "BsFrameAlloc.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	CoreThread::AccessorData CoreThread::mAccessor;
	BS_THREADLOCAL CoreThread::AccessorContainer* CoreThread::AccessorData::current = nullptr;

	CoreThread::CoreThread()
		: mActiveFrameAlloc(0)
		, mCoreThreadShutdown(false)
		, mCoreThreadStarted(false)
		, mCommandQueue(nullptr)
		, mMaxCommandNotifyId(0)
		, mSyncedCoreAccessor(nullptr)
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
			Lock lock(mAccessorMutex);

			for(auto& accessor : mAccessors)
			{
				bs_delete(accessor);
			}

			mAccessors.clear();
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
			mSyncedCoreAccessor = bs_new<CoreThreadAccessor<CommandQueueSync>>(BS_THREAD_CURRENT_ID);
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
						bs_delete(mSyncedCoreAccessor);
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

	SPtr<CoreThreadAccessor<CommandQueueNoSync>> CoreThread::getAccessor()
	{
		if(mAccessor.current == nullptr)
		{
			SPtr<CoreThreadAccessor<CommandQueueNoSync>> newAccessor = bs_shared_ptr_new<CoreThreadAccessor<CommandQueueNoSync>>(BS_THREAD_CURRENT_ID);
			mAccessor.current = bs_new<AccessorContainer>();
			mAccessor.current->accessor = newAccessor;
			mAccessor.current->isMain = BS_THREAD_CURRENT_ID == mSimThreadId;

			Lock lock(mAccessorMutex);
			mAccessors.push_back(mAccessor.current);
		}

		return mAccessor.current->accessor;
	}

	SyncedCoreAccessor& CoreThread::getSyncedAccessor()
	{
		return *mSyncedCoreAccessor;
	}

	void CoreThread::submitAccessors(bool blockUntilComplete)
	{
		Vector<AccessorContainer*> accessorCopies;

		{
			Lock lock(mAccessorMutex);

			accessorCopies = mAccessors;
		}

		// Submit workers first
		AccessorContainer* mainAccessor = nullptr;
		for (auto& accessor : accessorCopies)
		{
			if (!accessor->isMain)
				accessor->accessor->submitToCoreThread(blockUntilComplete);
			else
				mainAccessor = accessor;
		}

		// Then main
		if (mainAccessor != nullptr)
			mainAccessor->accessor->submitToCoreThread(blockUntilComplete);

		// Then synced
		mSyncedCoreAccessor->submitToCoreThread(blockUntilComplete);
	}

	AsyncOp CoreThread::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback, bool blockUntilComplete)
	{
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");

		AsyncOp op;
		UINT32 commandId = -1;
		{
			Lock lock(mCommandQueueMutex);

			if(blockUntilComplete)
			{
				commandId = mMaxCommandNotifyId++;
				op = mCommandQueue->queueReturn(commandCallback, true, commandId);
			}
			else
				op = mCommandQueue->queueReturn(commandCallback);
		}

		mCommandReadyCondition.notify_all();

		if(blockUntilComplete)
			blockUntilCommandCompleted(commandId);

		return op;
	}

	void CoreThread::queueCommand(std::function<void()> commandCallback, bool blockUntilComplete)
	{
		assert(BS_THREAD_CURRENT_ID != getCoreThreadId() && "Cannot queue commands on the core thread for the core thread");

		UINT32 commandId = -1;
		{
			Lock lock(mCommandQueueMutex);

			if(blockUntilComplete)
			{
				commandId = mMaxCommandNotifyId++;
				mCommandQueue->queue(commandCallback, true, commandId);
			}
			else
				mCommandQueue->queue(commandCallback);
		}

		mCommandReadyCondition.notify_all();

		if(blockUntilComplete)
			blockUntilCommandCompleted(commandId);
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

	CoreThreadAccessor<CommandQueueNoSync>& gCoreAccessor()
	{
		return *CoreThread::instance().getAccessor();
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