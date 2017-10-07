//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Input/BsInputFwd.h"
#include "Utility/BsEvent.h"

namespace bs
{
	class Mouse;
	class Keyboard;
	class Gamepad;
	struct InputPrivateData;

	/** @addtogroup Input-Internal
	 *  @{
	 */

	/**
	 * Contains relative position of an input axis. Relative state represents the difference between current and last state.
	 */
	struct RawAxisState
	{
		RawAxisState()
		 :rel(0.0f)
		{ }

		float rel;
	};

	/**
	 * Represents a specific way of acquiring low-level input. Input class (which provides a higher level input) must have 
	 * at least one RawInputHandler attached. Raw input handlers receive input as sent by the hardware without OS 
	 * modifications.
	 */
	class BS_CORE_EXPORT RawInputHandler
	{
	public:
		RawInputHandler(UINT64 windowHandle);
		virtual ~RawInputHandler();

		/** Enables or disables mouse smoothing. Smoothing makes the changes to mouse axes more gradual. */
		void setMouseSmoothing(bool enabled) { mMouseSmoothingEnabled = enabled; }

		/** Returns the number of detected devices of the specified type. */
		UINT32 getDeviceCount(InputDevice device) const;

		/**
		 * Triggered when user presses a button. Parameters include device index, button code of the pressed button, 
		 * and a timestamp of the button press event.
		 */
		Event<void(UINT32, ButtonCode, UINT64)> onButtonDown;

		/**
		 * Triggered when user releases a button. Parameters include device index, button code of the released button, 
		 * and a timestamp of the button release event.
		 */
		Event<void(UINT32, ButtonCode, UINT64)> onButtonUp;

		/**
		 * Triggered whenever the specified axis state changes. Parameters include device index, axis state data, and axis 
		 * type.
		 */
		Event<void(UINT32, const RawAxisState&, UINT32)> onAxisMoved;

		/** Called once per frame. Capture input here if needed. */
		void _update();

		/**
		 * Called whenever the active window changes.
		 *
		 * @param[in]	win	Newly active window.
		 */
		void _inputWindowChanged(const RenderWindow& win);

		/** Returns internal, platform specific privata data. */
		InputPrivateData* _getPrivateData() const { return mPlatformData; }

		/** Returns a handle to the window that is currently receiving input. */
		UINT64 _getWindowHandle() const { return mWindowHandle; }

		/** Called by Mouse when mouse movement is detected. */
		void _notifyMouseMoved(INT32 relX, INT32 relY, INT32 relZ);

		/** Called by any of the devices when analog axis movement is detected. */
		void _notifyAxisMoved(UINT32 gamepadIdx, UINT32 axisIdx, INT32 value);

		/** Called by any of the devices when a button is pressed. */
		void _notifyButtonPressed(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/** Called by any of the devices when a button is released. */
		void _notifyButtonReleased(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);
	protected:
		/** Performs platform specific input system initialization. */
		void initialize();

		/** Performs platform specific input system cleanup. */
		void cleanUp();

		/**
		 * Smooths the input mouse axis value. Smoothing makes the changes to the axis more gradual depending on previous
		 * values.
		 *
		 * @param[in]	value	Value to smooth.
		 * @param[in]	idx		Index of the mouse axis to smooth, 0 - horizontal, 1 - vertical.
		 * @return				Smoothed value.
		 */
		float smoothMouse(float value, UINT32 idx);

		bool mMouseSmoothingEnabled;
		UINT64 mWindowHandle;

		Mouse* mMouse;
		Keyboard* mKeyboard;
		Vector<Gamepad*> mGamepads;

		float mTotalMouseSamplingTime[2];
		UINT32 mTotalMouseNumSamples[2];
		float mMouseZeroTime[2];
		INT32 mMouseSampleAccumulator[2];
		float mMouseSmoothedAxis[2];
		UINT64 mLastMouseUpdateFrame;

		UINT64 mTimestampClockOffset;

		InputPrivateData* mPlatformData;
	};

	/** @} */
}