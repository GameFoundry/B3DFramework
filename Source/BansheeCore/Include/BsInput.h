//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsOSInputHandler.h"
#include "BsRawInputHandler.h"
#include "BsInputFwd.h"

namespace bs
{
	/** @addtogroup Input
	 *  @{
	 */

	/**
	 * Primary module used for dealing with input. Allows you to receieve and query raw or OS input for 
	 * mouse/keyboard/gamepad.
	 *
	 * All inputs are received through an input handler, which can be overriden to provide custom input functionality.
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

			Vector<RawAxisState> axes;
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

		/** Registers a new input handler. Replaces any previous input handler. */
		void _registerRawInputHandler(SPtr<RawInputHandler> inputHandler);

		/**
		 * Called every frame. Detects button state changes and prepares callback events to trigger via a call to 
		 * _triggerCallbacks().
		 */
		void _update();

		/** Triggers any queued input event callbacks. */
		void _triggerCallbacks();

		/** @} */

	private:
		/**	Triggered by input handler when a button is pressed. */
		void buttonDown(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/**	Triggered by input handler when a button is released. */
		void buttonUp(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp);

		/**	Triggered by input handler when a single character is input. */
		void charInput(UINT32 chr);

		/**	Triggered by input handler when a mouse/joystick axis is moved. */
		void axisMoved(UINT32 deviceIdx, const RawAxisState& state, UINT32 axis);

		/**	Cursor movement as OS reports it. Used for screen cursor position. */
		void cursorMoved(const PointerEvent& event);

		/**	Cursor button presses as OS reports it. */
		void cursorPressed(const PointerEvent& event);

		/**	Cursor button releases as OS reports it. */
		void cursorReleased(const PointerEvent& event);
		
		/**	Cursor button releases as OS reports it. */
		void cursorDoubleClick(const PointerEvent& event);

		/** Input commands as OS reports them. */
		void inputCommandEntered(InputCommandType commandType);

		/** Called when window in focus changes, as reported by the OS. */
		void inputWindowChanged(RenderWindow& win);

	private:
		SPtr<RawInputHandler> mRawInputHandler;
		SPtr<OSInputHandler> mOSInputHandler;

		Vector<DeviceData> mDevices;
		Vector2I mPointerPosition;
		Vector2I mPointerDelta;
		ButtonState mPointerButtonStates[3];
		bool mPointerDoubleClicked;
		bool mLastPositionSet;

		Vector<QueuedEvent> mQueuedEvents;

		Vector<TextInputEvent> mTextInputEvents;
		Vector<InputCommandType> mCommandEvents;
		Vector<PointerEvent> mPointerDoubleClickEvents;
		Vector<PointerEvent> mPointerReleasedEvents;
		Vector<PointerEvent> mPointerPressedEvents;
		Vector<PointerEvent> mPointerMovedEvents;

		Vector<ButtonEvent> mButtonDownEvents;
		Vector<ButtonEvent> mButtonUpEvents;

		/************************************************************************/
		/* 								STATICS		                      		*/
		/************************************************************************/
		static const int HISTORY_BUFFER_SIZE; // Size of buffer used for input smoothing
		static const float WEIGHT_MODIFIER;
	};

	/** Provides easier access to Input. */
	BS_CORE_EXPORT Input& gInput();

	/** @} */
}