//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOISPrerequisites.h"
#include "BsRawInputHandler.h"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include <OIS/OISJoystick.h>

namespace bs
{
	/** @addtogroup OISInput
	 *  @{
	 */

	class InputHandlerOIS;

	/**	Listens for events from a specific OIS joystick device. */
	class BS_OIS_EXPORT GamepadEventListener : public OIS::JoyStickListener
	{
	public:
		GamepadEventListener(InputHandlerOIS* parentHandler, UINT32 joystickIdx);

		/**	Called by OIS whenever a gamepad/joystick button is pressed. */
		virtual bool buttonPressed(const OIS::JoyStickEvent& arg, int button) override;

		/**	Called by OIS whenever a gamepad/joystick button is released. */
		virtual bool buttonReleased(const OIS::JoyStickEvent& arg, int button) override;

		/**	Called by OIS whenever a gamepad/joystick axis is moved. */
		virtual bool axisMoved(const OIS::JoyStickEvent& arg, int axis) override;

	private:
		UINT32 mGamepadIdx;
		InputHandlerOIS* mParentHandler;
	};

	/**	Raw input handler using OIS library for acquiring input. */
	class BS_OIS_EXPORT InputHandlerOIS : public RawInputHandler, public OIS::KeyListener, 
		public OIS::MouseListener
	{
		/**	Holding data about an active gamepad object. */
		struct GamepadData
		{
			OIS::JoyStick* gamepad;
			GamepadEventListener* listener;
		};

	public:
		InputHandlerOIS(unsigned int hWnd);
		virtual ~InputHandlerOIS();

	private:
		/**	Called by OIS whenever a keyboard button is pressed. */
		virtual bool keyPressed(const OIS::KeyEvent& arg) override;

		/**	Called by OIS whenever a keyboard button is released. */
		virtual bool keyReleased(const OIS::KeyEvent& arg) override;

		/**	Called by OIS whenever mouse is moved. */
		virtual bool mouseMoved(const OIS::MouseEvent& arg) override;

		/**	Called by OIS whenever is a mouse button pressed. */
		virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id) override;

		/**	Called by OIS whenever is a mouse button released. */
		virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id) override;

		/**	Converts an OIS key code into engine button code. */
		static ButtonCode keyCodeToButtonCode(OIS::KeyCode keyCode);

		/**	Converts an OIS mouse button code into engine button code. */
		static ButtonCode mouseButtonToButtonCode(OIS::MouseButtonID mouseBtn);

		/**	Converts an OIS gamepad button code into engine button code. */
		static ButtonCode gamepadButtonToButtonCode(INT32 joystickCode);

		/** @name Internal 
		 *  @{
		 */

		/** Called once per frame. */
		virtual void _update() override;

		/** Called whenever the currently focused window changes. */
		virtual void _inputWindowChanged(const RenderWindow& win) override;

		/** @} */

	private:
		friend class GamepadEventListener;

		/**
		 * Smooths the input mouse axis value. Smoothing makes the changes to the axis more gradual depending on previous
		 * values.
		 *
		 * @param[in]	value	Value to smooth.
		 * @param[in]	idx		Index of the mouse axis to smooth, 0 - horizontal, 1 - vertical.
		 * @return				Smoothed value.
		 */
		float smoothMouse(float value, UINT32 idx);

		OIS::InputManager* mInputManager;
		OIS::Mouse*	mMouse;
		OIS::Keyboard* mKeyboard;
		Vector<GamepadData> mGamepads;

		float mTotalMouseSamplingTime[2];
		UINT32 mTotalMouseNumSamples[2];
		float mMouseZeroTime[2];
		INT32 mMouseSampleAccumulator[2];
		float mMouseSmoothedAxis[2];
		UINT64 mLastMouseUpdateFrame;

		UINT64 mTimestampClockOffset;
	};

	/** @} */
}