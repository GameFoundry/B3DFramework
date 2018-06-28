//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsTime.h"
#include "Utility/BsTimer.h"
#include "Math/BsMath.h"

namespace bs
{
	/** Determines how many fixed updates per frame are allowed. Only relevant when framerate is low. */
	static constexpr UINT32 MAX_FIXED_UPDATES_PER_FRAME = 4;

	const double Time::MICROSEC_TO_SEC = 1.0/1000000.0;

	Time::Time()
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
		mTimeSinceStart = mTimeSinceStartMs / 1000.0f;
		
		mLastFrameTime = currentFrameTime;

		mCurrentFrame.fetch_add(1, std::memory_order_relaxed);
	}
	
	UINT32 Time::_getFixedUpdateStep(UINT64& step)
	{
		const UINT64 currentTime = getTimePrecise();

		// Skip fixed update first frame (time delta is zero, and no input received yet)
		if (mFirstFrame)
		{
			mLastFixedUpdateTime = currentTime;
			mFirstFrame = false;
		}

		const UINT64 nextFrameTime = mLastFixedUpdateTime + mFixedStep;
		if (nextFrameTime <= currentTime)
		{
			const INT64 simulationAmount = (INT64)std::max(currentTime - mLastFixedUpdateTime, mFixedStep); // At least one step
			auto numIterations = (UINT32)Math::divideAndRoundUp(simulationAmount, (INT64)mFixedStep);

			// If too many iterations are required, increase time step. This should only happen in extreme 
			// situations (or when debugging).
			auto stepus = (INT64)mFixedStep;
			if (numIterations > (INT32)MAX_FIXED_UPDATES_PER_FRAME)
			{
				stepus = Math::divideAndRoundUp(simulationAmount, (INT64)MAX_FIXED_UPDATES_PER_FRAME);
				numIterations = (UINT32)Math::divideAndRoundUp(simulationAmount, (INT64)stepus);
			}

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

	Time& gTime()
	{
		return Time::instance();
	}
}
