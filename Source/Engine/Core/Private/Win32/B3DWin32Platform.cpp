//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/B3DWin32Platform.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "Image/B3DPixelUtility.h"
#include "B3DApplication.h"
#include "Debug/B3DDebug.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Platform/B3DDropTarget.h"
#include "Private/Win32/B3DWin32DropTarget.h"
#include "Private/Win32/B3DWin32PlatformUtility.h"
#include "String/B3DUnicode.h"
#include "TimeAPI.h"
#include <shellapi.h>
#include <WinUser.h>

using namespace b3d;

/** Encapsulate native cursor data so we can avoid including windows.h as it pollutes the global namespace. */
struct B3D_EXPORT NativeCursorData
{
	HCURSOR Cursor;
};

/**	Encapsulate drop target data so we can avoid including windows.h as it pollutes the global namespace. */
struct B3D_EXPORT NativeDropTargetData
{
	Map<const RenderWindow*, Win32DropTarget*> DropTargetsPerWindow;
	Vector<Win32DropTarget*> DropTargetsToInitialize;
	Vector<Win32DropTarget*> DropTargetsToDestroy;
};

struct Platform::Private
{
	bool IsCursorHidden = false;
	NativeCursorData Cursor;
	bool IsUsingCustomCursor = false;
	Map<const RenderWindow*, WindowNonClientAreaData> NonClientAreas;

	bool IsTrackingMouse = false;
	NativeDropTargetData DropTargets;

	bool RequiresStartUp = false;
	bool RequiresShutDown = false;

	bool CursorClipping = false;
	HWND ClipWindow = 0;
	RECT ClipRect;

	bool IsActive = false;

	Mutex Sync;
};

Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorMoved;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonPressed;
Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonReleased;
Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorDoubleClick;
Event<void(InputCommandType)> Platform::onInputCommand;
Event<void(float)> Platform::onMouseWheelScrolled;
Event<void(u32)> Platform::onCharInput;

Event<void()> Platform::onMouseCaptureChanged;

Platform::Private* Platform::mData = B3DNew<Platform::Private>();

/** Checks if any of the windows of the current application are active. */
bool IsAppActive(Platform::Private* data)
{
	Lock lock(data->Sync);

	return data->IsActive;
}

/** Enables or disables cursor clipping depending on the stored data. */
void ApplyClipping(Platform::Private* data)
{
	if(data->CursorClipping)
	{
		if(data->ClipWindow)
		{
			// Clip cursor to the window
			RECT clipWindowRect;
			if(GetWindowRect(data->ClipWindow, &clipWindowRect))
				ClipCursor(&clipWindowRect);
		}
		else
			ClipCursor(&data->ClipRect);
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
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	PostMessage((HWND)hwnd, WM_BS_SETCAPTURE, WPARAM((HWND)hwnd), 0);
}

void Platform::ReleaseMouseCapture()
{
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	PostMessage((HWND)hwnd, WM_BS_RELEASECAPTURE, WPARAM((HWND)hwnd), 0);
}

bool Platform::IsPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
{
	const u64 hwndToCheck = window.GetPlatformWindowHandle();
	if(hwndToCheck == 0)
		return false; // No window in headless mode

	POINT point;
	point.x = screenPos.X;
	point.y = screenPos.Y;

	HWND hwndUnderPos = WindowFromPoint(point);
	return hwndUnderPos == (HWND)hwndToCheck;
}

void Platform::HideCursor()
{
	if(mData->IsCursorHidden)
		return;

	mData->IsCursorHidden = true;

	// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
	// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

void Platform::ShowCursor()
{
	if(!mData->IsCursorHidden)
		return;

	mData->IsCursorHidden = false;

	// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
	// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

bool Platform::IsCursorHidden()
{
	return mData->IsCursorHidden;
}

void Platform::ClipCursorToWindow(const RenderWindow& window)
{
	const u64 hwnd = window.GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	mData->CursorClipping = true;
	mData->ClipWindow = (HWND)hwnd;

	if(IsAppActive(mData))
		ApplyClipping(mData);
}

void Platform::ClipCursorToRect(const Area2I& screenRect)
{
	mData->CursorClipping = true;
	mData->ClipWindow = 0;

	mData->ClipRect.left = screenRect.X;
	mData->ClipRect.top = screenRect.Y;
	mData->ClipRect.right = screenRect.X + screenRect.Width;
	mData->ClipRect.bottom = screenRect.Y + screenRect.Height;

	if(IsAppActive(mData))
		ApplyClipping(mData);
}

void Platform::ClipCursorDisable()
{
	mData->CursorClipping = false;
	mData->ClipWindow = 0;

	if(IsAppActive(mData))
		ApplyClipping(mData);
}

// TODO - Add support for animated custom cursor
void Platform::SetCursor(PixelData& pixelData, const Vector2I& hotSpot)
{
	if(mData->IsUsingCustomCursor)
	{
		::SetCursor(0);
		DestroyIcon(mData->Cursor.Cursor);
	}

	mData->IsUsingCustomCursor = true;

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

	mData->Cursor.Cursor = CreateIconIndirect(&iconinfo);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	// Make sure we notify the message loop to perform the actual cursor update
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

	PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
}

void Platform::SetIcon(const PixelData& pixelData)
{
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	const u64 hwnd = primaryWindow->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No-op in headless mode

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
	PostMessage((HWND)hwnd, WM_SETICON, WPARAM(ICON_BIG), (LPARAM)icon);
}

void Platform::SetCaptionNonClientAreas(const RenderWindow& window, const Vector<Area2I>& nonClientAreas)
{
	Lock lock(mData->Sync);

	mData->NonClientAreas[&window].MoveAreas = nonClientAreas;
}

void Platform::SetResizeNonClientAreas(const RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
{
	Lock lock(mData->Sync);

	mData->NonClientAreas[&window].ResizeAreas = nonClientAreas;
}

void Platform::ResetNonClientAreas(const RenderWindow& window)
{
	Lock lock(mData->Sync);

	auto iterFind = mData->NonClientAreas.find(&window);

	if(iterFind != end(mData->NonClientAreas))
		mData->NonClientAreas.erase(iterFind);
}

void Platform::Sleep(u32 duration)
{
	::Sleep((DWORD)duration);
}

void Win32Platform::RegisterDropTarget(DropTarget* target)
{
	const RenderWindow* window = target->GetOwnerWindowInternal();
	const u64 hwnd = window->GetPlatformWindowHandle();
	if(hwnd == 0)
		return; // No drag-drop in headless mode

	Win32DropTarget* win32DropTarget = nullptr;
	auto iterFind = mData->DropTargets.DropTargetsPerWindow.find(window);
	if(iterFind == mData->DropTargets.DropTargetsPerWindow.end())
	{
		win32DropTarget = B3DNew<Win32DropTarget>((HWND)hwnd);
		mData->DropTargets.DropTargetsPerWindow[window] = win32DropTarget;

		{
			Lock lock(mData->Sync);
			mData->DropTargets.DropTargetsToInitialize.push_back(win32DropTarget);
		}
	}
	else
		win32DropTarget = iterFind->second;

	win32DropTarget->RegisterDropTarget(target);
}

void Win32Platform::UnregisterDropTarget(DropTarget* target)
{
	auto iterFind = mData->DropTargets.DropTargetsPerWindow.find(target->GetOwnerWindowInternal());
	if(iterFind == mData->DropTargets.DropTargetsPerWindow.end())
	{
		B3D_LOG(Warning, LogPlatform, "Attempting to destroy a drop target but cannot find its parent window.");
	}
	else
	{
		Win32DropTarget* win32DropTarget = iterFind->second;
		win32DropTarget->UnregisterDropTarget(target);

		if(win32DropTarget->GetNumDropTargets() == 0)
		{
			mData->DropTargets.DropTargetsPerWindow.erase(iterFind);

			{
				Lock lock(mData->Sync);
				mData->DropTargets.DropTargetsToDestroy.push_back(win32DropTarget);
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

	return StringUtility::kBlank;
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
	Lock lock(mData->Sync);

	if(timeBeginPeriod(1) == TIMERR_NOCANDO)
	{
		B3D_LOG(Warning, LogPlatform, "Unable to set timer resolution to 1ms. This can cause significant waste "
								  "in performance for waiting threads.");
	}

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	mData->RequiresStartUp = true;
}

void Platform::UpdateInternal()
{
	for(auto& dropTarget : mData->DropTargets.DropTargetsPerWindow)
	{
		dropTarget.second->Update();
	}

	{
		Lock lock(mData->Sync);
		if(mData->RequiresStartUp)
		{
			OleInitialize(nullptr);

			mData->RequiresStartUp = false;
		}
	}

	{
		Lock lock(mData->Sync);
		for(auto& dropTargetToDestroy : mData->DropTargets.DropTargetsToDestroy)
		{
			dropTargetToDestroy->UnregisterWithOs();
			dropTargetToDestroy->Release();
		}

		mData->DropTargets.DropTargetsToDestroy.clear();
	}

	{
		Lock lock(mData->Sync);
		for(auto& dropTargetToInit : mData->DropTargets.DropTargetsToInitialize)
		{
			dropTargetToInit->RegisterWithOs();
		}

		mData->DropTargets.DropTargetsToInitialize.clear();
	}

	MessagePumpInternal();

	{
		Lock lock(mData->Sync);
		if(mData->RequiresShutDown)
		{
			OleUninitialize();
			mData->RequiresShutDown = false;
		}
	}
}

void Platform::ShutDownInternal()
{
	Lock lock(mData->Sync);

	timeEndPeriod(1);
	mData->RequiresShutDown = true;
}

/**	Translate engine non client area to win32 non client area. */
LRESULT TranslateNonClientAreaType(NonClientAreaBorderType type)
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
void GetMouseData(HWND hWnd, WPARAM wParam, LPARAM lParam, bool nonClient, Vector2I& mousePos, OSPointerButtonStates& btnStates)
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
bool GetCommand(unsigned int virtualKeyCode, InputCommandType& command)
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
		return 0;
	}

	RenderWindow* win = (RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
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
					Lock lock(mData->Sync);

					mData->IsActive = true;
				}

				ApplyClipping(mData);
				break;
			case WA_INACTIVE:
				{
					Lock lock(mData->Sync);

					mData->IsActive = false;
				}

				ClipCursor(nullptr);
				break;
			}

			return 0;
		}
	case WM_SETFOCUS:
		{
			if(!win->GetRenderWindowProperties().HasFocus)
				win->NotifyWindowEvent(WindowEventType::FocusReceived);

			return 0;
		}
	case WM_KILLFOCUS:
		{
			if(win->GetRenderWindowProperties().HasFocus)
				win->NotifyWindowEvent(WindowEventType::FocusLost);

			return 0;
		}
	case WM_SYSCHAR:
		if(wParam != VK_SPACE)
			return 0;
		break;
	case WM_MOVE:
		win->NotifyWindowEvent(WindowEventType::Moved);
		return 0;
	case WM_DISPLAYCHANGE:
		win->NotifyWindowEvent(WindowEventType::Resized);
		break;
	case WM_SIZE:
		win->NotifyWindowEvent(WindowEventType::Resized);

		if(wParam == SIZE_MAXIMIZED)
			win->NotifyWindowEvent(WindowEventType::Maximized);
		else if(wParam == SIZE_MINIMIZED)
			win->NotifyWindowEvent(WindowEventType::Minimized);
		else if(wParam == SIZE_RESTORED)
			win->NotifyWindowEvent(WindowEventType::Restored);

		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		win->NotifyWindowEvent(WindowEventType::Redraw);
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

			::SetCursor(mData->Cursor.Cursor);
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
	case WM_DPICHANGED:
		{
			win->NotifyWindowEvent(WindowEventType::DPIScaleChanged);
			break;
		}
	case WM_CLOSE:
		{
			win->NotifyWindowEvent(WindowEventType::CloseRequested);

			return 0;
		}
	case WM_NCHITTEST:
		{
			auto iterFind = mData->NonClientAreas.find(win);
			if(iterFind == mData->NonClientAreas.end())
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
					return TranslateNonClientAreaType(area.Type);
			}

			Vector<Area2I>& moveAreasPerWindow = iterFind->second.MoveAreas;
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
			mData->IsTrackingMouse = false; // TrackMouseEvent ends when this message is received and needs to be re-applied

			Lock lock(mData->Sync);
			win->NotifyWindowEvent(WindowEventType::MouseLeft);
		}
		return 0;
	case WM_LBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Left, btnStates);

			return 0;
		}
	case WM_MBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Middle, btnStates);

			return 0;
		}
	case WM_RBUTTONUP:
		{
			ReleaseCapture();

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonReleased.Empty())
				onCursorButtonReleased(intMousePos, OSMouseButton::Right, btnStates);

			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Left, btnStates);
		}
		return 0;
	case WM_MBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Middle, btnStates);
		}
		return 0;
	case WM_RBUTTONDOWN:
		{
			SetCapture(hWnd);

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorButtonPressed.Empty())
				onCursorButtonPressed(intMousePos, OSMouseButton::Right, btnStates);
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		{
			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

			if(!onCursorDoubleClick.Empty())
				onCursorDoubleClick(intMousePos, btnStates);
		}
		return 0;
	case WM_NCMOUSEMOVE:
	case WM_MOUSEMOVE:
		{
			// Set up tracking so we get notified when mouse leaves the window
			if(!mData->IsTrackingMouse)
			{
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;

				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);

				mData->IsTrackingMouse = true;
			}

			Vector2I intMousePos;
			OSPointerButtonStates btnStates;

			GetMouseData(hWnd, wParam, lParam, uMsg == WM_NCMOUSEMOVE, intMousePos, btnStates);

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
			if(GetCommand((unsigned int)wParam, command))
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
