//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Threading
	 *  @{
	 */

	class Thread;
	class ThreadPool;

	/**	Wrapper around a thread that is used within ThreadPool. */
	class B3D_EXPORT PooledThread
	{
	public:
		PooledThread(const String& name);
		virtual ~PooledThread() = default;

		/** Initializes the pooled thread. Must be called right after construction. */
		void Initialize();

		/** Returns the underlying thread. */
		Thread& GetThread() const { return *mThread; }

		/**
		 * Starts executing the given worker method.
		 *
		 * @note
		 * Caller must ensure worker method is not null and that the thread is currently idle, otherwise undefined behavior
		 * will occur.
		 */
		void Start(std::function<void()> workerMethod);

		/**
		 * Attempts to join the currently running thread and destroys it. Caller must ensure that any worker method
		 * currently running properly returns, otherwise this will block indefinitely.
		 */
		void Destroy();

		/**	Returns true if the thread is idle and new worker method can be scheduled on it. */
		bool IsIdle();

		/** Returns how long has the thread been idle. Value is undefined if thread is not idle. */
		time_t IdleTime();

		/**	Sets a name of the thread. */
		void SetName(const String& name);

		/**	Blocks the current thread until this thread completes. Returns immediately if the thread is idle. */
		void BlockUntilComplete();

		/**	Called when the thread is first created. */
		virtual void OnThreadStarted(const String& name) = 0;

		/**	Called when the thread is being shut down. */
		virtual void OnThreadEnded(const String& name) = 0;

	protected:
		friend class HThread;

		/** Primary worker method that is ran when the thread is first initialized. */
		void Run();

#if B3D_PLATFORM_WIN32
		void RunFunctionHelper(const std::function<void()>& function) const;
#endif

	protected:
		std::function<void()> mWorkerMethod;
		String mName;
		bool mIsThreadIdle = true;
		bool mIsThreadStarted = false;
		bool mIsThreadReady = false;

		time_t mIdleTime = 0;

		Thread* mThread;
		mutable Mutex mMutex;
		ConditionVariable mThreadStartedSignal;
		ConditionVariable mThreadReadySignal;
		ConditionVariable mWorkerFinishedSignal;
	};

	/**
	 * @copydoc	PooledThread
	 *
	 * @tparam	ThreadPolicy Allows you specify a policy with methods that will get called whenever a new thread is created
	 *		or when a thread is destroyed.
	 */
	template <class ThreadPolicy>
	class TPooledThread : public PooledThread
	{
	public:
		using PooledThread::PooledThread;

		void OnThreadStarted(const String& name) override
		{
			ThreadPolicy::OnThreadStarted(name);
		}

		void OnThreadEnded(const String& name) override
		{
			ThreadPolicy::OnThreadEnded(name);
		}
	};

	/**
	 * Class that maintains a pool of threads we can easily retrieve and use for any task. This saves on the cost of
	 * creating and destroying threads.
	 */
	class B3D_EXPORT ThreadPool : public Module<ThreadPool>
	{
	public:
		/**
		 * Constructs a new thread pool
		 *
		 * @param	threadCapacity	Default thread capacity, the pool will always try to keep this many threads available.
		 * @param	idleTimeout   	(optional) How many seconds do threads need to be idle before we remove them from the pool.
		 */
		ThreadPool(u32 threadCapacity, u32 idleTimeout = 60);
		virtual ~ThreadPool();

		/**
		 * Find an unused thread (or creates a new one) and runs the specified worker method on it.
		 *
		 * @param	name			A name you may use for more easily identifying the thread.
		 * @param	workerMethod	The worker method to be called by the thread.
		 * @return					A thread handle you may use for monitoring the thread execution.
		 */
		SPtr<PooledThread> Run(const String& name, std::function<void()> workerMethod);

		/**
		 * Stops all threads and destroys them. Caller must ensure each threads worker method returns otherwise this will
		 * never return.
		 */
		void StopAll();

		/** Clear any unused threads that are over the capacity. */
		void ClearUnused();

		/**	Returns the number of running threads in the pool. */
		u32 GetNumActive() const;

		/**	Returns the total number of created threads in the pool	(both running and unused). */
		u32 GetNumAllocated() const;

	protected:
		friend class HThread;

		Vector<SPtr<PooledThread>> mThreads;

		/**	Creates a new thread to be used by the pool. */
		virtual SPtr<PooledThread> CreateThread(const String& name) = 0;

		/**
		 * Returns the first unused thread if one exists, otherwise creates a new one.
		 *
		 * @param	name	Name to assign the thread.
		 *
		 * @note	Throws an exception if we have reached our maximum thread capacity.
		 */
		SPtr<PooledThread> GetThread(const String& name);

		u32 mDefaultCapacity;
		u32 mIdleTimeout;
		/** unused check counter */
		u32 mAge = 0;

		mutable Mutex mMutex;
	};

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Threading-Internal
	 *  @{
	 */

	/** Policy used for thread start & end used by the ThreadPool. */
	class ThreadNoPolicy
	{
	public:
		static void OnThreadStarted(const String& name) {}

		static void OnThreadEnded(const String& name) {}
	};

	/**
	 * @copydoc ThreadPool
	 *
	 * @tparam	ThreadPolicy Allows you specify a policy with methods that will get called whenever a new thread is created
	 *		or when a thread is destroyed.
	 */
	template <class ThreadPolicy = ThreadNoPolicy>
	class TThreadPool : public ThreadPool
	{
	public:
		TThreadPool(u32 threadCapacity, u32 idleTimeout = 60)
			: ThreadPool(threadCapacity, idleTimeout)
		{
		}

	protected:
		SPtr<PooledThread> CreateThread(const String& name) override
		{
			SPtr<PooledThread> output(B3DNew<TPooledThread<ThreadPolicy>>(name), [](PooledThread* pooledThread)
				{
					pooledThread->Destroy();
					B3DDelete(pooledThread);
				});
			output->Initialize();

			return output;
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
