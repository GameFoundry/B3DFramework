//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Platform/BsPlatform.h"
#include "Input/BsInputFwd.h"

namespace bs
{
	class Mouse;
	class Keyboard;
	class Gamepad;
	struct InputPrivateData;

	/** @addtogroup Input
	 *  @{
	 */

	/**
	 * Primary module used for dealing with input. Allows you to receieve and query raw or OS input for 
	 * mouse/keyboard/gamepad.
	 */
	class BS_CORE_EXPORT Input : public Module<Input>
	{
		/** Possible button states. */
		enum class ButtonState
		{
			Off, /**< Button is not being pressed. */
			On, /**< Button is being pressed. */
			ToggledOn, /**< Button has been pressed this frame. */
			ToggledOff, /**< Button has been released this frame. */
			ToggledOnOff, /**< Button has been pressed and released this frame. */
		};

		/** Contains axis and device data per device. */
		struct DeviceData
		{
			DeviceData();

			Vector<float> axes;
			ButtonState keyStates[BC_Count];
		};

		/**	Different types of possible input event callbacks. */
		enum class EventType
		{
			ButtonUp, ButtonDown, PointerMoved, PointerUp, PointerDown, PointerDoubleClick, TextInput, Command
		};

		/**	Stores information about a queued input event that is to be triggered later. */
		struct QueuedEvent
		{
			QueuedEvent(EventType type, UINT32 idx)
				:type(type), idx(idx)
			{ }

			EventType type;
			UINT32 idx;
		};

	public:
		Input();
		~Input();

		/**
		 * Returns value of the specified input axis. Normally in range [-1.0, 1.0] but can be outside the range for 
		 * devices with unbound axes (for example mouse).
		 *
		 * @param[in]	type		Type of axis to query. Usually a type from InputAxis but can be a custom value.
		 * @param[in]	deviceIdx	Index of the device in case more than one is hooked up (0 - primary).
		 */
		float getAxisValue(UINT32 type, UINT32 deviceIdx = 0) const;

		/**
		 * Query if the provided button is currently being held (this frame or previous frames).
		 *
		 * @param[in]	keyCode		Code of the button to query.
		 * @param[in]	deviceIdx	Device to query the button on (0 - primary).
		 */
		bool isButtonHeld(ButtonCode keyCode, UINT32 deviceIdx = 0) const;

		/**
		 * Query if the provided button is currently being released (only true for one frame).
		 *
		 * @param[in]	keyCode		Code of the button to query.
		 * @param[in]	deviceIdx	Device to query the button on (0 - primary).
		 */
		bool isButtonUp(ButtonCode keyCode, UINT32 deviceIdx = 0) const;

		/**
		 * Query if the provided button is currently being pressed (only true for one frame).
		 *
		 * @param[in]	keyCode		Code of the button to query.
		 * @param[in]	deviceIdx	Device to query the button on (0 - primary).
		 */
		bool isButtonDown(ButtonCode keyCode, UINT32 deviceIdx = 0) const;

		/** Returns position of the pointer (for example mouse cursor) relative to the screen. */
		Vector2I getPointerPosition() const;

		/** Returns difference between pointer position between current and last frame. */
		Vector2I getPointerDelta() const { return mPointerDelta; }

		/**
		 * Query if the provided pointer button is currently being held (this frame or previous frames).
		 *
		 * @param[in]	pointerButton		Code of the button to query.
		 */
		bool isPointerButtonHeld(PointerEventButton pointerButton) const;

		/**
		 * Query if the provided pointer button is currently being released (only true for one frame).
		 *
		 * @param[in]	pointerButton		Code of the button to query.
		 */
		bool isPointerButtonUp(PointerEventButton pointerButton) const;

		/**
		 * Query if the provided pointer button is currently being pressed (only true for one frame).
		 *
		 * @param[in]	pointerButton		Code of the button to query.
		 */
		bool isPointerButtonDown(PointerEventButton pointerButton) const;

		/** Query has the left pointer button has been double-clicked this frame. */
		bool isPointerDoubleClicked() const;

		/** Enables or disables mouse smoothing. Smoothing makes the changes to mouse axes more gradual. */
		void setMouseSmoothing(bool enabled);

		/** Returns the number of detected devices of the specified type. */
		UINT32 getDeviceCount(InputDevice device) const;

		/** Returns the name of a specific input device. Returns empty string if the device doesn't exist. */
		String getDeviceName(InputDevice type, UINT32 idx);

		/** Triggered whenever a button is first pressed. */
		Event<void(const ButtonEvent&)> onButtonDown;

		/**	Triggered whenever a button is first released. */
		Event<void(const ButtonEvent&)> onButtonUp;

		/**	Triggered whenever user inputs a text character. */
		Event<void(const TextInputEvent&)> onCharInput;

		/**	Triggers when some pointing device (mouse cursor, touch) moves. */
		Event<void(const PointerEvent&)> onPointerMoved;

		/**	Triggers when some pointing device (mouse cursor, touch) button is pressed. */
		Event<void(const PointerEvent&)> onPointerPressed;

		/**	Triggers when some pointing device (mouse cursor, touch) button is released. */
		Event<void(const PointerEvent&)> onPointerReleased;

		/**	Triggers when some pointing device (mouse cursor, touch) button is double clicked. */
		Event<void(const PointerEvent&)> onPointerDoubleClick;

		// TODO Low priority: Remove this, I can emulate it using virtual input
		/**	Triggers on special input commands. */
		Event<void(InputCommandType)> onInputCommand;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Called every frame. Detects button state changes and prepares callback events to trigger via a call to 
		 * _triggerCallbacks().
		 */
		void _update();

		/** Triggers any queued input event callbacks. */
		void _triggerCallbacks();

		/** Returns internal, platform specific privata data. */
		InputPrivateData* _getPrivateData() const { return mPlatformData; }

		/** Returns a handle to the window that is currently receiving input. */
		UINT64 _getWindowHandle() const { return mWindowHandle; }

		/** Called by Mouse when mouse movement is detected. */
		void _notifyMouseMoved(INT32 relX, INT32 relY, INT32 relZ);

		/** Called by any of the raw input devices when analog axis movement is detected. */
		void _notifyAxisMoved(UINT32 gamepadIdx, UINT32 axisIdx, INT32 value);

		/** Called by any of the raw input devices when a button is pressed. */
		void _notifyButtonPressed(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/** Called by any of the raw input devices when a button is released. */
		void _notifyButtonReleased(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/** @} */

	private:
		/** Performs platform specific raw input system initialization. */
		void initRawInput();

		/** Performs platform specific raw input system cleanup. */
		void cleanUpRawInput();
		
		/**
		 * Smooths the input mouse axis value. Smoothing makes the changes to the axis more gradual depending on previous
		 * values.
		 *
		 * @param[in]	value	Value to smooth.
		 * @param[in]	idx		Index of the mouse axis to smooth, 0 - horizontal, 1 - vertical.
		 * @return				Smoothed value.
		 */
		float smoothMouse(float value, UINT32 idx);

		/**	Triggered by input handler when a button is pressed. */
		void buttonDown(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/**	Triggered by input handler when a button is released. */
		void buttonUp(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/**	Triggered by input handler when a mouse/joystick axis is moved. */
		void axisMoved(UINT32 deviceIdx, float value, UINT32 axis);

		/**
		 * Called from the message loop to notify user has entered a character.
		 * 			
		 * @see		onCharInput
		 */
		void charInput(UINT32 character);

		/**
		 * Called from the message loop to notify user has moved the cursor.
		 * 			
		 * @see		onCursorMoved
		 */
		void cursorMoved(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates);

		/**
		 * Called from the message loop to notify user has pressed a mouse button.
		 * 			
		 * @see		onCursorPressed
		 */
		void cursorPressed(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates);

		/**
		 * Called from the message loop to notify user has released a mouse button.
		 * 			
		 * @see		onCursorReleased
		 */
		void cursorReleased(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates);

		/**
		 * Called from the message loop to notify user has double-clicked a mouse button.
		 * 
		 * @see		onDoubleClick
		 */
		void cursorDoubleClick(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates);

		/**
		 * Called from the message loop to notify user has entered an input command.
		 * 			
		 * @see		onInputCommand
		 */
		void inputCommandEntered(InputCommandType commandType);

		/**
		 * Called from the message loop to notify user has scrolled the mouse wheel.
		 * 			
		 * @see		onMouseWheelScrolled
		 */
		void mouseWheelScrolled(float scrollPos);

		/** Called when window in focus changes, as reported by the OS. */
		void inputWindowChanged(RenderWindow& win);

	private:
		Mutex mMutex;

		Vector<DeviceData> mDevices;
		Vector2I mLastPointerPosition;
		Vector2I mPointerDelta;
		ButtonState mPointerButtonStates[3];
		bool mPointerDoubleClicked;
		bool mLastPositionSet;

		// Thread safe
		Vector2I mPointerPosition;
		float mMouseScroll;
		OSPointerButtonStates mPointerState;

		Vector<QueuedEvent> mQueuedEvents[2];

		Vector<TextInputEvent> mTextInputEvents[2];
		Vector<InputCommandType> mCommandEvents[2];
		Vector<PointerEvent> mPointerDoubleClickEvents[2];
		Vector<PointerEvent> mPointerReleasedEvents[2];
		Vector<PointerEvent> mPointerPressedEvents[2];

		Vector<ButtonEvent> mButtonDownEvents[2];
		Vector<ButtonEvent> mButtonUpEvents[2];

		// OS input events
		HEvent mCharInputConn;
		HEvent mCursorMovedConn;
		HEvent mCursorPressedConn;
		HEvent mCursorReleasedConn;
		HEvent mCursorDoubleClickConn;
		HEvent mInputCommandConn;
		HEvent mMouseWheelScrolledConn;

		// Raw input
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

		/************************************************************************/
		/* 								STATICS		                      		*/
		/************************************************************************/
		static const int HISTORY_BUFFER_SIZE; // Size of buffer used for input smoothing
		static const float WEIGHT_MODIFIER;
	};

	/** Provides global access to Input. */
	BS_CORE_EXPORT Input& gInput();

	/** @} */
}