//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DTimer.h"
#include "Utility/B3DBitwise.h"

#include <chrono>

using namespace std::chrono;

using namespace b3d;

Timer::Timer()
{
	Reset();
}

void Timer::Reset()
{
	mStartTime = mHRClock.now();
}

u64 Timer::GetMilliseconds() const
{
	auto newTime = mHRClock.now();
	duration<double> dur = newTime - mStartTime;

	return duration_cast<milliseconds>(dur).count();
}

u64 Timer::GetMicroseconds() const
{
	auto newTime = mHRClock.now();
	duration<double> dur = newTime - mStartTime;

	return duration_cast<microseconds>(dur).count();
}

u64 Timer::GetStartMs() const
{
	nanoseconds startTimeNs = mStartTime.time_since_epoch();

	return duration_cast<milliseconds>(startTimeNs).count();
}
