//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#define B3D_AUTO_MUTEX_NAME mutex

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <shared_mutex>

#include "Threading/B3DSpinLock.h"

namespace b3d
{
	/** @addtogroup Threading
	 *  @{
	 */

	/** Returns the number of logical CPU cores. */
#define B3D_THREAD_HARDWARE_CONCURRENCY std::thread::hardware_concurrency()

	/** Returns the ThreadId of the current thread. */
#define B3D_CURRENT_THREAD_ID std::this_thread::get_id()

	/** Causes the current thread to sleep for the provided amount of milliseconds. */
#define B3D_THREAD_SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

	/** Wrapper for the C++ std::mutex. */
	using Mutex = std::mutex;

	/** Wrapper for the C++ std::shared_mutex. */
	using SharedMutex = std::shared_mutex;

	/** Wrapper for the C++ std::recursive_mutex. */
	using RecursiveMutex = std::recursive_mutex;

	/** Wrapper for the C++ std::condition_variable. */
	using ConditionVariable = std::condition_variable;

	/** Wrapper for the C++ std::thread::id. */
	using ThreadId = std::thread::id;

	/** Wrapper for the C++ std::unique_lock<std::mutex>. */
	using Lock = std::unique_lock<Mutex>;

	/** Wrapper for the C++ std::shared_lock<std::shared_mutex>. */
	using SharedLock = std::shared_lock<SharedMutex>;

	/** Wrapper for the C++ std::unique_lock<std::shared_mutex>. */
	using UniqueLock = std::unique_lock<SharedMutex>;

	/** Wrapper for the C++ std::unique_lock<std::recursive_mutex>. */
	using RecursiveLock = std::unique_lock<RecursiveMutex>;

	/** @} */

	/**
	 * Policy that allows the calls its used in to pick between no locking and mutex locking through a template parameter.
	 */
	template <bool LOCK>
	class LockingPolicy
	{};

	/** Scoped lock that provides RAII-style locking and accepts both a normal mutex and a locking policy as input. */
	template <bool LOCK>
	class ScopedLock
	{};

	/** Specialization of LockingPolicy that performs no locking. */
	template <>
	class LockingPolicy<false> final
	{
	public:
		LockingPolicy() = default;

		void Lock(){};

		void Unlock() {}
	};

	/** Specialization of LockingPolicy that uses a mutex for locking. */
	template <>
	class LockingPolicy<true> final
	{
	public:
		LockingPolicy()
			: mLock(mMutex, std::defer_lock)
		{}

		void Lock()
		{
			mLock.lock();
		};

		void Unlock()
		{
			mLock.unlock();
		}

	private:
		friend class ScopedLock<true>;

		Mutex mMutex;
		::b3d::Lock mLock;
	};

	/** Scoped lock that performs no locking internally. Can only be used with a LockingPolicy. */
	template <>
	class ScopedLock<false>
	{
	public:
		ScopedLock(LockingPolicy<false>& policy)
		{}
	};

	/** Scoped lock that automatically locks when created and unlocks when it goes out of scope. */
	template <>
	class ScopedLock<true>
	{
	public:
		ScopedLock(LockingPolicy<true>& policy)
			: mLockGuard(policy.mMutex)
		{}

		ScopedLock(Mutex& mutex)
			: mLockGuard(mutex)
		{}

	private:
		std::lock_guard<Mutex> mLockGuard;
	};

	/** Controls thread safety in variety of cases. Thread unsafe types are generally faster as the cost of safety. */
	enum ThreadSafetyPolicy
	{
		ThreadSafe,
		ThreadUnsafe
	};

} // namespace b3d
