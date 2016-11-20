//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsInputFwd.h"
#include "BsVector2I.h"
#include "BsRect2I.h"
#include "BsEvent.h"

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

	/** Type of drop event type. This is used when dragging items over drop targets. */
	enum class OSDropType
	{
		FileList,
		None
	};

	/**
	 * Drop targets allow you to register a certain portion of a window as a drop target that accepts certain drop types 
	 * from the OS (platform) specific drag and drop system. Accepted drop types are provided by the OS and include things
	 * like file and item dragging.
	 *
	 * You will receive events with the specified drop area as long as it is active.
	 */
	class BS_CORE_EXPORT OSDropTarget
	{
	public:
		/**
		 * Triggered when a pointer is being dragged over the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onDragOver;

		/**
		 * Triggered when the user completes a drop while pointer is over the drop area. Provides window coordinates of the 
		 * pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onDrop;

		/**
		 * Triggered when a pointer enters the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onEnter;

		/** Triggered when a pointer leaves the drop area. */
		Event<void()> onLeave;

		/**	Sets the drop target area, in local window coordinates. */
		void setArea(INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**	Gets the type of drop that this drop target is looking for. Only valid after a drop has been triggered. */
		OSDropType getDropType() const { return mDropType; }

		/**	
		 * Returns a list of files received by the drop target. Only valid after a drop of FileList type has been triggered.
		 */
		const Vector<WString>& getFileList() const { return *mFileList; }

		/** Clears all internal values. */
		void _clear();

		/** Sets the file list and marks the drop event as FileList. */
		void _setFileList(const Vector<WString>& fileList);

		/** Marks the drop area as inactive or active. */
		void _setActive(bool active) { mActive = active; }

		/**	Checks is the specified position within the current drop area. Position should be in window local coordinates. */
		bool _isInside(const Vector2I& pos) const;

		/** Returns true if the drop target is active. */
		bool _isActive() const { return mActive; }
	private:
		friend class Platform;

		OSDropTarget(const RenderWindow* ownerWindow, INT32 x, INT32 y, UINT32 width, UINT32 height);
		~OSDropTarget();

		/**	Returns a render window this drop target is attached to. */
		const RenderWindow* getOwnerWindow() const { return mOwnerWindow; }
	private:
		INT32 mX, mY;
		UINT32 mWidth, mHeight;
		bool mActive;
		const RenderWindow* mOwnerWindow;

		OSDropType mDropType;

		union
		{
			Vector<WString>* mFileList;
		};
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
		static void setCaptionNonClientAreas(const RenderWindowCore& window, const Vector<Rect2I>& nonClientAreas);

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
		static void setResizeNonClientAreas(const RenderWindowCore& window, const Vector<NonClientResizeArea>& nonClientAreas);

		/**
		 * Resets the non client areas for the specified windows and allows the platform to use the default values.
		 *
		 * @note	Thread safe.
		 */
		static void resetNonClientAreas(const RenderWindowCore& window);

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
		 * Creates a drop target that you can use for tracking OS drag and drop operations performed over a certain area 
		 * on the specified window.
		 *
		 * @param[in]	window	The window on which to track drop operations.
		 * @param[in]	x	  	The x coordinate of the area to track, relative to window.
		 * @param[in]	y	  	The y coordinate of the area to track, relative to window.
		 * @param[in]	width 	The width of the area to track.
		 * @param[in]	height	The height of the area to track.
		 * @return				OSDropTarget that you will use to receive all drop data. When no longer needed make sure 
		 *						to destroy it with destroyDropTarget().
		 */
		static OSDropTarget& createDropTarget(const RenderWindow* window, int x, int y, unsigned int width, unsigned int height);

		/** Destroys a drop target previously created with createDropTarget. */
		static void destroyDropTarget(OSDropTarget& target);

		/**
		 * Displays a platform specific file/folder open/save dialog.
		 *
		 * @param[in]	type		Type of dialog to open.
		 * @param[in]	defaultPath	Initial path the dialog will be set to once opened.
		 * @param[in]	filterList	Semi-colon separated list of file names or types to display in the dialog, 
		 *							for example "exe;txt;png". Ignored if dialog is to display folders instead of files.
		 * @param[out]	paths		Output list of selected file or folder paths (if any).
		 * @return					True if file was selected and false if selection was canceled.
		 */
		static bool openBrowseDialog(FileDialogType type, const Path& defaultPath, const WString& filterList,
			Vector<Path>& paths);

		/**
		 * Message pump. Processes OS messages and returns when it's free.
		 *
		 * @note	Core thread only.
		 */
		static void _messagePump();

		/** Called during application start up from the sim thread. Must be called before any other operations are done. */
		static void _startUp();

		/**
		 * Called once per frame from the sim thread.
		 *
		 * @note	Sim thread only.
		 */
		static void _update();

		/**
		 * Called once per frame from the core thread.
		 *
		 * @note	Core thread only.
		 */
		static void _coreUpdate();

		/**
		 * Called during application shut down from the sim thread.
		 *
		 * @note	Sim thread only.
		 */
		static void _shutDown();

		/**
		 * Triggered when a pointer leaves the provided window.
		 *
		 * @note	Sim thread only.
		 */
		static Event<void(RenderWindowCore*)> onMouseLeftWindow;

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