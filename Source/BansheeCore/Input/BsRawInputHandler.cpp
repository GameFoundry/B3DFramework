//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsRawInputHandler.h"
#include "Input/BsMouse.h"
#include "Input/BsKeyboard.h"
#include "Input/BsGamepad.h"
#include "Utility/BsTime.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Math/BsMath.h"

namespace bs
{
	RawInputHandler::RawInputHandler(UINT64 windowHandle)
		: mMouseSmoothingEnabled(false), mWindowHandle(windowHandle), mMouse(nullptr), mKeyboard(nullptr)
	{
		mMouseSampleAccumulator[0] = 0;
		mMouseSampleAccumulator[1] = 0;
		mTotalMouseSamplingTime[0] = 1.0f / 125.0f; // Use 125Hz as initial pooling rate for mice
		mTotalMouseSamplingTime[1] = 1.0f / 125.0f;
		mTotalMouseNumSamples[0] = 1;
		mTotalMouseNumSamples[1] = 1;
		mMouseSmoothedAxis[0] = 0.0f;
		mMouseSmoothedAxis[1] = 0.0f;
		mMouseZeroTime[0] = 0.0f;
		mMouseZeroTime[1] = 0.0f;

		initialize();

		mTimestampClockOffset = gTime().getStartTimeMs();
	}

	RawInputHandler::~RawInputHandler()
	{
		if (mMouse != nullptr)
			bs_delete(mMouse);

		if (mKeyboard != nullptr)
			bs_delete(mKeyboard);

		for (auto& gamepad : mGamepads)
			bs_delete(gamepad);

		cleanUp();
	}

	void RawInputHandler::_update()
	{
		if (mMouse != nullptr)
			mMouse->capture();

		if (mKeyboard != nullptr)
			mKeyboard->capture();

		for (auto& gamepad : mGamepads)
			gamepad->capture();

		float rawXValue = 0.0f;
		float rawYValue = 0.0f;

		// Smooth mouse axes if needed
		if (mMouseSmoothingEnabled)
		{
			rawXValue = smoothMouse((float)mMouseSampleAccumulator[0], 0);
			rawYValue = smoothMouse((float)mMouseSampleAccumulator[1], 1);
		}
		else
		{
			rawXValue = (float)mMouseSampleAccumulator[0];
			rawYValue = (float)mMouseSampleAccumulator[1];
		}

		rawXValue *= 0.1f;
		rawYValue *= 0.1f;

		mMouseSampleAccumulator[0] = 0;
		mMouseSampleAccumulator[1] = 0;

		RawAxisState xState;
		xState.rel = -rawXValue;
		onAxisMoved(0, xState, (UINT32)InputAxis::MouseX);

		RawAxisState yState;
		yState.rel = -rawYValue;
		
		onAxisMoved(0, yState, (UINT32)InputAxis::MouseY);
	}

	void RawInputHandler::_inputWindowChanged(const RenderWindow& win)
	{
		UINT64 hWnd = 0;
		win.getCustomAttribute("WINDOW", &hWnd);

		mKeyboard->changeCaptureContext(hWnd);
		mMouse->changeCaptureContext(hWnd);

		for (auto& gamepad : mGamepads)
			gamepad->changeCaptureContext(hWnd);
	}

	void RawInputHandler::_notifyMouseMoved(INT32 relX, INT32 relY, INT32 relZ)
	{
		mMouseSampleAccumulator[0] += relX;
		mMouseSampleAccumulator[1] += relY;

		mTotalMouseNumSamples[0] += Math::roundToInt(Math::abs((float)relX));
		mTotalMouseNumSamples[1] += Math::roundToInt(Math::abs((float)relY));

		// Update sample times used for determining sampling rate. But only if something was
		// actually sampled, and only if this isn't the first non-zero sample.
		if (mLastMouseUpdateFrame != gTime().getFrameIdx())
		{
			if (relX != 0 && !Math::approxEquals(mMouseSmoothedAxis[0], 0.0f))
				mTotalMouseSamplingTime[0] += gTime().getFrameDelta();

			if (relY != 0 && !Math::approxEquals(mMouseSmoothedAxis[1], 0.0f))
				mTotalMouseSamplingTime[1] += gTime().getFrameDelta();

			mLastMouseUpdateFrame = gTime().getFrameIdx();
		}

		RawAxisState zState;
		zState.rel = (float)relZ;

		onAxisMoved(0, zState, (UINT32)InputAxis::MouseZ);
	}

	void RawInputHandler::_notifyAxisMoved(UINT32 gamepadIdx, UINT32 axisIdx, INT32 value)
	{
		// Move axis values into [-1.0f, 1.0f] range
		float axisRange = Math::abs((float)Gamepad::MAX_AXIS) + Math::abs((float)Gamepad::MIN_AXIS);

		RawAxisState axisState;
		axisState.rel = ((value + Math::abs((float)Gamepad::MIN_AXIS)) / axisRange) * 2.0f - 1.0f;

		onAxisMoved(gamepadIdx, axisState, axisIdx);
	}

	void RawInputHandler::_notifyButtonPressed(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		onButtonDown(deviceIdx, code, timestamp - mTimestampClockOffset);
	}

	void RawInputHandler::_notifyButtonReleased(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		onButtonUp(deviceIdx, code, timestamp - mTimestampClockOffset);
	}

	float RawInputHandler::smoothMouse(float value, UINT32 idx)
	{
		UINT32 sampleCount = 1;

		float deltaTime = gTime().getFrameDelta();
		if (deltaTime < 0.25f)
		{
			float secondsPerSample = mTotalMouseSamplingTime[idx] / mTotalMouseNumSamples[idx];

			if (value == 0.0f)
			{
				mMouseZeroTime[idx] += deltaTime;
				if (mMouseZeroTime[idx] < secondsPerSample)
					value = mMouseSmoothedAxis[idx] * deltaTime / secondsPerSample;
				else
					mMouseSmoothedAxis[idx] = 0;
			}
			else
			{
				mMouseZeroTime[idx] = 0;
				if (mMouseSmoothedAxis[idx] != 0)
				{
					if (deltaTime < secondsPerSample * (sampleCount + 1))
						value = value * deltaTime / (secondsPerSample * sampleCount);
					else
						sampleCount = Math::roundToInt(deltaTime / secondsPerSample);
				}

				mMouseSmoothedAxis[idx] = value / sampleCount;
			}
		}
		else
		{
			mMouseSmoothedAxis[idx] = 0.0f;
			mMouseZeroTime[idx] = 0.0f;
		}

		return value;
	}
}
