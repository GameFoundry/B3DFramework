//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/BsTaskScheduler.h"
#include "Threading/BsThreadPool.h"

namespace bs
{
	Task::Task(const PrivatelyConstruct& dummy, const String& name, std::function<void()> taskWorker,
		TaskPriority priority, SPtr<Task> dependency)
		: mName(name), mPriority(priority), mTaskWorker(std::move(taskWorker)), mTaskDependency(std::move(dependency))
	{

	}

	SPtr<Task> Task::create(const String& name, std::function<void()> taskWorker, TaskPriority priority, 
		SPtr<Task> dependency)
	{
		return bs_shared_ptr_new<Task>(PrivatelyConstruct(), name, std::move(taskWorker), priority, std::move(dependency));
	}

	bool Task::isComplete() const
	{
		return mState == 2;
	}

	bool Task::isCanceled() const
	{
		return mState == 3;
	}

	void Task::wait()
	{
		if(mParent != nullptr)
			mParent->waitUntilComplete(this);
	}

	void Task::cancel()
	{
		mState = 3;
	}

	TaskScheduler::TaskScheduler()
		:mTaskQueue(&TaskScheduler::taskCompare)
	{
		mMaxActiveTasks = BS_THREAD_HARDWARE_CONCURRENCY;

		mTaskSchedulerThread = ThreadPool::instance().run("TaskScheduler", std::bind(&TaskScheduler::runMain, this));
	}

	TaskScheduler::~TaskScheduler()
	{
		// Wait until all tasks complete
		{
			Lock activeTaskLock(mReadyMutex);

			while (mActiveTasks.size() > 0)
			{
				SPtr<Task> task = mActiveTasks[0];
				activeTaskLock.unlock();

				task->wait();
				activeTaskLock.lock();
			}
		}

		// Start shutdown of the main queue worker and wait until it exits
		{
			Lock lock(mReadyMutex);

			mShutdown = true;
		}

		mTaskReadyCond.notify_one();

		mTaskSchedulerThread.blockUntilComplete();
	}

	void TaskScheduler::addTask(SPtr<Task> task)
	{
		Lock lock(mReadyMutex);

		assert(task->mState != 1 && "Task is already executing, it cannot be executed again until it finishes.");

		task->mParent = this;
		task->mTaskId = mNextTaskId++;
		task->mState.store(0); // Reset state in case the task is getting re-queued

		mCheckTasks = true;
		mTaskQueue.insert(std::move(task));

		// Wake main scheduler thread
		mTaskReadyCond.notify_one();
	}

	void TaskScheduler::addWorker()
	{
		Lock lock(mReadyMutex);

		mMaxActiveTasks++;

		// A spot freed up, queue new tasks on main scheduler thread if they exist
		mTaskReadyCond.notify_one();
	}

	void TaskScheduler::removeWorker()
	{
		Lock lock(mReadyMutex);

		if(mMaxActiveTasks > 0)
			mMaxActiveTasks--;
	}

	void TaskScheduler::runMain()
	{
		while(true)
		{
			Lock lock(mReadyMutex);

			while((!mCheckTasks || (UINT32)mActiveTasks.size() >= mMaxActiveTasks) && !mShutdown)
				mTaskReadyCond.wait(lock);

			mCheckTasks = false;

			if(mShutdown)
				break;

			for(auto iter = mTaskQueue.begin(); iter != mTaskQueue.end();)
			{
				if ((UINT32)mActiveTasks.size() >= mMaxActiveTasks)
					break;	

				SPtr<Task> curTask = *iter;

				if(curTask->isCanceled())
				{
					iter = mTaskQueue.erase(iter);
					continue;
				}

				if(curTask->mTaskDependency != nullptr && !curTask->mTaskDependency->isComplete())
				{
					++iter;
					continue;
				}

				iter = mTaskQueue.erase(iter);

				curTask->mState.store(1);
				mActiveTasks.push_back(curTask);

				ThreadPool::instance().run(curTask->mName, std::bind(&TaskScheduler::runTask, this, curTask));
			}
		}
	}

	void TaskScheduler::runTask(SPtr<Task> task)
	{
		task->mTaskWorker();

		{
			Lock lock(mReadyMutex);

			auto findIter = std::find(mActiveTasks.begin(), mActiveTasks.end(), task);
			if (findIter != mActiveTasks.end())
				mActiveTasks.erase(findIter);
		}

		{
			Lock lock(mCompleteMutex);
			task->mState.store(2);

			mTaskCompleteCond.notify_all();
		}

		// Wake the main scheduler thread in case there are other tasks waiting or this task was someone's dependency
		{
			Lock lock(mReadyMutex);

			mCheckTasks = true;
			mTaskReadyCond.notify_one();
		}
	}

	void TaskScheduler::waitUntilComplete(const Task* task)
	{
		if(task->isCanceled())
			return;

		{
			Lock lock(mCompleteMutex);

			while(!task->isComplete())
			{
				addWorker();
				mTaskCompleteCond.wait(lock);
				removeWorker();
			}
		}
	}

	bool TaskScheduler::taskCompare(const SPtr<Task>& lhs, const SPtr<Task>& rhs)
	{
		// If one tasks priority is higher, that one goes first
		if(lhs->mPriority > rhs->mPriority)
			return true;

		// Otherwise we go by smaller id, as that task was queued earlier than the other
		return lhs->mTaskId < rhs->mTaskId;
	}
}
