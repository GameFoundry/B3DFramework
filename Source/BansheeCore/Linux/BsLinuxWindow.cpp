//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLinuxWindow.h"
#include "BsLinuxPlatform.h"
#include "BsLinuxDropTarget.h"

#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
#define _NET_WM_STATE_TOGGLE 2

#define _NET_WM_MOVERESIZE_MOVE 8
#define _NET_WM_MOVERESIZE_CANCEL 11

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

		Vector<Rect2I> dragZones;

		void* userData = nullptr;
	};

	LinuxWindow::LinuxWindow(const WINDOW_DESC &desc)
	{
		m = bs_new<Pimpl>();

		::Display* display = LinuxPlatform::getXDisplay();

		// Find the screen of the chosen monitor, as well as its current dimensions
		INT32 screen = XDefaultScreen(display);
		UINT32 outputIdx = 0;

		RROutput primaryOutput = XRRGetOutputPrimary(display, RootWindow(display, screen));
		INT32 monitorX = 0;
		INT32 monitorY = 0;
		UINT32 monitorWidth = 0;
		UINT32 monitorHeight = 0;

		INT32 screenCount = XScreenCount(display);
		for(INT32 i = 0; i < screenCount; i++)
		{
			XRRScreenResources* screenRes = XRRGetScreenResources(display, RootWindow(display, i));

			bool foundMonitor = false;
			for (INT32 j = 0; j < screenRes->noutput; j++)
			{
				XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, screenRes->outputs[j]);
				if (outputInfo == nullptr || outputInfo->crtc == 0 || outputInfo->connection == RR_Disconnected)
				{
					XRRFreeOutputInfo(outputInfo);

					continue;
				}

				XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
				if (crtcInfo == nullptr)
				{
					XRRFreeCrtcInfo(crtcInfo);
					XRRFreeOutputInfo(outputInfo);

					continue;
				}

				if(desc.screen == (UINT32)-1)
				{
					if(screenRes->outputs[j] == primaryOutput)
						foundMonitor = true;
				}
				else
					foundMonitor = outputIdx == desc.screen;

				if(foundMonitor)
				{
					screen = i;

					monitorX = crtcInfo->x;
					monitorY = crtcInfo->y;
					monitorWidth = crtcInfo->width;
					monitorHeight = crtcInfo->height;

					foundMonitor = true;
					break;
				}
			}

			if(foundMonitor)
				break;
		}

		XSetWindowAttributes attributes;
		attributes.background_pixel = XWhitePixel(display, screen);
		attributes.border_pixel = XBlackPixel(display, screen);
		attributes.background_pixmap = 0;

		attributes.colormap = XCreateColormap(display,
				XRootWindow(display, screen),
				desc.visualInfo.visual,
				AllocNone);

		// If no position specified, center on the requested monitor
		if (desc.x == -1)
			m->x = monitorX + (monitorWidth - desc.width) / 2;
		else if (desc.screen != (UINT32)-1)
			m->x = monitorX + desc.x;
		else
			m->x = desc.x;

		if (desc.y == -1)
			m->y = monitorY + (monitorHeight - desc.height) / 2;
		else if (desc.screen != (UINT32)-1)
			m->y = monitorY + desc.y;
		else
			m->y = desc.y;

		m->width = desc.width;
		m->height = desc.height;

		m->xWindow = XCreateWindow(display,
				XRootWindow(display, screen),
				m->x, m->y,
				m->width, m->height,
				0, desc.visualInfo.depth,
				InputOutput, desc.visualInfo.visual,
				CWBackPixel | CWBorderPixel | CWColormap | CWBackPixmap, &attributes);

		XStoreName(display, m->xWindow, desc.title.c_str());

		// Position/size might have (and usually will) get overridden by the WM, so re-apply them
		XSizeHints hints;
		hints.flags = PPosition | PSize;
		hints.x = m->x;
		hints.y = m->y;
		hints.width = m->width;
		hints.height = m->height;

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

		XClassHint* classHint = XAllocClassHint();

		classHint->res_class = (char*)"banshee3d";
		classHint->res_name = (char*)desc.title.c_str();

		XSetClassHint(display, m->xWindow, classHint);
		XFree(classHint);

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

		// Make sure we get the window delete message from WM, so we can clean up ourselves
		Atom atomDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, m->xWindow, &atomDeleteWindow, 1);

		// Enable drag and drop
		LinuxDragAndDrop::makeDNDAware(m->xWindow);

		// Set background image if assigned
		if(desc.background)
		{
			Pixmap pixmap = LinuxPlatform::createPixmap(*desc.background, (UINT32)desc.visualInfo.depth);

			XSetWindowBackgroundPixmap(display, m->xWindow, pixmap);
			XFreePixmap(display, pixmap);
			XSync(display, 0);
		}

		// Show the window (needs to happen after setting the background pixmap)
		if(!desc.hidden)
			XMapWindow(display, m->xWindow);

		if(!desc.showOnTaskBar)
			showOnTaskbar(false);

		m->hasTitleBar = desc.showDecorations;
		m->resizeDisabled = !desc.allowResize;

		LinuxPlatform::_registerWindow(m->xWindow, this);
	}

	LinuxWindow::~LinuxWindow()
	{
		if(m->xWindow != 0)
		{
			XUnmapWindow(LinuxPlatform::getXDisplay(), m->xWindow);
			XSync(LinuxPlatform::getXDisplay(), 0);

			XDestroyWindow(LinuxPlatform::getXDisplay(), m->xWindow);
			XSync(LinuxPlatform::getXDisplay(), 0);

			_cleanUp();
		}

		bs_delete(m);
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
		XUnmapWindow(LinuxPlatform::getXDisplay(), m->xWindow);
	}

	void LinuxWindow::show()
	{
		XMapWindow(LinuxPlatform::getXDisplay(), m->xWindow);
		XMoveResizeWindow(LinuxPlatform::getXDisplay(), m->xWindow, m->x, m->y, m->width, m->height);
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
		::Display* display = LinuxPlatform::getXDisplay();
		Pixmap iconPixmap = LinuxPlatform::createPixmap(data, (UINT32)XDefaultDepth(display, XDefaultScreen(display)));

		XWMHints* hints = XAllocWMHints();
		hints->flags = IconPixmapHint;
		hints->icon_pixmap = iconPixmap;

		XSetWMHints(display, m->xWindow, hints);
		XFlush(display);

		XFree(hints);
		XFreePixmap(display, iconPixmap);
	}

	void LinuxWindow::_cleanUp()
	{
		LinuxPlatform::_unregisterWindow(m->xWindow);
		m->xWindow = 0;
	}

	void LinuxWindow::_setDragZones(const Vector<Rect2I>& rects)
	{
		m->dragZones = rects;
	}

	void LinuxWindow::_dragStart(const XButtonEvent& event)
	{
		// Make sure to reset the flag since WM could have (and probably has) handled the drag end event, so we never
		// received _dragEnd() call.
		m->dragInProgress = false;

		// If window has a titlebar, custom drag zones aren't used
		if(m->hasTitleBar)
			return;

		for(auto& entry : m->dragZones)
		{
			if (entry.width == 0 || entry.height == 0)
				continue;

			if(entry.contains(Vector2I(event.x, event.y)))
			{
				XUngrabPointer(LinuxPlatform::getXDisplay(), 0L);
				XFlush(LinuxPlatform::getXDisplay());

				Atom wmMoveResize = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_MOVERESIZE", False);

				XEvent xev;
				memset(&xev, 0, sizeof(xev));
				xev.type = ClientMessage;
				xev.xclient.window = m->xWindow;
				xev.xclient.message_type = wmMoveResize;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = event.x_root;
				xev.xclient.data.l[1] = event.y_root;
				xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_MOVE;
				xev.xclient.data.l[3] = Button1;
				xev.xclient.data.l[4] = 0;

				XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
						SubstructureRedirectMask | SubstructureNotifyMask, &xev);
				XSync(LinuxPlatform::getXDisplay(), 0);

				m->dragInProgress = true;
				return;
			}
		}

		return;
	}

	void LinuxWindow::_dragEnd()
	{
		// WM failed to end the drag, send the cancel drag event
		if(m->dragInProgress)
		{
			Atom wmMoveResize = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_MOVERESIZE", False);

			XEvent xev;
			memset(&xev, 0, sizeof(xev));
			xev.type = ClientMessage;
			xev.xclient.window = m->xWindow;
			xev.xclient.message_type = wmMoveResize;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 0;
			xev.xclient.data.l[1] = 0;
			xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_CANCEL;
			xev.xclient.data.l[3] = Button1;
			xev.xclient.data.l[4] = 0;

			XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
					SubstructureRedirectMask | SubstructureNotifyMask, &xev);

			m->dragInProgress = false;
		}
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

	void LinuxWindow::showOnTaskbar(bool enable)
	{
		Atom wmState = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE", False);
		Atom wmSkipTaskbar = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_SKIP_TASKBAR", False);
		Atom wmSkipPager = XInternAtom(LinuxPlatform::getXDisplay(), "_NET_WM_STATE_SKIP_PAGER", False);

		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->xWindow;
		xev.xclient.message_type = wmState;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = enable ? _NET_WM_STATE_REMOVE : _NET_WM_STATE_ADD;
		xev.xclient.data.l[1] = wmSkipTaskbar;

		XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
				SubstructureRedirectMask | SubstructureNotifyMask, &xev);

		xev.xclient.data.l[1] = wmSkipPager;
		XSendEvent(LinuxPlatform::getXDisplay(), DefaultRootWindow(LinuxPlatform::getXDisplay()), False,
				SubstructureRedirectMask | SubstructureNotifyMask, &xev);

		XSync(LinuxPlatform::getXDisplay(), 0);
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

