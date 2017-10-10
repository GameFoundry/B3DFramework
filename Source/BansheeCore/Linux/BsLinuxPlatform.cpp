//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Image/BsPixelData.h"
#include "Image/BsPixelUtil.h"
#include "String/BsUnicode.h"
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxWindow.h"
#include "RenderAPI/BsRenderWindow.h"
#include "BsLinuxDropTarget.h"
#include "BsCoreApplication.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

namespace bs
{
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorMoved;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonPressed;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonReleased;
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorDoubleClick;
	Event<void(InputCommandType)> Platform::onInputCommand;
	Event<void(float)> Platform::onMouseWheelScrolled;
	Event<void(UINT32)> Platform::onCharInput;

	Event<void()> Platform::onMouseCaptureChanged;

	enum class X11CursorType
	{
		Arrow,
		ArrowDrag,
		ArrowLeftRight,
		Wait,
		IBeam,
		SizeTopLeft,
		SizeTopRight,
		SizeBotLeft,
		SizeBotRight,
		SizeLeft,
		SizeRight,
		SizeTop,
		SizeBottom,
		Deny,

		Count
	};;

	struct Platform::Pimpl
	{
		::Display* xDisplay = nullptr;
		::Window mainXWindow = 0;
		::Window fullscreenXWindow = 0;
		UnorderedMap<::Window, LinuxWindow*> windowMap;
		Vector<::Window> toDestroy;
		Mutex lock;

		XIM IM;
		XIC IC;
		Time lastButtonPressTime;

		Atom atomDeleteWindow;
		Atom atomWmState;
		Atom atomWmStateHidden;
		Atom atomWmStateMaxVert;
		Atom atomWmStateMaxHorz;

		// Clipboard
		WString clipboardData;

		// Cursor
		::Cursor currentCursor = None;
		::Cursor emptyCursor = None;
		bool isCursorHidden = false;

		Rect2I cursorClipRect;
		LinuxWindow* cursorClipWindow = nullptr;
		bool cursorClipEnabled = false;
	};

	static const UINT32 DOUBLE_CLICK_MS = 500;

	Vector2I _getCursorPosition(Platform::Pimpl* data)
	{
		Vector2I pos;
		UINT32 screenCount = (UINT32)XScreenCount(data->xDisplay);

		for (UINT32 i = 0; i < screenCount; ++i)
		{
			::Window outRoot, outChild;
			INT32 childX, childY;
			UINT32 mask;
			if(XQueryPointer(data->xDisplay, XRootWindow(data->xDisplay, i), &outRoot, &outChild, &pos.x,
					&pos.y, &childX, &childY, &mask))
				break;
		}

		return pos;
	}

	void _setCursorPosition(Platform::Pimpl* data, const Vector2I& screenPos)
	{
		UINT32 screenCount = (UINT32)XScreenCount(data->xDisplay);

		// Note assuming screens are laid out horizontally left to right
		INT32 screenX = 0;
		for(UINT32 i = 0; i < screenCount; ++i)
		{
			::Window root = XRootWindow(data->xDisplay, i);
			INT32 screenXEnd = screenX + XDisplayWidth(data->xDisplay, i);

			if(screenPos.x >= screenX && screenPos.x < screenXEnd)
			{
				XWarpPointer(data->xDisplay, None, root, 0, 0, 0, 0, screenPos.x, screenPos.y);
				XFlush(data->xDisplay);
				return;
			}

			screenX = screenXEnd;
		}
	}

	void applyCurrentCursor(Platform::Pimpl* data, ::Window window)
	{
		if(data->isCursorHidden)
			XDefineCursor(data->xDisplay, window, data->emptyCursor);
		else
		{
			if (data->currentCursor != None)
				XDefineCursor(data->xDisplay, window, data->currentCursor);
			else
				XUndefineCursor(data->xDisplay, window);
		}
	}

	void updateClipBounds(Platform::Pimpl* data, LinuxWindow* window)
	{
		if(!data->cursorClipEnabled || data->cursorClipWindow != window)
			return;

		data->cursorClipRect.x = window->getLeft();
		data->cursorClipRect.y = window->getTop();
		data->cursorClipRect.width = window->getWidth();
		data->cursorClipRect.height = window->getHeight();
	}

	bool clipCursor(Platform::Pimpl* data, Vector2I& pos)
	{
		if(!data->cursorClipEnabled)
			return false;

		INT32 clippedX = pos.x - data->cursorClipRect.x;
		INT32 clippedY = pos.y - data->cursorClipRect.y;

		if(clippedX < 0)
			clippedX = data->cursorClipRect.x + data->cursorClipRect.width + clippedX;
		else if(clippedX >= (INT32)data->cursorClipRect.width)
			clippedX = data->cursorClipRect.x + (clippedX - data->cursorClipRect.width);
		else
			clippedX = data->cursorClipRect.x + clippedX;

		if(clippedY < 0)
			clippedY = data->cursorClipRect.y + data->cursorClipRect.height + clippedY;
		else if(clippedY >= (INT32)data->cursorClipRect.height)
			clippedY = data->cursorClipRect.y + (clippedY - data->cursorClipRect.height);
		else
			clippedY = data->cursorClipRect.y + clippedY;

		if(clippedX != pos.x || clippedY != pos.y)
		{
			pos.x = clippedX;
			pos.y = clippedY;

			return true;
		}

		return false;
	}

	void setCurrentCursor(Platform::Pimpl* data, ::Cursor cursor)
	{
		if(data->currentCursor)
			XFreeCursor(data->xDisplay, data->currentCursor);

		data->currentCursor = cursor;
		for(auto& entry : data->windowMap)
			applyCurrentCursor(data, entry.first);
	}

	/**
	 * Searches the window hierarchy, from top to bottom, looking for the top-most window that contains the specified
	 * point. Returns 0 if one is not found.
	 */
	::Window getWindowUnderPoint(::Display* display, ::Window rootWindow, ::Window window, const Vector2I& screenPos)
	{
		::Window outRoot, outParent;
		::Window* children;
		UINT32 numChildren;
		XQueryTree(display, window, &outRoot, &outParent, &children, &numChildren);

		if(children == nullptr || numChildren == 0)
			return window;

		for(UINT32 j = 0; j < numChildren; j++)
		{
			::Window curWindow = children[numChildren - j - 1];

			XWindowAttributes xwa;
			XGetWindowAttributes(display, curWindow, &xwa);

			if(xwa.map_state != IsViewable || xwa.c_class != InputOutput)
				continue;

			// Get position in root window coordinates
			::Window outChild;
			Vector2I pos;
			if(!XTranslateCoordinates(display, curWindow, rootWindow, 0, 0, &pos.x, &pos.y, &outChild))
				continue;

			Rect2I area(pos.x, pos.y, (UINT32)xwa.width, (UINT32)xwa.height);
			if(area.contains(screenPos))
			{
				XFree(children);
				return getWindowUnderPoint(display, rootWindow, curWindow, screenPos);
			}
		}

		XFree(children);
		return 0;
	}

	int x11ErrorHandler(::Display* display, XErrorEvent* event)
	{
		// X11 by default crashes the app on error, even though some errors can be just fine. So we provide our own handler.

		char buffer[256];
		XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
		LOGWRN("X11 error: " + String(buffer));

		return 0;
	}

	Platform::Pimpl* Platform::mData = bs_new<Platform::Pimpl>();

	Platform::~Platform()
	{ }

	Vector2I Platform::getCursorPosition()
	{
		Lock lock(mData->lock);
		return _getCursorPosition(mData);
	}

	void Platform::setCursorPosition(const Vector2I& screenPos)
	{
		Lock lock(mData->lock);

		_setCursorPosition(mData, screenPos);
	}

	void Platform::captureMouse(const RenderWindow& window)
	{
		Lock lock(mData->lock);

		LinuxWindow* linuxWindow;
		window.getCustomAttribute("LINUX_WINDOW", &linuxWindow);

		UINT32 mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
		XGrabPointer(mData->xDisplay, linuxWindow->_getXWindow(), False, mask, GrabModeAsync,
				GrabModeAsync, None, None, CurrentTime);
		XSync(mData->xDisplay, False);
	}

	void Platform::releaseMouseCapture()
	{
		Lock lock(mData->lock);

		XUngrabPointer(mData->xDisplay, CurrentTime);
		XSync(mData->xDisplay, False);
	}

	bool Platform::isPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
	{
		Lock lock(mData->lock);

		LinuxWindow* linuxWindow;
		window.getCustomAttribute("LINUX_WINDOW", &linuxWindow);
		::Window xWindow = linuxWindow->_getXWindow();

		UINT32 screenCount = (UINT32)XScreenCount(mData->xDisplay);

		for (UINT32 i = 0; i < screenCount; ++i)
		{
			::Window rootWindow = XRootWindow(mData->xDisplay, i);

			::Window curWindow = getWindowUnderPoint(mData->xDisplay, rootWindow, rootWindow, screenPos);
			return curWindow == xWindow;
		}

		return false;
	}

	void Platform::hideCursor()
	{
		Lock lock(mData->lock);
		mData->isCursorHidden = true;

		for(auto& entry : mData->windowMap)
			applyCurrentCursor(mData, entry.first);
	}

	void Platform::showCursor()
	{
		Lock lock(mData->lock);
		mData->isCursorHidden = false;

		for(auto& entry : mData->windowMap)
			applyCurrentCursor(mData, entry.first);
	}

	bool Platform::isCursorHidden()
	{
		Lock lock(mData->lock);
		return mData->isCursorHidden;
	}

	void Platform::clipCursorToWindow(const RenderWindow& window)
	{
		Lock lock(mData->lock);

		LinuxWindow* linuxWindow;
		window.getCustomAttribute("LINUX_WINDOW", &linuxWindow);

		mData->cursorClipEnabled = true;
		mData->cursorClipWindow = linuxWindow;

		updateClipBounds(mData, linuxWindow);

		Vector2I pos = _getCursorPosition(mData);

		if(clipCursor(mData, pos))
			_setCursorPosition(mData, pos);
	}

	void Platform::clipCursorToRect(const Rect2I& screenRect)
	{
		Lock lock(mData->lock);

		mData->cursorClipEnabled = true;
		mData->cursorClipRect = screenRect;
		mData->cursorClipWindow = nullptr;

		Vector2I pos = _getCursorPosition(mData);

		if(clipCursor(mData, pos))
			_setCursorPosition(mData, pos);
	}

	void Platform::clipCursorDisable()
	{
		Lock lock(mData->lock);

		mData->cursorClipEnabled = false;
		mData->cursorClipWindow = None;
	}

	void Platform::setCursor(PixelData& pixelData, const Vector2I& hotSpot)
	{
		SPtr<PixelData> bgraData = PixelData::create(pixelData.getWidth(), pixelData.getHeight(), 1, PF_BGRA8);
		PixelUtil::bulkPixelConversion(pixelData, *bgraData);

		Lock lock(mData->lock);

		XcursorImage* image = XcursorImageCreate((int)bgraData->getWidth(), (int)bgraData->getHeight());
		image->xhot = (XcursorDim)hotSpot.x;
		image->yhot = (XcursorDim)hotSpot.y;
		image->delay = 0;

		memcpy(image->pixels, bgraData->getData(), bgraData->getSize());

		::Cursor cursor = XcursorImageLoadCursor(mData->xDisplay, image);
		XcursorImageDestroy(image);

		setCurrentCursor(mData, cursor);
	}

	void Platform::setIcon(const PixelData& pixelData)
	{
		SPtr<PixelData> resizedData = PixelData::create(32, 32, 1, PF_RGBA8);
		PixelUtil::scale(pixelData, *resizedData);

		if(!mData->mainXWindow)
			return;

		auto iterFind = mData->windowMap.find(mData->mainXWindow);
		if(iterFind == mData->windowMap.end())
			return;

		LinuxWindow* mainLinuxWindow = iterFind->second;

		Lock lock(mData->lock);
		mainLinuxWindow->setIcon(pixelData);
	}

	void Platform::setCaptionNonClientAreas(const ct::RenderWindow& window, const Vector<Rect2I>& nonClientAreas)
	{
		if(nonClientAreas.size() == 0)
			return;

		Lock lock(mData->lock);

		LinuxWindow* linuxWindow;
		window.getCustomAttribute("LINUX_WINDOW", &linuxWindow);

		// Note: Only supporting a single area
		linuxWindow->_setDragZone(nonClientAreas[0]);
	}

	void Platform::setResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
	{
		// Do nothing, resize areas not supported on Linux (but they are provided even on undecorated windows by the WM)
	}

	void Platform::resetNonClientAreas(const ct::RenderWindow& window)
	{
		// Do nothing, resize areas not supported on Linux (but they are provided even on undecorated windows by the WM)
	}

	void Platform::sleep(UINT32 duration)
	{
		usleep(duration * 1000);
	}

	void Platform::copyToClipboard(const WString& string)
	{
		Lock lock(mData->lock);
		mData->clipboardData = string;

		Atom clipboardAtom = XInternAtom(mData->xDisplay, "CLIPBOARD", 0);
		XSetSelectionOwner(mData->xDisplay, clipboardAtom, mData->mainXWindow, CurrentTime);
	}

	WString Platform::copyFromClipboard()
	{
		Lock lock(mData->lock);
		Atom clipboardAtom = XInternAtom(mData->xDisplay, "CLIPBOARD", 0);
		::Window selOwner = XGetSelectionOwner(mData->xDisplay, clipboardAtom);

		if(selOwner == None)
			return L"";

		if(selOwner == mData->mainXWindow)
			return mData->clipboardData;

		XConvertSelection(mData->xDisplay, clipboardAtom, XA_STRING, clipboardAtom, mData->mainXWindow,
				CurrentTime);
		XFlush(mData->xDisplay);

		// Note: This might discard events if there are any in between the one we need. Ideally we let the
		// processEvents() handle them
		while(true)
		{
			XEvent event;
			XNextEvent(mData->xDisplay, &event);

			if(event.type == SelectionNotify && event.xselection.requestor == mData->mainXWindow)
				break;
		}

		Atom actualType;
		INT32 actualFormat;
		unsigned long length;
		unsigned long bytesRemaining;
		UINT8* data;
		XGetWindowProperty(mData->xDisplay, mData->mainXWindow, clipboardAtom,
				0, 0, False, AnyPropertyType, &actualType, &actualFormat, &length, &bytesRemaining, &data);

		if(bytesRemaining > 0)
		{
			unsigned long unused;
			INT32 result = XGetWindowProperty(mData->xDisplay, mData->mainXWindow, clipboardAtom,
					0, bytesRemaining, False, AnyPropertyType, &actualType, &actualFormat, &length,
					&unused, &data);

			if(result == Success)
				return UTF8::toWide(String((const char*)data));

			XFree(data);
		}

		return L"";
	}

	/** Maps Banshee button codes to X11 names for physical key locations. */
	const char* keyCodeToKeyName(ButtonCode code)
	{
		switch(code)
		{
			// Row #1
		case BC_F1:			return "FK01";
		case BC_F2:			return "FK02";
		case BC_F3:			return "FK03";
		case BC_F4:			return "FK04";
		case BC_F5:			return "FK05";
		case BC_F6:			return "FK06";
		case BC_F7:			return "FK07";
		case BC_F8:			return "FK08";
		case BC_F9:			return "FK09";
		case BC_F10:		return "FK10";
		case BC_F11:		return "FK11";
		case BC_F12:		return "FK12";

			// Row #2
		case BC_GRAVE:		return "TLDE";
		case BC_1: 			return "AE01";
		case BC_2:			return "AE02";
		case BC_3:			return "AE03";
		case BC_4:			return "AE04";
		case BC_5:			return "AE05";
		case BC_6:			return "AE06";
		case BC_7:			return "AE07";
		case BC_8:			return "AE08";
		case BC_9:			return "AE09";
		case BC_0:			return "AE10";
		case BC_MINUS:		return "AE11";
		case BC_EQUALS:		return "AE12";
		case BC_BACK:		return "BKSP";

			// Row #3
		case BC_TAB:		return "TAB";
		case BC_Q:			return "AD01";
		case BC_W:			return "AD02";
		case BC_E:			return "AD03";
		case BC_R:			return "AD04";
		case BC_T:			return "AD05";
		case BC_Y:			return "AD06";
		case BC_U:			return "AD07";
		case BC_I:			return "AD08";
		case BC_O:			return "AD09";
		case BC_P:			return "AD10";
		case BC_LBRACKET:	return "AD11";
		case BC_RBRACKET:	return "AD12";

			// Row #4
		case BC_A:			return "AC01";
		case BC_S:			return "AC02";
		case BC_D:			return "AC03";
		case BC_F:			return "AC04";
		case BC_G:			return "AC05";
		case BC_H:			return "AC06";
		case BC_J:			return "AC07";
		case BC_K:			return "AC08";
		case BC_L:			return "AC09";
		case BC_SEMICOLON:	return "AC10";
		case BC_APOSTROPHE:	return "AC11";
		case BC_BACKSLASH:	return "BKSL";

			// Row #5
		case BC_Z:			return "AB01";
		case BC_X:			return "AB02";
		case BC_C:			return "AB03";
		case BC_V:			return "AB04";
		case BC_B:			return "AB05";
		case BC_N:			return "AB06";
		case BC_M:			return "AB07";
		case BC_COMMA:		return "AB08";
		case BC_PERIOD:		return "AB09";
		case BC_SLASH:		return "AB10";

			// Keypad
		case BC_NUMPAD0:	return "KP0";
		case BC_NUMPAD1:	return "KP1";
		case BC_NUMPAD2:	return "KP2";
		case BC_NUMPAD3:	return "KP3";
		case BC_NUMPAD4:	return "KP4";
		case BC_NUMPAD5:	return "KP5";
		case BC_NUMPAD6:	return "KP6";
		case BC_NUMPAD7:	return "KP7";
		case BC_NUMPAD8:	return "KP8";
		case BC_NUMPAD9:	return "KP9";

		default:
			break;
		}

		return nullptr;
	}

	WString Platform::keyCodeToUnicode(UINT32 buttonCode)
	{
		Lock lock(mData->lock);

		static bool mapInitialized = false;
		static UnorderedMap<String, KeyCode> keyMap;
		if(!mapInitialized)
		{
			char name[XkbKeyNameLength + 1];

			XkbDescPtr desc = XkbGetMap(mData->xDisplay, 0, XkbUseCoreKbd);
			XkbGetNames(mData->xDisplay, XkbKeyNamesMask, desc);

			for(UINT32 keyCode = desc->min_key_code; keyCode <= desc->max_key_code; keyCode++)
			{
				memcpy(name, desc->names->keys[keyCode].name, XkbKeyNameLength);
				name[XkbKeyNameLength] = '\0';

				keyMap[String(name)] = keyCode;
			}

			XkbFreeNames(desc, XkbKeyNamesMask, True);
			XkbFreeKeyboard(desc, 0, True);

			mapInitialized = true;
		}

		const char* keyName = keyCodeToKeyName((ButtonCode)buttonCode);
		if(keyName == nullptr)
		{
			// Not a printable key
			return L"";
		}

		auto iterFind = keyMap.find(String(keyName));
		if(iterFind == keyMap.end())
		{
			// Cannot find mapping, although this shouldn't really happen
			return L"";
		}

		XKeyPressedEvent event;
		bs_zero_out(event);
		event.type = KeyPress;
		event.keycode = iterFind->second;
		event.display = mData->xDisplay;
		event.time = CurrentTime;
		event.window = mData->mainXWindow;
		event.root = RootWindow(mData->xDisplay, XDefaultScreen(mData->xDisplay));

		Status status;
		char buffer[16];

		INT32 length = Xutf8LookupString(mData->IC, &event, buffer, sizeof(buffer), nullptr, &status);
		if(length > 0)
		{
			buffer[length] = '\0';

			return UTF8::toWide(String(buffer));
		}

		return L"";
	}

	void Platform::openFolder(const Path& path)
	{
		String pathString = path.toString();

		const char* commandPattern = "xdg-open '%s'";

		char* commandStr = (char*)bs_stack_alloc((UINT32)pathString.size() + (UINT32)strlen(commandPattern) + 1);
		sprintf(commandStr, commandPattern, pathString.c_str());

		system(commandStr);
		bs_stack_free(commandStr);
	}

	/**
	 * Converts an X11 KeySym code into an input command, if possible. Returns true if conversion was done.
	 *
	 * @param[in]	keySym			KeySym to try to translate to a command.
	 * @param[in]	shift			True if the shift key was held down when the key was pressed.
	 * @param[out]	command			Input command. Only valid if function returns true.
	 * @return						True if the KeySym is an input command.
	 */
	bool parseInputCommand(KeySym keySym, bool shift, InputCommandType& command)
	{
		switch (keySym)
		{
		case XK_Left:
			command = shift ? InputCommandType::SelectLeft : InputCommandType::CursorMoveLeft;
			return true;
		case XK_Right:
			command = shift ? InputCommandType::SelectRight : InputCommandType::CursorMoveRight;
			return true;
		case XK_Up:
			command = shift ? InputCommandType::SelectUp : InputCommandType::CursorMoveUp;
			return true;
		case XK_Down:
			command = shift ? InputCommandType::SelectDown : InputCommandType::CursorMoveDown;
			return true;
		case XK_Escape:
			command = InputCommandType::Escape;
			return true;
		case XK_Return:
			command = shift ? InputCommandType::Return : InputCommandType::Confirm;
			return true;
		case XK_BackSpace:
			command = InputCommandType::Backspace;
			return true;
		case XK_Delete:
			command = InputCommandType::Delete;
			return true;
		}

		return false;
	}

	/** Returns a LinuxWindow from a native X11 window handle. */
	LinuxWindow* getLinuxWindow(LinuxPlatform::Pimpl* data, ::Window xWindow)
	{
		auto iterFind = data->windowMap.find(xWindow);
		if (iterFind != data->windowMap.end())
		{
			LinuxWindow* window = iterFind->second;
			return window;
		}

		return nullptr;
	}

	/** Returns a RenderWindow from a native X11 window handle. Returns null if the window isn't a RenderWindow */
	ct::RenderWindow* getRenderWindow(LinuxPlatform::Pimpl* data, ::Window xWindow)
	{
		LinuxWindow* linuxWindow = getLinuxWindow(data, xWindow);
		if(linuxWindow != nullptr)
			return (ct::RenderWindow*)linuxWindow->_getUserData();

		return nullptr;
	}

	void Platform::_messagePump()
	{
		while(true)
		{
			Lock lock(mData->lock);
			if(XPending(mData->xDisplay) <= 0)
			{
				// No more events, destroy any queued windows
				for(auto& entry : mData->toDestroy)
					XDestroyWindow(mData->xDisplay, entry);

				mData->toDestroy.clear();
				XSync(mData->xDisplay, false);

				break;
			}

			XEvent event;
			XNextEvent(mData->xDisplay, &event);

			switch (event.type)
			{
			case ClientMessage:
			{
				if(LinuxDragAndDrop::handleClientMessage(event.xclient))
					break;

				if((Atom)event.xclient.data.l[0] == mData->atomDeleteWindow)
				{
					// We queue the window for destruction as soon as we process all current events (since some of those
					// events could still refer to this window)
					mData->toDestroy.push_back(event.xclient.window);

					XUnmapWindow(mData->xDisplay, event.xclient.window);
					XSync(mData->xDisplay, false);
				}
			}
				break;
			case DestroyNotify:
			{
				LinuxWindow* window = getLinuxWindow(mData, event.xdestroywindow.window);
				if(window != nullptr)
				{
					CoreApplication::instance().quitRequested();
					window->_cleanUp();

					if (mData->mainXWindow == 0)
						return;
				}
			}
				break;
			case KeyPress:
			{
				// Process text input
				KeySym keySym = XkbKeycodeToKeysym(mData->xDisplay, (KeyCode)event.xkey.keycode, 0, 0);

				//// Check if input manager wants this event. If not, we process it.
				if(XFilterEvent(&event, None) == False)
				{
					// Don't consider Return key a character
					if(keySym != XK_Return)
					{
						Status status;
						char buffer[16];

						INT32 length = Xutf8LookupString(mData->IC, &event.xkey, buffer, sizeof(buffer), nullptr,
								&status);

						if (length > 0)
						{
							buffer[length] = '\0';

							U32String utfStr = UTF8::toUTF32(String(buffer));
							if (utfStr.length() > 0)
								onCharInput((UINT32) utfStr[0]);
						}
					}
				}

				// Handle input commands
				InputCommandType command = InputCommandType::Backspace;

				bool shift = (event.xkey.state & ShiftMask) != 0;

				if(parseInputCommand(keySym, shift, command))
				{
					if(!onInputCommand.empty())
						onInputCommand(command);
				}
			}
				break;
			case KeyRelease:
				// Do nothing
				break;
			case ButtonPress:
			{
				UINT32 button = event.xbutton.button;

				OSMouseButton mouseButton;
				bool validPress = false;
				switch(button)
				{
				case Button1:
					mouseButton = OSMouseButton::Left;
					validPress = true;
					break;
				case Button2:
					mouseButton = OSMouseButton::Middle;
					validPress = true;
					break;
				case Button3:
					mouseButton = OSMouseButton::Right;
					validPress = true;
					break;

				default:
					break;
				}

				if(validPress)
				{
					// Send event
					Vector2I pos;
					pos.x = event.xbutton.x_root;
					pos.y = event.xbutton.y_root;

					OSPointerButtonStates btnStates;
					btnStates.ctrl = (event.xbutton.state & ControlMask) != 0;
					btnStates.shift = (event.xbutton.state & ShiftMask) != 0;
					btnStates.mouseButtons[0] = (event.xbutton.state & Button1Mask) != 0;
					btnStates.mouseButtons[1] = (event.xbutton.state & Button2Mask) != 0;
					btnStates.mouseButtons[2] = (event.xbutton.state & Button3Mask) != 0;

					onCursorButtonPressed(pos, mouseButton, btnStates);

					// Handle double-click
					if(button == Button1)
					{
						if (event.xbutton.time < (mData->lastButtonPressTime + DOUBLE_CLICK_MS))
						{
							onCursorDoubleClick(pos, btnStates);
							mData->lastButtonPressTime = 0;
						}
						else
							mData->lastButtonPressTime = event.xbutton.time;
					}
				}

				// Handle window dragging for windows without a title bar
				if(button == Button1)
				{
					LinuxWindow* window = getLinuxWindow(mData, event.xbutton.window);
					if(window != nullptr)
						window->_dragStart(event.xbutton.x, event.xbutton.y);
				}

				break;
			}
			case ButtonRelease:
			{
				UINT32 button = event.xbutton.button;

				Vector2I pos;
				pos.x = event.xbutton.x_root;
				pos.y = event.xbutton.y_root;

				OSPointerButtonStates btnStates;
				btnStates.ctrl = (event.xbutton.state & ControlMask) != 0;
				btnStates.shift = (event.xbutton.state & ShiftMask) != 0;
				btnStates.mouseButtons[0] = (event.xbutton.state & Button1Mask) != 0;
				btnStates.mouseButtons[1] = (event.xbutton.state & Button2Mask) != 0;
				btnStates.mouseButtons[2] = (event.xbutton.state & Button3Mask) != 0;

				switch(button)
				{
				case Button1:
					onCursorButtonReleased(pos, OSMouseButton::Left, btnStates);
					break;
				case Button2:
					onCursorButtonReleased(pos, OSMouseButton::Middle, btnStates);
					break;
				case Button3:
					onCursorButtonReleased(pos, OSMouseButton::Right, btnStates);
					break;
				case Button4: // Vertical mouse wheel
				case Button5:
				{
					INT32 delta = button == Button4 ? 1 : -1;
					onMouseWheelScrolled((float)delta);
				}
					break;
				default:
					break;
				}

				// Handle window dragging for windows without a title bar
				if(button == Button1)
				{
					LinuxWindow* window = getLinuxWindow(mData, event.xbutton.window);
					if(window != nullptr)
						window->_dragEnd();
				}

				break;
			}
			case MotionNotify:
			{
				Vector2I pos;
				pos.x = event.xmotion.x_root;
				pos.y = event.xmotion.y_root;

				// Handle clipping if enabled
				if(clipCursor(mData, pos))
					_setCursorPosition(mData, pos);

				// Send event
				OSPointerButtonStates btnStates;
				btnStates.ctrl = (event.xmotion.state & ControlMask) != 0;
				btnStates.shift = (event.xmotion.state & ShiftMask) != 0;
				btnStates.mouseButtons[0] = (event.xmotion.state & Button1Mask) != 0;
				btnStates.mouseButtons[1] = (event.xmotion.state & Button2Mask) != 0;
				btnStates.mouseButtons[2] = (event.xmotion.state & Button3Mask) != 0;

				onCursorMoved(pos, btnStates);

				// Handle window dragging for windows without a title bar
				LinuxWindow* window = getLinuxWindow(mData, event.xmotion.window);
				if(window != nullptr)
					window->_dragUpdate(event.xmotion.x, event.xmotion.y);
			}
				break;
			case EnterNotify:
				// Do nothing
				break;
			case LeaveNotify:
			{
				if (event.xcrossing.mode == NotifyNormal)
				{
					Vector2I pos;
					pos.x = event.xcrossing.x_root;
					pos.y = event.xcrossing.y_root;

					if (clipCursor(mData, pos))
						_setCursorPosition(mData, pos);
				}

				ct::RenderWindow* renderWindow = getRenderWindow(mData, event.xcrossing.window);
				if(renderWindow != nullptr)
					renderWindow->_notifyMouseLeft();
			}
				break;
			case ConfigureNotify:
			{
				LinuxWindow* window = getLinuxWindow(mData, event.xconfigure.window);
				if(window != nullptr)
				{
					updateClipBounds(mData, window);

					ct::RenderWindow* renderWindow = (ct::RenderWindow*)window->_getUserData();
					if(renderWindow != nullptr)
						renderWindow->_windowMovedOrResized();
				}
			}
				break;
			case FocusIn:
			{
				// Update input context focus
				XSetICFocus(mData->IC);

				// Send event to render window
				ct::RenderWindow* renderWindow = getRenderWindow(mData, event.xfocus.window);

				// Not a render window, so it doesn't care about these events
				if (renderWindow != nullptr)
				{
					if (!renderWindow->getProperties().hasFocus)
						renderWindow->_windowFocusReceived();
				}
			}
				break;
			case FocusOut:
			{
				// Update input context focus
				XUnsetICFocus(mData->IC);

				// Send event to render window
				ct::RenderWindow* renderWindow = getRenderWindow(mData, event.xfocus.window);

				// Not a render window, so it doesn't care about these events
				if (renderWindow != nullptr)
				{
					if (renderWindow->getProperties().hasFocus)
						renderWindow->_windowFocusLost();
				}
			}
				break;
			case SelectionNotify:
				LinuxDragAndDrop::handleSelectionNotify(event.xselection);
				break;
			case SelectionRequest:
			{
				// Send the data saved by the last clipboard copy operation
				Atom compoundTextAtom = XInternAtom(mData->xDisplay, "COMPOUND_TEXT", 0);
				Atom utf8StringAtom = XInternAtom(mData->xDisplay, "UTF8_STRING", 0);
				Atom targetsAtom = XInternAtom(mData->xDisplay, "TARGETS", 0);

				XSelectionRequestEvent& selReq = event.xselectionrequest;
				XEvent response;
				if(selReq.target == XA_STRING || selReq.target == compoundTextAtom || selReq.target == utf8StringAtom)
				{
					String utf8data = UTF8::fromWide(mData->clipboardData);

					const UINT8* data = (const UINT8*)utf8data.c_str();
					INT32 dataLength = (INT32)utf8data.length();

					XChangeProperty(mData->xDisplay, selReq.requestor, selReq.property,
							selReq.target, 8, PropModeReplace, data, dataLength);

					response.xselection.property = selReq.property;
				}
				else if(selReq.target == targetsAtom)
				{
					Atom data[2];
					data[0] = utf8StringAtom;
					data[1] = XA_STRING;
					XChangeProperty (mData->xDisplay, selReq.requestor, selReq.property, selReq.target,
							8, PropModeReplace, (unsigned char*)&data, sizeof (data));

					response.xselection.property = selReq.property;
				}
				else
				{
					response.xselection.property = None;
				}

				response.xselection.type = SelectionNotify;
				response.xselection.display = selReq.display;
				response.xselection.requestor = selReq.requestor;
				response.xselection.selection = selReq.selection;
				response.xselection.target = selReq.target;
				response.xselection.time = selReq.time;

				XSendEvent (mData->xDisplay, selReq.requestor, 0, 0, &response);
				XFlush (mData->xDisplay);
			}
				break;
			case PropertyNotify:
				// Report minimize, maximize and restore events
				if(event.xproperty.atom == mData->atomWmState)
				{
					Atom type;
					INT32 format;
					unsigned long count, bytesRemaining;
					UINT8* data = nullptr;

					INT32 result = XGetWindowProperty(mData->xDisplay, event.xproperty.window, mData->atomWmState,
							0, 1024, False, AnyPropertyType, &type, &format,
							&count, &bytesRemaining, &data);

					if (result == Success)
					{
						ct::RenderWindow* renderWindow = getRenderWindow(mData, event.xproperty.window);

						// Not a render window, so it doesn't care about these events
						if(renderWindow == nullptr)
							continue;

						Atom* atoms = (Atom*)data;

						bool foundHorz = false;
						bool foundVert = false;
						for (unsigned long i = 0; i < count; i++)
						{
							if (atoms[i] == mData->atomWmStateMaxHorz) foundHorz = true;
							if (atoms[i] == mData->atomWmStateMaxVert) foundVert = true;

							if (foundVert && foundHorz)
							{
								if(event.xproperty.state == PropertyNewValue)
									renderWindow->_notifyMaximized();
								else
									renderWindow->_notifyRestored();
							}

							if(atoms[i] == mData->atomWmStateHidden)
							{
								if(event.xproperty.state == PropertyNewValue)
									renderWindow->_notifyMinimized();
								else
									renderWindow->_notifyRestored();
							}
						}

						XFree(atoms);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	void Platform::_startUp()
	{
		Lock lock(mData->lock);
		mData->xDisplay = XOpenDisplay(nullptr);
		XSetErrorHandler(x11ErrorHandler);

		if(XSupportsLocale())
		{
			XSetLocaleModifiers("@im=none");
			mData->IM = XOpenIM(mData->xDisplay, nullptr, nullptr, nullptr);

			// Note: Currently our windows don't support pre-edit and status areas, which are used for more complex types
			// of character input. Later on it might be beneficial to support them.
			mData->IC = XCreateIC(mData->IM, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, nullptr);
		}

		mData->atomDeleteWindow = XInternAtom(mData->xDisplay, "WM_DELETE_WINDOW", False);
		mData->atomWmState = XInternAtom(mData->xDisplay, "_NET_WM_STATE", False);
		mData->atomWmStateHidden = XInternAtom(mData->xDisplay, "_NET_WM_STATE_HIDDEN", False);
		mData->atomWmStateMaxHorz = XInternAtom(mData->xDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		mData->atomWmStateMaxVert = XInternAtom(mData->xDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);

		// Drag and drop
		LinuxDragAndDrop::startUp(mData->xDisplay);

		// Create empty cursor
		char data[1];
		memset(data, 0, sizeof(data));

		Pixmap pixmap = XCreateBitmapFromData(mData->xDisplay, DefaultRootWindow(mData->xDisplay), data, 1, 1);

		XColor color;
		color.red = color.green = color.blue = 0;
		mData->emptyCursor = XCreatePixmapCursor(mData->xDisplay, pixmap, pixmap, &color, &color, 0, 0);

		XFreePixmap(mData->xDisplay, pixmap);
	}

	void Platform::_update()
	{
		LinuxDragAndDrop::update();
	}

	void Platform::_coreUpdate()
	{
		_messagePump();
	}

	void Platform::_shutDown()
	{
		Lock lock(mData->lock);

		// Free empty cursor
		XFreeCursor(mData->xDisplay, mData->emptyCursor);
		mData->emptyCursor = None;

		// Shutdown drag and drop
		LinuxDragAndDrop::shutDown();

		if(mData->IC)
		{
			XDestroyIC(mData->IC);
			mData->IC = 0;
		}

		if(mData->IM)
		{
			XCloseIM(mData->IM);
			mData->IM = 0;
		}

		XCloseDisplay(mData->xDisplay);
		mData->xDisplay = nullptr;

		bs_delete(mData);
		mData = nullptr;
	}

	::Display* LinuxPlatform::getXDisplay()
	{
		return mData->xDisplay;
	}

	::Window LinuxPlatform::getMainXWindow()
	{
		return mData->mainXWindow;
	}

	void LinuxPlatform::lockX()
	{
		mData->lock.lock();
	}

	void LinuxPlatform::unlockX()
	{
		mData->lock.unlock();
	}

	void LinuxPlatform::_registerWindow(::Window xWindow, LinuxWindow* window)
	{
		// First window is assumed to be the main
		if(mData->mainXWindow == 0)
		{
			mData->mainXWindow = xWindow;

			// Input context client window must be set before use
			XSetICValues(mData->IC,
					XNClientWindow, xWindow,
					XNFocusWindow, xWindow,
					nullptr);
		}

		mData->windowMap[xWindow] = window;

		applyCurrentCursor(mData, xWindow);
	}

	void LinuxPlatform::_unregisterWindow(::Window xWindow)
	{
		auto iterFind = mData->windowMap.find(xWindow);
		if(iterFind != mData->windowMap.end())
		{
			if(mData->cursorClipEnabled && mData->cursorClipWindow == iterFind->second)
				clipCursorDisable();

			mData->windowMap.erase(iterFind);
		}

		if(mData->mainXWindow == xWindow)
			mData->mainXWindow = 0;
	}

	Pixmap LinuxPlatform::createPixmap(const PixelData& data)
	{
		SPtr<PixelData> bgraData = PixelData::create(data.getWidth(), data.getHeight(), 1, PF_BGRA8);
		PixelUtil::bulkPixelConversion(data, *bgraData);

		UINT32 depth = (UINT32)XDefaultDepth(mData->xDisplay, 0);
		XImage* image = XCreateImage(mData->xDisplay, XDefaultVisual(mData->xDisplay, 0), depth, ZPixmap, 0,
				(char*)bgraData->getData(), data.getWidth(), data.getHeight(), 32, 0);

		Pixmap pixmap = XCreatePixmap(mData->xDisplay, XDefaultRootWindow(mData->xDisplay),
				data.getWidth(), data.getHeight(), depth);

		XGCValues gcValues;
		GC gc = XCreateGC(mData->xDisplay, pixmap, 0, &gcValues);
		XPutImage(mData->xDisplay, pixmap, gc, image, 0, 0, 0, 0, data.getWidth(), data.getHeight());
		XFreeGC(mData->xDisplay, gc);

		// Make sure XDestroyImage doesn't free the data pointed to by 'data.bytes'
		image->data = nullptr;
		XDestroyImage(image);

		return pixmap;
	}
}