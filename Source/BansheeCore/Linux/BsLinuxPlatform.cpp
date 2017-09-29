//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Image/BsPixelData.h"
#include "Image/BsPixelUtil.h"
#include "String/BsUnicode.h"
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxWindow.h"
#include "RenderAPI/BsRenderWindow.h"
#include "BsLinuxDragAndDrop.h"
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>

namespace bs
{
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorMoved;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonPressed;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonReleased;
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorDoubleClick;
	Event<void(InputCommandType)> Platform::onInputCommand;
	Event<void(float)> Platform::onMouseWheelScrolled;
	Event<void(UINT32)> Platform::onCharInput;

	Event<void(ct::RenderWindow*)> Platform::onMouseLeftWindow;
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
		std::unordered_map<::Window, LinuxWindow*> windowMap;
		Mutex lock;

		XIM IM;
		XIC IC;
		Time lastButtonPressTime;

		Atom atomDeleteWindow;

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

		int32_t clippedX = pos.x - data->cursorClipRect.x;
		int32_t clippedY = pos.y - data->cursorClipRect.y;

		if(clippedX < 0)
			clippedX = data->cursorClipRect.x + data->cursorClipRect.width + clippedX;
		else if(clippedX >= data->cursorClipRect.width)
			clippedX = data->cursorClipRect.x + (clippedX - data->cursorClipRect.width);
		else
			clippedX = data->cursorClipRect.x + clippedX;

		if(clippedY < 0)
			clippedY = data->cursorClipRect.y + data->cursorClipRect.height + clippedY;
		else if(clippedY >= data->cursorClipRect.height)
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
		window.getCustomAttribute("WINDOW", &linuxWindow);

		uint32_t mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
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
		window.getCustomAttribute("WINDOW", &linuxWindow);
		::Window xWindow = linuxWindow->_getXWindow();

		Vector2I pos;
		UINT32 screenCount = (UINT32)XScreenCount(mData->xDisplay);

		for (UINT32 i = 0; i < screenCount; ++i)
		{
			::Window outRoot, outChild;
			INT32 childX, childY;
			UINT32 mask;
			if(XQueryPointer(mData->xDisplay, XRootWindow(mData->xDisplay, i), &outRoot, &outChild, &pos.x,
					&pos.y, &childX, &childY, &mask))
			{
				return outChild == xWindow;
			}
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
		window.getCustomAttribute("WINDOW", &linuxWindow);

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
		image->xhot = hotSpot.x;
		image->yhot = hotSpot.y;
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
		window.getCustomAttribute("WINDOW", &linuxWindow);

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

	OSDropTarget& Platform::createDropTarget(const RenderWindow* window, INT32 x, INT32 y, UINT32 width, UINT32 height)
	{
		return LinuxDragAndDrop::createDropTarget(window, x, y, width, height);
	}

	void Platform::destroyDropTarget(OSDropTarget& target)
	{
		LinuxDragAndDrop::destroyDropTarget(target);
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

	WString Platform::keyCodeToUnicode(UINT32 keyCode)
	{
		Lock lock(mData->lock);
		// TODOPORT
		return L"";
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

			switch (event.type)
			{
			case ClientMessage:
			{
				if(LinuxDragAndDrop::handleClientMessage(event.xclient))
					break;

				if(event.xclient.data.l[0] == mData->atomDeleteWindow)
					XDestroyWindow(mData->xDisplay, event.xclient.window);
			}
				break;
			case DestroyNotify:
			{
				auto iterFind = mData->windowMap.find(event.xdestroywindow.window);
				if (iterFind == mData->windowMap.end())
					break;

				LinuxWindow* window = iterFind->second;
				window->_cleanUp();

				if(mData->mainXWindow == 0)
					return;
			}
				break;
			case KeyPress:
			{
				// Process text input
				//// Check if input manager wants this event. If not, we process it.
				if(!XFilterEvent(&event, None))
				{
					Status status;
					char buffer[16];

					INT32 length = Xutf8LookupString(mData->IC, &event.xkey, buffer, sizeof(buffer), nullptr,
							&status);

					if(length > 0)
					{
						buffer[length] = '\0';

						U32String utfStr = UTF8::toUTF32(String(buffer));
						if(utfStr.length() > 0)
							onCharInput((UINT32)utfStr[0]);
					}
				}

				// Process normal key press
				{
					static XComposeStatus keyboard;
					uint8_t buffer[16];
					KeySym symbol;
					XLookupString(&event.xkey, (char*)buffer, sizeof(buffer), &symbol, &keyboard);

					bool alt = event.xkey.state & Mod1Mask;
					bool control = event.xkey.state & ControlMask;
					bool shift = event.xkey.state & ShiftMask;

					// TODOPORT - Report key press
				}
			}
				break;
			case KeyRelease:
			{
				uint8_t buffer[16];
				KeySym symbol;
				XLookupString(&event.xkey, (char *) buffer, sizeof(buffer), &symbol, nullptr);

				bool alt = (event.xkey.state & Mod1Mask) != 0;
				bool control = (event.xkey.state & ControlMask) != 0;
				bool shift = (event.xkey.state & ShiftMask) != 0;

				// TODOPORT - Report key release
			}
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
					auto iterFind = mData->windowMap.find(event.xbutton.window);
					if (iterFind != mData->windowMap.end())
					{
						LinuxWindow* window = iterFind->second;
						window->_dragStart(event.xbutton.x, event.xbutton.y);
					}
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
					auto iterFind = mData->windowMap.find(event.xbutton.window);
					if (iterFind != mData->windowMap.end())
					{
						LinuxWindow* window = iterFind->second;
						window->_dragEnd();
					}
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
				auto iterFind = mData->windowMap.find(event.xmotion.window);
				if (iterFind != mData->windowMap.end())
				{
					LinuxWindow* window = iterFind->second;
					window->_dragUpdate(event.xmotion.x, event.xmotion.y);
				}
			}
				break;
			case EnterNotify:
				if(event.xcrossing.mode == NotifyNormal)
				{
					// TODOPORT - Send mouse enter event
				}
				break;
			case LeaveNotify:
				if(event.xcrossing.mode == NotifyNormal)
				{
					Vector2I pos;
					pos.x = event.xcrossing.x_root;
					pos.y = event.xcrossing.y_root;

					if(clipCursor(mData, pos))
						_setCursorPosition(mData, pos);

					// TODOPORT - Send mouse leave event
				}
				break;
			case ConfigureNotify:
			{
				const XConfigureEvent &xce = event.xconfigure;

				auto iterFind = mData->windowMap.find(event.xdestroywindow.window);
				if (iterFind == mData->windowMap.end())
					break;

				LinuxWindow* window = iterFind->second;
				updateClipBounds(mData, window);

				// TODOPORT - Send move/resize event
			}
				break;
			case FocusIn:
				// Update input context focus
				XSetICFocus(mData->IC);
				break;
			case FocusOut:
				// Update input context focus
				XUnsetICFocus(mData->IC);
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
			default:
				break;
			}
		}
	}

	void Platform::_startUp()
	{
		Lock lock(mData->lock);
		mData->xDisplay = XOpenDisplay(nullptr);

		if(XSupportsLocale())
		{
			XSetLocaleModifiers("");
			mData->IM = XOpenIM(mData->xDisplay, nullptr, nullptr, nullptr);

			// Note: Currently our windows don't support pre-edit and status areas, which are used for more complex types
			// of character input. Later on it might be beneficial to support them.
			mData->IC = XCreateIC(mData->IM, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, nullptr);
		}

		mData->atomDeleteWindow = XInternAtom(mData->xDisplay, "WM_DELETE_WINDOW", False);

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