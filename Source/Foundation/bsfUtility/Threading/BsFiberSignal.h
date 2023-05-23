//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScheduler.h"
#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Threading
	 *  @{
	 */

	/** Similar to std::condition_variable, but also works with fibers in a way that allows waiting fibers to yield rather than blocking the thread. */
	class FiberSignal
	{
	public:
		FiberSignal() = default;

		/** Notifies one waiting fiber or thread. */
		inline void NotifyOne();

		/** Notifies all waiting fibers and threads. */
		inline void NotifyAll();

		/** Yields the current fiber, or blocks the thread (if not running in fiber context) until it is woken via one of the Notify*() calls and the predicate returns true. */
		template <typename Predicate>
		void Wait(Lock& lock, Predicate&& predicate);

		/** Yields the current fiber, or blocks the thread (if not running in fiber context) until it is woken via one of the Notify*() calls and the predicate returns true, or the timeout expires. Returns the value of the predicate (can be false in case the timeout expired, otherwise true). */
		template <typename Rep, typename Period, typename Predicate>
		bool WaitFor(Lock& lock, const std::chrono::duration<Rep, Period>& duration, Predicate&& predicate);

		/** Yields the current fiber, or blocks the thread (if not running in fiber context) until it is woken via one of the Notify*() calls and the predicate returns true, or the provided time point is reached. Returns the value of the predicate (can be false in case the time point was reached, otherwise true). */
		template <typename Clock, typename Duration, typename Predicate>
		bool WaitUntil(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, Predicate&& predicate);

	private:
		FiberSignal(const FiberSignal&) = delete;
		FiberSignal(FiberSignal&&) = delete;
		FiberSignal& operator=(const FiberSignal&) = delete;
		FiberSignal& operator=(FiberSignal&&) = delete;

		Mutex mMutex;
		List<Fiber*> mWaitingFibers;
		std::condition_variable mCondition;
		std::atomic<int> mTotalWaitingCount = { 0 };
		std::atomic<int> mThreadWaitingCount = { 0 };
	};

	void FiberSignal::NotifyOne()
	{
		if (mTotalWaitingCount == 0)
			return;

		{
			Lock lock(mMutex);
			if (mWaitingFibers.size() > 0)
			{
				(*mWaitingFibers.begin())->TryResume();
				return;
			}
		}

		if (mThreadWaitingCount > 0)
			mCondition.notify_one();
	}

	void FiberSignal::NotifyAll()
	{
		if (mTotalWaitingCount == 0)
			return;

		{
			Lock lock(mMutex);
			for (auto fiber : mWaitingFibers)
				fiber->TryResume();
		}

		if (mThreadWaitingCount > 0)
			mCondition.notify_all();
	}

	template <typename Predicate>
	void FiberSignal::Wait(Lock& lock, Predicate&& predicate)
	{
		if (predicate())
			return;

		mTotalWaitingCount++;
		if (Fiber* const fiber = Fiber::Get())
		{
			mMutex.lock();
			mWaitingFibers.emplace_front(fiber);
			mMutex.unlock();

			fiber->Wait(lock, predicate);

			mMutex.lock();
			mWaitingFibers.erase(std::find(mWaitingFibers.begin(), mWaitingFibers.end(), fiber));
			mMutex.unlock();
		}
		else
		{
			mThreadWaitingCount++;
			mCondition.wait(lock, predicate);
			mThreadWaitingCount--;
		}

		mTotalWaitingCount--;
	}

	template <typename Rep, typename Period, typename Predicate>
	bool FiberSignal::WaitFor(Lock& lock, const std::chrono::duration<Rep, Period>& duration, Predicate&& predicate)
	{
		return WaitUntil(lock, std::chrono::system_clock::now() + duration, predicate);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool FiberSignal::WaitUntil(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, Predicate&& predicate)
	{
		if (predicate())
			return true;

		if (Fiber* const fiber = Fiber::Get())
		{
			mTotalWaitingCount++;

			mMutex.lock();
			mWaitingFibers.emplace_front(fiber);
			mMutex.unlock();

			auto res = fiber->Wait(lock, timeout, predicate);

			mMutex.lock();
			mWaitingFibers.erase(std::find(mWaitingFibers.begin(), mWaitingFibers.end(), fiber));
			mMutex.unlock();

			mTotalWaitingCount--;
			return res;
		}

		mTotalWaitingCount++;
		mThreadWaitingCount++;

		bool result = mCondition.wait_until(lock, timeout, predicate);

		mThreadWaitingCount--;
		mTotalWaitingCount--;

		return result;
	}

	/** @} */

} // namespace bs
