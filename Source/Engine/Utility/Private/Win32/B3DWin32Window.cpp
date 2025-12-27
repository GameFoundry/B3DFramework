//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/B3DWin32Window.h"

#include "Math/B3DVector2.h"
#include "Private/Win32/B3DWin32PlatformUtility.h"

#include <ShellScalingApi.h>

using namespace b3d;

Vector<Win32Window*> Win32Window::sAllWindows;
Vector<Win32Window*> Win32Window::sModalWindowStack;
Mutex Win32Window::sWindowsMutex;

struct Win32Window::Pimpl
{
	HWND HWnd = nullptr;
	Area2I ClientArea;
	Area2I WindowArea;
	bool IsExternal = false;
	bool IsModal = false;
	bool IsHidden = false;
	DWORD Style = 0;
	DWORD StyleEx = 0;
};

Win32Window::Win32Window(const WindowCreateInformation& createInformation)
{
	m = B3DNew<Pimpl>();
	m->IsModal = createInformation.Modal;
	m->IsHidden = createInformation.Hidden;

	HMONITOR hMonitor = createInformation.Monitor;

	if(!createInformation.External)
	{
		m->Style = WS_CLIPCHILDREN;

		// If we didn't specify the adapter index, deduce monitor from coordinates
		if(hMonitor == nullptr)
		{
			// Note: This may be window or client area coordinates, but we don't care at this point
			POINT windowAnchorPoint;
			windowAnchorPoint.x = (createInformation.Position.X >= 0 ? createInformation.Position.X : 0) + createInformation.Size.Width/2;
			windowAnchorPoint.y = (createInformation.Position.Y >= 0 ? createInformation.Position.Y : 0) + createInformation.Size.Height/2;

			// Get the nearest monitor to this window.
			hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTOPRIMARY);
		}

		// Get the target monitor info
		MONITORINFO monitorInfo;
		memset(&monitorInfo, 0, sizeof(MONITORINFO));
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);

		// Get monitor DPI
		UINT dpiX, dpiY;
		HRESULT hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		if (hr != S_OK)
		{
			dpiX = 96;
			dpiY = 96;
		}

		// Determine window style
		if(!createInformation.Fullscreen)
		{
			if(createInformation.ShowTitleBar)
			{
				if(createInformation.ShowBorder || createInformation.AllowResize)
					m->Style |= WS_OVERLAPPEDWINDOW; // Floating 'normal' window (with title, borders, min/max/close buttons)
				else
					m->Style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX; // As above, minus border
			}
			else
			{
				if(createInformation.ShowBorder || createInformation.AllowResize)
					m->Style |= WS_POPUP | WS_BORDER; // As below, but with a border
				else
					m->Style |= WS_POPUP; // Floating window with no additional elements (title, borders, min/max/close buttons)
			}

			if(createInformation.BackgroundPixels != nullptr)
				m->StyleEx |= WS_EX_LAYERED;

			if(createInformation.ToolWindow) // Makes the window not show up as a separate entry in the taskbar. Also makes the title bar smaller if present.
				m->StyleEx = WS_EX_TOOLWINDOW;
			else if(createInformation.Parent != 0)
				m->StyleEx = WS_EX_APPWINDOW; // Forces a child window to appear in the taskbar
		}
		else
			m->Style |= WS_POPUP;

		// Calculate window size
		if(!createInformation.OuterDimensions)
		{
			RECT clientAreaRect;
			SetRect(&clientAreaRect,
				createInformation.Position.X >= 0 ? createInformation.Position.X : 0,
				createInformation.Position.Y >= 0 ? createInformation.Position.Y : 0,
				createInformation.Size.Width,
				createInformation.Size.Height);

			AdjustWindowRectExForDpi(&clientAreaRect, m->Style, false, m->StyleEx, dpiX);
			m->WindowArea.X = clientAreaRect.left;
			m->WindowArea.Y = clientAreaRect.top;
			m->WindowArea.Width = (u32)(clientAreaRect.right - clientAreaRect.left);
			m->WindowArea.Height = (u32)(clientAreaRect.bottom - clientAreaRect.top);
		}
		else
		{
			m->WindowArea = Area2I(
				createInformation.Position.X >= 0 ? createInformation.Position.X : 0,
				createInformation.Position.Y >= 0 ? createInformation.Position.Y : 0,
				(u32)createInformation.Size.Width,
				(u32)createInformation.Size.Height);
		}

		const i32 screenWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
		const i32 screenHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

		// Clamp window dimensions the monitor dimensions
		m->WindowArea.Width = std::min(m->WindowArea.Width, (u32)screenWidth);
		m->WindowArea.Height = std::min(m->WindowArea.Height, (u32)screenHeight);

		// No specified top or left -> Center the window in the middle of the monitor
		if(createInformation.Position.X < 0)
			m->WindowArea.X = monitorInfo.rcWork.left + (screenWidth - (i32)m->WindowArea.Width) / 2;
		else if(hMonitor != nullptr)
			m->WindowArea.X += monitorInfo.rcWork.left;

		if(createInformation.Position.Y < 0)
			m->WindowArea.Y = monitorInfo.rcWork.top + (screenHeight - (i32)m->WindowArea.Height) / 2;
		else if(hMonitor != nullptr)
			m->WindowArea.Y += monitorInfo.rcWork.top;

		if(createInformation.Fullscreen)
		{
			m->WindowArea.X = 0;
			m->WindowArea.Y = 0;
			m->ClientArea = m->WindowArea;
		}

		UINT classStyle = 0;
		if(createInformation.EnableDoubleClick)
			classStyle |= CS_DBLCLKS;

		// Register the window class
		WNDCLASS wc = { classStyle, createInformation.WndProc, 0, 0, createInformation.Module,
						LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW),
						(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "Win32Wnd" };

		RegisterClass(&wc);

		// Create main window
		m->HWnd = CreateWindowEx(m->StyleEx, "Win32Wnd", createInformation.Title.c_str(), m->Style,
			m->WindowArea.X, m->WindowArea.Y, (i32)m->WindowArea.Width, (i32)m->WindowArea.Height, createInformation.Parent,
			nullptr, createInformation.Module, createInformation.CreationParams);
		m->IsExternal = false;
	}
	else
	{
		m->HWnd = createInformation.External;

		RECT windowRect;
		GetWindowRect(m->HWnd, &windowRect);
		m->WindowArea = Area2I(windowRect.left, windowRect.top, (u32)(windowRect.right - windowRect.left), (u32)(windowRect.bottom - windowRect.top));
		m->IsExternal = true;
	}

	RECT clientRect;
	GetClientRect(m->HWnd, &clientRect);
	m->ClientArea = Area2I(clientRect.left, clientRect.top, (u32)(clientRect.right - clientRect.left), (u32)(clientRect.bottom - clientRect.top));

	// Set background, if any
	if(createInformation.BackgroundPixels != nullptr)
	{
		HBITMAP backgroundBitmap = Win32PlatformUtility::CreateBitmap(
			createInformation.BackgroundPixels, createInformation.BackgroundWidth, createInformation.BackgroundHeight, true);

		HDC hdcScreen = GetDC(nullptr);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, backgroundBitmap);

		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		POINT zero = { 0 };

		UpdateLayeredWindow(m->HWnd, hdcScreen, NULL, NULL, hdcMem, &zero, RGB(0, 0, 0), &blend, createInformation.AlphaBlending ? ULW_ALPHA : ULW_OPAQUE);

		SelectObject(hdcMem, hOldBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(nullptr, hdcScreen);
	}
}

void Win32Window::Initialize()
{
	// Note: We don't do this in the constructor as RenderWindow needs to be able to create Win32Window before we call SetFocus below, as that calls the message
	// loop which attempts to fetch the HWND from a null Win32Window otherwise.

	if(!m->IsHidden)
		ShowWindow(m->HWnd, SW_SHOWNORMAL);

	// Handle modal windows
	B3DMarkAllocatorFrame();

	bool shouldFocus = true;
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

void Win32Window::Move(const Vector2I& position)
{
	if(!m->HWnd)
		return;

	m->WindowArea.X = position.X;
	m->WindowArea.Y = position.Y;

	SetWindowPos(m->HWnd, HWND_TOP, position.X, position.Y, 0, 0, SWP_NOSIZE);

	RECT clientRect;
	GetClientRect(m->HWnd, &clientRect);
	m->ClientArea = Area2I(clientRect.left, clientRect.top, (i32)m->ClientArea.Width, (i32)m->ClientArea.Height);
}

void Win32Window::Resize(const Size2I& size)
{
	if(!m->HWnd)
		return;

	m->ClientArea.Width = (u32)size.Width;
	m->ClientArea.Height = (u32)size.Height;

	const UINT DPI = GetDPI();

	RECT clientRect = { m->ClientArea.X, m->ClientArea.Y, m->ClientArea.X + (i32)size.Width, m->ClientArea.Y + size.Height };
	AdjustWindowRectExForDpi(&clientRect, GetWindowLong(m->HWnd, GWL_STYLE), false, GetWindowLong(m->HWnd, GWL_EXSTYLE), DPI);

	m->WindowArea.Width = (u32)(clientRect.right - clientRect.left);
	m->WindowArea.Height = (u32)(clientRect.bottom - clientRect.top);

	SetWindowPos(m->HWnd, HWND_TOP, 0, 0, (i32)m->WindowArea.Width, (i32)m->WindowArea.Height, SWP_NOMOVE);
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

	DoOnWindowMovedOrResized();
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

	DoOnWindowMovedOrResized();
}

void Win32Window::DoOnWindowMovedOrResized()
{
	if(!m->HWnd || IsIconic(m->HWnd))
		return;

	RECT windowRect;
	GetWindowRect(m->HWnd, &windowRect);
	m->WindowArea = Area2I(windowRect.left, windowRect.top, (u32)(windowRect.right - windowRect.left), (u32)(windowRect.bottom - windowRect.top));

	RECT clientRect;
	GetClientRect(m->HWnd, &clientRect);
	m->ClientArea = Area2I(clientRect.left, clientRect.top, (u32)(clientRect.right - clientRect.left), (u32)(clientRect.bottom - clientRect.top));
}

Vector2I Win32Window::ScreenToWindowPosition(const Vector2I& screenPos) const
{
	POINT pos;
	pos.x = screenPos.X;
	pos.y = screenPos.Y;

	ScreenToClient(m->HWnd, &pos);
	return Vector2I(pos.x, pos.y);
}

Vector2I Win32Window::WindowToScreenPosition(const Vector2I& windowPos) const
{
	POINT pos;
	pos.x = windowPos.X;
	pos.y = windowPos.Y;

	ClientToScreen(m->HWnd, &pos);
	return Vector2I(pos.x, pos.y);
}

const Area2I& Win32Window::GetWindowArea() const
{
	return m->WindowArea;
}

const Area2I& Win32Window::GetClientArea() const
{
	return m->ClientArea;
}

UINT Win32Window::GetDPI() const
{
	return GetDpiForWindow(m->HWnd);
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
