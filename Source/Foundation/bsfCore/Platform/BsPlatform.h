//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Input/BsInputFwd.h"
#include "Math/BsVector2I.h"
#include "Math/BsRect2I.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Contains values representing default mouse cursor types. */
	enum class PlatformCursorType
	{
		Arrow,
		Wait,
		IBeam,
		Help,
		Hand,
		SizeAll,
		SizeNESW,
		SizeNS,
		SizeNWSE,
		SizeWE
	};

	/**
	 * Contains values reprenting window non client areas.
	 *
	 * @note	These are used for things like resize/move and tell the OS where each of those areas are on our window.
	 */
	enum class NonClientAreaBorderType
	{
		TopLeft,
		Top,
		TopRight,
		Left,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};

	/** Types of mouse buttons provided by the OS. */
	enum class OSMouseButton
	{
		Left, Middle, Right, Count
	};

	/** Describes pointer (mouse, touch) states as reported by the OS. */
	struct BS_CORE_EXPORT OSPointerButtonStates
	{
		OSPointerButtonStates()
		{
			mouseButtons[0] = false;
			mouseButtons[1] = false;
			mouseButtons[2] = false;

			shift = false;
			ctrl = false;
		}

		bool mouseButtons[(UINT32)OSMouseButton::Count];
		bool shift, ctrl;
	};

	/**	Represents a specific non client area used for window resizing. */
	struct BS_CORE_EXPORT NonClientResizeArea
	{
		NonClientAreaBorderType type;
		Rect2I area;
	};

	/** Contains a list of window move and resize non client areas. */
	struct BS_CORE_EXPORT WindowNonClientAreaData
	{
		Vector<NonClientResizeArea> resizeAreas;
		Vector<Rect2I> moveAreas;
	};

	/**	Provides access to various operating system functions, including the main message pump. */
	class BS_CORE_EXPORT Platform
	{
	public:
		struct Pimpl;

		Platform() { }
		virtual ~Platform();

		/**
		 * Retrieves the cursor position in screen coordinates.
		 *
		 * @note	Thread safe.
		 */
		static Vector2I getCursorPosition();

		/**
		 * Moves the cursor to the specified screen position.
		 *
		 * @note	Thread safe.
		 */
		static void setCursorPosition(const Vector2I& screenPos);

		/**
		 * Capture mouse to this window so that we get mouse input even if the mouse leaves the window area.
		 *
		 * @note	Thread safe.
		 */
		static void captureMouse(const RenderWindow& window);

		/**
		 * Releases the mouse capture set by captureMouse().
		 *
		 * @note	Thread safe.
		 */
		static void releaseMouseCapture();

		/**
		 * Checks if provided over screen position is over the specified window.
		 */
		static bool isPointOverWindow(const RenderWindow& window, const Vector2I& screenPos);

		/**
		 * Limit cursor movement to the specified window.
		 *
		 * @note	Thread safe.
		 */
		static void clipCursorToWindow(const RenderWindow& window);

		/**
		 * Clip cursor to specific area on the screen.
		 *
		 * @note	Thread safe.
		 */

		static void clipCursorToRect(const Rect2I& screenRect);

		/**
		 * Disables cursor clipping.
		 *
		 * @note	Thread safe.
		 */
		static void clipCursorDisable();

		/**
		 * Hides the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void hideCursor();

		/**
		 * Shows the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void showCursor();

		/**
		 * Query if the cursor is hidden.
		 *
		 * @note	Thread safe.
		 */
		static bool isCursorHidden();

		/**
		 * Sets a cursor using a custom image.
		 *
		 * @param[in] 	pixelData	Cursor image data.
		 * @param[in]	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *							Arrow cursor).
		 *
		 * @note	Thread safe.
		 */
		static void setCursor(PixelData& pixelData, const Vector2I& hotSpot);

		/**
		 * Sets an icon for the main application window.
		 *
		 * @param[in] 	pixelData	Icon image data. This will be resized to the required icon size, depending on platform
		 * 							implementation.
		 *
		 * @note	Thread safe.
		 */
		static void setIcon(const PixelData& pixelData);

		/**
		 * Sets custom caption non client areas for the specified window. Using custom client areas will override window 
		 * move/drag operation and trigger when user interacts with the custom area.
		 *
		 * @note	
		 * Thread safe.
		 * @note
		 * All provided areas are relative to the specified window. Mostly useful for frameless windows that don't have 
		 * typical caption bar.
		 */
		static void setCaptionNonClientAreas(const ct::RenderWindow& window, const Vector<Rect2I>& nonClientAreas);

		/**
		 * Sets custom non client areas for the specified window. Using custom client areas will override window resize 
		 * operation and trigger when user interacts with the custom area.
		 *
		 * @note	
		 * Thread safe.
		 * @note
		 * All provided areas are relative to the specified window. Mostly useful for frameless windows that don't have 
		 * typical border.
		 */
		static void setResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas);

		/**
		 * Resets the non client areas for the specified windows and allows the platform to use the default values.
		 *
		 * @note	Thread safe.
		 */
		static void resetNonClientAreas(const ct::RenderWindow& window);

		/** 
		 * Causes the current thread to pause execution for the specified amount of time.
		 *
		 * @param[in]	duration	Duration in milliseconds. Providing zero will give up the current time-slice.
		 *
		 * @note	This method relies on timer granularity being set to 1 millisecond. If it is not, you can expect
		 *			this method to potentially take significantly longer if you are providing it with low ms values (<10).
		 */
		static void sleep(UINT32 duration);

		/**
		 * Opens the provided folder using the default application, as specified by the operating system.
		 *
		 * @param[in]	path	Absolute path to the folder to open.
		 */
		static void openFolder(const Path& path);

		/**
		 * Adds a string to the clipboard.
		 *
		 * @note	Thread safe.
		 */
		static void copyToClipboard(const String& string);

		/**
		 * Reads a string from the clipboard and returns it. If there is no string in the clipboard it returns an empty
		 * string.
		 *
		 * @note
		 * Both wide and normal strings will be read, but normal strings will be converted to a wide string before returning.
		 * @note
		 * Thread safe.
		 */
		static String copyFromClipboard();

		/**
		 * Converts a keyboard key-code to a Unicode character.
		 *
		 * @note
		 * Normally this will output a single character, but it can happen it outputs multiple in case a accent/diacritic
		 * character could not be combined with the virtual key into a single character.
		 */
		static String keyCodeToUnicode(UINT32 keyCode);

		/**
		 * Message pump. Processes OS messages and returns when it's free.
		 *
		 * @note	Core thread only.
		 */
		static void _messagePump();

		/** Called during application start up from the sim thread. Must be called before any other operations are done. */
		static void _startUp();

		/** Called once per frame from the sim thread. */
		static void _update();

		/** Called once per frame from the core thread. */
		static void _coreUpdate();

		/** Called during application shut down from the sim thread. */
		static void _shutDown();

		/**
		 * Triggered whenever the pointer moves.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, const OSPointerButtonStates&)> onCursorMoved;

		/**
		 * Triggered whenever a pointer button is pressed.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> onCursorButtonPressed;

		/**
		 * Triggered whenever pointer button is released.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> onCursorButtonReleased;

		/**
		 * Triggered whenever a pointer button is double clicked.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, const OSPointerButtonStates&)> onCursorDoubleClick;

		/**
		 * Triggered whenever an input command is entered.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(InputCommandType)> onInputCommand;

		/**
		 * Triggered whenever the mouse wheel is scolled.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(float)> onMouseWheelScrolled;

		/**
		 * Triggered whenever a character is entered.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(UINT32)> onCharInput;

		/**
		 * Triggered whenever mouse capture state for the window is changed (it receives or loses it).
		 *
		 * @note	Core thread only.
		 */
		static Event<void()> onMouseCaptureChanged;
	protected:
		static Pimpl* mData;
	};

	/** @} */
}