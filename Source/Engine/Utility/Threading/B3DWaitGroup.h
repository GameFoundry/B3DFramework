//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSignal.h"
#include "Debug/B3DDebug.h"

namespace b3d
{
	/** Provides an easy way to wait for N operations to complete executing. */
	class WaitGroup : INonCopyable
	{
	public:
		inline WaitGroup(u32 initialOperationCount = 0);

		/** Notifies the group that an operation completed executing. If anything is waiting for the wait group, it will be unblocked if the operation count reached 0. Returns true if operation count reached 0. */
		inline bool NotifyDone();

		/** Increments the operation count. */
		inline void Increment(u32 count = 1);

		/** Blocks the caller until the operation count reaches 0. */
		inline void Wait();

	private:
		Mutex mMutex;
		Signal mSignal;
		std::atomic<u32> mOperationCount;
	};

	WaitGroup::WaitGroup(u32 initialOperationCount)
		: mOperationCount(initialOperationCount)
	{ }

	bool WaitGroup::NotifyDone()
	{
		Lock lock(mMutex);

		const u32 newCount = --mOperationCount;
		B3D_ENSURE(newCount != std::numeric_limits<u32>::max()); // Overflow

		if(newCount == 0)
		{
			lock.unlock();
			mSignal.NotifyAll();

			return true;
		}

		return false;
	}

	void WaitGroup::Increment(u32 count)
	{
		mOperationCount += count;
	}

	void WaitGroup::Wait()
	{
		Lock lock(mMutex);

		mSignal.Wait(lock, [this] { return mOperationCount == 0; });
	}
}  // namespace b3d
