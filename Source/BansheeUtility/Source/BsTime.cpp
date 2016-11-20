//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTime.h"
#include "BsTimer.h"

namespace bs
{
	const double Time::MICROSEC_TO_SEC = 1.0/1000000.0;

	Time::Time()
		:mFrameDelta(0.0f), mTimeSinceStart(0.0f), mTimeSinceStartMs(0), mAppStartTime(0), mLastFrameTime(0), 
		mCurrentFrame(0UL)
	{
		mTimer = bs_new<Timer>();
		mAppStartTime = mTimer->getStartMs();
		mLastFrameTime = mTimer->getMicroseconds();
	}

	Time::~Time()
	{
		bs_delete(mTimer);
	}

	void Time::_update()
	{
		UINT64 currentFrameTime = mTimer->getMicroseconds();

		mFrameDelta = (float)((currentFrameTime - mLastFrameTime) * MICROSEC_TO_SEC);
		mTimeSinceStartMs = (UINT64)(currentFrameTime / 1000);
		mTimeSinceStart = (mTimeSinceStartMs - mAppStartTime) / 1000.0f;
		
		mLastFrameTime = currentFrameTime;

		mCurrentFrame.fetch_add(1, std::memory_order_relaxed);
	}

	UINT64 Time::getTimePrecise() const
	{
		return mTimer->getMicroseconds();
	}

	Time& gTime()
	{
		return Time::instance();
	}
}