//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsTimer.h"
#include "Utility/BsBitwise.h"

#include <chrono>

using namespace std::chrono;

namespace bs
{
	Timer::Timer()
	{
		reset();
	}

	void Timer::Reset()
	{
		mStartTime = mHRClock.now();
	}

	UINT64 Timer::GetMilliseconds() const
	{
		auto newTime = mHRClock.now();
		duration<double> dur = newTime - mStartTime;

		return duration_cast<milliseconds>(dur).count();
	}

	UINT64 Timer::GetMicroseconds() const
	{
		auto newTime = mHRClock.now();
		duration<double> dur = newTime - mStartTime;

		return duration_cast<microseconds>(dur).count();
	}

	UINT64 Timer::GetStartMs() const
	{
		nanoseconds startTimeNs = mStartTime.time_since_epoch();

		return duration_cast<milliseconds>(startTimeNs).count();
	}
}
