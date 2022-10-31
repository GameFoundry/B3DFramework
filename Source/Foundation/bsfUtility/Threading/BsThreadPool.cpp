//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/BsThreadPool.h"
#include "Debug/BsDebug.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#	include "windows.h"

#	if BS_COMPILER == BS_COMPILER_MSVC
// disable: nonstandard extension used: 'X' uses SEH and 'Y' has destructor
// We don't care about this as any exception is meant to crash the program.
#		pragma warning(disable : 4509)
#	endif // BS_COMPILER == BS_COMPILER_MSVC

#endif // BS_PLATFORM == BS_PLATFORM_WIN32

using namespace bs;

/** The thread pool will check for unused threads every UNUSED_CHECK_PERIOD getThread() calls*/
static constexpr int kUnusedCheckPeriod = 32;

HThread::HThread(ThreadPool* pool, u32 threadId)
	: mThreadId(threadId), mPool(pool)
{}

void HThread::BlockUntilComplete()
{
	PooledThread* parentThread = nullptr;

	{
		Lock lock(mPool->mMutex);

		for(auto& thread : mPool->mThreads)
		{
			if(thread->GetId() == mThreadId)
			{
				parentThread = thread;
				break;
			}
		}
	}

	if(parentThread != nullptr)
	{
		Lock lock(parentThread->mMutex);

		if(parentThread->mId == mThreadId) // Check again in case it changed
		{
			while(!parentThread->mIdle)
				parentThread->mWorkerEndedCond.wait(lock);
		}
	}
}

void PooledThread::Initialize()
{
	mThread = bs_new<Thread>(std::bind(&PooledThread::Run, this));

	Lock lock(mMutex);

	while(!mThreadStarted)
		mStartedCond.wait(lock);
}

void PooledThread::Start(std::function<void()> workerMethod, u32 id)
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

void PooledThread::Run()
{
	OnThreadStarted(mName);

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

				while(!mThreadReady)
					mReadyCond.wait(lock);

				worker = mWorkerMethod;
			}

			if(worker == nullptr)
			{
				OnThreadEnded(mName);
				return;
			}
		}

#if BS_PLATFORM == BS_PLATFORM_WIN32
		RunFunctionHelper(worker);
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

#if BS_PLATFORM == BS_PLATFORM_WIN32
void PooledThread::RunFunctionHelper(const std::function<void()>& function) const
{
	__try
	{
		function();
	}
	__except(GetCrashHandler().ReportCrash(GetExceptionInformation()))
	{
		PlatformUtility::Terminate(true);
	}
}
#endif

void PooledThread::Destroy()
{
	BlockUntilComplete();

	{
		Lock lock(mMutex);
		mWorkerMethod = nullptr;
		mThreadReady = true;
	}

	mReadyCond.notify_one();
	mThread->join();
	bs_delete(mThread);
}

void PooledThread::BlockUntilComplete()
{
	Lock lock(mMutex);

	while(!mIdle)
		mWorkerEndedCond.wait(lock);
}

bool PooledThread::IsIdle()
{
	Lock lock(mMutex);

	return mIdle;
}

time_t PooledThread::IdleTime()
{
	Lock lock(mMutex);

	return (time(nullptr) - mIdleTime);
}

void PooledThread::SetName(const String& name)
{
	mName = name;
}

u32 PooledThread::GetId() const
{
	Lock lock(mMutex);

	return mId;
}

ThreadPool::ThreadPool(u32 threadCapacity, u32 maxCapacity, u32 idleTimeout)
	: mDefaultCapacity(threadCapacity), mMaxCapacity(maxCapacity), mIdleTimeout(idleTimeout)
{
}

ThreadPool::~ThreadPool()
{
	StopAll();
}

HThread ThreadPool::Run(const String& name, std::function<void()> workerMethod)
{
	PooledThread* thread = GetThread(name);
	thread->Start(workerMethod, mUniqueId++);

	return HThread(this, thread->GetId());
}

void ThreadPool::StopAll()
{
	Lock lock(mMutex);
	for(auto& thread : mThreads)
	{
		DestroyThread(thread);
	}

	mThreads.clear();
}

void ThreadPool::ClearUnused()
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

	idleThreads.insert(idleThreads.end(), expiredThreads.begin(), expiredThreads.end());
	u32 limit = std::min((u32)idleThreads.size(), mDefaultCapacity);

	u32 i = 0;
	mThreads.clear();

	for(auto& thread : idleThreads)
	{
		if(i < limit)
		{
			mThreads.push_back(thread);
			i++;
		}
		else
			DestroyThread(thread);
	}

	mThreads.insert(mThreads.end(), activeThreads.begin(), activeThreads.end());
}

void ThreadPool::DestroyThread(PooledThread* thread)
{
	thread->Destroy();
	bs_delete(thread);
}

PooledThread* ThreadPool::GetThread(const String& name)
{
	u32 age = 0;
	{
		Lock lock(mMutex);
		age = ++mAge;
	}

	if(age == kUnusedCheckPeriod)
		ClearUnused();

	Lock lock(mMutex);

	for(auto& thread : mThreads)
	{
		if(thread->IsIdle())
		{
			thread->SetName(name);
			return thread;
		}
	}

	if(mThreads.size() >= mMaxCapacity)
		BS_EXCEPT(InvalidStateException, "Unable to create a new thread in the pool because maximum capacity has been reached.");

	PooledThread* newThread = CreateThread(name);
	mThreads.push_back(newThread);

	return newThread;
}

u32 ThreadPool::GetNumAvailable() const
{
	u32 numAvailable = mMaxCapacity;

	Lock lock(mMutex);
	for(auto& thread : mThreads)
	{
		if(!thread->IsIdle())
			numAvailable--;
	}

	return numAvailable;
}

u32 ThreadPool::GetNumActive() const
{
	u32 numActive = 0;

	Lock lock(mMutex);
	for(auto& thread : mThreads)
	{
		if(!thread->IsIdle())
			numActive++;
	}

	return numActive;
}

u32 ThreadPool::GetNumAllocated() const
{
	Lock lock(mMutex);

	return (u32)mThreads.size();
}
