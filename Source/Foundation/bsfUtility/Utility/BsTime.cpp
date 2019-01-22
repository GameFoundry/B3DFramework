//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsTime.h"
#include "Utility/BsTimer.h"
#include "Math/BsMath.h"

namespace bs
{
	constexpr UINT32 Time::MAX_ACCUM_FIXED_UPDATES;
	constexpr UINT32 Time::NEW_FIXED_UPDATES_PER_FRAME;

	const double Time::MICROSEC_TO_SEC = 1.0/1000000.0;

	Time::Time()
	{
		mTimer = bs_new<Timer>();
		mAppStartTime = mTimer->getStartMs();
		mLastFrameTime = mTimer->getMicroseconds();
		mAppStartUpDate = std::time(nullptr);
	}

	Time::~Time()
	{
		bs_delete(mTimer);
	}

	void Time::_update()
	{
		UINT64 currentFrameTime = mTimer->getMicroseconds();

		if(!mFirstFrame)
			mFrameDelta = (float)((currentFrameTime - mLastFrameTime) * MICROSEC_TO_SEC);
		else
		{
			mFrameDelta = 0.0f;
			mFirstFrame = false;
		}

		mTimeSinceStartMs = (UINT64)(currentFrameTime / 1000);
		mTimeSinceStart = mTimeSinceStartMs / 1000.0f;
		
		mLastFrameTime = currentFrameTime;

		mCurrentFrame.fetch_add(1, std::memory_order_relaxed);
	}
	
	UINT32 Time::_getFixedUpdateStep(UINT64& step)
	{
		const UINT64 currentTime = getTimePrecise();

		// Skip fixed update first frame (time delta is zero, and no input received yet)
		if (mFirstFixedFrame)
		{
			mLastFixedUpdateTime = currentTime;
			mFirstFixedFrame = false;
		}

		const UINT64 nextFrameTime = mLastFixedUpdateTime + mFixedStep;
		if (nextFrameTime <= currentTime)
		{
			const INT64 simulationAmount = (INT64)std::max(currentTime - mLastFixedUpdateTime, mFixedStep); // At least one step
			auto numIterations = (UINT32)Math::divideAndRoundUp(simulationAmount, (INT64)mFixedStep);

			// Prevent physics from completely hogging the CPU. If the framerate is low, the physics will want to run many
			// iterations per frame, slowing down the game even further. Therefore we limit the number of physics updates
			// to a certain number (at the cost of simulation stability). 
			
			// However we don't use a fixed number per frame because performance spikes can cause some frames to take a very
			// long time. These spikes can happen even in an otherwise well-performing application and will can wreak havoc
			// on the physics simulation.

			// Therefore we keep a "pool" which determines the number of physics frame iterations allowed to run. This pool
			// gets exhausted with every iteration, and replenished with every new frame. The pool can hold a large number
			// of frames which can then get used up during performance spikes, ensuring simulation stability. If the
			// performance is consistently low (not just a spike), then the pool will get exhausted and physics updates
			// will slow down to free up the CPU (at the cost of stability, but this time we have no other option).

			auto stepus = (INT64)mFixedStep;
			if (numIterations > mNumRemainingFixedUpdates)
			{
				stepus = Math::divideAndRoundUp(simulationAmount, (INT64)mNumRemainingFixedUpdates);
				numIterations = (UINT32)Math::divideAndRoundUp(simulationAmount, (INT64)stepus);
			}

			assert(numIterations <= mNumRemainingFixedUpdates);

			mNumRemainingFixedUpdates -= numIterations;
			mNumRemainingFixedUpdates = std::min(MAX_ACCUM_FIXED_UPDATES, mNumRemainingFixedUpdates + NEW_FIXED_UPDATES_PER_FRAME);

			step = stepus;
			return numIterations;
		}

		step = 0;
		return 0;
	}

	void Time::_advanceFixedUpdate(UINT64 step)
	{
		mLastFixedUpdateTime += step;
	}

	UINT64 Time::getTimePrecise() const
	{
		return mTimer->getMicroseconds();
	}

	String Time::getCurrentDateTimeString(bool isUTC)
	{
		std::time_t t = std::time(nullptr);
		char out[100];
		if (isUTC)
			std::strftime(out, sizeof(out), "%A, %B %d, %Y %T", std::gmtime(&t));
		else
			std::strftime(out, sizeof(out), "%A, %B %d, %Y %T", std::localtime(&t));
		return String(out);
	}

	String Time::getCurrentTimeString(bool isUTC)
	{
		std::time_t t = std::time(nullptr);
		char out[15];
		if (isUTC)
			std::strftime(out, sizeof(out), "%T", std::gmtime(&t));
		else
			std::strftime(out, sizeof(out), "%T", std::localtime(&t));
		return String(out);
	}

	String Time::getAppStartUpDateString(bool isUTC)
	{
		char out[100];
		if (isUTC)
			std::strftime(out, sizeof(out), "%A, %B %d, %Y %T", std::gmtime(&mAppStartUpDate));
		else
			std::strftime(out, sizeof(out), "%A, %B %d, %Y %T", std::localtime(&mAppStartUpDate));
		return String(out);
	}

	Time& gTime()
	{
		return Time::instance();
	}
}
