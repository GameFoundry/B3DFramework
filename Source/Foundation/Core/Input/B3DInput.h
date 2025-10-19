//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Platform/B3DPlatform.h"
#include "Input/B3DInputFwd.h"

namespace b3d
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) Input : public Module<Input>
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

			Vector<float> Axes;
			ButtonState KeyStates[static_cast<size_t>(ButtonCode::TotalKeyCount)];
		};

		/**	Different types of possible input event callbacks. */
		enum class EventType
		{
			ButtonUp,
			ButtonDown,
			PointerMoved,
			PointerUp,
			PointerDown,
			PointerDoubleClick,
			TextInput,
			Command
		};

		/**	Stores information about a queued input event that is to be triggered later. */
		struct QueuedEvent
		{
			QueuedEvent(EventType type, u32 eventIndex)
				: Type(type), Index(eventIndex)
			{}

			EventType Type;
			u32 Index;
		};

	public:
		Input();
		~Input();

		/**
		 * Returns value of the specified input axis. Normally in range [-1.0, 1.0] but can be outside the range for
		 * devices with unbound axes (for example mouse).
		 *
		 * @param	type			Type of axis to query. Usually a type from InputAxis but can be a custom value.
		 * @param	deviceIndex		Index of the device in case more than one is hooked up (0 - primary).
		 */
		B3D_SCRIPT_EXPORT()
		float GetAxisValue(u32 type, u32 deviceIndex = 0) const;

		/**
		 * Query if the provided button is currently being held (this frame or previous frames).
		 *
		 * @param	keyCode			Code of the button to query.
		 * @param	deviceIndex		Device to query the button on (0 - primary).
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonHeld(ButtonCode keyCode, u32 deviceIndex = 0) const;

		/**
		 * Query if the provided button is currently being released (only true for one frame).
		 *
		 * @param	keyCode			Code of the button to query.
		 * @param	deviceIndex		Device to query the button on (0 - primary).
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonUp(ButtonCode keyCode, u32 deviceIndex = 0) const;

		/**
		 * Query if the provided button is currently being pressed (only true for one frame).
		 *
		 * @param	keyCode			Code of the button to query.
		 * @param	deviceIndex		Device to query the button on (0 - primary).
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonDown(ButtonCode keyCode, u32 deviceIndex = 0) const;

		/** Returns position of the pointer (for example mouse cursor) relative to the screen. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(PointerPosition))
		Vector2I GetPointerPosition() const;

		/** Returns difference between pointer position between current and last frame. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(PointerDelta))
		Vector2I GetPointerDelta() const { return mPointerDelta; }

		/**
		 * Query if the provided pointer button is currently being held (this frame or previous frames).
		 *
		 * @param	pointerButton	Code of the button to query.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsPointerButtonHeld(PointerEventButton pointerButton) const;

		/**
		 * Query if the provided pointer button is currently being released (only true for one frame).
		 *
		 * @param	pointerButton	Code of the button to query.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsPointerButtonUp(PointerEventButton pointerButton) const;

		/**
		 * Query if the provided pointer button is currently being pressed (only true for one frame).
		 *
		 * @param	pointerButton	Code of the button to query.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsPointerButtonDown(PointerEventButton pointerButton) const;

		/** Query has the left pointer button has been double-clicked this frame. */
		B3D_SCRIPT_EXPORT()
		bool IsPointerDoubleClicked() const;

		/** Enables or disables mouse smoothing. Smoothing makes the changes to mouse axes more gradual. */
		B3D_SCRIPT_EXPORT()
		void SetMouseSmoothing(bool enabled);

		/** Returns the number of detected devices of the specified type. */
		u32 GetDeviceCount(InputDevice device) const;

		/** Returns the name of a specific input device. Returns empty string if the device doesn't exist. */
		String GetDeviceName(InputDevice type, u32 deviceIndex);

		/** Triggered whenever a button is first pressed. */
		B3D_SCRIPT_EXPORT()
		Event<void(const ButtonEvent&)> OnButtonDown;

		/**	Triggered whenever a button is first released. */
		B3D_SCRIPT_EXPORT()
		Event<void(const ButtonEvent&)> OnButtonUp;

		/**	Triggered whenever user inputs a text character. */
		B3D_SCRIPT_EXPORT()
		Event<void(const TextInputEvent&)> OnCharInput;

		/**	Triggers when some pointing device (mouse cursor, touch) moves. */
		B3D_SCRIPT_EXPORT()
		Event<void(const PointerEvent&)> OnPointerMoved;

		/**	Triggers when some pointing device (mouse cursor, touch) button is pressed. */
		B3D_SCRIPT_EXPORT()
		Event<void(const PointerEvent&)> OnPointerPressed;

		/**	Triggers when some pointing device (mouse cursor, touch) button is released. */
		B3D_SCRIPT_EXPORT()
		Event<void(const PointerEvent&)> OnPointerReleased;

		/**	Triggers when some pointing device (mouse cursor, touch) button is double clicked. */
		B3D_SCRIPT_EXPORT()
		Event<void(const PointerEvent&)> OnPointerDoubleClick;

		// TODO Low priority: Remove this, I can emulate it using virtual input
		/**	Triggers on special input commands. */
		Event<void(InputCommandType)> OnInputCommand;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Called every frame. Detects button state changes and prepares callback events to trigger via a call to
		 * TriggerCallbacksInternal().
		 */
		void UpdateInternal();

		/** Triggers any queued input event callbacks. */
		void TriggerCallbacksInternal();

		/** Returns internal, platform specific privata data. */
		InputPrivateData* GetPrivateDataInternal() const { return mPlatformData; }

		/** Returns a handle to the window that is currently receiving input. */
		u64 GetWindowHandle() const { return mWindowHandle; }

		/** Called by Mouse when mouse movement is detected. */
		void NotifyMouseMovedInternal(i32 relativeX, i32 relativeY, i32 relativeZ);

		/** Called by any of the raw input devices when analog axis movement is detected. */
		void NotifyAxisMovedInternal(u32 gamepadIndex, u32 axisIndex, i32 value);

		/** Called by any of the raw input devices when a button is pressed. */
		void NotifyButtonPressedInternal(u32 deviceIndex, ButtonCode code, u64 timestamp);

		/** Called by any of the raw input devices when a button is released. */
		void NotifyButtonReleasedInternal(u32 deviceIndex, ButtonCode code, u64 timestamp);

		/** @} */

	private:
		/** Performs platform specific raw input system initialization. */
		void InitRawInput();

		/** Performs platform specific raw input system cleanup. */
		void CleanUpRawInput();

		/**
		 * Smooths the input mouse axis value. Smoothing makes the changes to the axis more gradual depending on previous
		 * values.
		 *
		 * @param	value		Value to smooth.
		 * @param	axisIndex	Index of the mouse axis to smooth, 0 - horizontal, 1 - vertical.
		 * @return				Smoothed value.
		 */
		float SmoothMouse(float value, u32 axisIndex);

		/**	Triggered by input handler when a button is pressed. */
		void ButtonDown(u32 deviceIndex, ButtonCode code, u64 timestamp);

		/**	Triggered by input handler when a button is released. */
		void ButtonUp(u32 deviceIndex, ButtonCode code, u64 timestamp);

		/**	Triggered by input handler when a mouse/joystick axis is moved. */
		void AxisMoved(u32 deviceIndex, float value, u32 axis);

		/**
		 * Called from the message loop to notify user has entered a character.
		 *
		 * @see		OnCharInput
		 */
		void CharInput(u32 character);

		/**
		 * Called from the message loop to notify user has moved the cursor.
		 *
		 * @see		OnPointerMoved
		 */
		void CursorMoved(const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates);

		/**
		 * Called from the message loop to notify user has pressed a mouse button.
		 *
		 * @see		OnPointerPressed
		 */
		void CursorPressed(const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates);

		/**
		 * Called from the message loop to notify user has released a mouse button.
		 *
		 * @see		OnPointerReleased
		 */
		void CursorReleased(const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates);

		/**
		 * Called from the message loop to notify user has double-clicked a mouse button.
		 *
		 * @see		OnPointerDoubleClick
		 */
		void CursorDoubleClick(const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates);

		/**
		 * Called from the message loop to notify user has entered an input command.
		 *
		 * @see		OnInputCommand
		 */
		void InputCommandEntered(InputCommandType commandType);

		/**
		 * Called from the message loop to notify user has scrolled the mouse wheel.
		 *
		 * @see		OnPointerMoved
		 */
		void MouseWheelScrolled(float scrollPos);

		/** Called when window in focus changes, as reported by the OS. */
		void InputWindowChanged(RenderWindow& win);

		/**
		 * Called when the current window loses input focus. This might be followed by inputWindowChanged() if the focus
		 * just switched to another of this application's windows.
		 */
		void InputFocusLost();

	private:
		Mutex mMutex;

		Vector<DeviceData> mDevices;
		Vector2I mLastPointerPosition{BsZero};
		Vector2I mPointerDelta{BsZero};
		ButtonState mPointerButtonStates[3];
		bool mPointerDoubleClicked = false;
		bool mLastPositionSet = false;

		// Thread safe
		Vector2I mPointerPosition{BsZero};
		float mMouseScroll = 0.0f;
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
		bool mMouseSmoothingEnabled = false;
		u64 mWindowHandle;

		Mouse* mMouse = nullptr;
		Keyboard* mKeyboard = nullptr;
		Vector<Gamepad*> mGamepads;

		float mTotalMouseSamplingTime[2];
		u32 mTotalMouseSampleCount[2];
		float mMouseZeroTime[2];
		i32 mMouseSampleAccumulator[2];
		float mMouseSmoothedAxis[2];
		u64 mLastMouseUpdateFrame;

		u64 mTimestampClockOffset;

		InputPrivateData* mPlatformData;

		/************************************************************************/
		/* 								STATICS		                      		*/
		/************************************************************************/
		static const int kHistoryBufferSize; // Size of buffer used for input smoothing
		static const float kWeightModifier;
	};

	/** Provides global access to Input. */
	B3D_EXPORT Input& GetInput();

	/** @} */
} // namespace b3d
