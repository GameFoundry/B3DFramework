//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTimer.h"
#include "BsBitwise.h"

#include <chrono>

using namespace std::chrono;

namespace BansheeEngine
{
	Timer::Timer()
	{
		reset();
	}

	void Timer::reset()
	{
		mStartTime = mHRClock.now();
	}

	UINT64 Timer::getMilliseconds() const
	{
		steady_clock::time_point newTime = mHRClock.now();
		nanoseconds newTimeNs = newTime.time_since_epoch();

		return duration_cast<milliseconds>(newTimeNs).count();
	}

	UINT64 Timer::getMicroseconds() const
	{
		steady_clock::time_point newTime = mHRClock.now();
		nanoseconds newTimeNs = newTime.time_since_epoch();

		return duration_cast<microseconds>(newTimeNs).count();
	}

	UINT64 Timer::getStartMs() const
	{
		nanoseconds startTimeNs = mStartTime.time_since_epoch();

		return duration_cast<milliseconds>(startTimeNs).count();
	}
}