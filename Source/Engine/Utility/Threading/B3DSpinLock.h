//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <atomic>

namespace b3d
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
		void Lock()
		{
			while(mLock.test_and_set(std::memory_order_acquire))
			{}
		}

		/**	Release the lock and allow other threads to acquire the lock. */
		void Unlock()
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
			: mSpinLock(spinLock)
		{
			mSpinLock.Lock();
		}

		~ScopedSpinLock()
		{
			mSpinLock.Unlock();
		}

	private:
		SpinLock& mSpinLock;
	};

	/** @} */
} // namespace b3d
