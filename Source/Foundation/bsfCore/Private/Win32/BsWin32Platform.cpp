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

namespace bs
{
	/** Encapsulate native cursor data so we can avoid including windows.h as it pollutes the global namespace. */
	struct BS_CORE_EXPORT NativeCursorData
	{
		HCURSOR cursor;
	};

	/**	Encapsulate drop target data so we can avoid including windows.h as it pollutes the global namespace. */
	struct BS_CORE_EXPORT NativeDropTargetData
	{
		Map<const RenderWindow*, Win32DropTarget*> dropTargetsPerWindow;
		Vector<Win32DropTarget*> dropTargetsToInitialize;
		Vector<Win32DropTarget*> dropTargetsToDestroy;
	};

	struct Platform::Pimpl
	{
		bool mIsCursorHidden = false;
		NativeCursorData mCursor;
		bool mUsingCustomCursor = false;
		Map<const ct::RenderWindow*, WindowNonClientAreaData> mNonClientAreas;

		bool mIsTrackingMouse = false;
		NativeDropTargetData mDropTargets;

		bool mRequiresStartUp = false;
		bool mRequiresShutDown = false;

		bool mCursorClipping = false;
		HWND mClipWindow = 0;
		RECT mClipRect;

		bool mIsActive = false;

		Mutex mSync;
	};

	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorMoved;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonPressed;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::onCursorButtonReleased;
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::onCursorDoubleClick;
	Event<void(InputCommandType)> Platform::onInputCommand;
	Event<void(float)> Platform::onMouseWheelScrolled;
	Event<void(UINT32)> Platform::onCharInput;

	Event<void()> Platform::onMouseCaptureChanged;

	Platform::Pimpl* Platform::mData = bs_new<Platform::Pimpl>();

	/** Checks if any of the windows of the current application are active. */
	bool isAppActive(Platform::Pimpl* data)
	{
		Lock lock(data->mSync);

		return data->mIsActive;
	}

	/** Enables or disables cursor clipping depending on the stored data. */
	void applyClipping(Platform::Pimpl* data)
	{
		if(data->mCursorClipping)
		{
			if(data->mClipWindow)
			{
				// Clip cursor to the window
				RECT clipWindowRect;
				if (GetWindowRect(data->mClipWindow, &clipWindowRect))
					ClipCursor(&clipWindowRect);
			}
			else
				ClipCursor(&data->mClipRect);
		}
		else
			ClipCursor(nullptr);
	}

	Platform::~Platform()
	{
		bs_delete(mData);
		mData = nullptr;
	}

	Vector2I Platform::getCursorPosition()
	{
		Vector2I screenPos;

		POINT cursorPos;
		GetCursorPos(&cursorPos);

		screenPos.x = cursorPos.x;
		screenPos.y = cursorPos.y;

		return screenPos;
	}

	void Platform::setCursorPosition(const Vector2I& screenPos)
	{
		SetCursorPos(screenPos.x, screenPos.y);
	}

	void Platform::captureMouse(const RenderWindow& window)
	{
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);
		
		PostMessage((HWND)hwnd, WM_BS_SETCAPTURE, WPARAM((HWND)hwnd), 0);
	}

	void Platform::releaseMouseCapture()
	{
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);

		PostMessage((HWND)hwnd, WM_BS_RELEASECAPTURE, WPARAM((HWND)hwnd), 0);
	}

	bool Platform::isPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
	{
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();

		POINT point;
		point.x = screenPos.x;
		point.y = screenPos.y;

		UINT64 hwndToCheck;
		window.getCustomAttribute("WINDOW", &hwndToCheck);

		HWND hwndUnderPos = WindowFromPoint(point);
		return hwndUnderPos == (HWND)hwndToCheck;
	}

	void Platform::hideCursor()
	{
		if (mData->mIsCursorHidden)
			return;

		mData->mIsCursorHidden = true;

		// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
		// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);

		PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	}

	void Platform::showCursor()
	{
		if (!mData->mIsCursorHidden)
			return;

		mData->mIsCursorHidden = false;

		// ShowCursor(FALSE) doesn't work. Presumably because we're in the wrong thread, and using
		// WM_SETCURSOR in message loop to hide the cursor is smarter solution anyway.

		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);

		PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	}

	bool Platform::isCursorHidden()
	{
		return mData->mIsCursorHidden;
	}

	void Platform::clipCursorToWindow(const RenderWindow& window)
	{
		UINT64 hwnd;
		window.getCustomAttribute("WINDOW", &hwnd);

		mData->mCursorClipping = true;
		mData->mClipWindow = (HWND)hwnd;

		if(isAppActive(mData))
			applyClipping(mData);
	}

	void Platform::clipCursorToRect(const Rect2I& screenRect)
	{
		mData->mCursorClipping = true;
		mData->mClipWindow = 0;

		mData->mClipRect.left = screenRect.x;
		mData->mClipRect.top = screenRect.y;
		mData->mClipRect.right = screenRect.x + screenRect.width;
		mData->mClipRect.bottom = screenRect.y + screenRect.height;

		if(isAppActive(mData))
			applyClipping(mData);
	}

	void Platform::clipCursorDisable()
	{
		mData->mCursorClipping = false;
		mData->mClipWindow = 0;

		if(isAppActive(mData))
			applyClipping(mData);
	}

	// TODO - Add support for animated custom cursor
	void Platform::setCursor(PixelData& pixelData, const Vector2I& hotSpot)
	{
		if (mData->mUsingCustomCursor)
		{
			SetCursor(0);
			DestroyIcon(mData->mCursor.cursor);
		}

		mData->mUsingCustomCursor = true;

		Vector<Color> pixels = pixelData.getColors();
		UINT32 width = pixelData.getWidth();
		UINT32 height = pixelData.getHeight();

		HBITMAP hBitmap = Win32PlatformUtility::createBitmap((Color*)pixels.data(), width, height, false);
		HBITMAP hMonoBitmap = CreateBitmap(width, height, 1, 1, nullptr);

		ICONINFO iconinfo = {0};
		iconinfo.fIcon = FALSE;
		iconinfo.xHotspot = (DWORD)hotSpot.x;
		iconinfo.yHotspot = (DWORD)hotSpot.y;
		iconinfo.hbmMask = hMonoBitmap;
		iconinfo.hbmColor = hBitmap;

		mData->mCursor.cursor = CreateIconIndirect(&iconinfo);
		
		DeleteObject(hBitmap);
		DeleteObject(hMonoBitmap);

		// Make sure we notify the message loop to perform the actual cursor update
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);

		PostMessage((HWND)hwnd, WM_SETCURSOR, WPARAM((HWND)hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	}

	void Platform::setIcon(const PixelData& pixelData)
	{
		Vector<Color> pixels = pixelData.getColors();
		UINT32 width = pixelData.getWidth();
		UINT32 height = pixelData.getHeight();

		HBITMAP hBitmap = Win32PlatformUtility::createBitmap((Color*)pixels.data(), width, height, false);
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
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		UINT64 hwnd;
		primaryWindow->getCustomAttribute("WINDOW", &hwnd);
		
		PostMessage((HWND)hwnd, WM_SETICON, WPARAM(ICON_BIG), (LPARAM)icon);
	}

	void Platform::setCaptionNonClientAreas(const ct::RenderWindow& window, const Vector<Rect2I>& nonClientAreas)
	{
		Lock lock(mData->mSync);

		mData->mNonClientAreas[&window].moveAreas = nonClientAreas;
	}

	void Platform::setResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
	{
		Lock lock(mData->mSync);

		mData->mNonClientAreas[&window].resizeAreas = nonClientAreas;
	}

	void Platform::resetNonClientAreas(const ct::RenderWindow& window)
	{
		Lock lock(mData->mSync);

		auto iterFind = mData->mNonClientAreas.find(&window);

		if (iterFind != end(mData->mNonClientAreas))
			mData->mNonClientAreas.erase(iterFind);
	}

	void Platform::sleep(UINT32 duration)
	{
		Sleep((DWORD)duration);
	}

	void Win32Platform::registerDropTarget(DropTarget* target)
	{
		const RenderWindow* window = target->_getOwnerWindow();

		Win32DropTarget* win32DropTarget = nullptr;
		auto iterFind = mData->mDropTargets.dropTargetsPerWindow.find(window);
		if (iterFind == mData->mDropTargets.dropTargetsPerWindow.end())
		{
			UINT64 hwnd;
			window->getCustomAttribute("WINDOW", &hwnd);

			win32DropTarget = bs_new<Win32DropTarget>((HWND)hwnd);
			mData->mDropTargets.dropTargetsPerWindow[window] = win32DropTarget;

			{
				Lock lock(mData->mSync);
				mData->mDropTargets.dropTargetsToInitialize.push_back(win32DropTarget);
			}
		}
		else
			win32DropTarget = iterFind->second;

		win32DropTarget->registerDropTarget(target);
	}

	void Win32Platform::unregisterDropTarget(DropTarget* target)
	{
		auto iterFind = mData->mDropTargets.dropTargetsPerWindow.find(target->_getOwnerWindow());
		if (iterFind == mData->mDropTargets.dropTargetsPerWindow.end())
		{
			BS_LOG(Warning, Platform, "Attempting to destroy a drop target but cannot find its parent window.");
		}
		else
		{
			Win32DropTarget* win32DropTarget = iterFind->second;
			win32DropTarget->unregisterDropTarget(target);

			if(win32DropTarget->getNumDropTargets() == 0)
			{
				mData->mDropTargets.dropTargetsPerWindow.erase(iterFind);

				{
					Lock lock(mData->mSync);
					mData->mDropTargets.dropTargetsToDestroy.push_back(win32DropTarget);
				}
			}
		}
	}

	void Platform::copyToClipboard(const String& string)
	{
		WString wideString = UTF8::toWide(string);

		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (wideString.size() + 1) * sizeof(WString::value_type));
		WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);

		wideString.copy(buffer, wideString.size());
		buffer[wideString.size()] = '\0';

		GlobalUnlock(hData);

		if (OpenClipboard(NULL))
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

	String Platform::copyFromClipboard()
	{
		if (OpenClipboard(NULL))
		{
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);

			if (hData != NULL)
			{
				WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);
				WString wideString(buffer);
				GlobalUnlock(hData);

				CloseClipboard();
				return UTF8::fromWide(wideString);
			}
			else
			{
				CloseClipboard();
				return u8"";
			}
		}

		return u8"";
	}

	String Platform::keyCodeToUnicode(UINT32 keyCode)
	{
		static HKL keyboardLayout = GetKeyboardLayout(0);
		static UINT8 keyboarState[256];

		if (GetKeyboardState(keyboarState) == FALSE)
			return 0;

		UINT virtualKey = MapVirtualKeyExW(keyCode, 1, keyboardLayout);

		wchar_t output[2];
		int count = ToUnicodeEx(virtualKey, keyCode, keyboarState, output, 2, 0, keyboardLayout);
		if (count > 0)
			return UTF8::fromWide(WString(output, count));

		return StringUtil::BLANK;
	}

	void Platform::openFolder(const Path& path)
	{
		WString pathString = UTF8::toWide(path.toString());

		ShellExecuteW(nullptr, L"open", pathString.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	void Platform::_messagePump()
	{
		MSG  msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Platform::_startUp()
	{
		Lock lock(mData->mSync);

		if (timeBeginPeriod(1) == TIMERR_NOCANDO)
		{
			BS_LOG(Warning, Platform, "Unable to set timer resolution to 1ms. This can cause significant waste "
				"in performance for waiting threads.");
		}

		mData->mRequiresStartUp = true;
	}

	void Platform::_update()
	{
		for (auto& dropTarget : mData->mDropTargets.dropTargetsPerWindow)
		{
			dropTarget.second->update();
		}
	}

	void Platform::_coreUpdate()
	{
		{
			Lock lock(mData->mSync);
			if (mData->mRequiresStartUp)
			{
				OleInitialize(nullptr);

				mData->mRequiresStartUp = false;
			}
		}

		{
			Lock lock(mData->mSync);
			for (auto& dropTargetToDestroy : mData->mDropTargets.dropTargetsToDestroy)
			{
				dropTargetToDestroy->unregisterWithOS();
				dropTargetToDestroy->Release();
			}

			mData->mDropTargets.dropTargetsToDestroy.clear();
		}

		{
			Lock lock(mData->mSync);
			for (auto& dropTargetToInit : mData->mDropTargets.dropTargetsToInitialize)
			{
				dropTargetToInit->registerWithOS();
			}

			mData->mDropTargets.dropTargetsToInitialize.clear();
		}

		_messagePump();

		{
			Lock lock(mData->mSync);
			if (mData->mRequiresShutDown)
			{
				OleUninitialize();
				mData->mRequiresShutDown = false;
			}
		}
	}

	void Platform::_shutDown()
	{
		Lock lock(mData->mSync);

		timeEndPeriod(1);
		mData->mRequiresShutDown = true;
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

		if (!nonClient)
			ClientToScreen(hWnd, &clientPoint);

		mousePos.x = clientPoint.x;
		mousePos.y = clientPoint.y;

		btnStates.mouseButtons[0] = (wParam & MK_LBUTTON) != 0;
		btnStates.mouseButtons[1] = (wParam & MK_MBUTTON) != 0;
		btnStates.mouseButtons[2] = (wParam & MK_RBUTTON) != 0;
		btnStates.shift = (wParam & MK_SHIFT) != 0;
		btnStates.ctrl = (wParam & MK_CONTROL) != 0;
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
		
		switch (virtualKeyCode)
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

	LRESULT CALLBACK Win32Platform::_win32WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CREATE)
		{	// Store pointer to Win32Window in user data area
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));

			ct::RenderWindow* newWindow = (ct::RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (newWindow != nullptr)
			{
				const RenderWindowProperties& props = newWindow->getProperties();
				if (!props.isHidden)
					ShowWindow(hWnd, SW_SHOWNORMAL);
			}
			else
				ShowWindow(hWnd, SW_SHOWNORMAL);

			return 0;
		}

		ct::RenderWindow* win = (ct::RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (!win)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);

		switch( uMsg )
		{
		case WM_ACTIVATE:
			{
			switch(wParam)
			{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				{
					Lock lock(mData->mSync);

					mData->mIsActive = true;
				}

				applyClipping(mData);
				break;
			case WA_INACTIVE:
				{
					Lock lock(mData->mSync);

					mData->mIsActive = false;
				}

				ClipCursor(nullptr);
				break;
			}

				return 0;
			}
		case WM_SETFOCUS:
			{
				if (!win->getProperties().hasFocus)
					win->_notifyWindowEvent(WindowEventType::FocusReceived);

				return 0;
			}
		case WM_KILLFOCUS:
			{
				if (win->getProperties().hasFocus)
					win->_notifyWindowEvent(WindowEventType::FocusLost);

				return 0;
			}
		case WM_SYSCHAR:
			if (wParam != VK_SPACE)
				return 0;
			break;
		case WM_MOVE:
			win->_notifyWindowEvent(WindowEventType::Moved);
			return 0;
		case WM_DISPLAYCHANGE:
			win->_notifyWindowEvent(WindowEventType::Resized);
			break;
		case WM_SIZE:
			win->_notifyWindowEvent(WindowEventType::Resized);

			if (wParam == SIZE_MAXIMIZED)
				win->_notifyWindowEvent(WindowEventType::Maximized);
			else if (wParam == SIZE_MINIMIZED)
				win->_notifyWindowEvent(WindowEventType::Minimized);
			else if (wParam == SIZE_RESTORED)
				win->_notifyWindowEvent(WindowEventType::Restored);

			return 0;
		case WM_SETCURSOR:
			if(isCursorHidden())
				SetCursor(nullptr);
			else
			{
				switch (LOWORD(lParam))
				{
				case HTTOPLEFT:
					SetCursor(LoadCursor(0, IDC_SIZENWSE));
					return 0;
				case HTTOP:
					SetCursor(LoadCursor(0, IDC_SIZENS));
					return 0;
				case HTTOPRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZENESW));
					return 0;
				case HTLEFT:
					SetCursor(LoadCursor(0, IDC_SIZEWE));
					return 0;
				case HTRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZEWE));
					return 0;
				case HTBOTTOMLEFT:
					SetCursor(LoadCursor(0, IDC_SIZENESW));
					return 0;
				case HTBOTTOM:
					SetCursor(LoadCursor(0, IDC_SIZENS));
					return 0;
				case HTBOTTOMRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZENWSE));
					return 0;
				}

				SetCursor(mData->mCursor.cursor);
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
			win->_notifyWindowEvent(WindowEventType::CloseRequested);

				return 0;
			}
		case WM_NCHITTEST:
			{
				auto iterFind = mData->mNonClientAreas.find(win);
				if (iterFind == mData->mNonClientAreas.end())
					break;

				POINT mousePos;
				mousePos.x = GET_X_LPARAM(lParam);
				mousePos.y = GET_Y_LPARAM(lParam);

				ScreenToClient(hWnd, &mousePos);

				Vector2I mousePosInt;
				mousePosInt.x = mousePos.x;
				mousePosInt.y = mousePos.y;

				Vector<NonClientResizeArea>& resizeAreasPerWindow = iterFind->second.resizeAreas;
				for(auto area : resizeAreasPerWindow)
				{
					if (area.area.contains(mousePosInt))
						return translateNonClientAreaType(area.type);
				}

				Vector<Rect2I>& moveAreasPerWindow = iterFind->second.moveAreas;
				for(auto area : moveAreasPerWindow)
				{
					if(area.contains(mousePosInt))
						return HTCAPTION;
				}

				return HTCLIENT;
			}
		case WM_NCLBUTTONDBLCLK:
			// Maximize/Restore on double-click
			if (wParam == HTCAPTION)
			{
				WINDOWPLACEMENT windowPlacement;
				windowPlacement.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(hWnd, &windowPlacement);

				if (windowPlacement.showCmd == SW_MAXIMIZE)
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
				mData->mIsTrackingMouse = false; // TrackMouseEvent ends when this message is received and needs to be re-applied

				Lock lock(mData->mSync);
				win->_notifyWindowEvent(WindowEventType::MouseLeft);
			}
			return 0;
		case WM_LBUTTONUP:
			{
				ReleaseCapture();

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Left, btnStates);

				return 0;
			}
		case WM_MBUTTONUP:
			{
				ReleaseCapture();

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Middle, btnStates);

				return 0;
			}
		case WM_RBUTTONUP:
			{
				ReleaseCapture();

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Right, btnStates);

				return 0;
			}
		case WM_LBUTTONDOWN:
			{
				SetCapture(hWnd);

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Left, btnStates);
			}
			return 0;
		case WM_MBUTTONDOWN:
			{
				SetCapture(hWnd);

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Middle, btnStates);
			}
			return 0;
		case WM_RBUTTONDOWN:
			{
				SetCapture(hWnd);

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Right, btnStates);
			}
			return 0;
		case WM_LBUTTONDBLCLK:
			{
				Vector2I intMousePos;
				OSPointerButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, false, intMousePos, btnStates);

				if(!onCursorDoubleClick.empty())
					onCursorDoubleClick(intMousePos, btnStates);
			}
			return 0;
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			{
				// Set up tracking so we get notified when mouse leaves the window
				if(!mData->mIsTrackingMouse)
				{
					TRACKMOUSEEVENT tme = { sizeof(tme) };
					tme.dwFlags = TME_LEAVE;

					tme.hwndTrack = hWnd;
					TrackMouseEvent(&tme);

					mData->mIsTrackingMouse = true;
				}

				Vector2I intMousePos;
				OSPointerButtonStates btnStates;
				
				getMouseData(hWnd, wParam, lParam, uMsg == WM_NCMOUSEMOVE, intMousePos, btnStates);

				if(!onCursorMoved.empty())
					onCursorMoved(intMousePos, btnStates);

				return 0;
			}
		case WM_MOUSEWHEEL:
			{
				INT16 wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				float wheelDeltaFlt = wheelDelta / (float)WHEEL_DELTA;
				if(!onMouseWheelScrolled.empty())
					onMouseWheelScrolled(wheelDeltaFlt);

				return true;
			}
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
				InputCommandType command = InputCommandType::Backspace;
				if(getCommand((unsigned int)wParam, command))
				{
					if(!onInputCommand.empty())
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
				if (wParam <= 23)
					break;

				// Ignore shortcut key combinations
				if(GetAsyncKeyState(VK_CONTROL) != 0 || GetAsyncKeyState(VK_MENU) != 0)
					break;

				switch (wParam)
				{
				case VK_ESCAPE:
					break;
				default:    // displayable character
					{
						UINT32 finalChar = (UINT32)wParam;

						if(!onCharInput.empty())
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
			if(!onMouseCaptureChanged.empty())
				onMouseCaptureChanged();
			return 0;
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}
