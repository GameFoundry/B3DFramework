//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DPixelData.h"
#include "Image/B3DPixelUtil.h"
#include "String/B3DUnicode.h"
#include "Private/Linux/B3DLinuxInput.h"
#include "Private/Linux/B3DLinuxPlatform.h"
#include "Private/Linux/B3DLinuxWindow.h"
#include "Private/Linux/B3DLinuxDropTarget.h"
#include "GpuBackend/B3DRenderWindow.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>
#include <cstring>
#include <pwd.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <cerrno>
#include <ctime>

extern char** environ;

using namespace b3d;

Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::OnPointerMoved;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::OnPointerButtonPressed;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::OnPointerButtonReleased;
Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::OnPointerDoubleClick;
Event<void(InputCommandType)> Platform::OnInputCommand;
Event<void(float)> Platform::OnMouseWheelScrolled;
Event<void(u32)> Platform::OnCharInput;

Event<void()> Platform::OnMouseCaptureChanged;
Event<void()> Platform::OnInputDevicesChanged;

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

struct Platform::Private
{
	::Display* XDisplay = nullptr;
	::Window MainXWindow = 0;
	::Window FullscreenXWindow = 0;
	UnorderedMap<::Window, LinuxWindow*> WindowMap;
	Mutex Sync;

	XIM IM;
	XIC IC;
	::Time LastButtonPressTime;

	Atom AtomDeleteWindow;
	Atom AtomWmState;
	Atom AtomWmStateHidden;
	Atom AtomWmStateMaxVert;
	Atom AtomWmStateMaxHorz;

	// X11 Event handling
	int XInput2Opcode;
	UnorderedMap<String, KeyCode> KeyNameMap; /**< Maps X11 key name (e.g. "TAB") to system-specific X11 KeyCode. */
	Vector<ButtonCode> KeyCodeMap; /**< Maps system-specific X11 KeyCode to Banshee ButtonCode. */

	// Clipboard
	String ClipboardData;

	// Cursor
	::Cursor CurrentCursor = None;
	::Cursor EmptyCursor = None;
	bool IsCursorHidden = false;

	Area2I CursorClipArea;
	LinuxWindow* CursorClipWindow = nullptr;
	bool CursorClipEnabled = false;

	// Input device attach/detach detection
	int InotifyHandle = -1;
	int InotifyDevInputWatch = -1;
};

static const u32 DOUBLE_CLICK_MS = 500;

static Vector2I GetCursorPositionInternal(Platform::Private* data)
{
	Vector2I pos;
	u32 screenCount = (u32)XScreenCount(data->XDisplay);

	for(u32 i = 0; i < screenCount; ++i)
	{
		::Window outRoot, outChild;
		i32 childX, childY;
		u32 mask;
		if(XQueryPointer(data->XDisplay, XRootWindow(data->XDisplay, i), &outRoot, &outChild, &pos.X, &pos.Y, &childX, &childY, &mask))
			break;
	}

	return pos;
}

static void SetCursorPositionInternal(Platform::Private* data, const Vector2I& screenPos)
{
	u32 screenCount = (u32)XScreenCount(data->XDisplay);

	// Note assuming screens are laid out horizontally left to right
	i32 screenX = 0;
	for(u32 i = 0; i < screenCount; ++i)
	{
		::Window root = XRootWindow(data->XDisplay, i);
		i32 screenXEnd = screenX + XDisplayWidth(data->XDisplay, i);

		if(screenPos.X >= screenX && screenPos.X < screenXEnd)
		{
			XWarpPointer(data->XDisplay, None, root, 0, 0, 0, 0, screenPos.X, screenPos.Y);
			XFlush(data->XDisplay);
			return;
		}

		screenX = screenXEnd;
	}
}

static void ApplyCurrentCursor(Platform::Private* data, ::Window window)
{
	if(data->IsCursorHidden)
		XDefineCursor(data->XDisplay, window, data->EmptyCursor);
	else
	{
		if(data->CurrentCursor != None)
			XDefineCursor(data->XDisplay, window, data->CurrentCursor);
		else
			XUndefineCursor(data->XDisplay, window);
	}
}

static void UpdateClipBounds(Platform::Private* data, LinuxWindow* window)
{
	if(!data->CursorClipEnabled || data->CursorClipWindow != window)
		return;

	data->CursorClipArea.X = window->GetLeft();
	data->CursorClipArea.Y = window->GetTop();
	data->CursorClipArea.Width = window->GetWidth();
	data->CursorClipArea.Height = window->GetHeight();
}

static bool ClipCursor(Platform::Private* data, Vector2I& pos)
{
	if(!data->CursorClipEnabled)
		return false;

	i32 clippedX = pos.X - data->CursorClipArea.X;
	i32 clippedY = pos.Y - data->CursorClipArea.Y;

	if(clippedX < 0)
		clippedX = 0;
	else if(clippedX >= (i32)data->CursorClipArea.Width)
		clippedX = data->CursorClipArea.Width > 0 ? data->CursorClipArea.Width - 1 : 0;

	if(clippedY < 0)
		clippedY = 0;
	else if(clippedY >= (i32)data->CursorClipArea.Height)
		clippedY = data->CursorClipArea.Height > 0 ? data->CursorClipArea.Height - 1 : 0;

	clippedX += data->CursorClipArea.X;
	clippedY += data->CursorClipArea.Y;

	if(clippedX != pos.X || clippedY != pos.Y)
	{
		pos.X = clippedX;
		pos.Y = clippedY;

		return true;
	}

	return false;
}

static void DisableCursorClipping(Platform::Private* data)
{
	data->CursorClipEnabled = false;
	data->CursorClipWindow = None;
}

static void SetCurrentCursor(Platform::Private* data, ::Cursor cursor)
{
	if(data->CurrentCursor)
		XFreeCursor(data->XDisplay, data->CurrentCursor);

	data->CurrentCursor = cursor;
	for(auto& entry : data->WindowMap)
		ApplyCurrentCursor(data, entry.first);
}

/**
 * Searches the window hierarchy, from top to bottom, looking for the top-most window that contains the specified
 * point. Returns 0 if one is not found.
 */
static ::Window GetWindowUnderPoint(::Display* display, ::Window rootWindow, ::Window window, const Vector2I& screenPos)
{
	::Window outRoot, outParent;
	::Window* children;
	u32 numChildren;
	XQueryTree(display, window, &outRoot, &outParent, &children, &numChildren);

	if(children == nullptr || numChildren == 0)
	{
		if(children != nullptr)
			XFree(children);

		return window;
	}

	for(u32 j = 0; j < numChildren; j++)
	{
		::Window curWindow = children[numChildren - j - 1];

		XWindowAttributes xwa;
		XGetWindowAttributes(display, curWindow, &xwa);

		if(xwa.map_state != IsViewable || xwa.c_class != InputOutput)
			continue;

		// Get position in root window coordinates
		::Window outChild;
		Vector2I pos;
		if(!XTranslateCoordinates(display, curWindow, rootWindow, 0, 0, &pos.X, &pos.Y, &outChild))
			continue;

		Area2I area(pos.X, pos.Y, (i32)xwa.width, (i32)xwa.height);
		if(area.Contains(screenPos))
		{
			XFree(children);
			return GetWindowUnderPoint(display, rootWindow, curWindow, screenPos);
		}
	}

	XFree(children);
	return 0;
}

static int X11ErrorHandler(::Display* display, XErrorEvent* event)
{
	// X11 by default crashes the app on error, even though some errors can be just fine. So we provide our own handler.

	char buffer[256];
	XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
	B3D_LOG(Warning, LogPlatform, "X11 error: " + String(buffer));

	return 0;
}

Platform::Private* Platform::mData = B3DNew<Platform::Private>();

Platform::~Platform()
{}

Vector2I Platform::GetCursorPosition()
{
	Lock lock(mData->Sync);
	return GetCursorPositionInternal(mData);
}

void Platform::SetCursorPosition(const Vector2I& screenPos)
{
	Lock lock(mData->Sync);

	SetCursorPositionInternal(mData, screenPos);
}

void Platform::CaptureMouse(const RenderWindow& window)
{
	Lock lock(mData->Sync);

	::Window xWindow = (::Window)window.GetPlatformWindowHandle();

	u32 mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
	XGrabPointer(mData->XDisplay, xWindow, False, mask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	XSync(mData->XDisplay, False);
}

void Platform::ReleaseMouseCapture()
{
	Lock lock(mData->Sync);

	XUngrabPointer(mData->XDisplay, CurrentTime);
	XSync(mData->XDisplay, False);
}

bool Platform::IsPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
{
	Lock lock(mData->Sync);

	::Window xWindow = (::Window)window.GetPlatformWindowHandle();

	u32 screenCount = (u32)XScreenCount(mData->XDisplay);
	for(u32 i = 0; i < screenCount; ++i)
	{
		::Window rootWindow = XRootWindow(mData->XDisplay, i);
		if(GetWindowUnderPoint(mData->XDisplay, rootWindow, rootWindow, screenPos) == xWindow)
			return true;
	}

	return false;
}

void Platform::HideCursor()
{
	Lock lock(mData->Sync);
	mData->IsCursorHidden = true;

	for(auto& entry : mData->WindowMap)
		ApplyCurrentCursor(mData, entry.first);
}

void Platform::ShowCursor()
{
	Lock lock(mData->Sync);
	mData->IsCursorHidden = false;

	for(auto& entry : mData->WindowMap)
		ApplyCurrentCursor(mData, entry.first);
}

bool Platform::IsCursorHidden()
{
	Lock lock(mData->Sync);
	return mData->IsCursorHidden;
}

void Platform::ClipCursorToWindow(const RenderWindow& window)
{
	Lock lock(mData->Sync);

	::Window xWindow = (::Window)window.GetPlatformWindowHandle();
	auto iterFind = mData->WindowMap.find(xWindow);
	if(iterFind == mData->WindowMap.end())
		return;

	LinuxWindow* linuxWindow = iterFind->second;

	mData->CursorClipEnabled = true;
	mData->CursorClipWindow = linuxWindow;

	UpdateClipBounds(mData, linuxWindow);

	Vector2I pos = GetCursorPositionInternal(mData);

	if(ClipCursor(mData, pos))
		SetCursorPositionInternal(mData, pos);
}

void Platform::ClipCursorToRect(const Area2I& screenRect)
{
	Lock lock(mData->Sync);

	mData->CursorClipEnabled = true;
	mData->CursorClipArea = screenRect;
	mData->CursorClipWindow = nullptr;

	Vector2I pos = GetCursorPositionInternal(mData);

	if(ClipCursor(mData, pos))
		SetCursorPositionInternal(mData, pos);
}

void Platform::ClipCursorDisable()
{
	Lock lock(mData->Sync);

	DisableCursorClipping(mData);
}

void Platform::SetCursor(PixelData& pixelData, const Vector2I& hotSpot)
{
	TShared<PixelData> bgraData = PixelData::Create(pixelData.GetWidth(), pixelData.GetHeight(), 1, PF_BGRA8);
	PixelUtil::bulkPixelConversion(pixelData, *bgraData);

	Lock lock(mData->Sync);

	XcursorImage* image = XcursorImageCreate((int)bgraData->GetWidth(), (int)bgraData->GetHeight());
	image->xhot = (XcursorDim)hotSpot.X;
	image->yhot = (XcursorDim)hotSpot.Y;
	image->delay = 0;

	memcpy(image->pixels, bgraData->GetData(), bgraData->GetSize());

	::Cursor cursor = XcursorImageLoadCursor(mData->XDisplay, image);
	XcursorImageDestroy(image);

	SetCurrentCursor(mData, cursor);
}

void Platform::SetIcon(const PixelData& pixelData)
{
	Lock lock(mData->Sync);

	if(!mData->MainXWindow)
		return;

	auto iterFind = mData->WindowMap.find(mData->MainXWindow);
	if(iterFind == mData->WindowMap.end())
		return;

	LinuxWindow* mainLinuxWindow = iterFind->second;

	mainLinuxWindow->SetIcon(pixelData);
}

void Platform::SetCaptionNonClientAreas(const RenderWindow& window, const Vector<Area2I>& nonClientAreas)
{
	if(nonClientAreas.size() == 0)
		return;

	Lock lock(mData->Sync);

	::Window xWindow = (::Window)window.GetPlatformWindowHandle();
	auto iterFind = mData->WindowMap.find(xWindow);
	if(iterFind == mData->WindowMap.end())
		return;

	LinuxWindow* linuxWindow = iterFind->second;
	linuxWindow->SetDragZonesInternal(nonClientAreas);
}

void Platform::SetResizeNonClientAreas(const RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
{
	// Do nothing, resize areas not supported on Linux (but they are provided even on undecorated windows by the WM)
}

void Platform::ResetNonClientAreas(const RenderWindow& window)
{
	Lock lock(mData->Sync);

	::Window xWindow = (::Window)window.GetPlatformWindowHandle();
	auto iterFind = mData->WindowMap.find(xWindow);
	if(iterFind == mData->WindowMap.end())
		return;

	LinuxWindow* linuxWindow = iterFind->second;
	linuxWindow->SetDragZonesInternal({});
}

void Platform::Sleep(u32 duration)
{
	timespec request;
	request.tv_sec = duration / 1000;
	request.tv_nsec = (long)(duration % 1000) * 1000000L;

	while(nanosleep(&request, &request) == -1 && errno == EINTR)
	{ }
}

void Platform::CopyToClipboard(const String& string)
{
	Lock lock(mData->Sync);
	mData->ClipboardData = string;

	Atom clipboardAtom = XInternAtom(mData->XDisplay, "CLIPBOARD", 0);
	XSetSelectionOwner(mData->XDisplay, clipboardAtom, mData->MainXWindow, CurrentTime);
}

String Platform::CopyFromClipboard()
{
	Lock lock(mData->Sync);
	Atom clipboardAtom = XInternAtom(mData->XDisplay, "CLIPBOARD", 0);
	::Window selOwner = XGetSelectionOwner(mData->XDisplay, clipboardAtom);

	if(selOwner == None)
		return "";

	if(selOwner == mData->MainXWindow)
		return mData->ClipboardData;

	// Prefer UTF8_STRING (the modern inter-client convention); fall back to Latin-1 XA_STRING for
	// legacy owners that don't offer UTF-8.
	Atom requestedTargets[] = { XInternAtom(mData->XDisplay, "UTF8_STRING", False), XA_STRING };
	for(Atom target : requestedTargets)
	{
		XConvertSelection(mData->XDisplay, clipboardAtom, target, clipboardAtom, mData->MainXWindow, CurrentTime);
		XFlush(mData->XDisplay);

		// Wait for the owner's SelectionNotify with a timeout, so a stalled or dead owner can't hang the
		// caller. XCheckTypedWindowEvent removes only the matching event; everything else stays queued
		// for MessagePump().
		bool receivedNotify = false;
		XEvent event;
		for(u32 waitedMs = 0; waitedMs < 500 && !receivedNotify; waitedMs += 5)
		{
			if(XCheckTypedWindowEvent(mData->XDisplay, mData->MainXWindow, SelectionNotify, &event))
				receivedNotify = true;
			else
			{
				timespec request;
				request.tv_sec = 0;
				request.tv_nsec = 5 * 1000000L;
				nanosleep(&request, nullptr);
			}
		}

		if(!receivedNotify || event.xselection.property == None)
			continue; // Owner rejected this target (or timed out) - try the next one

		// Note: INCR (incremental) transfers are not supported; very large clipboard contents will be truncated to the first property's worth of data.
		// First a zero-length probe to learn the property size, then the actual transfer.
		Atom actualType;
		i32 actualFormat;
		unsigned long length;
		unsigned long bytesRemaining;
		u8* data = nullptr;
		XGetWindowProperty(mData->XDisplay, mData->MainXWindow, clipboardAtom, 0, 0, False, AnyPropertyType,
			&actualType, &actualFormat, &length, &bytesRemaining, &data);
		if(data != nullptr)
			XFree(data);

		if(bytesRemaining > 0)
		{
			unsigned long unused;
			data = nullptr;

			// long_length is measured in 32-bit units; round the byte count up
			i32 result = XGetWindowProperty(mData->XDisplay, mData->MainXWindow, clipboardAtom, 0,
				(long)((bytesRemaining + 3) / 4), False, AnyPropertyType, &actualType, &actualFormat,
				&length, &unused, &data);

			if(result == Success && data != nullptr)
			{
				String output((const char*)data);
				XFree(data);
				XDeleteProperty(mData->XDisplay, mData->MainXWindow, clipboardAtom);

				return output;
			}

			if(data != nullptr)
				XFree(data);
		}

		XDeleteProperty(mData->XDisplay, mData->MainXWindow, clipboardAtom);
	}

	return "";
}

/** Maps X11 mouse button codes to engine button codes. */
static ButtonCode XButtonToButtonCode(int button)
{
	switch(button)
	{
	case Button1:
		return ButtonCode::MouseLeft;
	case Button2:
		return ButtonCode::MouseMiddle;
	case Button3:
		return ButtonCode::MouseRight;
	case Button4: // Vertical scroll wheel: delivered through the scroll axis/wheel events, not as a button
	case Button5:
	case 6: // Horizontal scroll wheel: unsupported
	case 7:
		return ButtonCode::Unassigned;
	default:
		{
			// X11 numbers additional physical buttons (back/forward, ...) from 8 upwards
			const u32 extraIdx = (u32)(button - 8);
			const u32 extraCount = (u32)ButtonCode::MouseButton32 - (u32)ButtonCode::MouseButton4 + 1;
			if(extraIdx < extraCount)
				return (ButtonCode)((u32)ButtonCode::MouseButton4 + extraIdx);

			return ButtonCode::Unassigned;
		}
	}
}

/** Maps Banshee button codes to X11 names for physical key locations. */
static const char* ButtonCodeToKeyName(ButtonCode code)
{
	switch(code)
	{
		// Row #1
	case ButtonCode::Escape: return "ESC";
	case ButtonCode::F1: return "FK01";
	case ButtonCode::F2: return "FK02";
	case ButtonCode::F3: return "FK03";
	case ButtonCode::F4: return "FK04";
	case ButtonCode::F5: return "FK05";
	case ButtonCode::F6: return "FK06";
	case ButtonCode::F7: return "FK07";
	case ButtonCode::F8: return "FK08";
	case ButtonCode::F9: return "FK09";
	case ButtonCode::F10: return "FK10";
	case ButtonCode::F11: return "FK11";
	case ButtonCode::F12: return "FK12";
	case ButtonCode::F13: return "FK13";
	case ButtonCode::F14: return "FK14";
	case ButtonCode::F15:
		return "FK15";

		// Row #2
	case ButtonCode::Grave: return "TLDE";
	case ButtonCode::Key1: return "AE01";
	case ButtonCode::Key2: return "AE02";
	case ButtonCode::Key3: return "AE03";
	case ButtonCode::Key4: return "AE04";
	case ButtonCode::Key5: return "AE05";
	case ButtonCode::Key6: return "AE06";
	case ButtonCode::Key7: return "AE07";
	case ButtonCode::Key8: return "AE08";
	case ButtonCode::Key9: return "AE09";
	case ButtonCode::Key0: return "AE10";
	case ButtonCode::Minus: return "AE11";
	case ButtonCode::Equals: return "AE12";
	case ButtonCode::Backspace:
		return "BKSP";

		// Row #3
	case ButtonCode::Tab: return "TAB";
	case ButtonCode::Q: return "AD01";
	case ButtonCode::W: return "AD02";
	case ButtonCode::E: return "AD03";
	case ButtonCode::R: return "AD04";
	case ButtonCode::T: return "AD05";
	case ButtonCode::Y: return "AD06";
	case ButtonCode::U: return "AD07";
	case ButtonCode::I: return "AD08";
	case ButtonCode::O: return "AD09";
	case ButtonCode::P: return "AD10";
	case ButtonCode::LeftBracket: return "AD11";
	case ButtonCode::RightBracket: return "AD12";
	case ButtonCode::Enter:
		return "RTRN";

		// Row #4
	case ButtonCode::CapsLock: return "CAPS";
	case ButtonCode::A: return "AC01";
	case ButtonCode::S: return "AC02";
	case ButtonCode::D: return "AC03";
	case ButtonCode::F: return "AC04";
	case ButtonCode::G: return "AC05";
	case ButtonCode::H: return "AC06";
	case ButtonCode::J: return "AC07";
	case ButtonCode::K: return "AC08";
	case ButtonCode::L: return "AC09";
	case ButtonCode::Semicolon: return "AC10";
	case ButtonCode::Apostrophe: return "AC11";
	case ButtonCode::Backslash:
		return "BKSL";

		// Row #5
	case ButtonCode::LeftShift: return "LFSH";
	case ButtonCode::Z: return "AB01";
	case ButtonCode::X: return "AB02";
	case ButtonCode::C: return "AB03";
	case ButtonCode::V: return "AB04";
	case ButtonCode::B: return "AB05";
	case ButtonCode::N: return "AB06";
	case ButtonCode::M: return "AB07";
	case ButtonCode::Comma: return "AB08";
	case ButtonCode::Period: return "AB09";
	case ButtonCode::Slash: return "AB10";
	case ButtonCode::RightShift:
		return "RTSH";

		// Row #6
	case ButtonCode::LeftControl: return "LCTL";
	case ButtonCode::LeftWindows: return "LWIN";
	case ButtonCode::LeftAlt: return "LALT";
	case ButtonCode::Space: return "SPCE";
	case ButtonCode::RightAlt: return "RALT";
	case ButtonCode::RightWindows: return "RWIN";
	case ButtonCode::RightControl:
		return "RCTL";

		// Keypad
	case ButtonCode::Numpad0: return "KP0";
	case ButtonCode::Numpad1: return "KP1";
	case ButtonCode::Numpad2: return "KP2";
	case ButtonCode::Numpad3: return "KP3";
	case ButtonCode::Numpad4: return "KP4";
	case ButtonCode::Numpad5: return "KP5";
	case ButtonCode::Numpad6: return "KP6";
	case ButtonCode::Numpad7: return "KP7";
	case ButtonCode::Numpad8: return "KP8";
	case ButtonCode::Numpad9: return "KP9";

	case ButtonCode::NumLock: return "NMLK";
	case ButtonCode::NumpadDivide: return "KPDV";
	case ButtonCode::NumpadMultiply: return "KPMU";
	case ButtonCode::NumpadMinus: return "KPSU";
	case ButtonCode::NumpadPlus: return "KPAD";
	case ButtonCode::NumpadDecimal: return "KPDL";
	case ButtonCode::NumpadEnter: return "KPEN";
	case ButtonCode::NumadEquals:
		return "KPEQ";

		// Special keys
	case ButtonCode::ScrollLock: return "SCLK";
	case ButtonCode::Pause: return "PAUS";

	case ButtonCode::Insert: return "INS";
	case ButtonCode::Home: return "HOME";
	case ButtonCode::PageUp: return "PGUP";
	case ButtonCode::Delete: return "DELE";
	case ButtonCode::End: return "END";
	case ButtonCode::PageDown: return "PGDN";

	case ButtonCode::ArrowUp: return "UP";
	case ButtonCode::ArrowLeft: return "LEFT";
	case ButtonCode::ArrowDown: return "DOWN";
	case ButtonCode::ArrowRight: return "RGHT";

	case ButtonCode::Mute: return "MUTE";
	case ButtonCode::VolumeDown: return "VOL-";
	case ButtonCode::VolumeUp: return "VOL+";
	case ButtonCode::Power:
		return "POWR";

		// International keys
	case ButtonCode::OEM102: return "LSGT"; // German keyboard: < > |
	case ButtonCode::Kana: return "AB11"; // Taking a guess here, many layouts map <AB11> to "kana_RO"
	case ButtonCode::Yen: return "AE13"; // Taking a guess, often mapped to yen

	default:
		// Missing Japanese (?): KATA, HIRA, HENK, MUHE, JPCM
		// Missing Korean (?): HNGL, HJCV
		// Missing because it's not clear which ButtonCode is correct: PRSC (print screen), LVL3 (AltGr), MENU
		// Misc: LNFD (line feed), I120, I126, I128, I129, COMP, STOP, AGAI (redo), PROP, UNDO, FRNT, COPY, OPEN, PAST
		// FIND, CUT, HELP, I147-I190, FK16-FK24, MDSW (mode switch), ALT, META, SUPR, HYPR, I208-I253
		break;
	}

	return nullptr;
}

String Platform::KeyCodeToUnicode(u32 buttonCode)
{
	Lock lock(mData->Sync);

	const char* keyName = ButtonCodeToKeyName((ButtonCode)buttonCode);
	if(keyName == nullptr)
	{
		// Not a printable key
		return "";
	}

	auto iterFind = mData->KeyNameMap.find(String(keyName));
	if(iterFind == mData->KeyNameMap.end())
	{
		// Cannot find mapping, although this shouldn't really happen
		return "";
	}

	XKeyPressedEvent event;
	B3DZeroOut(event);
	event.type = KeyPress;
	event.keycode = iterFind->second;
	event.display = mData->XDisplay;
	event.time = CurrentTime;
	event.window = mData->MainXWindow;
	event.root = RootWindow(mData->XDisplay, XDefaultScreen(mData->XDisplay));

	Status status;
	char buffer[16];

	i32 length = Xutf8LookupString(mData->IC, &event, buffer, sizeof(buffer), nullptr, &status);
	if(length > 0)
	{
		buffer[length] = '\0';

		return String(buffer);
	}

	return "";
}

void Platform::OpenFolder(const Path& path)
{
	String pathString = path.ToString();

	// Launch xdg-open directly (no shell) so paths containing quotes or other shell metacharacters can't break out of the command
	const char* argv[] = { "xdg-open", pathString.c_str(), nullptr };

	pid_t processId = 0;
	const int result = posix_spawnp(&processId, "xdg-open", nullptr, nullptr, const_cast<char* const*>(argv), environ);
	if(result != 0)
	{
		B3D_LOG(Error, LogPlatform, "Failed to launch xdg-open (error {0}).", String(strerror(result)));
		return;
	}

	// xdg-open hands off to the file manager and exits quickly; reap it so it doesn't linger as a zombie.
	waitpid(processId, nullptr, 0);
}

/**
 * Converts an X11 KeySym code into an input command, if possible. Returns true if conversion was done.
 *
 * @param[in]	keySym			KeySym to try to translate to a command.
 * @param[in]	shift			True if the shift key was held down when the key was pressed.
 * @param[out]	command			Input command. Only valid if function returns true.
 * @return						True if the KeySym is an input command.
 */
static bool ParseInputCommand(KeySym keySym, bool shift, InputCommandType& command)
{
	switch(keySym)
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
static LinuxWindow* GetLinuxWindow(Platform::Private* data, ::Window xWindow)
{
	auto iterFind = data->WindowMap.find(xWindow);
	if(iterFind != data->WindowMap.end())
	{
		LinuxWindow* window = iterFind->second;
		return window;
	}

	return nullptr;
}

/** Returns a RenderWindow from a native X11 window handle. Returns null if the window isn't a RenderWindow */
static RenderWindow* GetRenderWindowFromXWindow(Platform::Private* data, ::Window xWindow)
{
	LinuxWindow* linuxWindow = GetLinuxWindow(data, xWindow);
	if(linuxWindow != nullptr)
		return (RenderWindow*)linuxWindow->GetUserDataInternal();

	return nullptr;
}

/**
 * Enqueue a button press/release event to be handled by the main thread
 *
 * @param bc        ButtonCode for the button that was pressed or released
 * @param pressed   true if the button was pressed, false if it was released
 * @param timestamp Time when the event happened
 */
static void EnqueueButtonEvent(ButtonCode bc, bool pressed, u64 timestamp)
{
	if(bc == ButtonCode::Unassigned)
		return;

	Lock eventLock(LinuxPlatform::eventLock);

	LinuxButtonEvent event;
	event.Button = bc;
	event.Pressed = pressed;
	event.Timestamp = timestamp;
	LinuxPlatform::buttonEvents.push(event);
}

void Platform::MessagePump()
{
	while(true)
	{
		Lock lock(mData->Sync);

		if(XPending(mData->XDisplay) <= 0)
			break;

		XEvent event;
		XNextEvent(mData->XDisplay, &event);

		XGenericEventCookie* cookie = &event.xcookie;
		if(cookie->type == GenericEvent && cookie->extension == mData->XInput2Opcode)
		{
			XGetEventData(mData->XDisplay, cookie);
			XIRawEvent* xInput2Event = (XIRawEvent*)cookie->data;
			switch(xInput2Event->evtype)
			{
			case XI_RawMotion:
				if(xInput2Event->valuators.mask_len > 0)
				{
					// Assume X/Y delta is stored in valuators 0/1 and vertical scroll in valuator 3.
					// While there is an API that reliably tells us the valuator index for vertical scroll, there's
					// nothing "more reliable" for X/Y axes, as the only way to possibly identify them from device
					// info is by axis name, so we can use the axis index directly just as well. GDK seems to assume
					// 0 for x and 1 for y too, so that's hopefully safe, and 3 appears to be common for the scroll
					// wheel.
					float deltas[4] = { 0 };
					int currentValuesIndex = 0;
					for(unsigned int valuator = 0; valuator < 4; valuator++)
						if(XIMaskIsSet(xInput2Event->valuators.mask, valuator))
							deltas[valuator] = xInput2Event->raw_values[currentValuesIndex++];

					Lock eventLock(LinuxPlatform::eventLock);
					LinuxPlatform::mouseMotionEvent.DeltaX += deltas[0];
					LinuxPlatform::mouseMotionEvent.DeltaY += deltas[1];
					LinuxPlatform::mouseMotionEvent.DeltaZ += deltas[3]; // Not a typo - 2 is for horizontal scroll.
				}
				break;
			}

			XFreeEventData(mData->XDisplay, cookie);
		}

		switch(event.type)
		{
		case ClientMessage:
			{
				if(LinuxDragAndDrop::HandleClientMessage(event.xclient))
					break;

				// User requested the window to close
				if((Atom)event.xclient.data.l[0] == mData->AtomDeleteWindow)
				{
					LinuxWindow* window = GetLinuxWindow(mData, event.xclient.window);
					if(window != nullptr)
					{
						// If it's a render window we allow the client code to handle the message
						RenderWindow* renderWindow = (RenderWindow*)window->GetUserDataInternal();
						if(renderWindow != nullptr)
							renderWindow->NotifyWindowEvent(WindowEventType::CloseRequested);
						else // If not, we just destroy the window
							window->DestroyInternal();
					}
				}
			}
			break;
		case KeyPress:
			{
				XKeyPressedEvent* keyEvent = (XKeyPressedEvent*)&event;
				const ButtonCode keyButtonCode = keyEvent->keycode < (u32)mData->KeyCodeMap.size()
					? mData->KeyCodeMap[keyEvent->keycode] : ButtonCode::Unassigned;
				EnqueueButtonEvent(keyButtonCode, true, (u64)keyEvent->time);

				// Process text input
				KeySym keySym = XkbKeycodeToKeysym(mData->XDisplay, (KeyCode)event.xkey.keycode, 0, 0);

				// Handle input commands
				InputCommandType command = InputCommandType::Backspace;
				bool shift = (event.xkey.state & ShiftMask) != 0;

				bool isInputCommand = ParseInputCommand(keySym, shift, command);

				// Check if input manager wants this event. If not, we process it.
				if(XFilterEvent(&event, None) == False && !isInputCommand)
				{
					// Send a text input event
					Status status;
					char buffer[16];

					i32 length = Xutf8LookupString(mData->IC, &event.xkey, buffer, sizeof(buffer), nullptr, &status);

					if(length > 0)
					{
						buffer[length] = '\0';

						U32String utfStr = UTF8::ToUtF32(String(buffer));
						if(utfStr.length() > 0)
							OnCharInput((u32)utfStr[0]);
					}
				}

				// Send an input command event
				if(isInputCommand)
				{
					if(!OnInputCommand.Empty())
						OnInputCommand(command);
				}
			}
			break;
		case KeyRelease:
			{
				XKeyReleasedEvent* keyEvent = (XKeyReleasedEvent*)&event;
				const ButtonCode keyButtonCode = keyEvent->keycode < (u32)mData->KeyCodeMap.size()
					? mData->KeyCodeMap[keyEvent->keycode] : ButtonCode::Unassigned;
				EnqueueButtonEvent(keyButtonCode, false, (u64)keyEvent->time);
			}
			break;
		case ButtonPress:
			{
				XButtonPressedEvent* buttonEvent = (XButtonPressedEvent*)&event;
				u32 button = event.xbutton.button;
				EnqueueButtonEvent(XButtonToButtonCode(button), true, (u64)buttonEvent->time);

				OSPointerButtonStates btnStates;
				btnStates.MouseButtons[0] = (event.xbutton.state & Button1Mask) != 0;
				btnStates.MouseButtons[1] = (event.xbutton.state & Button2Mask) != 0;
				btnStates.MouseButtons[2] = (event.xbutton.state & Button3Mask) != 0;

				OSMouseButton mouseButton;
				bool validPress = false;
				switch(button)
				{
				case Button1:
					mouseButton = OSMouseButton::Left;
					btnStates.MouseButtons[0] = true;
					validPress = true;
					break;
				case Button2:
					mouseButton = OSMouseButton::Middle;
					btnStates.MouseButtons[1] = true;
					validPress = true;
					break;
				case Button3:
					mouseButton = OSMouseButton::Right;
					btnStates.MouseButtons[2] = true;
					validPress = true;
					break;

				default:
					break;
				}

				if(validPress)
				{
					// Send event
					Vector2I pos;
					pos.X = event.xbutton.x_root;
					pos.Y = event.xbutton.y_root;

					btnStates.Ctrl = (event.xbutton.state & ControlMask) != 0;
					btnStates.Shift = (event.xbutton.state & ShiftMask) != 0;

					OnPointerButtonPressed(pos, mouseButton, btnStates);

					// Handle double-click
					if(button == Button1)
					{
						if(event.xbutton.time < (mData->LastButtonPressTime + DOUBLE_CLICK_MS))
						{
							OnPointerDoubleClick(pos, btnStates);
							mData->LastButtonPressTime = 0;
						}
						else
							mData->LastButtonPressTime = event.xbutton.time;
					}
				}

				// Handle window dragging for windows without a title bar
				if(button == Button1)
				{
					LinuxWindow* window = GetLinuxWindow(mData, event.xbutton.window);
					if(window != nullptr)
						window->DragStartInternal(event.xbutton);
				}

				break;
			}
		case ButtonRelease:
			{
				XButtonReleasedEvent* buttonEvent = (XButtonReleasedEvent*)&event;
				u32 button = event.xbutton.button;
				EnqueueButtonEvent(XButtonToButtonCode(button), false, (u64)buttonEvent->time);

				Vector2I pos;
				pos.X = event.xbutton.x_root;
				pos.Y = event.xbutton.y_root;

				OSPointerButtonStates btnStates;
				btnStates.Ctrl = (event.xbutton.state & ControlMask) != 0;
				btnStates.Shift = (event.xbutton.state & ShiftMask) != 0;
				btnStates.MouseButtons[0] = (event.xbutton.state & Button1Mask) != 0;
				btnStates.MouseButtons[1] = (event.xbutton.state & Button2Mask) != 0;
				btnStates.MouseButtons[2] = (event.xbutton.state & Button3Mask) != 0;

				switch(button)
				{
				case Button1:
					btnStates.MouseButtons[0] = false;
					OnPointerButtonReleased(pos, OSMouseButton::Left, btnStates);
					break;
				case Button2:
					btnStates.MouseButtons[1] = false;
					OnPointerButtonReleased(pos, OSMouseButton::Middle, btnStates);
					break;
				case Button3:
					btnStates.MouseButtons[2] = false;
					OnPointerButtonReleased(pos, OSMouseButton::Right, btnStates);
					break;
				case Button4: // Vertical mouse wheel
				case Button5:
					{
						i32 delta = button == Button4 ? 1 : -1;
						OnMouseWheelScrolled((float)delta);
					}
					break;
				default:
					break;
				}

				// Handle window dragging for windows without a title bar
				if(button == Button1)
				{
					LinuxWindow* window = GetLinuxWindow(mData, event.xbutton.window);
					if(window != nullptr)
						window->DragEndInternal();
				}

				break;
			}
		case MotionNotify:
			{
				Vector2I pos;
				pos.X = event.xmotion.x_root;
				pos.Y = event.xmotion.y_root;

				// Handle clipping if enabled
				if(ClipCursor(mData, pos))
					SetCursorPositionInternal(mData, pos);

				// Send event
				OSPointerButtonStates btnStates;
				btnStates.Ctrl = (event.xmotion.state & ControlMask) != 0;
				btnStates.Shift = (event.xmotion.state & ShiftMask) != 0;
				btnStates.MouseButtons[0] = (event.xmotion.state & Button1Mask) != 0;
				btnStates.MouseButtons[1] = (event.xmotion.state & Button2Mask) != 0;
				btnStates.MouseButtons[2] = (event.xmotion.state & Button3Mask) != 0;

				OnPointerMoved(pos, btnStates);
			}
			break;
		case EnterNotify:
			// Do nothing
			break;
		case LeaveNotify:
			{
				if(event.xcrossing.mode == NotifyNormal)
				{
					Vector2I pos;
					pos.X = event.xcrossing.x_root;
					pos.Y = event.xcrossing.y_root;

					if(ClipCursor(mData, pos))
						SetCursorPositionInternal(mData, pos);
				}

				RenderWindow* renderWindow = GetRenderWindowFromXWindow(mData, event.xcrossing.window);
				if(renderWindow != nullptr)
					renderWindow->NotifyWindowEvent(WindowEventType::MouseLeft);
			}
			break;
		case ConfigureNotify:
			{
				LinuxWindow* window = GetLinuxWindow(mData, event.xconfigure.window);
				if(window != nullptr)
				{
					UpdateClipBounds(mData, window);

					RenderWindow* renderWindow = (RenderWindow*)window->GetUserDataInternal();
					if(renderWindow != nullptr)
					{
						renderWindow->NotifyWindowEvent(WindowEventType::Resized);
						renderWindow->NotifyWindowEvent(WindowEventType::Moved);
					}
				}
			}
			break;
		case FocusIn:
			{
				// Update input context focus
				XSetICFocus(mData->IC);

				// Send event to render window
				RenderWindow* renderWindow = GetRenderWindowFromXWindow(mData, event.xfocus.window);

				// Not a render window, so it doesn't care about these events
				if(renderWindow != nullptr)
				{
					if(!renderWindow->GetRenderWindowProperties().HasFocus)
						renderWindow->NotifyWindowEvent(WindowEventType::FocusReceived);
				}
			}
			break;
		case FocusOut:
			{
				// Update input context focus
				XUnsetICFocus(mData->IC);

				// Send event to render window
				RenderWindow* renderWindow = GetRenderWindowFromXWindow(mData, event.xfocus.window);

				// Not a render window, so it doesn't care about these events
				if(renderWindow != nullptr)
				{
					if(renderWindow->GetRenderWindowProperties().HasFocus)
						renderWindow->NotifyWindowEvent(WindowEventType::FocusLost);
				}
			}
			break;
		case SelectionNotify:
			LinuxDragAndDrop::HandleSelectionNotify(event.xselection);
			break;
		case SelectionRequest:
			{
				// Send the data saved by the last clipboard copy operation
				Atom compoundTextAtom = XInternAtom(mData->XDisplay, "COMPOUND_TEXT", 0);
				Atom utf8StringAtom = XInternAtom(mData->XDisplay, "UTF8_STRING", 0);
				Atom targetsAtom = XInternAtom(mData->XDisplay, "TARGETS", 0);

				XSelectionRequestEvent& selReq = event.xselectionrequest;
				XEvent response;
				if(selReq.target == XA_STRING || selReq.target == compoundTextAtom || selReq.target == utf8StringAtom)
				{
					String utf8data = mData->ClipboardData;

					const u8* data = (const u8*)utf8data.c_str();
					i32 dataLength = (i32)utf8data.length();

					XChangeProperty(mData->XDisplay, selReq.requestor, selReq.property, selReq.target, 8, PropModeReplace, data, dataLength);

					response.xselection.property = selReq.property;
				}
				else if(selReq.target == targetsAtom)
				{
					Atom data[2];
					data[0] = utf8StringAtom;
					data[1] = XA_STRING;
					XChangeProperty(mData->XDisplay, selReq.requestor, selReq.property, selReq.target, 8, PropModeReplace, (unsigned char*)&data, sizeof(data));

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

				XSendEvent(mData->XDisplay, selReq.requestor, 0, 0, &response);
				XFlush(mData->XDisplay);
			}
			break;
		case PropertyNotify:
			// Report minimize, maximize and restore events
			if(event.xproperty.atom == mData->AtomWmState)
			{
				// Check that the window hasn't been destroyed
				if(GetLinuxWindow(mData, event.xproperty.window) == nullptr)
					break;

				Atom type;
				i32 format;
				unsigned long count, bytesRemaining;
				u8* data = nullptr;

				i32 result = XGetWindowProperty(mData->XDisplay, event.xproperty.window, mData->AtomWmState, 0, 1024, False, AnyPropertyType, &type, &format, &count, &bytesRemaining, &data);

				if(result == Success)
				{
					RenderWindow* renderWindow = GetRenderWindowFromXWindow(mData, event.xproperty.window);

					// Not a render window, so it doesn't care about these events
					if(renderWindow == nullptr)
					{
						XFree(data);
						break;
					}

					Atom* atoms = (Atom*)data;

					bool foundHorz = false;
					bool foundVert = false;
					for(unsigned long i = 0; i < count; i++)
					{
						if(atoms[i] == mData->AtomWmStateMaxHorz) foundHorz = true;
						if(atoms[i] == mData->AtomWmStateMaxVert) foundVert = true;

						if(foundVert && foundHorz)
						{
							if(event.xproperty.state == PropertyNewValue)
								renderWindow->NotifyWindowEvent(WindowEventType::Maximized);
							else
								renderWindow->NotifyWindowEvent(WindowEventType::Restored);
						}

						if(atoms[i] == mData->AtomWmStateHidden)
						{
							if(event.xproperty.state == PropertyNewValue)
								renderWindow->NotifyWindowEvent(WindowEventType::Minimized);
							else
								renderWindow->NotifyWindowEvent(WindowEventType::Restored);
						}
					}

					XFree(data);
				}
			}
			break;
		default:
			break;
		}
	}
}

void Platform::StartUp()
{
	XInitThreads(); // TODO: It might be fine to remove XLib locks everywhere, but best get things working first

	Lock lock(mData->Sync);
	mData->XDisplay = XOpenDisplay(nullptr);
	XSetErrorHandler(X11ErrorHandler);

	// Watch for input devices getting attached/detached, used for gamepad hot-plug detection. IN_ATTRIB is needed on
	// top of IN_CREATE because a new device node only becomes readable once udev adjusts its permissions, which happens
	// shortly after the node gets created.
	mData->InotifyHandle = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if(mData->InotifyHandle != -1)
	{
		mData->InotifyDevInputWatch = inotify_add_watch(mData->InotifyHandle, "/dev/input",
			IN_CREATE | IN_DELETE | IN_ATTRIB);

		if(mData->InotifyDevInputWatch == -1)
			B3D_LOG(Error, LogPlatform, "Unable to watch /dev/input, gamepad hot-plug won't be detected.");
	}
	else
		B3D_LOG(Error, LogPlatform, "Failed to initialize inotify, gamepad hot-plug won't be detected.");

	// For raw, relative mouse motion events, XInput2 extension is required
	int firstEvent;
	int firstError;
	if(!XQueryExtension(mData->XDisplay, "XInputExtension", &mData->XInput2Opcode, &firstEvent, &firstError))
		B3D_LOG(Fatal, LogPlatform, "X Server doesn't support the XInput extension");

	int majorVersion = 2;
	int minorVersion = 0;
	if(XIQueryVersion(mData->XDisplay, &majorVersion, &minorVersion) != Success)
		B3D_LOG(Fatal, LogPlatform, "X Server doesn't support at least the XInput 2.0 extension");

	// Let XInput know we are interested in raw mouse movement events
	constexpr int maskLen = XIMaskLen(XI_LASTEVENT);
	XIEventMask mask;
	mask.deviceid = XIAllDevices;
	mask.mask_len = maskLen;

	unsigned char maskBuffer[maskLen] = { 0 };
	mask.mask = maskBuffer;
	XISetMask(mask.mask, XI_RawMotion);

	// "RawEvents are sent exclusively to all root windows", so this should receive all events, even though we only
	// select on one display's root window (untested for lack of second screen).
	XISelectEvents(mData->XDisplay, XRootWindow(mData->XDisplay, DefaultScreen(mData->XDisplay)), &mask, 1);
	XFlush(mData->XDisplay);

	if(XSupportsLocale())
	{
		XSetLocaleModifiers("@im=none");
		mData->IM = XOpenIM(mData->XDisplay, nullptr, nullptr, nullptr);

		// Note: Currently our windows don't support pre-edit and status areas, which are used for more complex types
		// of character input. Later on it might be beneficial to support them.
		mData->IC = XCreateIC(mData->IM, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, nullptr);
	}

	mData->AtomDeleteWindow = XInternAtom(mData->XDisplay, "WM_DELETE_WINDOW", False);
	mData->AtomWmState = XInternAtom(mData->XDisplay, "_NET_WM_STATE", False);
	mData->AtomWmStateHidden = XInternAtom(mData->XDisplay, "_NET_WM_STATE_HIDDEN", False);
	mData->AtomWmStateMaxHorz = XInternAtom(mData->XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	mData->AtomWmStateMaxVert = XInternAtom(mData->XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);

	// Drag and drop
	LinuxDragAndDrop::StartUp(mData->XDisplay);

	// Create empty cursor
	char data[1];
	memset(data, 0, sizeof(data));

	Pixmap pixmap = XCreateBitmapFromData(mData->XDisplay, DefaultRootWindow(mData->XDisplay), data, 1, 1);

	XColor color;
	color.red = color.green = color.blue = 0;
	mData->EmptyCursor = XCreatePixmapCursor(mData->XDisplay, pixmap, pixmap, &color, &color, 0, 0);

	XFreePixmap(mData->XDisplay, pixmap);

	// Initialize "unique X11 keyname" -> "X11 keycode" map
	char name[XkbKeyNameLength + 1];

	XkbDescPtr desc = XkbGetMap(mData->XDisplay, 0, XkbUseCoreKbd);
	XkbGetNames(mData->XDisplay, XkbKeyNamesMask, desc);

	for(u32 keyCode = desc->min_key_code; keyCode <= desc->max_key_code; keyCode++)
	{
		memcpy(name, desc->names->keys[keyCode].name, XkbKeyNameLength);
		name[XkbKeyNameLength] = '\0';

		mData->KeyNameMap[String(name)] = keyCode;
	}

	// Initialize "X11 keycode" -> "Banshee ButtonCode" map, based on the KeyNameMap and ButtonCodeToKeyName()
	mData->KeyCodeMap.resize(desc->max_key_code + 1, ButtonCode::Unassigned);

	XkbFreeNames(desc, XkbKeyNamesMask, True);
	XkbFreeKeyboard(desc, 0, True);

	for(u32 buttonCodeNum = (u32)ButtonCode::Unassigned; buttonCodeNum <= (u32)ButtonCode::KeyboardKeyCount; buttonCodeNum++)
	{
		ButtonCode buttonCode = (ButtonCode)buttonCodeNum;
		const char* keyNameCStr = ButtonCodeToKeyName(buttonCode);

		if(keyNameCStr != nullptr)
		{
			String keyName = String(keyNameCStr);
			auto iterFind = mData->KeyNameMap.find(keyName);
			if(iterFind != mData->KeyNameMap.end())
			{
				KeyCode keyCode = iterFind->second;
				mData->KeyCodeMap[keyCode] = buttonCode;
			}
		}
	}
}

/** Checks if any input devices got attached/detached and triggers OnInputDevicesChanged if they did. */
static void CheckInputDevicesChanged(Platform::Private* data)
{
	if(data->InotifyDevInputWatch == -1)
		return;

	alignas(inotify_event) char buffer[4096];
	bool devicesChanged = false;

	while(true)
	{
		const ssize_t numReadBytes = read(data->InotifyHandle, buffer, sizeof(buffer));
		if(numReadBytes <= 0)
			break;

		ssize_t offset = 0;
		while(offset < numReadBytes)
		{
			const auto* event = (const inotify_event*)(buffer + offset);

			// Gamepads are only ever enumerated through the evdev (/dev/input/eventX) nodes
			if(event->len > 0 && strncmp(event->name, "event", 5) == 0)
				devicesChanged = true;

			offset += (ssize_t)(sizeof(inotify_event) + event->len);
		}
	}

	if(devicesChanged && !Platform::OnInputDevicesChanged.Empty())
		Platform::OnInputDevicesChanged();
}

void Platform::Update()
{
	LinuxDragAndDrop::Update();
	CheckInputDevicesChanged(mData);
	MessagePump();
}

void Platform::ShutDown()
{
	Lock lock(mData->Sync);

	// Free empty cursor
	XFreeCursor(mData->XDisplay, mData->EmptyCursor);
	mData->EmptyCursor = None;

	// Shutdown drag and drop
	LinuxDragAndDrop::ShutDown();

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

	XCloseDisplay(mData->XDisplay);
	mData->XDisplay = nullptr;

	if(mData->InotifyHandle != -1)
	{
		if(mData->InotifyDevInputWatch != -1)
			inotify_rm_watch(mData->InotifyHandle, mData->InotifyDevInputWatch);

		close(mData->InotifyHandle);
		mData->InotifyHandle = -1;
		mData->InotifyDevInputWatch = -1;
	}

	B3DDelete(mData);
	mData = nullptr;
}

::Display* LinuxPlatform::GetXDisplay()
{
	return mData->XDisplay;
}

::Window LinuxPlatform::GetMainXWindow()
{
	return mData->MainXWindow;
}

Path LinuxPlatform::GetHomeDir()
{
	const char* homeDir = getenv("HOME");
	if(!homeDir)
		homeDir = getpwuid(getuid())->pw_dir;

	return Path(homeDir);
}

void LinuxPlatform::LockX()
{
	mData->Sync.lock();
}

void LinuxPlatform::UnlockX()
{
	mData->Sync.unlock();
}

void LinuxPlatform::RegisterWindowInternal(::Window xWindow, LinuxWindow* window)
{
	// First window is assumed to be the main
	if(mData->MainXWindow == 0)
	{
		mData->MainXWindow = xWindow;

		// Input context client window must be set before use
		XSetICValues(mData->IC, XNClientWindow, xWindow, XNFocusWindow, xWindow, nullptr);
	}

	mData->WindowMap[xWindow] = window;

	ApplyCurrentCursor(mData, xWindow);
}

void LinuxPlatform::UnregisterWindowInternal(::Window xWindow)
{
	auto iterFind = mData->WindowMap.find(xWindow);
	if(iterFind != mData->WindowMap.end())
	{
		if(mData->CursorClipEnabled && mData->CursorClipWindow == iterFind->second)
			DisableCursorClipping(mData);

		mData->WindowMap.erase(iterFind);
	}

	if(mData->MainXWindow == xWindow)
		mData->MainXWindow = 0;
}

LinuxWindow* LinuxPlatform::GetWindowInternal(::Window xWindow)
{
	auto found = mData->WindowMap.find(xWindow);
	return found != mData->WindowMap.end() ? found->second : nullptr;
}

Pixmap LinuxPlatform::CreatePixmap(const PixelData& data, u32 depth)
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
	TShared<PixelData> bgraData = PixelData::Create(data.GetWidth(), data.GetHeight(), 1, PF_BGRA8);
	bgraData->SetColors(colors);

	XImage* image = XCreateImage(mData->XDisplay, CopyFromParent, depth, ZPixmap, 0, (char*)bgraData->GetData(), data.GetWidth(), data.GetHeight(), 32, 0);

	Pixmap pixmap = XCreatePixmap(mData->XDisplay, XDefaultRootWindow(mData->XDisplay), data.GetWidth(), data.GetHeight(), depth);

	XGCValues gcValues;
	GC gc = XCreateGC(mData->XDisplay, pixmap, 0, &gcValues);
	XPutImage(mData->XDisplay, pixmap, gc, image, 0, 0, 0, 0, data.GetWidth(), data.GetHeight());
	XFreeGC(mData->XDisplay, gc);

	// Make sure XDestroyImage doesn't free the data pointed to by 'data.bytes'
	image->data = nullptr;
	XDestroyImage(image);

	return pixmap;
}
