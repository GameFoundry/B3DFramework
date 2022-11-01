//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/BsWin32Window.h"
#include "Private/Win32/BsWin32PlatformUtility.h"

using namespace bs;

Vector<Win32Window*> Win32Window::sAllWindows;
Vector<Win32Window*> Win32Window::sModalWindowStack;
Mutex Win32Window::sWindowsMutex;

struct Win32Window::Pimpl
{
	HWND HWnd = nullptr;
	i32 Left = 0;
	i32 Top = 0;
	u32 Width = 0;
	u32 Height = 0;
	bool IsExternal = false;
	bool IsModal = false;
	bool IsHidden = false;
	DWORD Style = 0;
	DWORD StyleEx = 0;
};

Win32Window::Win32Window(const WINDOW_DESC& desc)
{
	m = B3DNew<Pimpl>();
	m->IsModal = desc.Modal;
	m->IsHidden = desc.Hidden;
	bool shouldFocus = true;

	HMONITOR hMonitor = desc.Monitor;
	if(!desc.External)
	{
		m->Style = WS_CLIPCHILDREN;

		i32 left = desc.Left;
		i32 top = desc.Top;

		// If we didn't specified the adapter index, or if we didn't find it
		if(hMonitor == nullptr)
		{
			POINT windowAnchorPoint;

			// Fill in anchor point.
			windowAnchorPoint.x = left;
			windowAnchorPoint.y = top;

			// Get the nearest monitor to this window.
			hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTOPRIMARY);
		}

		// Get the target monitor info
		MONITORINFO monitorInfo;
		memset(&monitorInfo, 0, sizeof(MONITORINFO));
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);

		u32 width = desc.Width;
		u32 height = desc.Height;

		// No specified top left -> Center the window in the middle of the monitor
		if(left == -1 || top == -1)
		{
			int screenw = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
			int screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

			// clamp window dimensions to screen size
			int outerw = (int(width) < screenw) ? int(width) : screenw;
			int outerh = (int(height) < screenh) ? int(height) : screenh;

			if(left == -1)
				left = monitorInfo.rcWork.left + (screenw - outerw) / 2;
			else if(hMonitor != nullptr)
				left += monitorInfo.rcWork.left;

			if(top == -1)
				top = monitorInfo.rcWork.top + (screenh - outerh) / 2;
			else if(hMonitor != nullptr)
				top += monitorInfo.rcWork.top;
		}
		else if(hMonitor != nullptr)
		{
			left += monitorInfo.rcWork.left;
			top += monitorInfo.rcWork.top;
		}

		if(!desc.Fullscreen)
		{
			if(desc.Parent)
			{
				if(desc.ToolWindow)
					m->StyleEx = WS_EX_TOOLWINDOW;
				else
					m->Style |= WS_CHILD;
			}
			else
			{
				if(desc.ToolWindow)
					m->StyleEx = WS_EX_TOOLWINDOW;
			}

			if(!desc.Parent || desc.ToolWindow)
			{
				if(desc.ShowTitleBar)
				{
					if(desc.ShowBorder || desc.AllowResize)
						m->Style |= WS_OVERLAPPEDWINDOW;
					else
						m->Style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
				}
				else
				{
					if(desc.ShowBorder || desc.AllowResize)
						m->Style |= WS_POPUP | WS_BORDER;
					else
						m->Style |= WS_POPUP;
				}
			}

			if(!desc.OuterDimensions)
			{
				// Calculate window dimensions required to get the requested client area
				RECT rect;
				SetRect(&rect, 0, 0, width, height);
				AdjustWindowRect(&rect, m->Style, false);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				// Clamp width and height to the desktop dimensions
				int screenw = GetSystemMetrics(SM_CXSCREEN);
				int screenh = GetSystemMetrics(SM_CYSCREEN);

				if((int)width > screenw)
					width = screenw;

				if((int)height > screenh)
					height = screenh;

				if(left < 0)
					left = (screenw - width) / 2;

				if(top < 0)
					top = (screenh - height) / 2;
			}

			if(desc.BackgroundPixels != nullptr)
				m->StyleEx |= WS_EX_LAYERED;
		}
		else
		{
			m->Style |= WS_POPUP;
			top = 0;
			left = 0;
		}

		UINT classStyle = 0;
		if(desc.EnableDoubleClick)
			classStyle |= CS_DBLCLKS;

		// Register the window class
		WNDCLASS wc = { classStyle, desc.WndProc, 0, 0, desc.Module,
						LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW),
						(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "Win32Wnd" };

		RegisterClass(&wc);

		// Create main window
		m->HWnd = CreateWindowEx(m->StyleEx, "Win32Wnd", desc.Title.c_str(), m->Style, left, top, width, height, desc.Parent, nullptr, desc.Module, desc.CreationParams);
		m->IsExternal = false;
	}
	else
	{
		m->HWnd = desc.External;
		m->IsExternal = true;
	}

	RECT rect;
	GetWindowRect(m->HWnd, &rect);
	m->Top = rect.top;
	m->Left = rect.left;

	GetClientRect(m->HWnd, &rect);
	m->Width = rect.right;
	m->Height = rect.bottom;

	// Set background, if any
	if(desc.BackgroundPixels != nullptr)
	{
		HBITMAP backgroundBitmap = Win32PlatformUtility::CreateBitmap(
			desc.BackgroundPixels, desc.BackgroundWidth, desc.BackgroundHeight, true);

		HDC hdcScreen = GetDC(nullptr);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, backgroundBitmap);

		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		POINT origin;
		origin.x = m->Left;
		origin.y = m->Top;

		SIZE size;
		size.cx = m->Width;
		size.cy = m->Height;

		POINT zero = { 0 };

		UpdateLayeredWindow(m->HWnd, hdcScreen, &origin, &size, hdcMem, &zero, RGB(0, 0, 0), &blend, desc.AlphaBlending ? ULW_ALPHA : ULW_OPAQUE);

		SelectObject(hdcMem, hOldBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(nullptr, hdcScreen);
	}

	// Handle modal windows
	B3DMarkAllocatorFrame();

	{
		FrameVector<HWND> windowsToDisable;
		FrameVector<HWND> windowsToBringToFront;
		{
			Lock lock(sWindowsMutex);

			if(m->IsModal)
			{
				if(!sModalWindowStack.empty())
				{
					Win32Window* curModalWindow = sModalWindowStack.back();
					windowsToDisable.push_back(curModalWindow->m->HWnd);
				}
				else
				{
					for(auto& window : sAllWindows)
						windowsToDisable.push_back(window->m->HWnd);
				}

				sModalWindowStack.push_back(this);
			}
			else
			{
				// A non-modal window was opened while another modal one is open,
				// immediately deactivate it and make sure the modal windows stay on top.
				if(!sModalWindowStack.empty())
				{
					shouldFocus = false;
					windowsToDisable.push_back(m->HWnd);

					for(auto window : sModalWindowStack)
						windowsToBringToFront.push_back(window->m->HWnd);
				}
			}

			sAllWindows.push_back(this);
		}

		for(auto& entry : windowsToDisable)
			EnableWindow(entry, FALSE);

		for(auto& entry : windowsToBringToFront)
			BringWindowToTop(entry);

		if(shouldFocus)
			SetFocus(m->HWnd);
	}

	B3DClearAllocatorFrame();
}

Win32Window::~Win32Window()
{
	if(m->HWnd && !m->IsExternal)
	{
		// Handle modal windows
		B3DMarkAllocatorFrame();

		{
			FrameVector<HWND> windowsToEnable;
			{
				Lock lock(sWindowsMutex);

				// Hidden dependency: All windows must be re-enabled before a window is destroyed, otherwise the incorrect
				// window in the z order will be activated.
				bool reenableWindows = false;
				if(!sModalWindowStack.empty())
				{
					// Start from back because the most common case is closing the top-most modal window
					for(auto iter = sModalWindowStack.rbegin(); iter != sModalWindowStack.rend(); ++iter)
					{
						if(*iter == this)
						{
							auto iterFwd = std::next(iter).base(); // erase doesn't accept reverse iter, so convert

							sModalWindowStack.erase(iterFwd);
							break;
						}
					}

					if(!sModalWindowStack.empty()) // Enable next modal window
					{
						Win32Window* curModalWindow = sModalWindowStack.back();
						windowsToEnable.push_back(curModalWindow->m->HWnd);
					}
					else
						reenableWindows = true; // No more modal windows, re-enable any remaining window
				}

				if(reenableWindows)
				{
					for(auto& window : sAllWindows)
						windowsToEnable.push_back(window->m->HWnd);
				}
			}

			for(auto& entry : windowsToEnable)
				EnableWindow(entry, TRUE);
		}
		B3DClearAllocatorFrame();

		DestroyWindow(m->HWnd);
	}

	{
		Lock lock(sWindowsMutex);

		auto iterFind = std::find(sAllWindows.begin(), sAllWindows.end(), this);
		sAllWindows.erase(iterFind);
	}

	B3DDelete(m);
}

void Win32Window::Move(i32 left, i32 top)
{
	if(m->HWnd)
	{
		m->Top = top;
		m->Left = left;

		SetWindowPos(m->HWnd, HWND_TOP, left, top, m->Width, m->Height, SWP_NOSIZE);
	}
}

void Win32Window::Resize(u32 width, u32 height)
{
	if(m->HWnd)
	{
		RECT rc = { 0, 0, (LONG)width, (LONG)height };
		AdjustWindowRect(&rc, GetWindowLong(m->HWnd, GWL_STYLE), false);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		m->Width = width;
		m->Height = height;

		SetWindowPos(m->HWnd, HWND_TOP, m->Left, m->Top, width, height, SWP_NOMOVE);
	}
}

void Win32Window::SetActive(bool state)
{
	if(m->HWnd)
	{
		if(state)
			ShowWindow(m->HWnd, SW_RESTORE);
		else
			ShowWindow(m->HWnd, SW_SHOWMINNOACTIVE);
	}
}

void Win32Window::SetHidden(bool hidden)
{
	if(hidden)
		ShowWindow(m->HWnd, SW_HIDE);
	else
		ShowWindow(m->HWnd, SW_SHOW);

	m->IsHidden = hidden;
}

void Win32Window::Minimize()
{
	if(m->HWnd)
		ShowWindow(m->HWnd, SW_MINIMIZE);

	if(m->IsHidden)
		ShowWindow(m->HWnd, SW_HIDE);
}

void Win32Window::Maximize()
{
	if(m->HWnd)
		ShowWindow(m->HWnd, SW_MAXIMIZE);

	if(m->IsHidden)
	{
		ShowWindow(m->HWnd, SW_HIDE);

		// Note: Doing a maximize followed by hide causes the window to lose focus, and the focus will fail to
		// restore when user clicks on the window, requiring him to alt-tab to re-gain focus. So we force focus here.
		// The other option is to delay maximizing until a hidden window is shown, but this requires us to manually
		// calculate the window size and notify the parent render window so it can immediately update the swap chain.
		SetFocus(m->HWnd);
	}
}

void Win32Window::Restore()
{
	if(m->HWnd)
		ShowWindow(m->HWnd, SW_RESTORE);

	if(m->IsHidden)
	{
		ShowWindow(m->HWnd, SW_HIDE);

		// Note: Doing a restore followed by hide causes the window to lose focus, and the focus will fail to
		// restore when user clicks on the window, requiring him to alt-tab to re-gain focus. So we force focus here.
		// The other option is to delay restoring until a hidden window is shown, but this requires us to manually
		// calculate the window size and notify the parent render window so it can immediately update the swap chain.
		SetFocus(m->HWnd);
	}
}

void Win32Window::WindowMovedOrResizedInternal()
{
	if(!m->HWnd || IsIconic(m->HWnd))
		return;

	RECT rc;
	GetWindowRect(m->HWnd, &rc);
	m->Top = rc.top;
	m->Left = rc.left;

	GetClientRect(m->HWnd, &rc);
	m->Width = rc.right - rc.left;
	m->Height = rc.bottom - rc.top;
}

Vector2I Win32Window::ScreenToWindowPos(const Vector2I& screenPos) const
{
	POINT pos;
	pos.x = screenPos.X;
	pos.y = screenPos.Y;

	ScreenToClient(m->HWnd, &pos);
	return Vector2I(pos.x, pos.y);
}

Vector2I Win32Window::WindowToScreenPos(const Vector2I& windowPos) const
{
	POINT pos;
	pos.x = windowPos.X;
	pos.y = windowPos.Y;

	ClientToScreen(m->HWnd, &pos);
	return Vector2I(pos.x, pos.y);
}

i32 Win32Window::GetLeft() const
{
	return m->Left;
}

i32 Win32Window::GetTop() const
{
	return m->Top;
}

u32 Win32Window::GetWidth() const
{
	return m->Width;
}

u32 Win32Window::GetHeight() const
{
	return m->Height;
}

HWND Win32Window::GetHWnd() const
{
	return m->HWnd;
}

DWORD Win32Window::GetStyle() const
{
	return m->Style;
}

DWORD Win32Window::GetStyleEx() const
{
	return m->StyleEx;
}

void Win32Window::EnableAllWindowsInternal()
{
	Vector<HWND> windowsToEnable;

	{
		Lock lock(sWindowsMutex);
		for(auto& window : sAllWindows)
			windowsToEnable.push_back(window->m->HWnd);
	}

	for(auto& entry : windowsToEnable)
		EnableWindow(entry, TRUE);
}

void Win32Window::RestoreModalWindowsInternal()
{
	FrameVector<HWND> windowsToDisable;
	HWND bringToFrontHwnd = 0;

	{
		Lock lock(sWindowsMutex);

		if(!sModalWindowStack.empty())
		{
			Win32Window* curModalWindow = sModalWindowStack.back();
			bringToFrontHwnd = curModalWindow->m->HWnd;

			for(auto& window : sAllWindows)
			{
				if(window != curModalWindow)
					windowsToDisable.push_back(window->m->HWnd);
			}
		}
	}

	for(auto& entry : windowsToDisable)
		EnableWindow(entry, FALSE);

	if(bringToFrontHwnd != nullptr)
		BringWindowToTop(bringToFrontHwnd);
}
