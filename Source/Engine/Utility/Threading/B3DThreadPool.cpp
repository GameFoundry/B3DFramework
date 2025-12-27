//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/B3DThreadPool.h"

#include "B3DThread.h"
#include "Debug/B3DDebug.h"

#if B3D_PLATFORM_WIN32
#	include "windows.h"

#	if B3D_COMPILER_MSVC
// disable: nonstandard extension used: 'X' uses SEH and 'Y' has destructor
// We don't care about this as any exception is meant to crash the program.
#		pragma warning(disable : 4509)
#	endif // B3D_COMPILER_MSVC

#endif // B3D_PLATFORM_WIN32

using namespace b3d;

/** The thread pool will check for unused threads every UNUSED_CHECK_PERIOD getThread() calls*/
static constexpr int kUnusedCheckPeriod = 32;

PooledThread::PooledThread(const String& name)
	:mName(name)
{
}

void PooledThread::Initialize()
{
	mThread = B3DNew<Thread>([this]() { Run(); });

	Lock lock(mMutex);

	while(!mIsThreadStarted)
		mThreadStartedSignal.wait(lock);
}

void PooledThread::Start(std::function<void()> workerMethod)
{
	{
		Lock lock(mMutex);

		mWorkerMethod = workerMethod;
		mIsThreadIdle = false;
		mIdleTime = std::time(nullptr);
		mIsThreadReady = true;
	}

	mThreadReadySignal.notify_one();
}

void PooledThread::Run()
{
	OnThreadStarted(mName);

	{
		Lock lock(mMutex);
		mIsThreadStarted = true;
	}

	mThreadStartedSignal.notify_one();

	while(true)
	{
		std::function<void()> worker = nullptr;

		{
			{
				Lock lock(mMutex);

				while(!mIsThreadReady)
					mThreadReadySignal.wait(lock);

				worker = mWorkerMethod;
			}

			if(worker == nullptr)
			{
				OnThreadEnded(mName);
				return;
			}
		}

#if B3D_PLATFORM_WIN32
		RunFunctionHelper(worker);
#else
		worker();
#endif

		{
			Lock lock(mMutex);

			mIsThreadIdle = true;
			mIdleTime = std::time(nullptr);
			mIsThreadReady = false;
			mWorkerMethod = nullptr; // Make sure to clear as it could have bound shared pointers and similar

			mWorkerFinishedSignal.notify_one();
		}
	}
}

#if B3D_PLATFORM_WIN32
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
		mIsThreadReady = true;
	}

	mThreadReadySignal.notify_one();
	mThread->WaitUntilComplete();
	B3DDelete(mThread);
}

void PooledThread::BlockUntilComplete()
{
	Lock lock(mMutex);

	while(!mIsThreadIdle)
		mWorkerFinishedSignal.wait(lock);
}

bool PooledThread::IsIdle()
{
	Lock lock(mMutex);

	return mIsThreadIdle;
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

ThreadPool::ThreadPool(u32 threadCapacity, u32 idleTimeout)
	: mDefaultCapacity(threadCapacity), mIdleTimeout(idleTimeout)
{
}

ThreadPool::~ThreadPool()
{
	StopAll();
}

SPtr<PooledThread> ThreadPool::Run(const String& name, std::function<void()> workerMethod)
{
	SPtr<PooledThread> thread = GetThread(name);
	thread->Start(workerMethod);

	return thread;
}

void ThreadPool::StopAll()
{
	Lock lock(mMutex);
	for(auto& thread : mThreads)
	{
		thread->BlockUntilComplete();
	}

	mThreads.clear();
}

void ThreadPool::ClearUnused()
{
	Lock lock(mMutex);
	mAge = 0;

	if(mThreads.size() <= mDefaultCapacity)
		return;

	TInlineArray<SPtr<PooledThread>, 4> idleThreads;
	TInlineArray<SPtr<PooledThread>, 4> expiredThreads;
	TInlineArray<SPtr<PooledThread>, 4> activeThreads;

	idleThreads.reserve(mThreads.size());
	expiredThreads.reserve(mThreads.size());
	activeThreads.reserve(mThreads.size());

	for(auto& thread : mThreads)
	{
		if(thread->IsIdle())
		{
			if(thread->IdleTime() >= mIdleTimeout)
				expiredThreads.Add(thread);

			idleThreads.Add(thread);
		}
		else
			activeThreads.Add(thread);
	}

	u32 limit = std::min((u32)idleThreads.size(), mDefaultCapacity);

	u32 threadIndex = 0;
	mThreads.clear();

	for(auto& thread : idleThreads)
	{
		if(threadIndex < limit)
		{
			mThreads.push_back(thread);
			threadIndex++;
		}
	}

	mThreads.insert(mThreads.end(), activeThreads.begin(), activeThreads.end());
}

SPtr<PooledThread> ThreadPool::GetThread(const String& name)
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

	SPtr<PooledThread> newThread = CreateThread(name);
	mThreads.push_back(newThread);

	return newThread;
}

u32 ThreadPool::GetNumActive() const
{
	u32 activeCount = 0;

	Lock lock(mMutex);
	for(auto& thread : mThreads)
	{
		if(!thread->IsIdle())
			activeCount++;
	}

	return activeCount;
}

u32 ThreadPool::GetNumAllocated() const
{
	Lock lock(mMutex);

	return (u32)mThreads.size();
}
