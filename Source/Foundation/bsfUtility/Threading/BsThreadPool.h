//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Threading
	 *  @{
	 */

	class ThreadPool;

	/** Handle to a thread managed by ThreadPool. */
	class B3D_UTILITY_EXPORT HThread
	{
	public:
		HThread() = default;
		;
		HThread(ThreadPool* pool, u32 threadId);

		/**	Block the calling thread until the thread this handle points to completes. */
		void BlockUntilComplete();

	private:
		u32 mThreadId = 0;
		ThreadPool* mPool = nullptr;
	};

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Threading-Internal
	 *  @{
	 */

	/**	Wrapper around a thread that is used within ThreadPool. */
	class B3D_UTILITY_EXPORT PooledThread
	{
	public:
		PooledThread(const String& name)
			: mName(name)
		{}

		virtual ~PooledThread() = default;

		/**	Initializes the pooled thread. Must be called right after the object is constructed. */
		void Initialize();

		/**
		 * Starts executing the given worker method.
		 *
		 * @note
		 * Caller must ensure worker method is not null and that the thread is currently idle, otherwise undefined behavior
		 * will occur.
		 */
		void Start(std::function<void()> workerMethod, u32 id);

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

		/**	Gets unique ID of the currently executing thread. */
		u32 GetId() const;

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

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
		void RunFunctionHelper(const std::function<void()>& function) const;
#endif

	protected:
		std::function<void()> mWorkerMethod;
		String mName;
		u32 mId = 0;
		bool mIdle = true;
		bool mThreadStarted = false;
		bool mThreadReady = false;

		time_t mIdleTime = 0;

		Thread* mThread;
		mutable Mutex mMutex;
		Signal mStartedCond;
		Signal mReadyCond;
		Signal mWorkerEndedCond;
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

	/** @} */
	/** @} */

	/** @addtogroup Threading
	 *  @{
	 */

	/**
	 * Class that maintains a pool of threads we can easily retrieve and use for any task. This saves on the cost of
	 * creating and destroying threads.
	 */
	class B3D_UTILITY_EXPORT ThreadPool : public Module<ThreadPool>
	{
	public:
		/**
		 * Constructs a new thread pool
		 *
		 * @param[in]	threadCapacity	Default thread capacity, the pool will always try to keep this many threads available.
		 * @param[in]	maxCapacity   	(optional) Maximum number of threads the pool can create. If we go over this limit an
		 *								exception will be thrown.
		 * @param[in]	idleTimeout   	(optional) How many seconds do threads need to be idle before we remove them from the pool.
		 */
		ThreadPool(u32 threadCapacity, u32 maxCapacity = 16, u32 idleTimeout = 60);
		virtual ~ThreadPool();

		/**
		 * Find an unused thread (or creates a new one) and runs the specified worker method on it.
		 *
		 * @param[in]	name			A name you may use for more easily identifying the thread.
		 * @param[in]	workerMethod	The worker method to be called by the thread.
		 * @return						A thread handle you may use for monitoring the thread execution.
		 */
		HThread Run(const String& name, std::function<void()> workerMethod);

		/**
		 * Stops all threads and destroys them. Caller must ensure each threads worker method returns otherwise this will
		 * never return.
		 */
		void StopAll();

		/** Clear any unused threads that are over the capacity. */
		void ClearUnused();

		/**	Returns the number of unused threads in the pool. */
		u32 GetNumAvailable() const;

		/**	Returns the number of running threads in the pool. */
		u32 GetNumActive() const;

		/**	Returns the total number of created threads in the pool	(both running and unused). */
		u32 GetNumAllocated() const;

	protected:
		friend class HThread;

		Vector<PooledThread*> mThreads;

		/**	Creates a new thread to be used by the pool. */
		virtual PooledThread* CreateThread(const String& name) = 0;

		/**	Destroys the specified thread. Caller needs to make sure the thread is actually shut down beforehand. */
		void DestroyThread(PooledThread* thread);

		/**
		 * Returns the first unused thread if one exists, otherwise creates a new one.
		 *
		 * @param[in]	name	Name to assign the thread.
		 *
		 * @note	Throws an exception if we have reached our maximum thread capacity.
		 */
		PooledThread* GetThread(const String& name);

		u32 mDefaultCapacity;
		u32 mMaxCapacity;
		u32 mIdleTimeout;
		/** unused check counter */
		u32 mAge = 0;

		std::atomic_uint mUniqueId;
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
		TThreadPool(u32 threadCapacity, u32 maxCapacity = 16, u32 idleTimeout = 60)
			: ThreadPool(threadCapacity, maxCapacity, idleTimeout)
		{
		}

	protected:
		PooledThread* CreateThread(const String& name) override
		{
			PooledThread* newThread = B3DNew<TPooledThread<ThreadPolicy>>(name);
			newThread->Initialize();

			return newThread;
		}
	};

	/** @} */
	/** @} */
} // namespace bs
