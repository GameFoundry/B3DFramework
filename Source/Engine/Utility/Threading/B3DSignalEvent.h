//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSignal.h"

namespace b3d
{
	/** Event that can be waited on until signalled. */
	class SignalEvent : INonCopyable
	{
	public:
		enum class Mode : uint8_t
		{
			/** Signaled state will be automatically reset once Wait() or CheckAndResetSignal() is called while in a signalled state. Signaling in this mode notifies only one of the Wait() calls. */
			AutomaticallyReset,

			/** Signaled state must be manually reset via a call to Reset(). Signaling in this mode notifies all of the Wait() calls. */
			ManuallyReset
		};

		inline SignalEvent(Mode mode = Mode::ManuallyReset, bool isInitiallySignalled = false);

		/** Returns true if the event has been signalled. */
		inline bool IsSignalled() const;

		/** Signals the event. If anything is waiting on the signal, the wait will be unblocked. */
		inline void Signal();

		/** Clears the signaled state. */
		inline void Reset();

		/** Blocks the caller until event has been signalled. If event is already signalled the wait will not block. If created using AutomaticallyReset mode, signalled state will be reset after wait unblocks. */
		inline void Wait();

		/** Blocks the caller until event has been signalled or a timeout expires. If event is already signalled the wait will not block. If created using AutomaticallyReset mode, signalled state will be reset after wait unblocks. */
		template <typename Rep, typename Period>
		bool WaitFor(const std::chrono::duration<Rep, Period>& duration);

		/** Blocks the caller until event has been signalled or a certain time point has been reached. If event is already signalled the wait will not block. If created using AutomaticallyReset mode, signalled state will be reset after wait unblocks. */
		template <typename Clock, typename Duration>
		bool WaitUntil(const std::chrono::time_point<Clock, Duration>& timeout);

	private:
		mutable Mutex mMutex;
		class Signal mSignal;
		const Mode mMode;
		bool mIsSignalled;
	};

	SignalEvent::SignalEvent(Mode mode, bool isInitiallySignalled)
		: mMode(mode), mIsSignalled(isInitiallySignalled)
	{ }

	inline bool SignalEvent::IsSignalled() const
	{
		Lock lock(mMutex);
		return mIsSignalled;
	}

	void SignalEvent::Signal()
	{
		Lock lock(mMutex);
		if (mIsSignalled)
			return;

		mIsSignalled = true;

		if (mMode == Mode::AutomaticallyReset)
			mSignal.NotifyOne();
		else
			mSignal.NotifyAll();
	}

	inline void SignalEvent::Reset()
	{
		Lock lock(mMutex);
		mIsSignalled = false;
	}

	void SignalEvent::Wait()
	{
		Lock lock(mMutex);

		mSignal.Wait(lock, [this] { return mIsSignalled; });

		if (mMode == Mode::AutomaticallyReset)
			mIsSignalled = false;
	}

	template <typename Rep, typename Period>
	bool SignalEvent::WaitFor(const std::chrono::duration<Rep, Period>& duration)
	{
		Lock lock(mMutex);

		if (!mSignal.WaitFor(lock, duration, [this] { return mIsSignalled; }))
			return false;

		if (mMode == Mode::AutomaticallyReset)
			mIsSignalled = false;

		return true;
	}

	template <typename Clock, typename Duration>
	bool SignalEvent::WaitUntil(const std::chrono::time_point<Clock, Duration>& timeout)
	{
		Lock lock(mMutex);

		if (!mSignal.WaitUntil(lock, timeout, [this] { return mIsSignalled; }))
			return false;

		if (mMode == Mode::AutomaticallyReset)
			mIsSignalled = false;

		return true;
	}
}  // namespace b3d
