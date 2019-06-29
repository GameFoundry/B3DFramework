//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsPixelData.h"
#include "Image/BsPixelUtil.h"
#include "String/BsUnicode.h"
#include "Private/Linux/BsLinuxInput.h"
#include "Private/Linux/BsLinuxPlatform.h"
#include "Private/Linux/BsLinuxWindow.h"
#include "Private/Linux/BsLinuxDropTarget.h"
#include "RenderAPI/BsRenderWindow.h"
#include "BsCoreApplication.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>
#include <pwd.h>

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

	Mutex LinuxPlatform::eventLock;
	Queue<LinuxButtonEvent> LinuxPlatform::buttonEvents;
	LinuxMouseMotionEvent LinuxPlatform::mouseMotionEvent;

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
	};

	struct Platform::Pimpl
	{
		::Display* xDisplay = nullptr;
		::Window mainXWindow = 0;
		::Window fullscreenXWindow = 0;
		UnorderedMap<::Window, LinuxWindow*> windowMap;
		Mutex lock;

		XIM IM;
		XIC IC;
		::Time lastButtonPressTime;

		Atom atomDeleteWindow;
		Atom atomWmState;
		Atom atomWmStateHidden;
		Atom atomWmStateMaxVert;
		Atom atomWmStateMaxHorz;

		// X11 Event handling
		int xInput2Opcode;
		UnorderedMap<String, KeyCode> keyNameMap; /**< Maps X11 key name (e.g. "TAB") to system-specific X11 KeyCode. */
		Vector<ButtonCode> keyCodeMap; /**< Maps system-specific X11 KeyCode to Banshee ButtonCode. */

		// Clipboard
		String clipboardData;

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
			clippedX = 0;
		else if(clippedX >= (INT32)data->cursorClipRect.width)
			clippedX = data->cursorClipRect.width > 0 ? data->cursorClipRect.width - 1 : 0;

		if(clippedY < 0)
			clippedY = 0;
		else if(clippedY >= (INT32)data->cursorClipRect.height)
			clippedY = data->cursorClipRect.height > 0 ? data->cursorClipRect.height - 1 : 0;

		clippedX += data->cursorClipRect.x;
		clippedY += data->cursorClipRect.y;

		if(clippedX != pos.x || clippedY != pos.y)
		{
			pos.x = clippedX;
			pos.y = clippedY;

			return true;
		}

		return false;
	}

	void clipCursorDisable(Platform::Pimpl* data)
	{
		data->cursorClipEnabled = false;
		data->cursorClipWindow = None;
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
		BS_LOG(Warning, Platform, "X11 error: " + String(buffer));

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

		bs::clipCursorDisable(mData);
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

		linuxWindow->_setDragZones(nonClientAreas);
	}

	void Platform::setResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
	{
		// Do nothing, resize areas not supported on Linux (but they are provided even on undecorated windows by the WM)
	}

	void Platform::resetNonClientAreas(const ct::RenderWindow& window)
	{
		Lock lock(mData->lock);

		LinuxWindow* linuxWindow;
		window.getCustomAttribute("LINUX_WINDOW", &linuxWindow);

		linuxWindow->_setDragZones({});
	}

	void Platform::sleep(UINT32 duration)
	{
		usleep(duration * 1000);
	}

	void Platform::copyToClipboard(const String& string)
	{
		Lock lock(mData->lock);
		mData->clipboardData = string;

		Atom clipboardAtom = XInternAtom(mData->xDisplay, "CLIPBOARD", 0);
		XSetSelectionOwner(mData->xDisplay, clipboardAtom, mData->mainXWindow, CurrentTime);
	}

	String Platform::copyFromClipboard()
	{
		Lock lock(mData->lock);
		Atom clipboardAtom = XInternAtom(mData->xDisplay, "CLIPBOARD", 0);
		::Window selOwner = XGetSelectionOwner(mData->xDisplay, clipboardAtom);

		if(selOwner == None)
			return "";

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
				return String((const char*)data);

			XFree(data);
		}

		return "";
	}

	/** Maps X11 mouse button codes to Banshee button codes. */
	ButtonCode xButtonToButtonCode(int button)
	{
		switch (button)
		{
		case Button1:
			return BC_MOUSE_LEFT;
		case Button2:
			return BC_MOUSE_MIDDLE;
		case Button3:
			return BC_MOUSE_RIGHT;
		default:
			return (ButtonCode)(BC_MOUSE_LEFT + button - 1);
		}
	}

	/** Maps Banshee button codes to X11 names for physical key locations. */
	const char* buttonCodeToKeyName(ButtonCode code)
	{
		switch(code)
		{
			// Row #1
		case BC_ESCAPE:		return "ESC";
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
		case BC_F13:		return "FK13";
		case BC_F14:		return "FK14";
		case BC_F15:		return "FK15";

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
		case BC_RETURN:		return "RTRN";

			// Row #4
		case BC_CAPITAL:	return "CAPS";
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
		case BC_LSHIFT:		return "LFSH";
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
		case BC_RSHIFT:		return "RTSH";

			// Row #6
		case BC_LCONTROL:	return "LCTL";
		case BC_LWIN:		return "LWIN";
		case BC_LMENU:		return "LALT";
		case BC_SPACE:		return "SPCE";
		case BC_RMENU:		return "RALT";
		case BC_RWIN:		return "RWIN";
		case BC_RCONTROL:	return "RCTL";

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

		case BC_NUMLOCK:		return "NMLK";
		case BC_DIVIDE:			return "KPDV";
		case BC_MULTIPLY:		return "KPMU";
		case BC_SUBTRACT:		return "KPSU";
		case BC_ADD:			return "KPAD";
		case BC_DECIMAL:		return "KPDL";
		case BC_NUMPADENTER:	return "KPEN";
		case BC_NUMPADEQUALS:	return "KPEQ";

			// Special keys
		case BC_SCROLL:		return "SCLK";
		case BC_PAUSE:		return "PAUS";

		case BC_INSERT:		return "INS";
		case BC_HOME:		return "HOME";
		case BC_PGUP:		return "PGUP";
		case BC_DELETE:		return "DELE";
		case BC_END:		return "END";
		case BC_PGDOWN:		return "PGDN";

		case BC_UP:			return "UP";
		case BC_LEFT:		return "LEFT";
		case BC_DOWN:		return "DOWN";
		case BC_RIGHT:		return "RGHT";

		case BC_MUTE:		return "MUTE";
		case BC_VOLUMEDOWN:	return "VOL-";
		case BC_VOLUMEUP:	return "VOL+";
		case BC_POWER:		return "POWR";

			// International keys
		case BC_OEM_102:	return "LSGT"; // German keyboard: < > |
		case BC_KANA:		return "AB11"; // Taking a guess here, many layouts map <AB11> to "kana_RO"
		case BC_YEN:		return "AE13"; // Taking a guess, often mapped to yen

		default:
			// Missing Japanese (?): KATA, HIRA, HENK, MUHE, JPCM
			// Missing Korean (?): HNGL, HJCV
			// Missing because it's not clear which BC_ is correct: PRSC (print screen), LVL3 (AltGr), MENU
			// Misc: LNFD (line feed), I120, I126, I128, I129, COMP, STOP, AGAI (redo), PROP, UNDO, FRNT, COPY, OPEN, PAST
			// FIND, CUT, HELP, I147-I190, FK16-FK24, MDSW (mode switch), ALT, META, SUPR, HYPR, I208-I253
			break;
		}

		return nullptr;
	}

	String Platform::keyCodeToUnicode(UINT32 buttonCode)
	{
		Lock lock(mData->lock);

		const char* keyName = buttonCodeToKeyName((ButtonCode)buttonCode);
		if(keyName == nullptr)
		{
			// Not a printable key
			return "";
		}

		auto iterFind = mData->keyNameMap.find(String(keyName));
		if(iterFind == mData->keyNameMap.end())
		{
			// Cannot find mapping, although this shouldn't really happen
			return "";
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

			return String(buffer);
		}

		return "";
	}

	void Platform::openFolder(const Path& path)
	{
		String pathString = path.toString();

		const char* commandPattern = "xdg-open '%s'";

		char* commandStr = (char*)bs_stack_alloc((UINT32)pathString.size() + (UINT32)strlen(commandPattern) + 1);
		sprintf(commandStr, commandPattern, pathString.c_str());

		if(system(commandStr)){};
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
		case XK_Tab:
			command = InputCommandType::Tab;
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

	/**
	 * Enqueue a button press/release event to be handled by the main thread
	 *
	 * @param bc        ButtonCode for the button that was pressed or released
	 * @param pressed   true if the button was pressed, false if it was released
	 * @param timestamp Time when the event happened
	 */
	void enqueueButtonEvent(ButtonCode bc, bool pressed, UINT64 timestamp)
	{
		if (bc == BC_UNASSIGNED)
			return;

		Lock eventLock(LinuxPlatform::eventLock);

		LinuxButtonEvent event;
		event.button = bc;
		event.pressed = pressed;
		event.timestamp = timestamp;
		LinuxPlatform::buttonEvents.push(event);
	}

	void Platform::_messagePump()
	{
		while(true)
		{
			Lock lock(mData->lock);

			if(XPending(mData->xDisplay) <= 0)
				break;

			XEvent event;
			XNextEvent(mData->xDisplay, &event);

			XGenericEventCookie* cookie = &event.xcookie;
			if (cookie->type == GenericEvent && cookie->extension == mData->xInput2Opcode)
			{
				XGetEventData(mData->xDisplay, cookie);
				XIRawEvent* xInput2Event = (XIRawEvent*) cookie->data;
				switch (xInput2Event->evtype)
				{
				case XI_RawMotion:
					if (xInput2Event->valuators.mask_len > 0)
					{
						// Assume X/Y delta is stored in valuators 0/1 and vertical scroll in valuator 3.
						// While there is an API that reliably tells us the valuator index for vertical scroll, there's
						// nothing "more reliable" for X/Y axes, as the only way to possibly identify them from device
						// info is by axis name, so we can use the axis index directly just as well. GDK seems to assume
						// 0 for x and 1 for y too, so that's hopefully safe, and 3 appears to be common for the scroll
						// wheel.
						float deltas[4] = {0};
						int currentValuesIndex = 0;
						for (unsigned int valuator = 0; valuator < 4; valuator++)
							if (XIMaskIsSet(xInput2Event->valuators.mask, valuator))
								deltas[valuator] = xInput2Event->raw_values[currentValuesIndex++];

						Lock eventLock(LinuxPlatform::eventLock);
						LinuxPlatform::mouseMotionEvent.deltaX += deltas[0];
						LinuxPlatform::mouseMotionEvent.deltaY += deltas[1];
						LinuxPlatform::mouseMotionEvent.deltaZ += deltas[3]; // Not a typo - 2 is for horizontal scroll.
					}
					break;
				}

				XFreeEventData(mData->xDisplay, cookie);
			}


			switch (event.type)
			{
			case ClientMessage:
			{
				if(LinuxDragAndDrop::handleClientMessage(event.xclient))
					break;

				// User requested the window to close
				if((Atom)event.xclient.data.l[0] == mData->atomDeleteWindow)
				{
					LinuxWindow* window = getLinuxWindow(mData, event.xclient.window);
					if(window != nullptr)
					{
						// If it's a render window we allow the client code to handle the message
						ct::RenderWindow* renderWindow = (ct::RenderWindow*)window->_getUserData();
						if(renderWindow != nullptr)
							renderWindow->_notifyWindowEvent(WindowEventType::CloseRequested);
						else // If not, we just destroy the window
							window->_destroy();
					}
				}
			}
				break;
			case KeyPress:
			{
				XKeyPressedEvent* keyEvent = (XKeyPressedEvent*) &event;
				enqueueButtonEvent(mData->keyCodeMap[keyEvent->keycode], true, (UINT64) keyEvent->time);

				// Process text input
				KeySym keySym = XkbKeycodeToKeysym(mData->xDisplay, (KeyCode)event.xkey.keycode, 0, 0);

				// Handle input commands
				InputCommandType command = InputCommandType::Backspace;
				bool shift = (event.xkey.state & ShiftMask) != 0;

				bool isInputCommand = parseInputCommand(keySym, shift, command);

				// Check if input manager wants this event. If not, we process it.
				if(XFilterEvent(&event, None) == False && !isInputCommand)
				{
					// Send a text input event
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

				// Send an input command event
				if(isInputCommand)
				{
					if(!onInputCommand.empty())
						onInputCommand(command);
				}
			}
				break;
			case KeyRelease:
			{
				XKeyReleasedEvent* keyEvent = (XKeyReleasedEvent*) &event;
				enqueueButtonEvent(mData->keyCodeMap[keyEvent->keycode], false, (UINT64) keyEvent->time);
			}
				break;
			case ButtonPress:
			{
				XButtonPressedEvent* buttonEvent = (XButtonPressedEvent*) &event;
				UINT32 button = event.xbutton.button;
				enqueueButtonEvent(xButtonToButtonCode(button), true, (UINT64) buttonEvent->time);

				OSPointerButtonStates btnStates;
				btnStates.mouseButtons[0] = (event.xbutton.state & Button1Mask) != 0;
				btnStates.mouseButtons[1] = (event.xbutton.state & Button2Mask) != 0;
				btnStates.mouseButtons[2] = (event.xbutton.state & Button3Mask) != 0;

				OSMouseButton mouseButton;
				bool validPress = false;
				switch(button)
				{
				case Button1:
					mouseButton = OSMouseButton::Left;
					btnStates.mouseButtons[0] = true;
					validPress = true;
					break;
				case Button2:
					mouseButton = OSMouseButton::Middle;
					btnStates.mouseButtons[1] = true;
					validPress = true;
					break;
				case Button3:
					mouseButton = OSMouseButton::Right;
					btnStates.mouseButtons[2] = true;
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

					btnStates.ctrl = (event.xbutton.state & ControlMask) != 0;
					btnStates.shift = (event.xbutton.state & ShiftMask) != 0;

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
						window->_dragStart(event.xbutton);
				}

				break;
			}
			case ButtonRelease:
			{
				XButtonReleasedEvent* buttonEvent = (XButtonReleasedEvent*) &event;
				UINT32 button = event.xbutton.button;
				enqueueButtonEvent(xButtonToButtonCode(button), false, (UINT64) buttonEvent->time);

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
					btnStates.mouseButtons[0] = false;
					onCursorButtonReleased(pos, OSMouseButton::Left, btnStates);
					break;
				case Button2:
					btnStates.mouseButtons[1] = false;
					onCursorButtonReleased(pos, OSMouseButton::Middle, btnStates);
					break;
				case Button3:
					btnStates.mouseButtons[2] = false;
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
					renderWindow->_notifyWindowEvent(WindowEventType::MouseLeft);
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
					{
						renderWindow->_notifyWindowEvent(WindowEventType::Resized);
						renderWindow->_notifyWindowEvent(WindowEventType::Moved);
					}
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
						renderWindow->_notifyWindowEvent(WindowEventType::FocusReceived);
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
						renderWindow->_notifyWindowEvent(WindowEventType::FocusLost);
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
					String utf8data = mData->clipboardData;

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
					// Check that the window hasn't been destroyed
					if(getLinuxWindow(mData, event.xproperty.window) == nullptr)
						break;

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
									renderWindow->_notifyWindowEvent(WindowEventType::Maximized);
								else
									renderWindow->_notifyWindowEvent(WindowEventType::Restored);
							}

							if(atoms[i] == mData->atomWmStateHidden)
							{
								if(event.xproperty.state == PropertyNewValue)
									renderWindow->_notifyWindowEvent(WindowEventType::Minimized);
								else
									renderWindow->_notifyWindowEvent(WindowEventType::Restored);
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

		// For raw, relative mouse motion events, XInput2 extension is required
		int firstEvent;
		int firstError;
		if (!XQueryExtension(mData->xDisplay, "XInputExtension", &mData->xInput2Opcode, &firstEvent, &firstError))
			BS_EXCEPT(InternalErrorException, "X Server doesn't support the XInput extension");

		int majorVersion = 2;
		int minorVersion = 0;
		if (XIQueryVersion(mData->xDisplay, &majorVersion, &minorVersion) != Success)
			BS_EXCEPT(InternalErrorException, "X Server doesn't support at least the XInput 2.0 extension");

		// Let XInput know we are interested in raw mouse movement events
		constexpr int maskLen = XIMaskLen(XI_LASTEVENT);
		XIEventMask mask;
		mask.deviceid = XIAllDevices;
		mask.mask_len = maskLen;

		unsigned char maskBuffer[maskLen] = {0};
		mask.mask = maskBuffer;
		XISetMask(mask.mask, XI_RawMotion);

		// "RawEvents are sent exclusively to all root windows", so this should receive all events, even though we only
		// select on one display's root window (untested for lack of second screen).
		XISelectEvents(mData->xDisplay, XRootWindow(mData->xDisplay, DefaultScreen(mData->xDisplay)), &mask, 1);
		XFlush(mData->xDisplay);

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

		// Initialize "unique X11 keyname" -> "X11 keycode" map
		char name[XkbKeyNameLength + 1];

		XkbDescPtr desc = XkbGetMap(mData->xDisplay, 0, XkbUseCoreKbd);
		XkbGetNames(mData->xDisplay, XkbKeyNamesMask, desc);

		for (UINT32 keyCode = desc->min_key_code; keyCode <= desc->max_key_code; keyCode++)
		{
			memcpy(name, desc->names->keys[keyCode].name, XkbKeyNameLength);
			name[XkbKeyNameLength] = '\0';

			mData->keyNameMap[String(name)] = keyCode;
		}

		// Initialize "X11 keycode" -> "Banshee ButtonCode" map, based on the keyNameMap and keyCodeToKeyName()
		mData->keyCodeMap.resize(desc->max_key_code + 1, BC_UNASSIGNED);

		XkbFreeNames(desc, XkbKeyNamesMask, True);
		XkbFreeKeyboard(desc, 0, True);

		for (UINT32 buttonCodeNum = BC_UNASSIGNED; buttonCodeNum <= BC_NumKeys; buttonCodeNum++)
		{
			ButtonCode buttonCode = (ButtonCode) buttonCodeNum;
			const char* keyNameCStr = buttonCodeToKeyName(buttonCode);

			if (keyNameCStr != nullptr)
			{
				String keyName = String(keyNameCStr);
				auto iterFind = mData->keyNameMap.find(keyName);
				if (iterFind != mData->keyNameMap.end())
				{
					KeyCode keyCode = iterFind->second;
					mData->keyCodeMap[keyCode] = buttonCode;
				}
			}
		}
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

	Path LinuxPlatform::getHomeDir()
	{
		const char* homeDir = getenv("HOME");
		if(!homeDir)
			homeDir = getpwuid(getuid())->pw_dir;

		return Path(homeDir);
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
				bs::clipCursorDisable(mData);

			mData->windowMap.erase(iterFind);
		}

		if(mData->mainXWindow == xWindow)
			mData->mainXWindow = 0;
	}

	Pixmap LinuxPlatform::createPixmap(const PixelData& data, UINT32 depth)
	{
		// Premultiply alpha
		Vector<Color> colors = data.getColors();
		for(auto& color : colors)
		{
			color.r *= color.a;
			color.g *= color.a;
			color.b *= color.a;
		}

		// Convert to BGRA
		SPtr<PixelData> bgraData = PixelData::create(data.getWidth(), data.getHeight(), 1, PF_BGRA8);
		bgraData->setColors(colors);

		XImage* image = XCreateImage(mData->xDisplay, CopyFromParent, depth, ZPixmap, 0,
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
