//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DThread.h"
#include "Math/B3DRandom.h"

namespace marl
{
	class OSFiber;
}

namespace b3d
{
	class SingleConsumerQueue;
	/** @addtogroup Threading
	 *  @{
	 */

	class SchedulerThread;

	/** Flags that control the behaviour of a SchedulerTask. */
	enum class SchedulerTaskFlag
	{
		None = 0,
		SameThread = 1 << 0, /**< Ensures the task runs on the same thread it was queued on. */
		NoStealing = 1 << 1, /**< Ensures the task cannot be stolen by another thread. Always true if SameThread flag is provided. */
	};

	using SchedulerTaskFlags = Flags<SchedulerTaskFlag>;
	B3D_FLAGS_OPERATORS(SchedulerTaskFlag);

	/** Represents a function to execute using the Scheduler. */
	class SchedulerTask
	{
	public:
		SchedulerTask() = default;
		SchedulerTask(const SchedulerTask& other): mWorkerFunction(other.mWorkerFunction), mFlags(other.mFlags) { }
		SchedulerTask(SchedulerTask&& other): mWorkerFunction(std::move(other.mWorkerFunction)), mFlags(other.mFlags) { }
		SchedulerTask(const Function<void()>& workerFunction, const char* name, SchedulerTaskFlags flags = SchedulerTaskFlag::None, const String& extraInformation = StringUtil::kBlank)
			: mName(name), mWorkerFunction(workerFunction), mFlags(flags), mExtraInformation(extraInformation)
		{ }

		SchedulerTask(Function<void()>&& workerFunction, SchedulerTaskFlags flags = SchedulerTaskFlag::None)
			: mWorkerFunction(std::move(workerFunction)), mFlags(flags)
		{ }

		SchedulerTask& operator=(const SchedulerTask& other)
		{
			mWorkerFunction = other.mWorkerFunction;
			mFlags = other.mFlags;

			return *this;
		}

		SchedulerTask& operator=(SchedulerTask&& other)
		{
			if (this == &other)
				return *this;

			std::exchange(mWorkerFunction, other.mWorkerFunction);
			std::exchange(mFlags, other.mFlags);

			return *this;
		}

		SchedulerTask& operator=(const Function<void()>& workerFunction)
		{
			mWorkerFunction = workerFunction;
			mFlags = SchedulerTaskFlag::None;

			return *this;
		}

		SchedulerTask operator=(Function<void()>&& workerFunction)
		{
			mWorkerFunction = std::move(workerFunction);
			mFlags = SchedulerTaskFlag::None;

			return *this;
		}

		/** Returns true if a valid function is assigned. */
		operator bool() const { return mWorkerFunction != nullptr; }

		/** Calls the task worker function. */
		void operator()() const { mWorkerFunction(); }

		/** Returns the flags the task was created with. */
		SchedulerTaskFlags GetFlags() const { return mFlags; }

	private:
		const char* mName = nullptr;
		String mExtraInformation;
		Function<void()> mWorkerFunction;
		SchedulerTaskFlags mFlags = SchedulerTaskFlag::None;
	};

	/**
	 * Fibers are light-weight alternatives to threads that allow for cooperative multitasking. Instead of blocking execution
	 * fibers can yield and let the other work continue on the same thread - once unblocked the fiber's context is restored
	 * and execution will continue from the point it yielded. To allow for this behaviour internally the fibers maintain their
	 * own stack and registers.
	 */
	class B3D_EXPORT Fiber
	{
	public:
		Fiber(UPtr<marl::OSFiber>&& osFiber, u32 id);

		/** Returns the thread that the fiber is running on. */
		const SchedulerThread& GetSchedulerThread() const { return *mOwningThread; }

		/**
		 * Yields execution of the current fiber and continues yielding until the provided predicate returns true.
		 * Fiber must be explicitly woken up with a call to TryResume() to re-check the predicate, otherwise it will
		 * yield indefinitely. Must only be called on the active fiber for the current thread.
		 *
		 * @param lock			Lock to be held while testing the predicate.
		 * @param predicate		Predicate to check when determining when to resume the fiber. This will be checked when
		 *						TryResume() is called, and if it returns false the fiber will continue to be suspended.
		 */
		void Wait(Lock& lock, const Function<bool()>& predicate);

		/**
		 * Yields execution of the current fiber and continues yielding until the provided predicate returns true, or the provided
		 * time expires. The predicate is only checked when TryResume() is called. Must only be called on the active fiber for the current thread.
		 *
		 * @param lock			Lock to be held while testing the predicate.
		 * @param timeout		Determines how long to wait before resuming the fiber.
		 * @param predicate		Predicate to check when determining when to resume the fiber. This will be checked when
		 *						TryResume() is called, and if it returns false the fiber will continue to be suspended (unless the timeout expired).
		 */
		template <typename Clock, typename Duration>
		bool Wait(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, const Function<bool()>& predicate);

		/**
		 * Yields execution of the current fiber so other tasks may run. Execution will be resumed after TryResume() has been called.
		 * Must only be called on the active fiber for the current thread.
		 *
		 * Note that this call requires no explicit lock, and therefore you cannot guarantee that a Wait() and TryResume() call from different
		 * threads will happen in the correct order. Only use if this ordering is not important.
		 */
		inline void Wait();

		/**
		 * Yields execution of the current fiber so other tasks may run. Execution will be resumed after TryResume() has been called, or
		 * the timeout expired. Must only be called on the active fiber for the current thread.
		 *
		 * Note that this call requires no explicit lock, and therefore you cannot guarantee that a Wait() and TryResume() call from different
		 * threads will happen in the correct order. Only use if this ordering is not important.
		 */
		template <typename Clock, typename Duration>
		bool Wait(const std::chrono::time_point<Clock, Duration>& timeout);

		/**
		 * Attempts to resume the execution of the fiber that was suspended via one of the Wait() methods. If the wait provided a predicate, the
		 * predicate will be re-checked and execution resumed only if the predicate returns true. Otherwise execution is resumed unconditionally.
		 */
		void TryResume();

		/**
		 * Creates a new fiber.
		 *
		 * @param	id				Unique id of the fiber within the current thread.
		 * @param	stackSize		Size of the fiber stack, in bytes.
		 * @param	workerFunction	Function to execute when the fiber gets switched to.
		 * @return					Newly allocated fiber.
		 */
		static UPtr<Fiber> Create(u32 id, u64 stackSize, const std::function<void()>& workerFunction);

		/**
		 * Creates a new fiber using the current thread's context.
		 *
		 * @param	id		Unique id of the fiber within the current thread.
		 * @return			Newly allocated fiber.
		 */
		static UPtr<Fiber> CreateFromCurrentThread(u32 id);

		/** Returns the currently executing fiber, or null if no scheduler is bound to this thread. */
		static Fiber* Get();

		const u32 Id; /**< Unique identifier of the fiber within the thread. */

	private:
		friend class Scheduler;
		friend class SchedulerThread;

		using ToDuration = std::chrono::system_clock::time_point::duration;
		using ToClock = std::chrono::system_clock::time_point::clock;

		/** Describes the state a fiber may be in. */
		enum class State
		{
			Idle, /**< Fiber is not used and is ready to be recycled. */
			Yielded, /**< Fiber is blocked due to a Wait() call with no timeout. */
			Waiting, /**< Fiber is blocked due to a Wait() call with a timeout. */
			Queued, /**< The fiber is queued for execution. */
			Running, /**< The fiber is currently executing. */
		};

		/** Performs a context switch from the current fiber to the provided fiber. This fiber must be the currently executing fiber. */
		void SwitchExecutionTo(Fiber* other);

		const SPtr<marl::OSFiber> mOSFiber;
		SchedulerThread* const mOwningThread;
		State mState = State::Running;
	};

	class Scheduler;

	/** Represents a single worker thread within Scheduler. */
	class SchedulerThread : public std::enable_shared_from_this<SchedulerThread>
	{
		using TimePoint = std::chrono::system_clock::time_point;

		/** Contains all fibers waiting for a timeout. */
		struct WaitingFibers
		{
			/** Returns true if there are any waiting fibers. */
			operator bool() const;

			/** Returns the next fiber that has exceeded its timeout, or null if no such fibers. */
			Fiber* TryPop(const TimePoint& timeout);

			/** Returns the time-point of the first fiber that is to execute next. */
			TimePoint PeekTime() const;

			/** Adds a new fiber to the list of the waiting fibers. */
			void Add(const TimePoint& timeout, Fiber* fiber);

			/** Removes a fiber from the waiting fiber list. */
			void Erase(Fiber* fiber);

			/** Returns true if the waiting fiber list contains the provided fiber. */
			bool Contains(Fiber* fiber) const;

		private:
			/** Contains information about a waiting fiber. */
			struct WaitingFiber
			{
				TimePoint Timepoint; /**< Time that the fiber is waiting for before continuing execution. */
				Fiber* Fiber; /**< Fiber that's waiting. */

				inline bool operator<(const WaitingFiber&) const;
			};

			Set<WaitingFiber, std::less<WaitingFiber>> mOrderedFibers;
			UnorderedMap<Fiber*, TimePoint> mFiberLookup;
		};

	public:
		/** Modes that the scheduler thread can be in. */
		enum class Mode
		{
			Internal, /**< Thread is a background thread spawned by the scheduler. */
			External, /**< Thread is managed by the user and bound to the scheduler. */
		};

		B3D_EXPORT SchedulerThread(Scheduler* scheduler, Mode mode, u32 id);

		/** Returns the underlying thread object. */
		B3D_EXPORT const Thread& GetThread() const { return mThread; }

		/** Returns a message queue that may be used for posting messages to this thread. */
		B3D_EXPORT SingleConsumerQueue& GetMessageQueue() const { return *mMessageQueue; }

		/** Queues a new task for execution on this thread. */
		B3D_EXPORT void Post(SchedulerTask&& task);

		/** Returns the fiber currently being executed. */
		Fiber* GetCurrentFiber() const { return mCurrentFiber; }

		const u32 Id; /**< Unique identifier of the scheduler thread. */

		/** Returns the scheduler thread bound to the current thread. */
		B3D_EXPORT static const SPtr<SchedulerThread>& Get() { return Current; }

	private:
		friend class Scheduler;
		friend class Fiber;

		/** Starts execution of the thread. Must be called before enqueuing any work. */
		void Start();

		/** Stops execution of the thread. This will block until all pending tasks have finished. Must be called before shutdown. */
		void Stop();

		/** Suspends execution of the current task until the task is woken up via a call to Enqueue() or timeout expires. See Fiber::Wait() overloads for more information. */
		bool B3D_EXPORT Wait(Lock& lock, const TimePoint* timeout, const Function<bool()>& predicate);

		/** Suspends execution of the current task until the task is woken up via a call to Enqueue() or timeout expires. See Fiber::Wait() overloads for more information. */
		bool B3D_EXPORT Wait(const TimePoint* timeout);

		/** Suspends execution of the current task until the task is woken up via a call to Enqueue() or timeout expires. See Fiber::Wait() overloads for more information. Mutex must be locked when this is called. */
		void WaitWithoutLocking(const TimePoint* timeout);

		/** Enqueues a suspended fiber to continue execution. */
		void Enqueue(Fiber* fiber);

		/** Enqueues a new unstarted task. Only safe to use if mode is SingleThreaded. */
		void Enqueue(SchedulerTask&& task);

		/** Attempts to lock the object for task enqueuing. Returns true if the lock was successful, after which caller must call EnqueueAndUnlock. You should use this instead of Enqueue() if the mode is MultiThreaded. */
		bool TryLockForEnqueue();

		/** Enqueues a task and unlocks the lock acquired via TryLock(). */
		void EnqueueAndUnlock(SchedulerTask&& task);

		/** Processes all tasks and fibers until there are no more and shutdown flag is set. */
		void RunUntilShutdown();

		/** Attempts to steal a Task from another worker. Returns true if a task was successfully stolen. */
		bool TryStealTask(SchedulerTask& outTask);

		/** Processes all tasks until Stop() is called. */
		void Run();

		/** Creates a new fiber that calls Run(). */
		Fiber* CreateWorkerFiber();

		/** Switches execution to the provided fiber. */
		void SwitchExecutionToFiber(Fiber* other);

		/** Executes all pending tasks and returns. */
		void RunUntilIdle();

		/** Waits until some work is available, first spinning and then potentially yielding the thread to the OS. */
		void WaitForWork();

		/** Attempts to steal work from another Worker, and keeps the thread awake for a short duration, hoping to find some work before we need to yield to OS. */
		void SpinForWork();

		/** Enqueues any fibers that have finished waiting. */
		void UpdateWaitingFibers();

		/** Waits until the mAddedSignal is notified and predicate returns true. */
		void WaitOnAddedSignal(const Function<bool()>& predicate);

		B3D_HIDDEN static thread_local SPtr<SchedulerThread> Current;

		const Mode mMode;
		Scheduler* const mOwnerScheduler;

		UPtr<Fiber> mMainFiber;
		Fiber* mCurrentFiber = nullptr;

		Thread mThread;
		Random mRandomNumberGenerator;
		SingleConsumerQueue* mMessageQueue = nullptr;

		std::atomic<u64> mReadyOperationCount = { 0 };
		u64 mBlockedFiberCount = 0;
		Deque<SchedulerTask> mPendingTasks;
		Deque<Fiber*> mReadyFibers;
		WaitingFibers mWaitingFibers;

		bool mTriggerNotifyOnAdd = true;
		ConditionVariable mAddedSignal;
		Mutex mMutex;

		UnorderedSet<Fiber*> mFreeFibers;
		TInlineArray<UPtr<Fiber>, 16> mAllFibers;
		bool mIsShutdownRequested = false;
	};
	
	template <typename Clock, typename Duration>
	bool Fiber::Wait(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, const Function<bool()>& predicate)
	{
		auto timePoint = std::chrono::time_point_cast<ToDuration, ToClock>(timeout);

		return mOwningThread->Wait(lock, &timePoint, predicate);
	}

	void Fiber::Wait()
	{
		mOwningThread->Wait(nullptr);
	}

	template <typename Clock, typename Duration>
	bool Fiber::Wait(const std::chrono::time_point<Clock, Duration>& timeout)
	{
		auto timePoint = std::chrono::time_point_cast<ToDuration, ToClock>(timeout);
		return mOwningThread->Wait(&timePoint);
	}

	/** Information describing a Scheduler. */
	struct SchedulerInformation
	{
		/**
		 * Number of worker threads to start internally by the scheduler.
		 * Set to 0 if you only want to bind external threads via BindToCurrentThread().
		 */
		u32 InternalWorkerThreadCount = 0;

		/** Callback to execute when a worker thread is started (both internal and external). */
		Function<void(u32 workerId)> ThreadInitializeCallback;

		/** Determines on which cores the internally created threads are allowed to execute on. */
		SPtr<ThreadAffinityPolicy> AffinityPolicy;

		/** Stack size for a single fiber, in bytes. */
		u64 FiberStackSize = 1024 * 1024;
	};

	/** Information used for creating a new Scheduler. */
	struct SchedulerCreateInformation : SchedulerInformation
	{
		SchedulerCreateInformation() = default;
		SchedulerCreateInformation(const SchedulerInformation& other)
			:SchedulerInformation(other)
		{ }
	};

	/**
	 * Allows the caller to post tasks which will then be executed on one of the threads managed by the scheduler.
	 * The scheduler can create internal worker threads to process tasks, or you can bind existing threads (created externally).
	 *
	 * External threads need to be bound/unbound to the scheduler via BindToCurrentThread/UnbindFromCurrentThread, and you need to
	 * call ProcessTasksOnCurrentThread() regularly for them to actually process queued tasks.
	 *
	 * All executing tasks are allowed to yield mid-execution, at which point a new task will start executing on the thread. The suspended
	 * task can be resumed from the point it yielded as the scheduler will preserve its context in a fiber.
	 */
	class Scheduler final
	{
	public:
		Scheduler(const SchedulerCreateInformation& createInformation);
		~Scheduler();

		/** Returns information describing the scheduler. */
		B3D_EXPORT const SchedulerInformation& GetInformation() const { return mInformation; }

		/**
		 * Binds the scheduler to the current thread, allowing it to process tasks.
		 * The thread is managed externally - you must call UnbindFromCurrentThread() before the thread exits.
		 * Can be called from multiple threads to bind them all to the scheduler.
		 *
		 * @return Worker ID assigned to this thread, or ~0u if binding failed (max threads reached or already bound).
		 */
		B3D_EXPORT u32 BindToCurrentThread();

		/**
		 * Processes all pending tasks on the current bound thread and returns.
		 * Must be called from a thread that has called BindToCurrentThread().
		 * Typically called in a loop from your own thread (e.g., game loop).
		 */
		B3D_EXPORT void ProcessTasksOnCurrentThread();

		/** Unbinds the scheduler currently bound on the calling thread. This will wait until all operations complete before returning. */
		B3D_EXPORT static void UnbindFromCurrentThread();

		/** Queues a new task for execution by the scheduler. The task will be executed on one of the worker threads (either internal or external). */
		B3D_EXPORT void Post(SchedulerTask&& task);

		/** Gets the scheduler bound to the current thread. */
		B3D_EXPORT static Scheduler* Get() { return Current; }

	private:
		friend class SchedulerThread;

		/**
		 * Attempts to steal work from another scheduler thread.
		 *
		 * @param	thief		Thread trying to steal the task.
		 * @param	random		Random value based on which to pick the worker to try to steal from.
		 * @param	outTask		Stolen task, if successful.
		 * @return				True if a task was stolen, false otherwise.
		 */
		bool TryStealWork(SchedulerThread* thief, u32 random, SchedulerTask& outTask);

		/** Notifies the scheduler that a scheduler thread has begun spinning. This allows the scheduler to prioritize work on this worker. */
		void NotifyOnBeginSpinning(u32 workerId);

		B3D_HIDDEN static thread_local Scheduler* Current;

		std::array<std::atomic<u32>, 8> mSpinningWorkers;
		std::atomic<u32> mNextSpinningWorkerIndex = { 0x8000000 };

		std::atomic<u32> mNextEnqueueIndex = { 0 };

		// Unified thread management - both internal and external threads
		Mutex mWorkerThreadsMutex;
		TArray<SPtr<SchedulerThread>> mWorkerThreads;
		TArray<u32> mInternalWorkerIndices;
		std::atomic<u32> mNextExternalWorkerId;

		// Synchronization for external thread unbinding
		ConditionVariable mExternalThreadsUnbindSignal;
		u32 mExternalThreadCount = 0;

		SchedulerInformation mInformation;
	};

	/** @} */
}

