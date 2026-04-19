//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DTime.h"
#include "Utility/B3DTimer.h"
#include "Math/B3DMath.h"
#include "String/B3DString.h"

using namespace b3d;

const double Time::kMicrosecToSec = 1.0 / 1000000.0;

Time::Time()
{
	mTimer = B3DNew<Timer>();
	mAppStartTime = mTimer->GetStartMs();
	mLastFrameTime = mTimer->GetMicroseconds();
	mAppStartUpDate = std::time(nullptr);
}

Time::~Time()
{
	B3DDelete(mTimer);
}

void Time::Update()
{
	u64 currentFrameTime = mTimer->GetMicroseconds();

	if(!mFirstFrame)
	{
		if(mFixedDeltaTimeMicrosec > 0)
			mFrameDelta = (float)(mFixedDeltaTimeMicrosec * kMicrosecToSec);
		else
			mFrameDelta = (float)((currentFrameTime - mLastFrameTime) * kMicrosecToSec);
	}
	else
	{
		mFrameDelta = 0.0f;
		mFirstFrame = false;
	}

	mTimeSinceStartMs = (u64)(currentFrameTime / 1000);
	mTimeSinceStart = mTimeSinceStartMs / 1000.0f;
	mLastFrameTime = currentFrameTime;

	mCurrentFrame.fetch_add(1, std::memory_order_relaxed);
}

void Time::SetFixedDeltaTime(float deltaSeconds)
{
	if(deltaSeconds > 0.0f)
		mFixedDeltaTimeMicrosec = (u64)(deltaSeconds / kMicrosecToSec);
	else
		mFixedDeltaTimeMicrosec = 0;
}

u64 Time::GetTimePrecise() const
{
	return mTimer->GetMicroseconds();
}

String Time::GetCurrentDateTimeString(bool isUTC)
{
	std::time_t t = std::time(nullptr);
	return ToString(t, isUTC, false, TimeToStringConversionType::Full);
}

String Time::GetCurrentTimeString(bool isUTC)
{
	std::time_t t = std::time(nullptr);
	return ToString(t, isUTC, false, TimeToStringConversionType::Time);
}

String Time::GetAppStartUpDateString(bool isUTC)
{
	return ToString(mAppStartUpDate, isUTC, false, TimeToStringConversionType::Full);
}

SceneTime::SceneTime()
{
	mTimer = B3DNew<Timer>();
	mLastFrameTime = mTimer->GetMicroseconds();
}

SceneTime::~SceneTime()
{
	B3DDelete(mTimer);
}

void SceneTime::Update()
{
	u64 currentFrameTime = mTimer->GetMicroseconds();

	if(!mFirstFrame)
	{
		if(mFixedDeltaTimeMicrosec > 0)
			mFrameDelta = (float)(mFixedDeltaTimeMicrosec * Time::kMicrosecToSec);
		else
			mFrameDelta = (float)((currentFrameTime - mLastFrameTime) * Time::kMicrosecToSec);
	}
	else
	{
		mFrameDelta = 0.0f;
		mFirstFrame = false;
	}

	if(!mIsTimePaused)
		mTimeInSeconds += mFrameDelta * mTimeScale;

	mLastFrameTime = currentFrameTime;
}

void SceneTime::SetScale(float scale)
{
	mTimeScale = Math::Max(0.0f, scale);
}

void SceneTime::SetFixedDeltaTimeUs(u64 delta)
{
	if(delta > 0)
		mFixedDeltaTimeMicrosec = delta;
	else
		mFixedDeltaTimeMicrosec = 0;
}

u32 SceneTime::GetFixedUpdateStep(u64& outStep)
{
	const u64 currentTime = GetTime().GetTimePrecise();

	// Skip fixed update first frame (time delta is zero, and no input received yet)
	if(mFirstFixedFrame)
	{
		mLastFixedUpdateTime = currentTime;
		mFirstFixedFrame = false;
	}

	const u64 nextFrameTime = mLastFixedUpdateTime + mFixedStep;
	if(nextFrameTime <= currentTime)
	{
		const i64 simulationAmount = (i64)std::max(currentTime - mLastFixedUpdateTime, mFixedStep); // At least one step
		auto iterationCount = (u32)Math::DivideAndRoundUp(simulationAmount, (i64)mFixedStep);

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

		auto stepMicroseconds = (i64)mFixedStep;
		if(iterationCount > mRemainingFixedUpdateCount)
		{
			stepMicroseconds = Math::DivideAndRoundUp(simulationAmount, (i64)mRemainingFixedUpdateCount);
			iterationCount = (u32)Math::DivideAndRoundUp(simulationAmount, (i64)stepMicroseconds);
		}

		B3D_ASSERT(iterationCount <= mRemainingFixedUpdateCount);

		mRemainingFixedUpdateCount -= iterationCount;
		mRemainingFixedUpdateCount = std::min(kMaximumAccumulatedFixedUpdates, mRemainingFixedUpdateCount + kNewFixedUpdatesPerFrame);

		outStep = stepMicroseconds;
		return iterationCount;
	}

	outStep = 0;
	return 0;
}

void SceneTime::AdvanceFixedUpdate(u64 step)
{
	mLastFixedUpdateTime += step;
}

namespace b3d
{
Time& GetTime()
{
	return Time::Instance();
}
} // namespace b3d
