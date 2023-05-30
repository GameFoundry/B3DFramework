//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "ThirdParty/marl/src/osfiber.h"  // Must come first. See osfiber_ucontext.h.
#include "Threading/BsScheduler.h"

#include "BsSingleConsumerQueue.h"
#include "BsThreadPool.h"
#include "Debug/BsDebug.h"

using namespace bs;

#if defined(_WIN32)
#include <intrin.h>  // __nop()
#endif

using namespace bs;

// Implementation based on Marl Scheduler (See ThirdParty/Marl)

Fiber::Fiber(UPtr<marl::OSFiber>&& osFiber, u32 id)
	: Id(id), mOSFiber(std::move(osFiber)), mOwningThread(SchedulerThread::Get().get())
{
	B3D_ASSERT(mOwningThread != nullptr && "No Scheduler thread found for fiber.");
}

Fiber* Fiber::Get()
{
	SchedulerThread* const schedulerThread = SchedulerThread::Get().get();
	return schedulerThread != nullptr ? schedulerThread->GetCurrentFiber() : nullptr;
}

void Fiber::TryResume()
{
	if (!B3D_ENSURE(mOwningThread != nullptr))
		return;

	mOwningThread->Enqueue(this);
}

void Fiber::Wait(Lock& lock, const Function<bool()>& predicate)
{
	B3D_ASSERT(mOwningThread == SchedulerThread::Get().get() && "Fiber::Wait() must only be called on the currently executing fiber.");
	mOwningThread->Wait(lock, nullptr, predicate);
}

void Fiber::SwitchExecutionTo(Fiber* to)
{
	B3D_ASSERT(mOwningThread == SchedulerThread::Get().get() && "Fiber::SwitchExecutionTo() must only be called on the currently executing fiber.");
	if (to != this)
		mOSFiber->switchTo(to->mOSFiber.get());
}

UPtr<Fiber> Fiber::Create(u32 id, size_t stackSize, const std::function<void()>& workerFunction)
{
	return B3DMakeUnique<Fiber>(marl::OSFiber::createFiber(stackSize, workerFunction), id);
}

UPtr<Fiber> Fiber::CreateFromCurrentThread(u32 id)
{
	return bs::B3DMakeUnique<Fiber>(marl::OSFiber::createFiberFromCurrentThread(), id);
}

static void nop()
{
#if defined(_WIN32)
	__nop();
#else
	__asm__ __volatile__("nop");
#endif
}

SchedulerThread::WaitingFibers::operator bool() const
{
	return !mFiberLookup.empty();
}

Fiber* SchedulerThread::WaitingFibers::TryPop(const TimePoint& timeout)
{
	if (!*this)
		return nullptr;

	auto it = mOrderedFibers.begin();
	if (timeout < it->Timepoint)
		return nullptr;

	Fiber* fiber = it->Fiber;
	mOrderedFibers.erase(it);
	mFiberLookup.erase(fiber);

	return fiber;
}

SchedulerThread::TimePoint SchedulerThread::WaitingFibers::PeekTime() const
{
	B3D_ASSERT(*this && "WaitingFibers::PeekTime() called when there' no waiting fibers.");
	return mOrderedFibers.begin()->Timepoint;
}

void SchedulerThread::WaitingFibers::Add(const TimePoint& timeout, Fiber* fiber)
{
	mOrderedFibers.emplace(WaitingFiber{ timeout, fiber });
	mFiberLookup.emplace(fiber, timeout);
}

void SchedulerThread::WaitingFibers::Erase(Fiber* fiber)
{
	auto it = mFiberLookup.find(fiber);
	if (it == mFiberLookup.end())
		return;

	auto timeout = it->second;
	mOrderedFibers.erase(WaitingFiber{ timeout, fiber });
	mFiberLookup.erase(it);
}

bool SchedulerThread::WaitingFibers::Contains(Fiber* fiber) const
{
	return mFiberLookup.count(fiber) != 0;
}

bool SchedulerThread::WaitingFibers::WaitingFiber::operator<(const WaitingFiber& other) const
{
	if (Timepoint != other.Timepoint)
		return Timepoint < other.Timepoint;

	return Fiber < other.Fiber;
}

template <typename T>
static T take(Deque<T>& queue)
{
	auto out = std::move(queue.front());
	queue.pop_front();

	return out;
}

template <typename T, typename H, typename E>
static T take(UnorderedSet<T, H, E>& set) {
	auto it = set.begin();
	auto out = std::move(*it);
	set.erase(it);

	return out;
}

thread_local SPtr<SchedulerThread> SchedulerThread::Current{ nullptr };

SchedulerThread::SchedulerThread(Scheduler* scheduler, Mode mode, u32 id)
	: Id(id), mMode(mode), mOwnerScheduler(scheduler)
{ }

void SchedulerThread::Start()
{
#if 0 // Disabled as it's causing a hang on shutdown
	if (mMessageQueue == nullptr)
		mMessageQueue = B3DNew<SingleConsumerQueue>();
#endif

	switch (mMode)
	{
		case Mode::MultiThreaded:
		{
			auto& affinityPolicy = mOwnerScheduler->GetInformation().AffinityPolicy;
			auto affinity = affinityPolicy->GetMaskForThread(Id);

			mThread = Thread(std::move(affinity), [this]
			{
				MemStack::BeginThread();

				Thread::SetName("Thread<%.2d>", int(Id));

				if (const auto& initializer = mOwnerScheduler->GetInformation().ThreadInitializeCallback)
					initializer(Id);

				Scheduler::Current = mOwnerScheduler;

				Current = shared_from_this();
				mMainFiber = Fiber::CreateFromCurrentThread(0);

				mCurrentFiber = mMainFiber.get();
				{
					Lock lock(mMutex);
					Run();
				}

				mMainFiber.reset();
				Current = nullptr;

				MemStack::EndThread();
			});

			break;
		}
		case Mode::SingleThreaded:
		{
			Current = shared_from_this();
			mMainFiber = Fiber::CreateFromCurrentThread(0);
			mCurrentFiber = mMainFiber.get();
			break;
		}
	}

#if 0 // Disabled as it's causing a hang on shutdown
	Post(SchedulerTask("Scheduler thread message queue", [this] { mMessageQueue->RunUntilShutdown(); }));
#endif
}

void SchedulerThread::Stop()
{
	if(mMessageQueue != nullptr)
		mMessageQueue->PostRequestShutdownCommand(true);

	switch (mMode)
	{
		case Mode::MultiThreaded:
		{
			Enqueue(SchedulerTask([this] { mIsShutdownRequested = true; }, SchedulerTaskFlag::SameThread));
			mThread.WaitUntilComplete();
			break;
		}
		case Mode::SingleThreaded:
		{
			Lock lock(mMutex);
			mIsShutdownRequested = true;
			RunUntilShutdown();
			Current = nullptr;
			break;
		}
	}

	if (mMessageQueue != nullptr)
	{
		B3DDelete(mMessageQueue);
		mMessageQueue = nullptr;
	}
}

bool SchedulerThread::Wait(const TimePoint* timeout)
{
	{
		Lock lock(mMutex);
		WaitWithoutLocking(timeout);
	}

	return timeout == nullptr || std::chrono::system_clock::now() < *timeout;
}

bool SchedulerThread::Wait(Lock& waitLock, const TimePoint* timeout, const Function<bool()>& predicate)
{
	while (!predicate())
	{
		mMutex.lock();

		// Must be called after mMutex is locked to ensure the fiber is not enqueued after this has been unlocked, as that could result in fiber never being woken up
		waitLock.unlock();

		// Let another fiber take over, or just spin/wait if no work. This will internally unlock the mMutex.
		WaitWithoutLocking(timeout);

		mMutex.unlock();

		// Re-lock the lock provided to us by the user
		waitLock.lock();

		if (timeout != nullptr && std::chrono::system_clock::now() >= *timeout)
			return false;

		// Spurious wake up. Spin again.
	}

	return true;
}

void SchedulerThread::WaitWithoutLocking(const TimePoint* timeout)
{
	if (timeout != nullptr)
	{
		mCurrentFiber->mState = Fiber::State::Waiting;
		mWaitingFibers.Add(*timeout, mCurrentFiber);
	}
	else
	{
		mCurrentFiber->mState = Fiber::State::Yielded;
	}

	WaitForWork();

	mBlockedFiberCount++;

	// First try to fetch some resumed fibers
	if (!mReadyFibers.empty())
	{
		mReadyOperationCount--;

		Fiber* fiber = take(mReadyFibers);
		B3D_ASSERT(fiber->mState == Fiber::State::Queued);

		SwitchExecutionToFiber(fiber);
	}
	// If no fibers to resume, start new tasks by trying to reuse unused fibers
	else if (!mFreeFibers.empty())
	{
		Fiber* fiber = take(mFreeFibers);
		B3D_ASSERT(fiber->mState == Fiber::State::Idle);

		SwitchExecutionToFiber(fiber);
	}
	// No unused fibers, start new task by creating a new fiber
	else
	{
		SwitchExecutionToFiber(CreateWorkerFiber());
	}

	mBlockedFiberCount--;
	mCurrentFiber->mState = Fiber::State::Running;
}

bool SchedulerThread::TryLockForEnqueue()
{
	return mMutex.try_lock();
}

void SchedulerThread::Enqueue(Fiber* fiber)
{
	bool isNotifyRequired = false;
	{
		Lock lock(mMutex);

		switch (fiber->mState)
		{
		case Fiber::State::Running:
		case Fiber::State::Queued:
			return; // Task already queued or running
		case Fiber::State::Waiting:
			mWaitingFibers.Erase(fiber);
			break;
		case Fiber::State::Idle:
		case Fiber::State::Yielded:
			break;
		}

		isNotifyRequired = mTriggerNotifyOnAdd;
		mReadyFibers.push_back(fiber);
		B3D_ASSERT(!mWaitingFibers.Contains(fiber));

		fiber->mState = Fiber::State::Queued;
		mReadyOperationCount++;
	}

	if (isNotifyRequired)
		mAddedSignal.notify_one();
}

void SchedulerThread::Enqueue(SchedulerTask&& task)
{
	mMutex.lock();
	EnqueueAndUnlock(std::move(task));
}

void SchedulerThread::EnqueueAndUnlock(SchedulerTask&& task)
{
	bool isNotifyRequired = mTriggerNotifyOnAdd;
	mPendingTasks.push_back(std::move(task));
	mReadyOperationCount++;
	mMutex.unlock();

	if (isNotifyRequired)
		mAddedSignal.notify_one();
}

void SchedulerThread::Post(SchedulerTask&& task)
{
	if (B3D_ENSURE(!mIsShutdownRequested))
		return;

	task.GetFlags().Set(SchedulerTaskFlag::NoStealing);
	Enqueue(std::move(task));
}

bool SchedulerThread::TryStealTask(SchedulerTask& out)
{
	if (mReadyOperationCount.load() == 0)
		return false;

	if (!mMutex.try_lock())
		return false;

	if (mPendingTasks.empty() || mPendingTasks.front().GetFlags().IsSetAny(SchedulerTaskFlag::SameThread | SchedulerTaskFlag::NoStealing))
	{
		mMutex.unlock();
		return false;
	}

	mReadyOperationCount--;
	out = take(mPendingTasks);
	mMutex.unlock();

	return true;
}

void SchedulerThread::WaitOnAddedSignal(const Function<bool()>& predicate)
{
	mTriggerNotifyOnAdd = true;

	if(mWaitingFibers)
	{
		Lock lock(mMutex, std::adopt_lock);
		mAddedSignal.wait_until(lock, mWaitingFibers.PeekTime(), predicate);
		lock.release(); // Keep lock held.
	}
	else
	{
		Lock lock(mMutex, std::adopt_lock);
		mAddedSignal.wait(lock, predicate);
		lock.release(); // Keep lock held.
	}

	mTriggerNotifyOnAdd = false;
}

void SchedulerThread::Run()
{
	if (mMode == Mode::MultiThreaded)
	{
		auto fnWaitCondition = [this]() { return mReadyOperationCount > 0 || mWaitingFibers || mIsShutdownRequested; };

		// Wait immediately to avoid spinning if there's no work yet
		WaitOnAddedSignal(fnWaitCondition);
	}

	B3D_ASSERT(mCurrentFiber->mState == Fiber::State::Running);

	RunUntilShutdown();
	SwitchExecutionToFiber(mMainFiber.get());
}

void SchedulerThread::RunUntilShutdown()
{
	while (!mIsShutdownRequested || mReadyOperationCount > 0 || mBlockedFiberCount > 0)
	{
		WaitForWork();
		RunUntilIdle();
	}
}

void SchedulerThread::WaitForWork()
{
	B3D_ASSERT(mReadyOperationCount == (mReadyFibers.size() + mPendingTasks.size()));
	if (mReadyOperationCount > 0)
		return;

	if (mMode == Mode::MultiThreaded)
	{
		mOwnerScheduler->NotifyOnBeginSpinning(Id);

		mMutex.unlock();
		SpinForWork();
		mMutex.lock();
	}

	auto fnWaitCondition = [this]() { return mReadyOperationCount > 0 || (mIsShutdownRequested && mBlockedFiberCount == 0); };
	WaitOnAddedSignal(fnWaitCondition);

	if (mWaitingFibers)
		UpdateWaitingFibers();
}

void SchedulerThread::UpdateWaitingFibers()
{
	auto now = std::chrono::system_clock::now();
	while (auto fiber = mWaitingFibers.TryPop(now))
	{
		fiber->mState = Fiber::State::Queued;
		mReadyFibers.push_back(fiber);
		mReadyOperationCount++;
	}
}

void SchedulerThread::SpinForWork()
{
	SchedulerTask stolenTask;

	constexpr auto duration = std::chrono::milliseconds(1);
	auto start = std::chrono::high_resolution_clock::now();
	while (std::chrono::high_resolution_clock::now() - start < duration)
	{
		for (int i = 0; i < 256; i++)
		{
			// clang-format off
			nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
			nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
			nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
			nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
			// clang-format on

			if (mReadyOperationCount > 0)
				return;
		}

		if (mOwnerScheduler->TryStealWork(this, mRandomNumberGenerator.Get(), stolenTask))
		{
			Lock lock(mMutex);
			mPendingTasks.emplace_back(std::move(stolenTask));
			mReadyOperationCount++;
			return;
		}

		std::this_thread::yield();
	}
}

void SchedulerThread::RunUntilIdle()
{
	B3D_ASSERT(mCurrentFiber->mState == Fiber::State::Running);
	B3D_ASSERT(mReadyOperationCount == (mReadyFibers.size() + mPendingTasks.size()));

	while (!mReadyFibers.empty() || !mPendingTasks.empty())
	{
		// Note: we cannot take and store on the stack more than a single fiber
		// or task at a time, as the Fiber may yield and these items may get
		// held on suspended fiber stack.

		while (!mReadyFibers.empty())
		{
			mReadyOperationCount--;
			auto fiber = take(mReadyFibers);

			B3D_ASSERT(mFreeFibers.count(fiber) == 0);
			B3D_ASSERT(fiber != mCurrentFiber);
			B3D_ASSERT(fiber->mState == Fiber::State::Queued);

			mCurrentFiber->mState = Fiber::State::Idle;

			auto added = mFreeFibers.emplace(mCurrentFiber).second;
			(void)added;
			B3D_ASSERT(added);

			SwitchExecutionToFiber(fiber);
			mCurrentFiber->mState = Fiber::State::Running;
		}

		if (!mPendingTasks.empty())
		{
			mReadyOperationCount--;
			auto task = take(mPendingTasks);
			mMutex.unlock();

			task();

			// std::function<> can carry arguments with complex destructors.
			// Ensure these are destructed outside of the lock.
			task = SchedulerTask();

			mMutex.lock();
		}
	}
}

Fiber* SchedulerThread::CreateWorkerFiber()
{
	auto fiberId = static_cast<u32>(mAllFibers.size() + 1);
	auto fiber = Fiber::Create(fiberId, mOwnerScheduler->GetInformation().FiberStackSize, [this]() { Run(); });
	auto ptr = fiber.get();
	mAllFibers.Add(std::move(fiber));

	return ptr;
}

void SchedulerThread::SwitchExecutionToFiber(Fiber* to)
{
	B3D_ASSERT(to == mMainFiber.get() || mFreeFibers.count(to) == 0);

	auto from = mCurrentFiber;
	mCurrentFiber = to;

	from->SwitchExecutionTo(to);
}

thread_local Scheduler* Scheduler::Current{ nullptr };

void Scheduler::BindToCurrentThread()
{
	B3D_ASSERT(Get() == nullptr && "Scheduler already bound to this thread.");

	Current = this;

	{
		Lock lock(mSingleThreadWorkerMutex);
		SPtr<SchedulerThread> schedulerThread = bs::B3DMakeShared<SchedulerThread>(this, SchedulerThread::Mode::SingleThreaded, ~0u);
		schedulerThread->Start();

		const std::thread::id threadId = std::this_thread::get_id();
		mSingleThreadWorkers.emplace(threadId, std::move(schedulerThread));
	}
}

void Scheduler::UnbindFromCurrentThread()
{
	B3D_ASSERT(Get() != nullptr && "No scheduler bound to this thread.");

	const SPtr<SchedulerThread> schedulerThread = SchedulerThread::Get();
	schedulerThread->Stop();

	{
		Lock lock(Get()->mSingleThreadWorkerMutex);

		const std::thread::id threadId = std::this_thread::get_id();

		auto& workers = Get()->mSingleThreadWorkers;
		auto it = workers.find(threadId);
		B3D_ASSERT(it != workers.end() && "Cannot find worker in the single threaded worker list.");
		B3D_ASSERT(it->second.get() == schedulerThread.get() && "Scheduler running a single threaded worker that is not currently bound.");

		workers.erase(it);

		if (workers.empty())
			Get()->mSingleThreadWorkerUnbindSignal.notify_one();
	}

	Current = nullptr;
}

Scheduler::Scheduler(const SchedulerCreateInformation& createInformation)
	: mInformation(createInformation)
{
	for (size_t i = 0; i < mSpinningWorkers.size(); i++) {
		mSpinningWorkers[i] = ~0u;
	}
	for (u32 i = 0; i < mInformation.WorkerThreadCount; i++)
		mWorkerThreads.push_back(B3DMakeShared<SchedulerThread>(this, SchedulerThread::Mode::MultiThreaded, i));

	for(auto& thread : mWorkerThreads)
		thread->Start();
}

Scheduler::~Scheduler()
{
	{
		// Wait until all the single threaded workers have been unbound.
		Lock lock(mSingleThreadWorkerMutex);
		mSingleThreadWorkerUnbindSignal.wait(lock, [this]() { return mSingleThreadWorkers.empty(); });
	}

	// Release all worker threads.
	// This will wait for all in-flight tasks to complete before returning.
	for(auto& thread : mWorkerThreads)
		thread->Stop();
}

void Scheduler::Post(SchedulerTask&& task)
{
	if (task.GetFlags().IsSet(SchedulerTaskFlag::SameThread))
	{
		SchedulerThread::Get()->Enqueue(std::move(task));
		return;
	}

	if (mInformation.WorkerThreadCount > 0)
	{
		while (true)
		{
			// Prioritize workers that have recently started spinning.
			u32 index = --mNextSpinningWorkerIndex % mSpinningWorkers.size();
			u32 workerId = mSpinningWorkers[index].exchange(~0u);
			if (workerId == ~0u)
			{
				// If a spinning worker couldn't be found, round-robin the workers.
				workerId = mNextEnqueueIndex++ % mInformation.WorkerThreadCount;
			}

			const SPtr<SchedulerThread>& worker = mWorkerThreads[workerId];
			if (worker->TryLockForEnqueue())
			{
				worker->EnqueueAndUnlock(std::move(task));
				return;
			}
		}
	}
	else
	{
		if (auto worker = SchedulerThread::Get())
		{
			worker->Enqueue(std::move(task));
		}
		else
		{
			B3D_EXCEPT(InvalidStateException, "No thread bound to the scheduler. Did you call Scheduler::BindToCurrentThread()?");
		}
	}
}

bool Scheduler::TryStealWork(SchedulerThread* thief, u32 random, SchedulerTask& out)
{
	if (mInformation.WorkerThreadCount > 0)
	{
		auto thread = mWorkerThreads[random % mInformation.WorkerThreadCount].get();
		if (thread != thief)
		{
			if (thread->TryStealTask(out))
				return true;
		}
	}

	return false;
}

void Scheduler::NotifyOnBeginSpinning(u32 workerId)
{
	const u32 index = mNextSpinningWorkerIndex++ % mSpinningWorkers.size();
	mSpinningWorkers[index] = workerId;
}
