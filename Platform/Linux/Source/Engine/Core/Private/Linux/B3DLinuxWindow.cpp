//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DColor.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DPixelUtil.h"
#include "Private/Linux/B3DLinuxWindow.h"
#include "Private/Linux/B3DLinuxPlatform.h"
#include "Private/Linux/B3DLinuxDropTarget.h"

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

using namespace b3d;

enum class WindowState
{
	Minimized,
	Maximized,
	Normal
};

struct LinuxWindow::Pimpl
{
	::Window XWindow = 0;

	i32 X, Y;
	u32 Width, Height;
	bool HasTitleBar = true;
	bool DragInProgress = false;
	bool ResizeDisabled = false;
	bool IsExternal = false;
	WindowState State = WindowState::Normal;

	Vector<Area2I> DragZones;

	void* UserData = nullptr;
};

LinuxWindow::LinuxWindow(const WindowCreateInformation& createInformation)
{
	m = B3DNew<Pimpl>();

	if(createInformation.External)
	{
		m->X = createInformation.X;
		m->Y = createInformation.Y;
		m->Width = createInformation.Width;
		m->Height = createInformation.Height;
		m->XWindow = createInformation.External;
		m->IsExternal = true;
	}
	else
	{
		::Display* display = LinuxPlatform::GetXDisplay();

		// Find the screen of the chosen monitor, as well as its current dimensions
		i32 screen = XDefaultScreen(display);
		u32 outputIdx = 0;

		RROutput primaryOutput = XRRGetOutputPrimary(display, RootWindow(display, screen));
		i32 monitorX = 0;
		i32 monitorY = 0;
		u32 monitorWidth = 0;
		u32 monitorHeight = 0;

		i32 screenCount = XScreenCount(display);
		for(i32 i = 0; i < screenCount; i++)
		{
			XRRScreenResources* screenRes = XRRGetScreenResources(display, RootWindow(display, i));

			bool foundMonitor = false;
			for(i32 j = 0; j < screenRes->noutput; j++)
			{
				XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, screenRes->outputs[j]);
				if(outputInfo == nullptr || outputInfo->crtc == 0 || outputInfo->connection == RR_Disconnected)
				{
					XRRFreeOutputInfo(outputInfo);

					continue;
				}

				XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
				if(crtcInfo == nullptr)
				{
					XRRFreeCrtcInfo(crtcInfo);
					XRRFreeOutputInfo(outputInfo);

					continue;
				}

				if(createInformation.Screen == (u32)-1)
				{
					if(screenRes->outputs[j] == primaryOutput)
						foundMonitor = true;
				}
				else
					foundMonitor = outputIdx == createInformation.Screen;

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

		attributes.colormap = XCreateColormap(display, XRootWindow(display, screen), createInformation.VisualInfo.visual, AllocNone);

		// If no position specified, center on the requested monitor
		if(createInformation.X == -1)
			m->X = monitorX + (monitorWidth - createInformation.Width) / 2;
		else if(createInformation.Screen != (u32)-1)
			m->X = monitorX + createInformation.X;
		else
			m->X = createInformation.X;

		if(createInformation.Y == -1)
			m->Y = monitorY + (monitorHeight - createInformation.Height) / 2;
		else if(createInformation.Screen != (u32)-1)
			m->Y = monitorY + createInformation.Y;
		else
			m->Y = createInformation.Y;

		m->Width = createInformation.Width;
		m->Height = createInformation.Height;

		m->XWindow = XCreateWindow(display, XRootWindow(display, screen), m->X, m->Y, m->Width, m->Height, 0, createInformation.VisualInfo.depth, InputOutput, createInformation.VisualInfo.visual, CWBackPixel | CWBorderPixel | CWColormap | CWBackPixmap, &attributes);

		XStoreName(display, m->XWindow, createInformation.Title.c_str());

		// Position/size might have (and usually will) get overridden by the WM, so re-apply them
		XSizeHints hints;
		hints.flags = PPosition | PSize;
		hints.x = m->X;
		hints.y = m->Y;
		hints.width = m->Width;
		hints.height = m->Height;

		if(!createInformation.AllowResize)
		{
			hints.flags |= PMinSize | PMaxSize;

			hints.min_height = createInformation.Height;
			hints.max_height = createInformation.Height;

			hints.min_width = createInformation.Width;
			hints.max_width = createInformation.Width;
		}

		XSetNormalHints(display, m->XWindow, &hints);

		SetShowDecorations(createInformation.ShowDecorations);

		XClassHint* classHint = XAllocClassHint();

		classHint->res_class = (char*)"banshee3d";
		classHint->res_name = (char*)createInformation.Title.c_str();

		XSetClassHint(display, m->XWindow, classHint);
		XFree(classHint);

		// Ensures the child window is always on top of the parent window
		if(createInformation.Parent)
			XSetTransientForHint(display, m->XWindow, createInformation.Parent);

		long eventMask =
			ExposureMask | FocusChangeMask |
			KeyPressMask | KeyReleaseMask |
			ButtonPressMask | ButtonReleaseMask |
			EnterWindowMask | LeaveWindowMask |
			PointerMotionMask | ButtonMotionMask |
			StructureNotifyMask | PropertyChangeMask;

		if(!createInformation.Parent)
			eventMask |= SubstructureNotifyMask | SubstructureRedirectMask;

		XSelectInput(display, m->XWindow, eventMask);

		// Make sure we get the window delete message from WM, so we can clean up ourselves
		Atom atomDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, m->XWindow, &atomDeleteWindow, 1);

		// Enable drag and drop
		LinuxDragAndDrop::MakeDNDAware(m->XWindow);

		// Set background image if assigned
		if(createInformation.Background)
		{
			Pixmap pixmap = LinuxPlatform::CreatePixmap(*createInformation.Background, (u32)createInformation.VisualInfo.depth);

			XSetWindowBackgroundPixmap(display, m->XWindow, pixmap);
			XFreePixmap(display, pixmap);
			XSync(display, 0);
		}

		// _NET_WM_STATE client messages are only honored for mapped windows; the EWMH spec requires the
		// initial state to be written as a property before the window is first mapped.
		Atom initialStates[3];
		u32 initialStateCount = 0;

		if(createInformation.Modal)
			initialStates[initialStateCount++] = XInternAtom(display, "_NET_WM_STATE_MODAL", False);

		if(!createInformation.ShowOnTaskBar)
		{
			initialStates[initialStateCount++] = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
			initialStates[initialStateCount++] = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", False);
		}

		if(initialStateCount > 0)
		{
			Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);

			// Atom is unsigned long, satisfying the format = 32 long-element requirement
			XChangeProperty(display, m->XWindow, wmState, XA_ATOM, 32, PropModeReplace, (unsigned char*)initialStates, (int)initialStateCount);
		}

		// Show the window (needs to happen after setting the background pixmap)
		if(!createInformation.Hidden)
			XMapWindow(display, m->XWindow);
	}

	m->HasTitleBar = createInformation.ShowDecorations;
	m->ResizeDisabled = !createInformation.AllowResize;

	LinuxPlatform::RegisterWindowInternal(m->XWindow, this);
}

LinuxWindow::~LinuxWindow()
{
	if(m->XWindow != 0)
		DestroyInternal();

	B3DDelete(m);
}

void LinuxWindow::Move(i32 x, i32 y)
{
	m->X = x;
	m->Y = y;

	XMoveWindow(LinuxPlatform::GetXDisplay(), m->XWindow, x, y);
}

void LinuxWindow::Resize(u32 width, u32 height)
{
	// If resize is disabled on WM level, we need to force it
	if(m->ResizeDisabled)
	{
		XSizeHints hints;
		hints.flags = PMinSize | PMaxSize;

		hints.min_height = height;
		hints.max_height = height;

		hints.min_width = width;
		hints.max_width = width;

		XSetNormalHints(LinuxPlatform::GetXDisplay(), m->XWindow, &hints);
	}

	m->Width = width;
	m->Height = height;

	XResizeWindow(LinuxPlatform::GetXDisplay(), m->XWindow, width, height);
}

void LinuxWindow::SetHidden(bool hidden)
{
	if(hidden)
		XUnmapWindow(LinuxPlatform::GetXDisplay(), m->XWindow);
	else
	{
		XMapWindow(LinuxPlatform::GetXDisplay(), m->XWindow);
		XMoveResizeWindow(LinuxPlatform::GetXDisplay(), m->XWindow, m->X, m->Y, m->Width, m->Height);
	}
}

void LinuxWindow::Maximize()
{
	Maximize(true);
}

void LinuxWindow::Minimize()
{
	Minimize(true);
}

void LinuxWindow::Restore()
{
	if(IsMaximized())
		Maximize(false);
	else if(IsMinimized())
		Minimize(false);
}

i32 LinuxWindow::GetLeft() const
{
	i32 x, y;
	::Window child;
	XTranslateCoordinates(LinuxPlatform::GetXDisplay(), m->XWindow, DefaultRootWindow(LinuxPlatform::GetXDisplay()), 0, 0, &x, &y, &child);

	return x;
}

i32 LinuxWindow::GetTop() const
{
	i32 x, y;
	::Window child;
	XTranslateCoordinates(LinuxPlatform::GetXDisplay(), m->XWindow, DefaultRootWindow(LinuxPlatform::GetXDisplay()), 0, 0, &x, &y, &child);

	return y;
}

u32 LinuxWindow::GetWidth() const
{
	XWindowAttributes xwa;
	XGetWindowAttributes(LinuxPlatform::GetXDisplay(), m->XWindow, &xwa);

	return (u32)xwa.width;
}

u32 LinuxWindow::GetHeight() const
{
	XWindowAttributes xwa;
	XGetWindowAttributes(LinuxPlatform::GetXDisplay(), m->XWindow, &xwa);

	return (u32)xwa.height;
}

Vector2I LinuxWindow::WindowToScreenPos(const Vector2I& windowPos) const
{
	Vector2I screenPos;

	::Window child;
	XTranslateCoordinates(LinuxPlatform::GetXDisplay(), m->XWindow, DefaultRootWindow(LinuxPlatform::GetXDisplay()), windowPos.X, windowPos.Y, &screenPos.X, &screenPos.Y, &child);

	return screenPos;
}

Vector2I LinuxWindow::ScreenToWindowPos(const Vector2I& screenPos) const
{
	Vector2I windowPos;

	::Window child;
	XTranslateCoordinates(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), m->XWindow, screenPos.X, screenPos.Y, &windowPos.X, &windowPos.Y, &child);

	return windowPos;
}

void LinuxWindow::SetIcon(const PixelData& data)
{
	constexpr u32 WIDTH = 128;
	constexpr u32 HEIGHT = 128;

	PixelData resizedData(WIDTH, HEIGHT, 1, PF_RGBA8);
	resizedData.allocateInternalBuffer();

	PixelUtil::scale(data, resizedData);

	::Display* display = LinuxPlatform::GetXDisplay();

	// Set icon the old way using IconPixmapHint.
	Pixmap iconPixmap = LinuxPlatform::CreatePixmap(resizedData, (u32)XDefaultDepth(display, XDefaultScreen(display)));

	XWMHints* hints = XAllocWMHints();
	hints->flags = IconPixmapHint;
	hints->icon_pixmap = iconPixmap;

	XSetWMHints(display, m->XWindow, hints);

	XFree(hints);
	XFreePixmap(display, iconPixmap);

	// Also try to set _NET_WM_ICON for modern window managers.
	// Using long because the spec for XChangeProperty states that format size of 32 = long (this means for 64-bit it
	// is padded in upper 4 bytes)
	Vector<long> wmIconData(2 + WIDTH * HEIGHT, 0);
	wmIconData[0] = WIDTH;
	wmIconData[1] = HEIGHT;
	for(u32 y = 0; y < HEIGHT; y++)
		for(u32 x = 0; x < WIDTH; x++)
			wmIconData[y * WIDTH + x + 2] = resizedData.getColorAt(x, y).getAsBGRA();

	Atom iconAtom = XInternAtom(display, "_NET_WM_ICON", False);
	Atom cardinalAtom = XInternAtom(display, "CARDINAL", False);
	XChangeProperty(display, m->XWindow, iconAtom, cardinalAtom, 32, PropModeReplace, (const unsigned char*)wmIconData.data(), wmIconData.size());

	XFlush(display);
}

void LinuxWindow::DoOnWindowMovedOrResized()
{
	::Display* display = LinuxPlatform::GetXDisplay();

	XWindowAttributes xwa;
	XGetWindowAttributes(display, m->XWindow, &xwa);

	i32 x, y;
	::Window child;
	XTranslateCoordinates(display, m->XWindow, DefaultRootWindow(display), 0, 0, &x, &y, &child);

	m->X = x;
	m->Y = y;
	m->Width = (u32)xwa.width;
	m->Height = (u32)xwa.height;
}

void LinuxWindow::DestroyInternal()
{
	if(!m->IsExternal)
	{
		XUnmapWindow(LinuxPlatform::GetXDisplay(), m->XWindow);
		XSync(LinuxPlatform::GetXDisplay(), 0);

		XDestroyWindow(LinuxPlatform::GetXDisplay(), m->XWindow);
		XSync(LinuxPlatform::GetXDisplay(), 0);
	}

	LinuxPlatform::UnregisterWindowInternal(m->XWindow);
	m->XWindow = 0;
}

void LinuxWindow::SetDragZonesInternal(const Vector<Area2I>& areas)
{
	m->DragZones = areas;
}

void LinuxWindow::DragStartInternal(const XButtonEvent& event)
{
	// Make sure to reset the flag since WM could have (and probably has) handled the drag end event, so we never
	// received DragEndInternal() call.
	m->DragInProgress = false;

	// If window has a titlebar, custom drag zones aren't used
	if(m->HasTitleBar)
		return;

	for(auto& entry : m->DragZones)
	{
		if(entry.Width == 0 || entry.Height == 0)
			continue;

		if(entry.Contains(Vector2I(event.x, event.y)))
		{
			XUngrabPointer(LinuxPlatform::GetXDisplay(), 0L);
			XFlush(LinuxPlatform::GetXDisplay());

			Atom wmMoveResize = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_MOVERESIZE", False);

			XEvent xev;
			memset(&xev, 0, sizeof(xev));
			xev.type = ClientMessage;
			xev.xclient.window = m->XWindow;
			xev.xclient.message_type = wmMoveResize;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = event.x_root;
			xev.xclient.data.l[1] = event.y_root;
			xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_MOVE;
			xev.xclient.data.l[3] = Button1;
			xev.xclient.data.l[4] = 0;

			XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
			XSync(LinuxPlatform::GetXDisplay(), 0);

			m->DragInProgress = true;
			return;
		}
	}

	return;
}

void LinuxWindow::DragEndInternal()
{
	// WM failed to end the drag, send the cancel drag event
	if(m->DragInProgress)
	{
		Atom wmMoveResize = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_MOVERESIZE", False);

		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->XWindow;
		xev.xclient.message_type = wmMoveResize;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 0;
		xev.xclient.data.l[1] = 0;
		xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_CANCEL;
		xev.xclient.data.l[3] = Button1;
		xev.xclient.data.l[4] = 0;

		XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

		m->DragInProgress = false;
	}
}

::Window LinuxWindow::GetXWindowInternal() const
{
	return m->XWindow;
}

void LinuxWindow::SetUserDataInternal(void* data)
{
	m->UserData = data;
}

void* LinuxWindow::GetUserDataInternal() const
{
	return m->UserData;
}

bool LinuxWindow::IsMaximized() const
{
	Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE", False);
	Atom type;
	i32 format;
	unsigned long length;
	unsigned long remaining;
	u8* data = nullptr;

	i32 result = XGetWindowProperty(LinuxPlatform::GetXDisplay(), m->XWindow, wmState, 0, 1024, False, XA_ATOM,
		&type, &format, &length, &remaining, &data);

	bool foundHorz = false;
	bool foundVert = false;

	if(result == Success && data != nullptr)
	{
		Atom* atoms = (Atom*)data;
		Atom wmMaxHorz = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		Atom wmMaxVert = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);

		for(unsigned long i = 0; i < length; i++)
		{
			if(atoms[i] == wmMaxHorz)
				foundHorz = true;
			if(atoms[i] == wmMaxVert)
				foundVert = true;
		}

		XFree(data);
	}

	return foundHorz && foundVert;
}

bool LinuxWindow::IsMinimized()
{
	Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "WM_STATE", True);
	Atom type;
	i32 format;
	unsigned long length;
	unsigned long remaining;
	u8* data = nullptr;

	i32 result = XGetWindowProperty(LinuxPlatform::GetXDisplay(), m->XWindow, wmState, 0, 1024, False, AnyPropertyType,
		&type, &format, &length, &remaining, &data);

	bool minimized = false;
	if(result == Success && data != nullptr && length > 0)
	{
		long* state = (long*)data;
		minimized = state[0] == WM_IconicState;
	}

	if(data != nullptr)
		XFree(data);

	return minimized;
}

void LinuxWindow::Maximize(bool enable)
{
	Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE", False);
	Atom wmMaxHorz = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	Atom wmMaxVert = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = m->XWindow;
	xev.xclient.message_type = wmState;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = enable ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
	xev.xclient.data.l[1] = wmMaxHorz;
	xev.xclient.data.l[2] = wmMaxVert;

	XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void LinuxWindow::Minimize(bool enable)
{
	XEvent xev;
	Atom wmChange = XInternAtom(LinuxPlatform::GetXDisplay(), "WM_CHANGE_STATE", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = m->XWindow;
	xev.xclient.message_type = wmChange;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = enable ? WM_IconicState : WM_NormalState;

	XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

// Only valid for mapped windows; the initial (pre-map) state is set as a property in the constructor.
void LinuxWindow::ShowOnTaskbar(bool enable)
{
	Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE", False);
	Atom wmSkipTaskbar = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_SKIP_TASKBAR", False);
	Atom wmSkipPager = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_SKIP_PAGER", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = m->XWindow;
	xev.xclient.message_type = wmState;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = enable ? _NET_WM_STATE_REMOVE : _NET_WM_STATE_ADD;
	xev.xclient.data.l[1] = wmSkipTaskbar;

	XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	xev.xclient.data.l[1] = wmSkipPager;
	XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	XSync(LinuxPlatform::GetXDisplay(), 0);
}

void LinuxWindow::SetFullscreenInternal(bool fullscreen)
{
	// Attempt to bypass compositor if switching to fullscreen
	if(fullscreen)
	{
		Atom wmBypassCompositor = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_BYPASS_COMPOSITOR", False);
		if(wmBypassCompositor)
		{
			// format = 32 properties are passed as long-sized elements (see SetShowDecorations note)
			const unsigned long enabled = 1;

			XChangeProperty(LinuxPlatform::GetXDisplay(), m->XWindow, wmBypassCompositor, XA_CARDINAL, 32,
				PropModeReplace, (unsigned char*)&enabled, 1);
		}
	}

	// Make the switch to fullscreen
	XEvent xev;
	Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE", False);
	Atom wmFullscreen = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_FULLSCREEN", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = m->XWindow;
	xev.xclient.message_type = wmState;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
	xev.xclient.data.l[1] = wmFullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void LinuxWindow::SetShowDecorations(bool show)
{
	static constexpr unsigned long MWM_HINTS_DECORATIONS = (1 << 1);

	struct MotifHints
	{
		unsigned long Flags;
		unsigned long Functions;
		unsigned long Decorations;
		long InputMode;
		unsigned long Status;
	};

	if(show)
		return;

	MotifHints motifHints;
	motifHints.Flags = MWM_HINTS_DECORATIONS;
	motifHints.Decorations = 0;
	motifHints.Functions = 0;
	motifHints.InputMode = 0;
	motifHints.Status = 0;

	Atom wmHintsAtom = XInternAtom(LinuxPlatform::GetXDisplay(), "_MOTIF_WM_HINTS", False);
	XChangeProperty(LinuxPlatform::GetXDisplay(), m->XWindow, wmHintsAtom, wmHintsAtom, 32, PropModeReplace, (unsigned char*)&motifHints, 5);
}

// Only valid for mapped windows; the initial (pre-map) state is set as a property in the constructor.
void LinuxWindow::SetIsModal(bool modal)
{
	if(modal)
	{
		Atom wmState = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE", False);
		Atom wmValue = XInternAtom(LinuxPlatform::GetXDisplay(), "_NET_WM_STATE_MODAL", False);

		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = m->XWindow;
		xev.xclient.message_type = wmState;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = _NET_WM_STATE_ADD;
		xev.xclient.data.l[1] = wmValue;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1;

		XSendEvent(LinuxPlatform::GetXDisplay(), DefaultRootWindow(LinuxPlatform::GetXDisplay()), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}
}
