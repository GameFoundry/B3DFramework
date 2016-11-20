//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPlatform.h"
#include "BsEvent.h"
#include "BsVector2I.h"

namespace bs
{
	/** @addtogroup Input-Internal
	 *  @{
	 */

	/**
	 * Represents a specific way of acquiring OS input. Input class (which provides a higher level input) must have at 
	 * least one OSInputHandler attached. Attach events handler to the provided signals to handle input.
	 * 			
	 * @note		
	 * Unlike RawInputHandler this class receives input from the operating system, and is used for receiving text input, 
	 * cursor position and similar.
	 */
	class BS_CORE_EXPORT OSInputHandler
	{
		/**	Contains information regarding a button state change event. */
		struct ButtonStateChange
		{
			Vector2I cursorPos;
			OSPointerButtonStates btnStates;
			OSMouseButton button;
			bool pressed;
		};

		/**	Contains information regarding a double click event. */
		struct DoubleClick
		{
			Vector2I cursorPos;
			OSPointerButtonStates btnStates;
		};

	public:
		OSInputHandler();
		virtual ~OSInputHandler();

		/** Called once per frame. Capture input here if needed. */
		virtual void _update();

		/**
		 * Called whenever the active window changes.
		 *
		 * @param[in]	win	Newly active window.
		 */
		virtual void _inputWindowChanged(const RenderWindow& win) { }

		/**
		 * Triggers when user inputs a character. The character might be a result of pressing multiple keys, so character 
		 * input will not necessarily correspond with button presses. Provide character code of the input character.
		 */
		Event<void(UINT32)> onCharInput;

		/**
		 * Triggers whenever user scrolls the mouse wheel. Returns the screen position of the mouse cursor and delta amount 
		 * of mouse scroll (can be negative or positive).
		 */
		Event<void(const Vector2I&, float)> onMouseWheelScrolled;

		/** Triggers whenever user moves the mouse cursor. */
		Event<void(const PointerEvent&)> onCursorMoved;

		/**	Triggers whenever user presses one of the mouse buttons. */
		Event<void(const PointerEvent&)> onCursorPressed;

		/**	Triggers whenever user releases one of the mouse buttons. */
		Event<void(const PointerEvent&)> onCursorReleased;

		/**	Triggers when user clicks a mouse button quickly twice in a row. */
		Event<void(const PointerEvent&)> onDoubleClick;

		/**	Triggers when user inputa a special input command, like commands user for manipulating text input. */
		Event<void(InputCommandType)> onInputCommand;

	private:
		Mutex mOSInputMutex;
		Vector2I mLastCursorPos;
		Vector2I mCursorPosition;
		Vector2I mDelta;
		bool mLastCursorPosSet;
		float mMouseScroll;
		WString mInputString;
		Queue<ButtonStateChange> mButtonStates;
		Queue<DoubleClick> mDoubleClicks;
		Queue<InputCommandType> mInputCommands;
		OSPointerButtonStates mMouseMoveBtnState;

		HEvent mCharInputConn;
		HEvent mCursorMovedConn;
		HEvent mCursorPressedConn;
		HEvent mCursorReleasedConn;
		HEvent mCursorDoubleClickConn;
		HEvent mInputCommandConn;
		HEvent mMouseWheelScrolledConn;

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
	};

	/** @} */
}