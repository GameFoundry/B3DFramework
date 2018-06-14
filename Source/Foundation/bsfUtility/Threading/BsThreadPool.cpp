//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/BsThreadPool.h"
#include "Debug/BsDebug.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "windows.h"

#if BS_COMPILER == BS_COMPILER_MSVC
// disable: nonstandard extension used: 'X' uses SEH and 'Y' has destructor
// We don't care about this as any exception is meant to crash the program.
#pragma warning(disable: 4509)
#endif // BS_COMPILER == BS_COMPILER_MSVC

#endif // BS_PLATFORM == BS_PLATFORM_WIN32

namespace bs
{
	/** The thread pool will check for unused threads every UNUSED_CHECK_PERIOD getThread() calls*/
	static constexpr int UNUSED_CHECK_PERIOD = 32;

	HThread::HThread(ThreadPool* pool, UINT32 threadId)
		:mThreadId(threadId), mPool(pool)
	{ }

	void HThread::blockUntilComplete()
	{
		PooledThread* parentThread = nullptr;

		{
			Lock lock(mPool->mMutex);

			for (auto& thread : mPool->mThreads)
			{
				if (thread->getId() == mThreadId)
				{
					parentThread = thread;
					break;
				}
			}
		}

		if (parentThread != nullptr)
		{
			Lock lock(parentThread->mMutex);

			if (parentThread->mId == mThreadId) // Check again in case it changed
			{
				while (!parentThread->mIdle)
					parentThread->mWorkerEndedCond.wait(lock);
			}
		}
	}

	void PooledThread::initialize()
	{
		mThread = bs_new<Thread>(std::bind(&PooledThread::run, this));

		Lock lock(mMutex);

		while(!mThreadStarted)
			mStartedCond.wait(lock);
	}

	void PooledThread::start(std::function<void()> workerMethod, UINT32 id)
	{
		{
			Lock lock(mMutex);

			mWorkerMethod = workerMethod;
			mIdle = false;
			mIdleTime = std::time(nullptr);
			mThreadReady = true;
			mId = id;
		}

		mReadyCond.notify_one();
	}

	void PooledThread::run()
	{
		onThreadStarted(mName);

		{
			Lock lock(mMutex);
			mThreadStarted = true;
		}

		mStartedCond.notify_one();

		while(true)
		{
			std::function<void()> worker = nullptr;

			{
				{
					Lock lock(mMutex);

					while (!mThreadReady)
						mReadyCond.wait(lock);

					worker = mWorkerMethod;
				}

				if (worker == nullptr)
				{
					onThreadEnded(mName);
					return;
				}
			}

#if BS_PLATFORM == BS_PLATFORM_WIN32
			__try
			{
				worker();
			}
			__except (gCrashHandler().reportCrash(GetExceptionInformation()))
			{
				PlatformUtility::terminate(true);
			}
#else
			worker();
#endif

			{
				Lock lock(mMutex);

				mIdle = true;
				mIdleTime = std::time(nullptr);
				mThreadReady = false;
				mWorkerMethod = nullptr; // Make sure to clear as it could have bound shared pointers and similar

				mWorkerEndedCond.notify_one();
			}
		}
	}

	void PooledThread::destroy()
	{
		blockUntilComplete();

		{
			Lock lock(mMutex);
			mWorkerMethod = nullptr;
			mThreadReady = true;
		}

		mReadyCond.notify_one();
		mThread->join();
		bs_delete(mThread);
	}

	void PooledThread::blockUntilComplete()
	{
		Lock lock(mMutex);

		while (!mIdle)
			mWorkerEndedCond.wait(lock);
	}

	bool PooledThread::isIdle()
	{
		Lock lock(mMutex);

		return mIdle;
	}

	time_t PooledThread::idleTime()
	{
		Lock lock(mMutex);

		return (time(nullptr) - mIdleTime);
	}

	void PooledThread::setName(const String& name)
	{
		mName = name;
	}

	UINT32 PooledThread::getId() const
	{
		Lock lock(mMutex);

		return mId;
	}

	ThreadPool::ThreadPool(UINT32 threadCapacity, UINT32 maxCapacity, UINT32 idleTimeout)
		:mDefaultCapacity(threadCapacity), mMaxCapacity(maxCapacity), mIdleTimeout(idleTimeout)
	{

	}

	ThreadPool::~ThreadPool()
	{
		stopAll();
	}

	HThread ThreadPool::run(const String& name, std::function<void()> workerMethod)
	{
		PooledThread* thread = getThread(name);
		thread->start(workerMethod, mUniqueId++);

		return HThread(this, thread->getId());
	}

	void ThreadPool::stopAll()
	{
		Lock lock(mMutex);
		for(auto& thread : mThreads)
		{
			destroyThread(thread);
		}

		mThreads.clear();
	}

	void ThreadPool::clearUnused()
	{
		Lock lock(mMutex);
		mAge = 0;

		if(mThreads.size() <= mDefaultCapacity)
			return;

		Vector<PooledThread*> idleThreads;
		Vector<PooledThread*> expiredThreads;
		Vector<PooledThread*> activeThreads;

		idleThreads.reserve(mThreads.size());
		expiredThreads.reserve(mThreads.size());
		activeThreads.reserve(mThreads.size());

		for(auto& thread : mThreads)
		{
			if(thread->isIdle())
			{
				if(thread->idleTime() >= mIdleTimeout)
					expiredThreads.push_back(thread);
				else
					idleThreads.push_back(thread);
			}
			else
				activeThreads.push_back(thread);
		}

		idleThreads.insert(idleThreads.end(), expiredThreads.begin(), expiredThreads.end());
		UINT32 limit = std::min((UINT32)idleThreads.size(), mDefaultCapacity);

		UINT32 i = 0;
		mThreads.clear();

		for(auto& thread : idleThreads)
		{
			if (i < limit)
			{
				mThreads.push_back(thread);
				i++;
			}
			else
				destroyThread(thread);
		}

		mThreads.insert(mThreads.end(), activeThreads.begin(), activeThreads.end());
	}

	void ThreadPool::destroyThread(PooledThread* thread)
	{
		thread->destroy();
		bs_delete(thread);
	}

	PooledThread* ThreadPool::getThread(const String& name)
	{
		UINT32 age = 0;
		{
			Lock lock(mMutex);
			age = ++mAge;
		}

		if(age == UNUSED_CHECK_PERIOD)
			clearUnused();

		Lock lock(mMutex);

		for(auto& thread : mThreads)
		{
			if(thread->isIdle())
			{
				thread->setName(name);
				return thread;
			}
		}

		if(mThreads.size() >= mMaxCapacity)
			BS_EXCEPT(InvalidStateException, "Unable to create a new thread in the pool because maximum capacity has been reached.");

		PooledThread* newThread = createThread(name);
		mThreads.push_back(newThread);

		return newThread;
	}

	UINT32 ThreadPool::getNumAvailable() const
	{
		UINT32 numAvailable = 0;

		Lock lock(mMutex);
		for(auto& thread : mThreads)
		{
			if(thread->isIdle())
				numAvailable++;
		}

		return numAvailable;
	}

	UINT32 ThreadPool::getNumActive() const
	{
		UINT32 numActive = 0;

		Lock lock(mMutex);
		for(auto& thread : mThreads)
		{
			if(!thread->isIdle())
				numActive++;
		}

		return numActive;
	}

	UINT32 ThreadPool::getNumAllocated() const
	{
		Lock lock(mMutex);

		return (UINT32)mThreads.size();
	}
}
