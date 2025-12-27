//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"
#include "Math/B3DArea2.h"
#include "Utility/B3DEvent.h"

namespace b3d
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
		Left,
		Middle,
		Right,
		Count
	};

	/** Describes pointer (mouse, touch) states as reported by the OS. */
	struct B3D_EXPORT OSPointerButtonStates
	{
		OSPointerButtonStates()
		{
			MouseButtons[0] = false;
			MouseButtons[1] = false;
			MouseButtons[2] = false;

			Shift = false;
			Ctrl = false;
		}

		bool MouseButtons[(u32)OSMouseButton::Count];
		bool Shift, Ctrl;
	};

	/**	Represents a specific non client area used for window resizing. */
	struct B3D_EXPORT NonClientResizeArea
	{
		NonClientAreaBorderType Type;
		Area2I Area;
	};

	/** Contains a list of window move and resize non client areas. */
	struct B3D_EXPORT WindowNonClientAreaData
	{
		Vector<NonClientResizeArea> ResizeAreas;
		Vector<Area2I> MoveAreas;
	};

	/**	Provides access to various operating system functions, including the main message pump. */
	class B3D_EXPORT Platform
	{
	public:
		struct Private;

		Platform() {}

		virtual ~Platform();

		/**
		 * Retrieves the cursor position in screen coordinates.
		 *
		 * @note	Thread safe.
		 */
		static Vector2I GetCursorPosition();

		/**
		 * Moves the cursor to the specified screen position.
		 *
		 * @note	Thread safe.
		 */
		static void SetCursorPosition(const Vector2I& screenPos);

		/**
		 * Capture mouse to this window so that we get mouse input even if the mouse leaves the window area.
		 *
		 * @note	Thread safe.
		 */
		static void CaptureMouse(const RenderWindow& window);

		/**
		 * Releases the mouse capture set by captureMouse().
		 *
		 * @note	Thread safe.
		 */
		static void ReleaseMouseCapture();

		/**
		 * Checks if provided over screen position is over the specified window.
		 */
		static bool IsPointOverWindow(const RenderWindow& window, const Vector2I& screenPos);

		/**
		 * Limit cursor movement to the specified window.
		 *
		 * @note	Thread safe.
		 */
		static void ClipCursorToWindow(const RenderWindow& window);

		/**
		 * Clip cursor to specific area on the screen.
		 *
		 * @note	Thread safe.
		 */

		static void ClipCursorToRect(const Area2I& screenRect);

		/**
		 * Disables cursor clipping.
		 *
		 * @note	Thread safe.
		 */
		static void ClipCursorDisable();

		/**
		 * Hides the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void HideCursor();

		/**
		 * Shows the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void ShowCursor();

		/**
		 * Query if the cursor is hidden.
		 *
		 * @note	Thread safe.
		 */
		static bool IsCursorHidden();

		/**
		 * Sets a cursor using a custom image.
		 *
		 * @param	pixelData	Cursor image data.
		 * @param	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *						Arrow cursor).
		 *
		 * @note	Thread safe.
		 */
		static void SetCursor(PixelData& pixelData, const Vector2I& hotSpot);

		/**
		 * Sets an icon for the main application window.
		 *
		 * @param	pixelData	Icon image data. This will be resized to the required icon size, depending on platform
		 * 						implementation.
		 *
		 * @note	Thread safe.
		 */
		static void SetIcon(const PixelData& pixelData);

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
		static void SetCaptionNonClientAreas(const RenderWindow& window, const Vector<Area2I>& nonClientAreas);

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
		static void SetResizeNonClientAreas(const RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas);

		/**
		 * Resets the non client areas for the specified windows and allows the platform to use the default values.
		 *
		 * @note	Thread safe.
		 */
		static void ResetNonClientAreas(const RenderWindow& window);

		/**
		 * Causes the current thread to pause execution for the specified amount of time.
		 *
		 * @param	duration	Duration in milliseconds. Providing zero will give up the current time-slice.
		 *
		 * @note	This method relies on timer granularity being set to 1 millisecond. If it is not, you can expect
		 *			this method to potentially take significantly longer if you are providing it with low ms values (<10).
		 */
		static void Sleep(u32 duration);

		/**
		 * Opens the provided folder using the default application, as specified by the operating system.
		 *
		 * @param	path	Absolute path to the folder to open.
		 */
		static void OpenFolder(const Path& path);

		/**
		 * Adds a string to the clipboard.
		 *
		 * @note	Thread safe.
		 */
		static void CopyToClipboard(const String& string);

		/**
		 * Reads a string from the clipboard and returns it. If there is no string in the clipboard it returns an empty
		 * string.
		 *
		 * @note
		 * Both wide and normal strings will be read, but normal strings will be converted to a wide string before returning.
		 * @note
		 * Thread safe.
		 */
		static String CopyFromClipboard();

		/**
		 * Converts a keyboard key-code to a Unicode character.
		 *
		 * @note
		 * Normally this will output a single character, but it can happen it outputs multiple in case a accent/diacritic
		 * character could not be combined with the virtual key into a single character.
		 */
		static String KeyCodeToUnicode(u32 keyCode);

		/**
		 * Message pump. Processes OS messages and returns when it's free.
		 *
		 * @note	Render thread only.
		 */
		static void MessagePumpInternal();

		/** Called during application start up from the main thread. Must be called before any other operations are done. */
		static void StartUpInternal();

		/** Called once per frame from the main thread. */
		static void UpdateInternal();

		/** Called during application shut down from the main thread. */
		static void ShutDownInternal();

		/**
		 * Triggered whenever the pointer moves.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(const Vector2I&, const OSPointerButtonStates&)> onCursorMoved;

		/**
		 * Triggered whenever a pointer button is pressed.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> onCursorButtonPressed;

		/**
		 * Triggered whenever pointer button is released.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> onCursorButtonReleased;

		/**
		 * Triggered whenever a pointer button is double clicked.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(const Vector2I&, const OSPointerButtonStates&)> onCursorDoubleClick;

		/**
		 * Triggered whenever an input command is entered.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(InputCommandType)> onInputCommand;

		/**
		 * Triggered whenever the mouse wheel is scolled.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(float)> onMouseWheelScrolled;

		/**
		 * Triggered whenever a character is entered.
		 *
		 * @note	Render  thread only.
		 */
		static Event<void(u32)> onCharInput;

		/**
		 * Triggered whenever mouse capture state for the window is changed (it receives or loses it).
		 *
		 * @note	Render  thread only.
		 */
		static Event<void()> onMouseCaptureChanged;

	protected:
		static Private* mData;
	};

	/** @} */
} // namespace b3d
