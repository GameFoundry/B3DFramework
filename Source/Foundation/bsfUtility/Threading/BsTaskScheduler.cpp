//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/BsTaskScheduler.h"
#include "Threading/BsThreadPool.h"

using namespace bs;

Task::Task(const PrivatelyConstruct& dummy, const String& name, std::function<void()> taskWorker, TaskPriority priority, SPtr<Task> dependency)
	: mName(name), mPriority(priority), mTaskWorker(std::move(taskWorker)), mTaskDependency(std::move(dependency))
{
}

SPtr<Task> Task::Create(const String& name, std::function<void()> taskWorker, TaskPriority priority, SPtr<Task> dependency)
{
	return B3DMakeShared<Task>(PrivatelyConstruct(), name, std::move(taskWorker), priority, std::move(dependency));
}

bool Task::IsComplete() const
{
	return mState == 2;
}

bool Task::IsCanceled() const
{
	return mState == 3;
}

bool Task::HasStarted() const
{
	u32 state = mState;

	return state == 1 || state == 2;
}

void Task::Wait()
{
	if(mParent != nullptr)
		mParent->WaitUntilComplete(this);
}

void Task::Cancel()
{
	mState = 3;
}

TaskGroup::TaskGroup(const PrivatelyConstruct& dummy, String name, std::function<void(u32)> taskWorker, u32 count, TaskPriority priority, SPtr<Task> dependency)
	: mName(std::move(name)), mCount(count), mPriority(priority), mTaskWorker(std::move(taskWorker)), mTaskDependency(std::move(dependency))
{
}

SPtr<TaskGroup> TaskGroup::Create(String name, std::function<void(u32)> taskWorker, u32 count, TaskPriority priority, SPtr<Task> dependency)
{
	return B3DMakeShared<TaskGroup>(PrivatelyConstruct(), std::move(name), std::move(taskWorker), count, priority, std::move(dependency));
}

bool TaskGroup::IsComplete() const
{
	return mNumRemainingTasks == 0;
}

void TaskGroup::Wait()
{
	if(mParent != nullptr)
		mParent->WaitUntilComplete(this);
}

TaskScheduler::TaskScheduler()
	: mTaskQueue(&TaskScheduler::TaskCompare)
{
	mMaxActiveTasks = BS_THREAD_HARDWARE_CONCURRENCY;

	mTaskSchedulerThread = ThreadPool::Instance().Run("TaskScheduler", std::bind(&TaskScheduler::RunMain, this));
}

TaskScheduler::~TaskScheduler()
{
	// Wait until all tasks complete
	{
		Lock activeTaskLock(mReadyMutex);

		while(mActiveTasks.size() > 0)
		{
			SPtr<Task> task = mActiveTasks[0];
			activeTaskLock.unlock();

			task->Wait();
			activeTaskLock.lock();
		}
	}

	// Start shutdown of the main queue worker and wait until it exits
	{
		Lock lock(mReadyMutex);

		mShutdown = true;
	}

	mTaskReadyCond.notify_one();

	mTaskSchedulerThread.BlockUntilComplete();
}

void TaskScheduler::AddTask(SPtr<Task> task)
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

void TaskScheduler::AddTaskGroup(const SPtr<TaskGroup>& taskGroup)
{
	Lock lock(mReadyMutex);

	for(u32 i = 0; i < taskGroup->mCount; i++)
	{
		const auto worker = [i, taskGroup]
		{
			taskGroup->mTaskWorker(i);
			--taskGroup->mNumRemainingTasks;
		};

		SPtr<Task> task = Task::Create(taskGroup->mName, worker, taskGroup->mPriority, taskGroup->mTaskDependency);
		task->mParent = this;
		task->mTaskId = mNextTaskId++;
		task->mState.store(0); // Reset state in case the task is getting re-queued

		mCheckTasks = true;
		mTaskQueue.insert(std::move(task));
	}

	taskGroup->mParent = this;

	// Wake main scheduler thread
	mTaskReadyCond.notify_one();
}

void TaskScheduler::AddWorker()
{
	Lock lock(mReadyMutex);

	mMaxActiveTasks++;

	// A spot freed up, queue new tasks on main scheduler thread if they exist
	mTaskReadyCond.notify_one();
}

void TaskScheduler::RemoveWorker()
{
	Lock lock(mReadyMutex);

	if(mMaxActiveTasks > 0)
		mMaxActiveTasks--;
}

void TaskScheduler::RunMain()
{
	while(true)
	{
		Lock lock(mReadyMutex);

		while((!mCheckTasks || (u32)mActiveTasks.size() >= mMaxActiveTasks) && !mShutdown)
			mTaskReadyCond.wait(lock);

		mCheckTasks = false;

		if(mShutdown)
			break;

		for(auto iter = mTaskQueue.begin(); iter != mTaskQueue.end();)
		{
			if((u32)mActiveTasks.size() >= mMaxActiveTasks)
				break;

			SPtr<Task> curTask = *iter;

			if(curTask->IsCanceled())
			{
				iter = mTaskQueue.erase(iter);
				continue;
			}

			if(curTask->mTaskDependency != nullptr && !curTask->mTaskDependency->IsComplete())
			{
				++iter;
				continue;
			}

			// Spin until a thread becomes available. This happens primarily because our mActiveTask count and
			// ThreadPool's thread idle count aren't synced, so while the task manager thinks it's free to run new
			// tasks, the ThreadPool might still have those threads as running, meaning their allocation will fail.
			// So we just spin here for a bit, in that rare case.
			if(ThreadPool::Instance().GetNumAvailable() == 0)
			{
				mCheckTasks = true;
				break;
			}

			iter = mTaskQueue.erase(iter);

			curTask->mState.store(1);
			mActiveTasks.push_back(curTask);

			ThreadPool::Instance().Run(curTask->mName, std::bind(&TaskScheduler::RunTask, this, curTask));
		}
	}
}

void TaskScheduler::RunTask(SPtr<Task> task)
{
	task->mTaskWorker();

	{
		Lock lock(mReadyMutex);

		auto findIter = std::find(mActiveTasks.begin(), mActiveTasks.end(), task);
		if(findIter != mActiveTasks.end())
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

void TaskScheduler::WaitUntilComplete(const Task* task)
{
	if(task->IsCanceled())
		return;

	if(task->mTaskDependency)
		task->mTaskDependency->Wait();

	// If we haven't started executing the task yet, just execute it right here
	SPtr<Task> queuedTask;
	{
		Lock lock(mReadyMutex);

		if(!task->HasStarted())
		{
			auto iterFind = std::find_if(mTaskQueue.begin(), mTaskQueue.end(), [task](const SPtr<Task>& x)
										 { return x.get() == task; });

			assert(iterFind != mTaskQueue.end());

			queuedTask = *iterFind;
			mTaskQueue.erase(iterFind);

			queuedTask->mState.store(1);
		}
	}

	if(queuedTask)
	{
		RunTask(queuedTask);
		return;
	}

	// Otherwise we wait until the task completes
	{
		Lock lock(mCompleteMutex);

		while(!task->IsComplete())
		{
			AddWorker();
			mTaskCompleteCond.wait(lock);
			RemoveWorker();
		}
	}
}

void TaskScheduler::WaitUntilComplete(const TaskGroup* taskGroup)
{
	Lock lock(mCompleteMutex);

	while(taskGroup->mNumRemainingTasks > 0)
	{
		AddWorker();
		mTaskCompleteCond.wait(lock);
		RemoveWorker();
	}
}

bool TaskScheduler::TaskCompare(const SPtr<Task>& lhs, const SPtr<Task>& rhs)
{
	// If priority is the same, sort by the order the tasks were queued
	if(lhs->mPriority == rhs->mPriority)
		return lhs->mTaskId < rhs->mTaskId;

	// Otherwise the task with the higher priority always goes first
	return lhs->mPriority > rhs->mPriority;
}
