//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLinuxWindow.h"
#include "BsLinuxPlatform.h"
#include "BsLinuxDropTarget.h"

#include <X11/Xatom.h>

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
#define _NET_WM_STATE_TOGGLE 2

#define WM_NormalState 1
#define WM_IconicState 3

namespace bs
{
	enum class WindowState
	{
		Minimized,
		Maximized,
		Normal
	};

	struct LinuxWindow::Pimpl
	{
		::Window xWindow = 0;

		INT32 x, y;
		UINT32 width, height;
		bool hasTitleBar = true;
		bool dragInProgress = false;
		bool resizeDisabled = false;
		WindowState state = WindowState::Normal;

		Rect2I dragZone;
		INT32 dragStartX, dragStartY;

		void* userData = nullptr;
	};

	LinuxWindow::LinuxWindow(const WINDOW_DESC &desc)
	{
		m = bs_new<Pimpl>();

		::Display* display = LinuxPlatform::getXDisplay();

		INT32 screen;
		if(desc.screen == (UINT32)-1)
			screen = XDefaultScreen(display);
		else
			screen = std::min((INT32)desc.screen, XScreenCount(display));

		XSetWindowAttributes attributes;
		attributes.background_pixel = XWhitePixel(display, screen);
		attributes.border_pixel = XBlackPixel(display, screen);

		attributes.colormap = XCreateColormap(display,
				XRootWindow(display, screen),
				desc.visualInfo.visual,
				AllocNone);

		UINT32 borderWidth = 0;

		m->x = desc.x;
		m->y = desc.y;
		m->width = desc.width;
		m->height = desc.height;
		m->xWindow = XCreateWindow(display,
				XRootWindow(display, screen),
				desc.x, desc.y,
				desc.width, desc.height,
				borderWidth, desc.visualInfo.depth,
				InputOutput, desc.visualInfo.visual,
				CWBackPixel | CWBorderPixel | CWColormap, &attributes);

		XStoreName(display, m->xWindow, desc.title.c_str());

		XSizeHints hints;
		hints.flags = PPosition | PSize;
		hints.x = desc.x;
		hints.y = desc.y;
		hints.width = desc.width;
		hints.height = desc.height;

		if(!desc.allowResize)
		{
			hints.flags |= PMinSize | PMaxSize;

			hints.min_height = desc.height;
			hints.max_height = desc.height;

			hints.min_width = desc.width;
			hints.max_width = desc.width;
		}

		XSetNormalHints(display, m->xWindow, &hints);

		setShowDecorations(desc.showDecorations);
		setIsModal(desc.modal);

		// Ensures the child window is always on top of the parent window
		if(desc.parent)
			XSetTransientForHint(display, m->xWindow, desc.parent);

		XSelectInput(display, m->xWindow,
				ExposureMask | FocusChangeMask |
				KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask |
				EnterWindowMask | LeaveWindowMask |
				PointerMotionMask | ButtonMotionMask |
				StructureNotifyMask | PropertyChangeMask
		);
		XMapWindow(display, m->xWindow);

		// Make sure we get the window delete message from WM, so we can clean up ourselves
		Atom atomDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, m->xWindow, &atomDeleteWindow, 1);

		// Enable drag and drop
		LinuxDragAndDrop::makeDNDAware(m->xWindow);

		// Set background image if assigned
		if(desc.background)
		{
			Pixmap pixmap = LinuxPlatform::createPixmap(*desc.background);
			XSetWindowBackgroundPixmap(display, m->xWindow, pixmap);
			XFreePixmap(display, pixmap);
		}

		m->hasTitleBar = desc.showDecorations;
		m->resizeDisabled = !desc.allowResize;

		LinuxPlatform::_registerWindow(m->xWindow, this);
	}

	LinuxWindow::~LinuxWindow()
	{
		if(m->xWindow != 0)
			_cleanUp();

		bs_delete(m);
	}

	void LinuxWindow::close()
	{
		XDestroyWindow(LinuxPlatform::getXDisplay(), m->xWindow);
		XFlush(LinuxPlatform::getXDisplay());

		_cleanUp();
	}

	void LinuxWindow::move(INT32 x, INT32 y)
	{
		m->x = x;
		m->y = y;

		XMoveWindow(LinuxPlatform::getXDisplay(), m->xWindow, x, y);
	}

	void LinuxWindow::resize(UINT32 width, UINT32 height)
	{
		// If resize is disabled on WM level, we need to force it
		if(m->resizeDisabled)
		{
			XSizeHints hints;
			hints.flags = PMinSize | PMaxSize;

			hints.min_height = height;
			hints.max_height = height;

			hints.min_width = width;
			hints.max_width = width;

			XSetNormalHints(LinuxPlatform::getXDisplay(), m->xWindow, &hints);
		}

		m->width = width;
		m->height = height;

		XResizeWindow(LinuxPlatform::getXDisplay(), m->xWindow, width, height);
	}

	void LinuxWindow::hide()
	{
		// TODOPORT - Need to track all states so I can restore them on show()

		XUnmapWindow(LinuxPlatform::getXDisplay(), m->xWindow);
	}

	void LinuxWindow::show()
	{
		XMapWindow(LinuxPlatform::getXDisplay(), m->xWindow);
		XMoveResizeWindow(LinuxPlatform::getXDisplay(), m->xWindow, m->x, m->y, m->width, m->height);

		// TODOPORT - Restore all states (pos, size and style)
	}

	void LinuxWindow::maximize()
	{
		maximize(true);
	}

	void LinuxWindow::minimize()
	{
		minimize(true);
	}

	void LinuxWindow::restore()
	{
		if(isMaximized())
			maximize(false);
		else if(isMinimized())
			minimize(false);
	}

	INT32 LinuxWindow::getLeft() const
	{
		INT32 x, y;
		::Window child;
		XTranslateCoordinates(LinuxPlatform::getXDisplay(), m->xWindow, DefaultRootWindow(LinuxPlatform::getXDisplay()),
				0, 0, &x, &y, &child);

		return x;
	}

	INT32 LinuxWindow::getTop() const
	{
		INT32 x, y;
		::Window child;
		XTranslateCoordinates(LinuxPlatform::getXDisplay(), m->xWindow, DefaultRootWindow(LinuxPlatform::getXDisplay()),
				0, 0, &x, &y, &child);

		return y;
	}

	UINT32 LinuxWindow::getWidth() const
	{
		XWindowAttributes xwa;
		XGetWindowAttributes(LinuxPlatform::getXDisplay(), m->xWindow, &xwa);

		return (UINT32)xwa.width;
	}

	UINT32 LinuxWindow::getHeight() const
	{
		XWindowAttributes xwa;
		XGetWindowAttributes(LinuxPlatform::getXDisplay(), m->xWindow, &xwa);

		return (UINT32)xwa.height;
	}

	Vector2I LinuxWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		Vector2I screenPos;

		::Window child;
		XTranslateCoordinates(LinuxPlatform::getXDisplay(), m->xWindow, DefaultRootWindow(LinuxPlatform::getXDisplay()),
				windowPos.x, windowPos.y, &screenPos.x, &screenPos.y, &child);

		return screenPos;
	}

	Vector2I LinuxWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		Vector2I windowPos;

		::Window child;
		XTranslateCoordinates(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), m->xWindow,
				screenPos.x, screenPos.y, &windowPos.x, &windowPos.y, &child);

		return windowPos;
	}

	void LinuxWindow::setIcon(const PixelData& data)
	{
		Pixmap iconPixmap = LinuxPlatform::createPixmap(data);

		XWMHints* hints = XAllocWMHints();
		hints->flags = IconPixmapHint;
		hints->icon_pixmap = iconPixmap;

		XSetWMHints(LinuxPlatform::getXDisplay(), m->xWindow, hints);
		XFlush(LinuxPlatform::getXDisplay());

		XFree(hints);
		XFreePixmap(LinuxPlatform::getXDisplay(), iconPixmap);
	}

	void LinuxWindow::_cleanUp()
	{
		LinuxPlatform::_unregisterWindow(m->xWindow);
		m->xWindow = 0;
	}

	void LinuxWindow::_setDragZone(const Rect2I& rect)
	{
		m->dragZone = rect;
	}

	bool LinuxWindow::_dragStart(INT32 x, INT32 y)
	{
		if(m->hasTitleBar)
			return false;

		if(m->dragZone.width == 0 || m->dragZone.height == 0)
			return false;

		if(x >= m->dragZone.x && x < (INT32)(m->dragZone.x + m->dragZone.width) &&
		   y >= m->dragZone.y && y < (INT32)(m->dragZone.y + m->dragZone.height))
		{
			m->dragStartX = x;
			m->dragStartY = y;

			m->dragInProgress = true;
			return true;
		}

		return false;
	}

	void LinuxWindow::_dragUpdate(INT32 x, INT32 y)
	{
		if(!m->dragInProgress)
			return;

		INT32 offsetX = x - m->dragStartX;
		INT32 offsetY = y - m->dragStartY;

		move(getLeft() + offsetX, getTop() + offsetY);
	}

	void LinuxWindow::_dragEnd()
	{
		m->dragInProgress = false;
	}

	::Window LinuxWindow::_getXWindow() const
	{
		return m->xWindow;
	}

	void LinuxWindow::_setUserData(void* data)
	{
		m->userData = data;
	}

	void* LinuxWindow::_getUserData() const
	{
		return m->userData;
	}

	bool LinuxWindow::isMaximized() const
	{
		Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE", False);
		Atom type;
		INT32 format;
		uint64_t length;
		uint64_t remaining;
		uint8_t* data = nullptr;

		INT32 result = XGetWindowProperty(LinuxPlatform::getXDisplay(), m->xWindow, wmState,
				0, 1024, False, XA_ATOM, &type, &format,
				&length, &remaining, &data);

		if (result == Success)
		{
			Atom* atoms = (Atom*)data;
			Atom wmMaxHorz = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
			Atom wmMaxVert = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);

			bool foundHorz = false;
			bool foundVert = false;
			for (UINT32 i = 0; i < length; i++)
			{
				if (atoms[i] == wmMaxHorz)
					foundHorz = true;
				if (atoms[i] == wmMaxVert)
					foundVert = true;

				if (foundVert && foundHorz)
					return true;
			}

			XFree(atoms);
		}

		return false;
	}

	bool LinuxWindow::isMinimized()
	{
		Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "WM_STATE", True);
		Atom type;
		INT32 format;
		uint64_t length;
		uint64_t remaining;
		uint8_t* data = nullptr;

		INT32 result = XGetWindowProperty(LinuxPlatform::getXDisplay(), m->xWindow, wmState,
				0, 1024, False, AnyPropertyType, &type, &format,
				&length, &remaining, &data);

		if(result == Success)
		{
			long* state = (long*) data;
			if(state[0] == WM_IconicState)
				return true;
		}

		return false;
	}

	void LinuxWindow::maximize(bool enable)
	{
		Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE", False);
		Atom wmMaxHorz = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		Atom wmMaxVert = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);

		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->xWindow;
		xev.xclient.message_type = wmState;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = enable ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = wmMaxHorz;
		xev.xclient.data.l[2] = wmMaxVert;

		XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
				SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}

	void LinuxWindow::minimize(bool enable)
	{
		XEvent xev;
		Atom wmChange = XInternAtom(LinuxPlatform::getXDisplay(), "WM_CHANGE_STATE", False);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->xWindow;
		xev.xclient.message_type = wmChange;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = enable ? WM_IconicState : WM_NormalState;

		XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
				SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}

	void LinuxWindow::_setFullscreen(bool fullscreen)
	{
		// Attempt to bypass compositor if switching to fullscreen
		if(fullscreen)
		{
			Atom wmBypassCompositor = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_BYPASS_COMPOSITOR", False);
			if (wmBypassCompositor)
			{
				static constexpr UINT32 enabled = 1;

				XChangeProperty(LinuxPlatform::getXDisplay(), m->xWindow, wmBypassCompositor,
						XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &enabled, 1);
			}
		}

		// Make the switch to fullscreen
		XEvent xev;
		Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE", False);
		Atom wmFullscreen = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_FULLSCREEN", False);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->xWindow;
		xev.xclient.message_type = wmState;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = wmFullscreen;
		xev.xclient.data.l[2] = 0;

		XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
				SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}

	void LinuxWindow::setShowDecorations(bool show)
	{
		static constexpr UINT32 MWM_HINTS_DECORATIONS		= (1 << 1);

		struct MotifHints
		{
			UINT32       flags;
			UINT32       functions;
			UINT32       decorations;
			INT32        inputMode;
			UINT32       status;
		};

		if(show)
			return;

		MotifHints motifHints;
		motifHints.flags = MWM_HINTS_DECORATIONS;
		motifHints.decorations = 0;
		motifHints.functions = 0;
		motifHints.inputMode = 0;
		motifHints.status = 0;

		Atom wmHintsAtom = XInternAtom(LinuxPlatform::getXDisplay(), "_MOTIF_WM_HINTS", False);
		XChangeProperty(LinuxPlatform::getXDisplay(), m->xWindow,
				wmHintsAtom, wmHintsAtom,
				32,
				PropModeReplace,
				(unsigned char *)&motifHints,
				5);
	}

	void LinuxWindow::setIsModal(bool modal)
	{
		if(modal)
		{
			Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE", False);
			Atom wmValue = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_MODAL", False);

			XEvent xev;
			memset(&xev, 0, sizeof(xev));
			xev.type = ClientMessage;
			xev.xclient.window = m->xWindow;
			xev.xclient.message_type = wmState;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = _NET_WM_STATE_ADD;
			xev.xclient.data.l[1] = wmValue;
			xev.xclient.data.l[2] = 0;
			xev.xclient.data.l[3] = 1;

			XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
					SubstructureRedirectMask | SubstructureNotifyMask, &xev);
		}
	}
}

