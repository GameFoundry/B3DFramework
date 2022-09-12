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

	void HThread::BlockUntilComplete()
	{
		PooledThread* parentThread = nullptr;

		{
			Lock Lock(mPool->mMutex);

			for (auto& thread : mPool->mThreads)
			{
				if (thread->GetId() == mThreadId)
				{
					parentThread = thread;
					break;
				}
			}
		}

		if (parentThread != nullptr)
		{
			Lock Lock(parentThread->mMutex);

			if (parentThread->mId == mThreadId) // Check again in case it changed
			{
				while (!parentThread->mIdle)
					parentThread->mWorkerEndedCond.Wait(lock);
			}
		}
	}

	void PooledThread::Initialize()
	{
		mThread = bs_new<Thread>(std::bind(&PooledThread::run, this));

		Lock Lock(mMutex);

		while(!mThreadStarted)
			mStartedCond.Wait(lock);
	}

	void PooledThread::Start(std::function<void()> workerMethod, UINT32 id)
	{
		{
			Lock Lock(mMutex);

			mWorkerMethod = workerMethod;
			mIdle = false;
			mIdleTime = std::time(nullptr);
			mThreadReady = true;
			mId = id;
		}

		mReadyCond.notify_one();
	}

	void PooledThread::Run()
	{
		onThreadStarted(mName);

		{
			Lock Lock(mMutex);
			mThreadStarted = true;
		}

		mStartedCond.notify_one();

		while(true)
		{
			std::function<void()> worker = nullptr;

			{
				{
					Lock Lock(mMutex);

					while (!mThreadReady)
						mReadyCond.Wait(lock);

					worker = mWorkerMethod;
				}

				if (worker == nullptr)
				{
					onThreadEnded(mName);
					return;
				}
			}

#if BS_PLATFORM == BS_PLATFORM_WIN32
			runFunctionHelper(worker);
#else
			worker();
#endif

			{
				Lock Lock(mMutex);

				mIdle = true;
				mIdleTime = std::time(nullptr);
				mThreadReady = false;
				mWorkerMethod = nullptr; // Make sure to clear as it could have bound shared pointers and similar

				mWorkerEndedCond.notify_one();
			}
		}
	}

#if BS_PLATFORM == BS_PLATFORM_WIN32
	void PooledThread::RunFunctionHelper(const std::function<void()>& function) const
	{
		__try {
			function();
		} __except(gCrashHandler().ReportCrash(GetExceptionInformation())) {
			PlatformUtility::terminate(true);
		}
	}
#endif

	void PooledThread::Destroy()
	{
		blockUntilComplete();

		{
			Lock Lock(mMutex);
			mWorkerMethod = nullptr;
			mThreadReady = true;
		}

		mReadyCond.notify_one();
		mThread->Join();
		bs_delete(mThread);
	}

	void PooledThread::BlockUntilComplete()
	{
		Lock Lock(mMutex);

		while (!mIdle)
			mWorkerEndedCond.Wait(lock);
	}

	bool PooledThread::IsIdle()
	{
		Lock Lock(mMutex);

		return mIdle;
	}

	time_t PooledThread::IdleTime()
	{
		Lock Lock(mMutex);

		return (time(nullptr) - mIdleTime);
	}

	void PooledThread::SetName(const String& name)
	{
		mName = name;
	}

	UINT32 PooledThread::GetId() const
	{
		Lock Lock(mMutex);

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

	HThread ThreadPool::Run(const String& name, std::function<void()> workerMethod)
	{
		PooledThread* thread = getThread(name);
		thread->Start(workerMethod, mUniqueId++);

		return HThread(this, thread->GetId());
	}

	void ThreadPool::StopAll()
	{
		Lock Lock(mMutex);
		for(auto& thread : mThreads)
		{
			destroyThread(thread);
		}

		mThreads.Clear();
	}

	void ThreadPool::ClearUnused()
	{
		Lock Lock(mMutex);
		mAge = 0;

		if(mThreads.Size() <= mDefaultCapacity)
			return;

		Vector<PooledThread*> idleThreads;
		Vector<PooledThread*> expiredThreads;
		Vector<PooledThread*> activeThreads;

		idleThreads.Reserve(mThreads.size());
		expiredThreads.Reserve(mThreads.size());
		activeThreads.Reserve(mThreads.size());

		for(auto& thread : mThreads)
		{
			if(thread->IsIdle())
			{
				if(thread->IdleTime() >= mIdleTimeout)
					expiredThreads.push_back(thread);
				else
					idleThreads.push_back(thread);
			}
			else
				activeThreads.push_back(thread);
		}

		idleThreads.Insert(idleThreads.end(), expiredThreads.begin(), expiredThreads.end());
		UINT32 limit = std::min((UINT32)idleThreads.Size(), mDefaultCapacity);

		UINT32 i = 0;
		mThreads.Clear();

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

		mThreads.Insert(mThreads.end(), activeThreads.begin(), activeThreads.end());
	}

	void ThreadPool::DestroyThread(PooledThread* thread)
	{
		thread->Destroy();
		bs_delete(thread);
	}

	PooledThread* ThreadPool::getThread(const String& name)
	{
		UINT32 age = 0;
		{
			Lock Lock(mMutex);
			age = ++mAge;
		}

		if(age == UNUSED_CHECK_PERIOD)
			clearUnused();

		Lock Lock(mMutex);

		for(auto& thread : mThreads)
		{
			if(thread->IsIdle())
			{
				thread->SetName(name);
				return thread;
			}
		}

		if(mThreads.Size() >= mMaxCapacity)
			BS_EXCEPT(InvalidStateException, "Unable to create a new thread in the pool because maximum capacity has been reached.");

		PooledThread* newThread = createThread(name);
		mThreads.push_back(newThread);

		return newThread;
	}

	UINT32 ThreadPool::GetNumAvailable() const
	{
		UINT32 numAvailable = mMaxCapacity;

		Lock Lock(mMutex);
		for(auto& thread : mThreads)
		{
			if(!thread->IsIdle())
				numAvailable--;
		}

		return numAvailable;
	}

	UINT32 ThreadPool::GetNumActive() const
	{
		UINT32 numActive = 0;

		Lock Lock(mMutex);
		for(auto& thread : mThreads)
		{
			if(!thread->IsIdle())
				numActive++;
		}

		return numActive;
	}

	UINT32 ThreadPool::GetNumAllocated() const
	{
		Lock Lock(mMutex);

		return (UINT32)mThreads.Size();
	}
}
