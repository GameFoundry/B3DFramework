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

	class B3D_UTILITY_EXPORT B3DSignal
	{
	public:
		B3DSignal() = default;

		inline void notify_one();
		inline void notify_all();

		template <typename Predicate>
		void wait(Lock& lock, Predicate&& predicate);

		template <typename Rep, typename Period, typename Predicate>
		bool wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& duration, Predicate&& predicate);

		template <typename Clock, typename Duration, typename Predicate>
		bool wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, Predicate&& predicate);

	private:
		B3DSignal(const B3DSignal&) = delete;
		B3DSignal(B3DSignal&&) = delete;
		B3DSignal& operator=(const B3DSignal&) = delete;
		B3DSignal& operator=(B3DSignal&&) = delete;

		Mutex mMutex;
		List<Fiber*> mWaitingFibers;
		std::condition_variable mCondition;
		std::atomic<int> mTotalWaitingCount = { 0 };
		std::atomic<int> mThreadWaitingCount = { 0 };
	};

	void B3DSignal::notify_one()
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

	void B3DSignal::notify_all()
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
	void B3DSignal::wait(Lock& lock, Predicate&& predicate)
	{
		if (predicate())
			return;

		mTotalWaitingCount++;
		if (Fiber* const fiber = Fiber::Get())
		{
			mMutex.lock();
			auto it = mWaitingFibers.emplace_front(fiber);
			mMutex.unlock();

			fiber->Wait(lock, predicate);

			mMutex.lock();
			mWaitingFibers.erase(it);
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
	bool B3DSignal::wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& duration, Predicate&& predicate)
	{
		return wait_until(lock, std::chrono::system_clock::now() + duration, predicate);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool B3DSignal::wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& timeout, Predicate&& predicate)
	{
		if (predicate())
			return true;

		if (Fiber* const fiber = Fiber::Get())
		{
			mTotalWaitingCount++;

			mMutex.lock();
			auto it = mWaitingFibers.emplace_front(fiber);
			mMutex.unlock();

			auto res = fiber->Wait(lock, timeout, predicate);

			mMutex.lock();
			mWaitingFibers.erase(it);
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
