//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsModule.h"
#include "Threading/BsThreadPool.h"

namespace bs
{
	/** @addtogroup Threading
	 *  @{
	 */
	class TaskScheduler;

	/** Task priority. Tasks with higher priority will get executed sooner. */
	enum class TaskPriority
	{
		VeryLow = 98,
		Low = 99,
		Normal = 100,
		High = 101,
		VeryHigh = 102
	};

	/**
	 * Represents a single task that may be queued in the TaskScheduler.
	 *
	 * @note	Thread safe.
	 */
	class B3D_UTILITY_EXPORT Task
	{
		struct PrivatelyConstruct
		{};

	public:
		Task(const PrivatelyConstruct& dummy, const String& name, std::function<void()> taskWorker, TaskPriority priority, SPtr<Task> dependency);

		/**
		 * Creates a new task. Task should be provided to TaskScheduler in order for it to start.
		 *
		 * @param[in]	name		Name you can use to more easily identify the task.
		 * @param[in]	taskWorker	Worker method that does all of the work in the task.
		 * @param[in]	priority  	(optional) Higher priority means the tasks will be executed sooner.
		 * @param[in]	dependency	(optional) Task dependency if one exists. If provided the task will
		 * 							not be executed until its dependency is complete.
		 */
		static SPtr<Task> Create(const String& name, std::function<void()> taskWorker, TaskPriority priority = TaskPriority::Normal, SPtr<Task> dependency = nullptr);

		/** Returns true if the task has completed. */
		bool IsComplete() const;

		/**	Returns true if the task has been canceled. */
		bool IsCanceled() const;

		/** Returns true if the task has started or completed execution. */
		bool HasStarted() const;

		/**
		 * Blocks the current thread until the task has completed.
		 *
		 * @note	While waiting adds a new worker thread, so that the blocking threads core can be utilized.
		 */
		void Wait();

		/** Cancels the task and removes it from the TaskSchedulers queue. */
		void Cancel();

	private:
		friend class TaskScheduler;

		String mName;
		TaskPriority mPriority;
		u32 mTaskId = 0;
		std::function<void()> mTaskWorker;
		SPtr<Task> mTaskDependency;
		std::atomic<u32> mState{ 0 }; /**< 0 - Inactive, 1 - In progress, 2 - Completed, 3 - Canceled */

		TaskScheduler* mParent = nullptr;
	};

	/**
	 * Represents a group of tasks that may be queued in the TaskScheduler to be processed in parallel.
	 *
	 * @note	Thread safe.
	 */
	class B3D_UTILITY_EXPORT TaskGroup
	{
		struct PrivatelyConstruct
		{};

	public:
		TaskGroup(const PrivatelyConstruct& dummy, String name, std::function<void(u32)> taskWorker, u32 count, TaskPriority priority, SPtr<Task> dependency);

		/**
		 * Creates a new task group. Task group should be provided to TaskScheduler in order for it to start.
		 *
		 * @param[in]	name		Name you can use to more easily identify the tasks in the group.
		 * @param[in]	taskWorker	Worker method that will get called for each item in the group. Each call will receive
		 *							a sequential index of the item in the group.
		 * @param[in]	count		Number of items in the task group. Each item will be processed in a worker thread.
		 * @param[in]	priority  	(optional) Higher priority means the tasks will be executed sooner.
		 * @param[in]	dependency	(optional) Task dependency if one exists. If provided the task will
		 * 							not be executed until its dependency is complete.
		 */
		static SPtr<TaskGroup> Create(String name, std::function<void(u32)> taskWorker, u32 count, TaskPriority priority = TaskPriority::Normal, SPtr<Task> dependency = nullptr);

		/** Returns true if all the tasks in the group have completed. */
		bool IsComplete() const;

		/**
		 * Blocks the current thread until all tasks in the group have completed.
		 *
		 * @note	While waiting adds a new worker thread, so that the blocking threads core can be utilized.
		 */
		void Wait();

	private:
		friend class TaskScheduler;

		String mName;
		u32 mCount;
		TaskPriority mPriority;
		std::function<void(u32)> mTaskWorker;
		SPtr<Task> mTaskDependency;
		std::atomic<u32> mNumRemainingTasks{ mCount };

		TaskScheduler* mParent = nullptr;
	};

	/**
	 * Represents a task scheduler running on multiple threads. You may queue tasks on it from any thread and they will be
	 * executed in user specified order on any available thread.
	 *
	 * @note
	 * Thread safe.
	 * @note
	 * This type of task scheduler uses a global queue and is best used for coarse granularity of tasks. (Number of tasks
	 * in the order of hundreds. Higher number of tasks might require different queuing and locking mechanism, potentially
	 * at the cost of flexibility.)
	 * @note
	 * By default the task scheduler will create as many threads as there are physical CPU cores. You may add or remove
	 * threads using addWorker()/removeWorker() methods.
	 */
	class B3D_UTILITY_EXPORT TaskScheduler : public Module<TaskScheduler>
	{
	public:
		TaskScheduler();
		~TaskScheduler();

		/** Queues a new task. */
		void AddTask(SPtr<Task> task);

		/** Queues a new task group. */
		void AddTaskGroup(const SPtr<TaskGroup>& taskGroup);

		/**	Adds a new worker thread which will be used for executing queued tasks. */
		void AddWorker();

		/**	Removes a worker thread (as soon as its current task is finished). */
		void RemoveWorker();

		/** Returns the maximum available worker threads (maximum number of tasks that can be executed simultaneously). */
		u32 GetNumWorkers() const { return mMaxActiveTasks; }

	protected:
		friend class Task;
		friend class TaskGroup;

		/**	Main task scheduler method that dispatches tasks to other threads. */
		void RunMain();

		/**	Worker method that runs a single task. */
		void RunTask(SPtr<Task> task);

		/**	Blocks the calling thread until the specified task has completed. */
		void WaitUntilComplete(const Task* task);

		/**	Blocks the calling thread until all the tasks in the provided task group have completed. */
		void WaitUntilComplete(const TaskGroup* taskGroup);

		/**	Method used for sorting tasks. */
		static bool TaskCompare(const SPtr<Task>& lhs, const SPtr<Task>& rhs);

		SPtr<PooledThread> mTaskSchedulerThread;
		Set<SPtr<Task>, std::function<bool(const SPtr<Task>&, const SPtr<Task>&)>> mTaskQueue;
		Vector<SPtr<Task>> mActiveTasks;
		u32 mMaxActiveTasks = 0;
		u32 mNextTaskId = 0;
		bool mShutdown = false;
		bool mCheckTasks = false;

		Mutex mReadyMutex;
		Mutex mCompleteMutex;
		ConditionVariable mTaskReadyCond;
		ConditionVariable mTaskCompleteCond;
	};

	/** @} */
} // namespace bs
