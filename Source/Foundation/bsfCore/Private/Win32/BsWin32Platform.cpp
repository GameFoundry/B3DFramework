//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/BsWin32Platform.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Image/BsPixelUtil.h"
#include "BsCoreApplication.h"
#include "Debug/BsDebug.h"
#include "Managers/BsRenderWindowManager.h"
#include "Platform/BsDropTarget.h"
#include "Private/Win32/BsWin32DropTarget.h"
#include "Private/Win32/BsWin32PlatformUtility.h"
#include "TimeAPI.h"
#include <shellapi.h>
#include "String/BsUnicode.h"

using namespace bs;

/** Encapsulate native cursor data so we can avoid including windows.h as it pollutes the global namespace. */
struct BS_CORE_EXPORT NativeCursorData
{
	HCURSOR Cursor;
};

/**	Encapsulate drop target data so we can avoid including windows.h as it pollutes the global namespace. */
struct BS_CORE_EXPORT NativeDropTargetData
{
	Map<const RenderWindow*, Win32DropTarget*> DropTargetsPerWindow;
	Vector<Win32DropTarget*> DropTargetsToInitialize;
	Vector<Win32DropTarget*> DropTargetsToDestroy;
};

struct Platform::Pimpl
{
	bool MIsCursorHidden = false;
	NativeCursorData MCursor;
	bool MUsingCustomCursor = false;
	Map<const ct::RenderWindow*, WindowNonClientAreaData> MNonClientAreas;

	bool MIsTrackingMouse = false;
	NativeDropTargetData MDropTargets;

	bool MRequiresStartUp = false;
	bool MRequiresShutDown = false;

	bool MCursorClipping = false;
	HWND MClipWindow = 0;
	RECT MClipRect;

	bool MIsActive = false;

	Mutex MSync;
};

Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorMoved;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonPressed;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonReleased;
Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorDoubleClick;
Event<void(InputCommandType)> Platform::onInputCommand;
Event<void(float)> Platform::onMouseWheelScrolled;
Event<void(u32)> Platform::onCharInput;

Event<void()> Platform::onMouseCaptureChanged;

Platform::Pimpl* Platform::mData = B3DNew<Platform::Pimpl>();

/** Checks if any of the windows of the current application are active. */
bool isAppActive(Platform::Pimpl* data)
{
	Lock lock(data->MSync);

	return data->MIsActive;
}

/** Enables or disables cursor clipping depending on the stored data. */
void applyClipping(Platform::Pimpl* data)
{
	if(data->MCursorClipping)
	{
		if(data->MClipWindow)
		{
			// Clip cursor to the window
			RECT clipWindowRect;
			if(GetWindowRect(data->MClipWindow, &clipWindowRect))
				ClipCursor(&clipWindowRect);
		}
		else
			ClipCursor(&data->MClipRect);
	}
	else
		ClipCursor(nullptr);
}

Platform::~Platform()
{
	B3DDelete(mData);
	mData = nullptr;
}

Vector2I Platform::GetCursorPosition()
{
	Vector2I screenPos;

	POINT cursorPos;
	GetCursorPos(&cursorPos);

	screenPos.X = cursorPos.x;
	screenPos.Y = cursorPos.y;

	return screenPos;
}

void Platform::SetCursorPosition(const Vector2I& screenPos)
{
	SetCursorPos(screenPos.X, screenPos.Y);
}

void Platform::CaptureMouse(const RenderWindow& window)
{
	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_BS_SETCAPTURE, WPARAM((HWND)hwnd), 0);
}

void Platform::ReleaseMouseCapture()
{
	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_BS_RELEASECAPTURE, WPARAM((HWND)hwnd), 0);
}

bool Platform::IsPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
{
	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();

	POINT point;
	point.x = screenPos.X;
	point.y = screenPos.Y;

	u64 hwndToCheck;
	window.GetCustomAttribute("WINDOW", &hwndToCheck);

	HWND hwndUnderPos = WindowFromPoint(point);
	return hwndUnderPos == (HWND)hwndToCheck;
}

void Platform::HideCursor()
{
	if(mData->MIsCursorHidden)
		return;

	mData->MIsCursorHidden = true;

	// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
	// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

void Platform::ShowCursor()
{
	if(!mData->MIsCursorHidden)
		return;

	mData->MIsCursorHidden = false;

	// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
	// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

bool Platform::IsCursorHidden()
{
	return mData->MIsCursorHidden;
}

void Platform::ClipCursorToWindow(const RenderWindow& window)
{
	u64 hwnd;
	window.GetCustomAttribute("WINDOW", &hwnd);

	mData->MCursorClipping = true;
	mData->MClipWindow = (HWND)hwnd;

	if(isAppActive(mData))
		applyClipping(mData);
}

void Platform::ClipCursorToRect(const Rect2I& screenRect)
{
	mData->MCursorClipping = true;
	mData->MClipWindow = 0;

	mData->MClipRect.left = screenRect.X;
	mData->MClipRect.top = screenRect.Y;
	mData->MClipRect.right = screenRect.X + screenRect.Width;
	mData->MClipRect.bottom = screenRect.Y + screenRect.Height;

	if(isAppActive(mData))
		applyClipping(mData);
}

void Platform::ClipCursorDisable()
{
	mData->MCursorClipping = false;
	mData->MClipWindow = 0;

	if(isAppActive(mData))
		applyClipping(mData);
}

// TODO - Add support for animated custom cursor
void Platform::SetCursor(PixelData& pixelData, const Vector2I& hotSpot)
{
	if(mData->MUsingCustomCursor)
	{
		::SetCursor(0);
		DestroyIcon(mData->MCursor.Cursor);
	}

	mData->MUsingCustomCursor = true;

	Vector<Color> pixels = pixelData.GetColors();
	u32 width = pixelData.GetWidth();
	u32 height = pixelData.GetHeight();

	HBITMAP hBitmap = Win32PlatformUtility::CreateBitmap((Color*)pixels.data(), width, height, false);
	HBITMAP hMonoBitmap = CreateBitmap(width, height, 1, 1, nullptr);

	ICONINFO iconinfo = { 0 };
	iconinfo.fIcon = FALSE;
	iconinfo.xHotspot = (DWORD)hotSpot.X;
	iconinfo.yHotspot = (DWORD)hotSpot.Y;
	iconinfo.hbmMask = hMonoBitmap;
	iconinfo.hbmColor = hBitmap;

	mData->MCursor.Cursor = CreateIconIndirect(&iconinfo);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	// Make sure we notify the message loop to perform the actual cursor update
	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

void Platform::SetIcon(const PixelData& pixelData)
{
	Vector<Color> pixels = pixelData.GetColors();
	u32 width = pixelData.GetWidth();
	u32 height = pixelData.GetHeight();

	HBITMAP hBitmap = Win32PlatformUtility::CreateBitmap((Color*)pixels.data(), width, height, false);
	HBITMAP hMonoBitmap = CreateBitmap(width, height, 1, 1, nullptr);

	ICONINFO iconinfo = { 0 };
	iconinfo.fIcon = TRUE;
	iconinfo.xHotspot = 0;
	iconinfo.yHotspot = 0;
	iconinfo.hbmMask = hMonoBitmap;
	iconinfo.hbmColor = hBitmap;

	HICON icon = CreateIconIndirect(&iconinfo);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	// Make sure we notify the message loop to perform the actual cursor update
	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	u64 hwnd;
	primaryWindow->GetCustomAttribute("WINDOW", &hwnd);

	PostMessage((HWND)hwnd, WM_SETICON, WPARAM(ICON_BIG), (LPARAM)icon);
}

void Platform::SetCaptionNonClientAreas(const ct::RenderWindow& window, const Vector<Rect2I>& nonClientAreas)
{
	Lock lock(mData->MSync);

	mData->MNonClientAreas[&window].MoveAreas = nonClientAreas;
}

void Platform::SetResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
{
	Lock lock(mData->MSync);

	mData->MNonClientAreas[&window].ResizeAreas = nonClientAreas;
}

void Platform::ResetNonClientAreas(const ct::RenderWindow& window)
{
	Lock lock(mData->MSync);

	auto iterFind = mData->MNonClientAreas.find(&window);

	if(iterFind != end(mData->MNonClientAreas))
		mData->MNonClientAreas.erase(iterFind);
}

void Platform::Sleep(u32 duration)
{
	::Sleep((DWORD)duration);
}

void Win32Platform::RegisterDropTarget(DropTarget* target)
{
	const RenderWindow* window = target->GetOwnerWindowInternal();

	Win32DropTarget* win32DropTarget = nullptr;
	auto iterFind = mData->MDropTargets.DropTargetsPerWindow.find(window);
	if(iterFind == mData->MDropTargets.DropTargetsPerWindow.end())
	{
		u64 hwnd;
		window->GetCustomAttribute("WINDOW", &hwnd);

		win32DropTarget = B3DNew<Win32DropTarget>((HWND)hwnd);
		mData->MDropTargets.DropTargetsPerWindow[window] = win32DropTarget;

		{
			Lock lock(mData->MSync);
			mData->MDropTargets.DropTargetsToInitialize.push_back(win32DropTarget);
		}
	}
	else
		win32DropTarget = iterFind->second;

	win32DropTarget->RegisterDropTarget(target);
}

void Win32Platform::UnregisterDropTarget(DropTarget* target)
{
	auto iterFind = mData->MDropTargets.DropTargetsPerWindow.find(target->GetOwnerWindowInternal());
	if(iterFind == mData->MDropTargets.DropTargetsPerWindow.end())
	{
		BS_LOG(Warning, Platform, "Attempting to destroy a drop target but cannot find its parent window.");
	}
	else
	{
		Win32DropTarget* win32DropTarget = iterFind->second;
		win32DropTarget->UnregisterDropTarget(target);

		if(win32DropTarget->GetNumDropTargets() == 0)
		{
			mData->MDropTargets.DropTargetsPerWindow.erase(iterFind);

			{
				Lock lock(mData->MSync);
				mData->MDropTargets.DropTargetsToDestroy.push_back(win32DropTarget);
			}
		}
	}
}

void Platform::CopyToClipboard(const String& string)
{
	WString wideString = UTF8::ToWide(string);

	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (wideString.size() + 1) * sizeof(WString::value_type));
	WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);

	wideString.copy(buffer, wideString.size());
	buffer[wideString.size()] = '\0';

	GlobalUnlock(hData);

	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hData);
		CloseClipboard();
	}
	else
	{
		GlobalFree(hData);
	}
}

String Platform::CopyFromClipboard()
{
	if(OpenClipboard(NULL))
	{
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);

		if(hData != NULL)
		{
			WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);
			WString wideString(buffer);
			GlobalUnlock(hData);

			CloseClipboard();
			return UTF8::FromWide(wideString);
		}
		else
		{
			CloseClipboard();
			return u8"";
		}
	}

	return u8"";
}

String Platform::KeyCodeToUnicode(u32 keyCode)
{
	static HKL keyboardLayout = GetKeyboardLayout(0);
	static u8 keyboarState[256];

	if(GetKeyboardState(keyboarState) == FALSE)
		return 0;

	UINT virtualKey = MapVirtualKeyExW(keyCode, 1, keyboardLayout);

	wchar_t output[2];
	int count = ToUnicodeEx(virtualKey, keyCode, keyboarState, output, 2, 0, keyboardLayout);
	if(count > 0)
		return UTF8::FromWide(WString(output, count));

	return StringUtil::kBlank;
}

void Platform::OpenFolder(const Path& path)
{
	WString pathString = UTF8::ToWide(path.ToString());

	ShellExecuteW(nullptr, L"open", pathString.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void Platform::MessagePumpInternal()
{
	MSG msg;
	while(PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Platform::StartUpInternal()
{
	Lock lock(mData->MSync);

	if(timeBeginPeriod(1) == TIMERR_NOCANDO)
	{
		BS_LOG(Warning, Platform, "Unable to set timer resolution to 1ms. This can cause significant waste "
								  "in performance for waiting threads.");
	}

	mData->MRequiresStartUp = true;
}

void Platform::UpdateInternal()
{
	for(auto& dropTarget : mData->MDropTargets.DropTargetsPerWindow)
	{
		dropTarget.second->Update();
	}
}

void Platform::CoreUpdateInternal()
{
	{
		Lock lock(mData->MSync);
		if(mData->MRequiresStartUp)
		{
			OleInitialize(nullptr);

			mData->MRequiresStartUp = false;
		}
	}

	{
		Lock lock(mData->MSync);
		for(auto& dropTargetToDestroy : mData->MDropTargets.DropTargetsToDestroy)
		{
			dropTargetToDestroy->UnregisterWithOs();
			dropTargetToDestroy->Release();
		}

		mData->MDropTargets.DropTargetsToDestroy.clear();
	}

	{
		Lock lock(mData->MSync);
		for(auto& dropTargetToInit : mData->MDropTargets.DropTargetsToInitialize)
		{
			dropTargetToInit->RegisterWithOs();
		}

		mData->MDropTargets.DropTargetsToInitialize.clear();
	}

	MessagePumpInternal();

	{
		Lock lock(mData->MSync);
		if(mData->MRequiresShutDown)
		{
			OleUninitialize();
			mData->MRequiresShutDown = false;
		}
	}
}

void Platform::ShutDownInternal()
{
	Lock lock(mData->MSync);

	timeEndPeriod(1);
	mData->MRequiresShutDown = true;
}

/**	Translate engine non client area to win32 non client area. */
LRESULT translateNonClientAreaType(NonClientAreaBorderType type)
{
	LRESULT dir = HTCLIENT;
	switch(type)
	{
	case NonClientAreaBorderType::Left:
		dir = HTLEFT;
		break;
	case NonClientAreaBorderType::TopLeft:
		dir = HTTOPLEFT;
		break;
	case NonClientAreaBorderType::Top:
		dir = HTTOP;
		break;
	case NonClientAreaBorderType::TopRight:
		dir = HTTOPRIGHT;
		break;
	case NonClientAreaBorderType::Right:
		dir = HTRIGHT;
		break;
	case NonClientAreaBorderType::BottomRight:
		dir = HTBOTTOMRIGHT;
		break;
	case NonClientAreaBorderType::Bottom:
		dir = HTBOTTOM;
		break;
	case NonClientAreaBorderType::BottomLeft:
		dir = HTBOTTOMLEFT;
		break;
	}

	return dir;
}

/**	Method triggered whenever a mouse event happens. */
void getMouseData(HWND hWnd, WPARAM wParam, LPARAM lParam, bool nonClient, Vector2I& mousePos, OSPointerButtonStates& btnStates)
{
	POINT clientPoint;

	clientPoint.x = GET_X_LPARAM(lParam);
	clientPoint.y = GET_Y_LPARAM(lParam);

	if(!nonClient)
		ClientToScreen(hWnd, &clientPoint);

	mousePos.X = clientPoint.x;
	mousePos.Y = clientPoint.y;

	btnStates.MouseButtons[0] = (wParam & MK_LBUTTON) != 0;
	btnStates.MouseButtons[1] = (wParam & MK_MBUTTON) != 0;
	btnStates.MouseButtons[2] = (wParam & MK_RBUTTON) != 0;
	btnStates.Shift = (wParam & MK_SHIFT) != 0;
	btnStates.Ctrl = (wParam & MK_CONTROL) != 0;
}

/**
 * Converts a virtual key code into an input command, if possible. Returns true if conversion was done.
 *
 * @param[in]	virtualKeyCode	Virtual key code to try to translate to a command.
 * @param[out]	command			Input command. Only valid if function returns true.
 */
bool getCommand(unsigned int virtualKeyCode, InputCommandType& command)
{
	bool isShiftPressed = GetAsyncKeyState(VK_SHIFT);

	switch(virtualKeyCode)
	{
	case VK_LEFT:
		command = isShiftPressed ? InputCommandType::SelectLeft : InputCommandType::CursorMoveLeft;
		return true;
	case VK_RIGHT:
		command = isShiftPressed ? InputCommandType::SelectRight : InputCommandType::CursorMoveRight;
		return true;
	case VK_UP:
		command = isShiftPressed ? InputCommandType::SelectUp : InputCommandType::CursorMoveUp;
		return true;
	case VK_DOWN:
		command = isShiftPressed ? InputCommandType::SelectDown : InputCommandType::CursorMoveDown;
		return true;
	case VK_ESCAPE:
		command = InputCommandType::Escape;
		return true;
	case VK_RETURN:
		command = isShiftPressed ? InputCommandType::Return : InputCommandType::Confirm;
		return true;
	case VK_BACK:
		command = InputCommandType::Backspace;
		return true;
	case VK_DELETE:
		command = InputCommandType::Delete;
		return true;
	case VK_TAB:
		command = InputCommandType::Tab;
		return true;
	}

	return false;
}

LRESULT CALLBACK Win32Platform::Win32WndProcInternal(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CREATE)
	{ // Store pointer to Win32Window in user data area
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));

		ct::RenderWindow* newWindow = (ct::RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if(newWindow != nullptr)
		{
			const RenderWindowProperties& props = newWindow->GetProperties();
			if(!props.IsHidden)
				ShowWindow(hWnd, SW_SHOWNORMAL);
		}
		else
			ShowWindow(hWnd, SW_SHOWNORMAL);

		return 0;
	}

	ct::RenderWindow* win = (ct::RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(!win)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
	case WM_ACTIVATE:
		{
			switch(wParam)
			{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				{
					Lock lock(mData->MSync);

					mData->MIsActive = true;
				}

				applyClipping(mData);
				break;
			case WA_INACTIVE:
				{
					Lock lock(mData->MSync);

					mData->MIsActive = false;
				}

				ClipCursor(nullptr);
				break;
			}

			return 0;
		}
	case WM_SETFOCUS:
		{
			if(!win->GetProperties().HasFocus)
				win->NotifyWindowEventInternal(WindowEventType::FocusReceived);

			return 0;
		}
	case WM_KILLFOCUS:
		{
			if(win->GetProperties().HasFocus)
				win->NotifyWindowEventInternal(WindowEventType::FocusLost);

			return 0;
		}
	case WM_SYSCHAR:
		if(wParam != VK_SPACE)
			return 0;
		break;
	case WM_MOVE:
		win->NotifyWindowEventInternal(WindowEventType::Moved);
		return 0;
	case WM_DISPLAYCHANGE:
		win->NotifyWindowEventInternal(WindowEventType::Resized);
		break;
	case WM_SIZE:
		win->NotifyWindowEventInternal(WindowEventType::Resized);

		if(wParam == SIZE_MAXIMIZED)
			win->NotifyWindowEventInternal(WindowEventType::Maximized);
		else if(wParam == SIZE_MINIMIZED)
			win->NotifyWindowEventInternal(WindowEventType::Minimized);
		else if(wParam == SIZE_RESTORED)
			win->NotifyWindowEventInternal(WindowEventType::Restored);

		return 0;
	case WM_SETCURSOR:
		if(IsCursorHidden())
			::SetCursor(nullptr);
		else
		{
			switch(LOWORD(lParam))
			{
			case HTTOPLEFT:
				::SetCursor(LoadCursor(0, IDC_SIZENWSE));
				return 0;
			case HTTOP:
				::SetCursor(LoadCursor(0, IDC_SIZENS));
				return 0;
			case HTTOPRIGHT:
				::SetCursor(LoadCursor(0, IDC_SIZENESW));
				return 0;
			case HTLEFT:
				::SetCursor(LoadCursor(0, IDC_SIZEWE));
				return 0;
			case HTRIGHT:
				::SetCursor(LoadCursor(0, IDC_SIZEWE));
				return 0;
			case HTBOTTOMLEFT:
				::SetCursor(LoadCursor(0, IDC_SIZENESW));
				return 0;
			case HTBOTTOM:
				::SetCursor(LoadCursor(0, IDC_SIZENS));
				return 0;
			case HTBOTTOMRIGHT:
				::SetCursor(LoadCursor(0, IDC_SIZENWSE));
				return 0;
			}

			::SetCursor(mData->MCursor.Cursor);
		}
		return true;
	case WM_GETMINMAXINFO:
		{
			// Prevent the window from going smaller than some minimu size
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;

			// Ensure maximizes window has proper size and doesn't cover the entire screen
			HMONITOR primaryMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(primaryMonitor, &monitorInfo);

			((MINMAXINFO*)lParam)->ptMaxPosition.x = monitorInfo.rcWork.left - monitorInfo.rcMonitor.left;
			((MINMAXINFO*)lParam)->ptMaxPosition.y = monitorInfo.rcWork.top - monitorInfo.rcMonitor.top;
			((MINMAXINFO*)lParam)->ptMaxSize.x = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
			((MINMAXINFO*)lParam)->ptMaxSize.y = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
		}
		break;
	case WM_CLOSE:
		{
			win->NotifyWindowEventInternal(WindowEventType::CloseRequested);

			return 0;
		}
	case WM_NCHITTEST:
		{
			auto iterFind = mData->MNonClientAreas.find(win);
			if(iterFind == mData->MNonClientAreas.end())
				break;

			POINT mousePos;
			mousePos.x = GET_X_LPARAM(lParam);
			mousePos.y = GET_Y_LPARAM(lParam);

			ScreenToClient(hWnd, &mousePos);

			Vector2I mousePosInt;
			mousePosInt.X = mousePos.x;
			mousePosInt.Y = mousePos.y;

			Vector<NonClientResizeArea>& resizeAreasPerWindow = iterFind->second.ResizeAreas;
			for(auto area : resizeAreasPerWindow)
			{
				if(area.Area.Contains(mousePosInt))
					return translateNonClientAreaType(area.Type);
			}

			Vector<Rect2I>& moveAreasPerWindow = iterFind->second.MoveAreas;
			for(auto area : moveAreasPerWindow)
			{
				if(area.Contains(mousePosInt))
					return HTCAPTION;
			}

			return HTCLIENT;
		}
	case WM_NCLBUTTONDBLCLK:
		// Maximize/Restore on double-click
		if(wParam == HTCAPTION)
		{
			WINDOWPLACEMENT windowPlacement;
			windowPlacement.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &windowPlacement);

			if(windowPlacement.showCmd == SW_MAXIMIZE)
				ShowWindow(hWnd, SW_RESTORE);
			else
				ShowWindow(hWnd, SW_MAXIMIZE);

			return 0;
		}
		break;
	case WM_MOUSELEAVE:
		{
			// Note: Right now I track only mouse leaving client area. So it's possible for the "mouse left window" callback
			// to trigger, while the mouse is still in the non-client area of the window.
			mData->MIsTrackingMouse = false; // TrackMouseEvent ends when this message is received and needs to be re-applied

			Lock lock(mData->MSync);
			win->NotifyWindowEventInternal(WindowEventType::MouseLeft);
		}
		return 0;
	case WM_LBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Left, btnStates);

			return 0;
		}
	case WM_MBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Middle, btnStates);

			return 0;
		}
	case WM_RBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Right, btnStates);

			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Left, btnStates);
		}
		return 0;
	case WM_MBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Middle, btnStates);
		}
		return 0;
	case WM_RBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Right, btnStates);
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		{
			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorDoubleClick.Empty())
				onCursorDoubleClick(intMousePos, btnStates);
		}
		return 0;
	case WM_NCMOUSEMOVE:
	case WM_MOUSEMOVE:
		{
			// Set up tracking so we get notified when mouse leaves the window
			if(!mData->MIsTrackingMouse)
			{
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;

				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);

				mData->MIsTrackingMouse = true;
			}

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			getMouseData(hWnd, wParam, lParam, uMsg == WM_NCMOUSEMOVE, intMousePos, btnStates);

			if(!onCursorMoved.Empty())
				onCursorMoved(intMousePos, btnStates);

			return 0;
		}
	case WM_MOUSEWHEEL:
		{
			i16 wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			float wheelDeltaFlt = wheelDelta / (float)WHEEL_DELTA;
			if(!onMouseWheelScrolled.Empty())
				onMouseWheelScrolled(wheelDeltaFlt);

			return true;
		}
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			InputCommandType command = InputCommandType::Backspace;
			if(getCommand((unsigned int)wParam, command))
			{
				if(!onInputCommand.Empty())
					onInputCommand(command);

				return 0;
			}

			break;
		}
	case WM_SYSKEYUP:
	case WM_KEYUP:
		return 0;
	case WM_CHAR:
		{
			// TODO - Not handling IME input

			// Ignore rarely used special command characters, usually triggered by ctrl+key
			// combinations. (We want to keep ctrl+key free for shortcuts instead)
			if(wParam <= 23)
				break;

			// Ignore shortcut key combinations
			if(GetAsyncKeyState(VK_CONTROL) != 0 || GetAsyncKeyState(VK_MENU) != 0)
				break;

			switch(wParam)
			{
			case VK_ESCAPE:
				break;
			default: // displayable character
				{
					u32 finalChar = (u32)wParam;

					if(!onCharInput.Empty())
						onCharInput(finalChar);

					return 0;
				}
			}

			break;
		}
	case WM_BS_SETCAPTURE:
		SetCapture(hWnd);
		break;
	case WM_BS_RELEASECAPTURE:
		ReleaseCapture();
		break;
	case WM_CAPTURECHANGED:
		if(!onMouseCaptureChanged.Empty())
			onMouseCaptureChanged();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
