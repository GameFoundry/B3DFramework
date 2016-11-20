//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <atomic>

namespace bs
{
	/** @addtogroup Threading
	 *  @{
	 */

	/**
	 * Synchronization primitive with low overhead.
	 *
	 * @note	
	 * However it will actively block the thread waiting for the lock, not allowing any other work to be done, so it is 
	 * best used for short locks.
	 */
	class SpinLock
	{
	public:
		SpinLock()
		{
			mLock.clear(std::memory_order_relaxed);
		}

		/** Lock any following operations with the spin lock, not allowing any other thread to access them. */
		void lock()
		{
			while(mLock.test_and_set(std::memory_order_acquire))
			{ }
		}

		/**	Release the lock and allow other threads to acquire the lock. */
		void unlock()
		{
			mLock.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag mLock;
	};

	/**
	 * Provides a safer method for locking a spin lock as the lock will get automatically locked when this objected is 
	 * created and unlocked as soon as it goes out of scope.
	 */
	class ScopedSpinLock
	{
	public:
		ScopedSpinLock(SpinLock& spinLock)
			:mSpinLock(spinLock)
		{
			mSpinLock.lock();
		}

		~ScopedSpinLock()
		{
			mSpinLock.unlock();
		}

	private:
		SpinLock& mSpinLock;
	};

	/** @} */
}