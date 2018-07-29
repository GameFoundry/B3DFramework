//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/BsWin32Window.h"
#include "Private/Win32/BsWin32PlatformUtility.h"

namespace bs
{
	Vector<Win32Window*> Win32Window::sAllWindows;
	Vector<Win32Window*> Win32Window::sModalWindowStack;
	Mutex Win32Window::sWindowsMutex;

	struct Win32Window::Pimpl
	{
		HWND hWnd = nullptr;
		INT32 left = 0;
		INT32 top = 0;
		UINT32 width = 0;
		UINT32 height = 0;
		bool isExternal = false;
		bool isModal = false;
		bool isHidden = false;
		DWORD style = 0;
		DWORD styleEx = 0;
	};

	Win32Window::Win32Window(const WINDOW_DESC& desc)
	{
		m = bs_new<Pimpl>();
		m->isModal = desc.modal;
		m->isHidden = desc.hidden;

		HMONITOR hMonitor = desc.monitor;
		if (!desc.external)
		{
			m->style = WS_CLIPCHILDREN;

			INT32 left = desc.left;
			INT32 top = desc.top;

			// If we didn't specified the adapter index, or if we didn't find it
			if (hMonitor == nullptr)
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

			UINT32 width = desc.width;
			UINT32 height = desc.height;

			// No specified top left -> Center the window in the middle of the monitor
			if (left == -1 || top == -1)
			{
				int screenw = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
				int screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

				// clamp window dimensions to screen size
				int outerw = (int(width) < screenw) ? int(width) : screenw;
				int outerh = (int(height) < screenh) ? int(height) : screenh;

				if (left == -1)
					left = monitorInfo.rcWork.left + (screenw - outerw) / 2;
				else if (hMonitor != nullptr)
					left += monitorInfo.rcWork.left;

				if (top == -1)
					top = monitorInfo.rcWork.top + (screenh - outerh) / 2;
				else if (hMonitor != nullptr)
					top += monitorInfo.rcWork.top;
			}
			else if (hMonitor != nullptr)
			{
				left += monitorInfo.rcWork.left;
				top += monitorInfo.rcWork.top;
			}

			if (!desc.fullscreen)
			{
				if (desc.parent)
				{
					if (desc.toolWindow)
						m->styleEx = WS_EX_TOOLWINDOW;
					else
						m->style |= WS_CHILD;
				}
				else
				{
					if (desc.toolWindow)
						m->styleEx = WS_EX_TOOLWINDOW;
				}

				if (!desc.parent || desc.toolWindow)
				{
					if(desc.showTitleBar)
					{
						if(desc.showBorder || desc.allowResize)
							m->style |= WS_OVERLAPPEDWINDOW;
						else
							m->style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
					}
					else
					{
						if(desc.showBorder || desc.allowResize)
							m->style |= WS_POPUP | WS_BORDER;
						else
							m->style |= WS_POPUP;
					}
				}

				if (!desc.outerDimensions)
				{
					// Calculate window dimensions required to get the requested client area
					RECT rect;
					SetRect(&rect, 0, 0, width, height);
					AdjustWindowRect(&rect, m->style, false);
					width = rect.right - rect.left;
					height = rect.bottom - rect.top;

					// Clamp width and height to the desktop dimensions
					int screenw = GetSystemMetrics(SM_CXSCREEN);
					int screenh = GetSystemMetrics(SM_CYSCREEN);

					if ((int)width > screenw)
						width = screenw;

					if ((int)height > screenh)
						height = screenh;

					if (left < 0)
						left = (screenw - width) / 2;

					if (top < 0)
						top = (screenh - height) / 2;
				}

				if (desc.backgroundPixels != nullptr)
					m->styleEx |= WS_EX_LAYERED;
			}
			else
			{
				m->style |= WS_POPUP;
				top = 0;
				left = 0;
			}

			UINT classStyle = 0;
			if (desc.enableDoubleClick)
				classStyle |= CS_DBLCLKS;

			// Register the window class
			WNDCLASS wc = { classStyle, desc.wndProc, 0, 0, desc.module,
				LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "Win32Wnd" };

			RegisterClass(&wc);

			// Create main window
			m->hWnd = CreateWindowEx(m->styleEx, "Win32Wnd", desc.title.c_str(), m->style,
				left, top, width, height, desc.parent, nullptr, desc.module, desc.creationParams);
			m->isExternal = false;
		}
		else
		{
			m->hWnd = desc.external;
			m->isExternal = true;
		}

		RECT rect;
		GetWindowRect(m->hWnd, &rect);
		m->top = rect.top;
		m->left = rect.left;

		GetClientRect(m->hWnd, &rect);
		m->width = rect.right;
		m->height = rect.bottom;

		// Set background, if any
		if (desc.backgroundPixels != nullptr)
		{
			HBITMAP backgroundBitmap = Win32PlatformUtility::createBitmap(
				desc.backgroundPixels, desc.backgroundWidth, desc.backgroundHeight, true);

			HDC hdcScreen = GetDC(nullptr);
			HDC hdcMem = CreateCompatibleDC(hdcScreen);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, backgroundBitmap);

			BLENDFUNCTION blend = { 0 };
			blend.BlendOp = AC_SRC_OVER;
			blend.SourceConstantAlpha = 255;
			blend.AlphaFormat = AC_SRC_ALPHA;

			POINT origin;
			origin.x = m->left;
			origin.y = m->top;

			SIZE size;
			size.cx = m->width;
			size.cy = m->height;

			POINT zero = { 0 };

			UpdateLayeredWindow(m->hWnd, hdcScreen, &origin, &size,
				hdcMem, &zero, RGB(0, 0, 0), &blend, desc.alphaBlending ? ULW_ALPHA : ULW_OPAQUE);

			SelectObject(hdcMem, hOldBitmap);
			DeleteDC(hdcMem);
			ReleaseDC(nullptr, hdcScreen);
		}

		// Handle modal windows
		bs_frame_mark();

		{
			FrameVector<HWND> windowsToDisable;
			FrameVector<HWND> windowsToBringToFront;
			{
				Lock lock(sWindowsMutex);

				if (m->isModal)
				{
					if (!sModalWindowStack.empty())
					{
						Win32Window* curModalWindow = sModalWindowStack.back();
						windowsToDisable.push_back(curModalWindow->m->hWnd);
					}
					else
					{
						for (auto& window : sAllWindows)
							windowsToDisable.push_back(window->m->hWnd);
					}

					sModalWindowStack.push_back(this);
				}
				else
				{
					// A non-modal window was opened while another modal one is open,
					// immediately deactivate it and make sure the modal windows stay on top.
					if (!sModalWindowStack.empty())
					{
						windowsToDisable.push_back(m->hWnd);

						for (auto window : sModalWindowStack)
							windowsToBringToFront.push_back(window->m->hWnd);
					}
				}

				sAllWindows.push_back(this);
			}

			for(auto& entry : windowsToDisable)
				EnableWindow(entry, FALSE);

			for (auto& entry : windowsToBringToFront)
				BringWindowToTop(entry);

			SetFocus(m->hWnd);
		}

		if(desc.hidden)
			setHidden(true);

		bs_frame_clear();
	}

	Win32Window::~Win32Window()
	{
		if (m->hWnd && !m->isExternal)
		{
			// Handle modal windows
			bs_frame_mark();
			
			{
				FrameVector<HWND> windowsToEnable;
				{
					Lock lock(sWindowsMutex);

					// Hidden dependency: All windows must be re-enabled before a window is destroyed, otherwise the incorrect
					// window in the z order will be activated.
					bool reenableWindows = false;
					if (!sModalWindowStack.empty())
					{
						// Start from back because the most common case is closing the top-most modal window
						for (auto iter = sModalWindowStack.rbegin(); iter != sModalWindowStack.rend(); ++iter)
						{
							if (*iter == this)
							{
								auto iterFwd = std::next(iter).base(); // erase doesn't accept reverse iter, so convert

								sModalWindowStack.erase(iterFwd);
								break;
							}
						}

						if (!sModalWindowStack.empty()) // Enable next modal window
						{
							Win32Window* curModalWindow = sModalWindowStack.back();
							windowsToEnable.push_back(curModalWindow->m->hWnd);
						}
						else
							reenableWindows = true; // No more modal windows, re-enable any remaining window
					}

					if (reenableWindows)
					{
						for (auto& window : sAllWindows)
							windowsToEnable.push_back(window->m->hWnd);
					}
				}

				for(auto& entry : windowsToEnable)
					EnableWindow(entry, TRUE);
			}
			bs_frame_clear();

			DestroyWindow(m->hWnd);
		}

		{
			Lock lock(sWindowsMutex);

			auto iterFind = std::find(sAllWindows.begin(), sAllWindows.end(), this);
			sAllWindows.erase(iterFind);
		}

		bs_delete(m);
	}

	void Win32Window::move(INT32 left, INT32 top)
	{
		if (m->hWnd)
		{
			m->top = top;
			m->left = left;

			SetWindowPos(m->hWnd, HWND_TOP, left, top, m->width, m->height, SWP_NOSIZE);
		}
	}

	void Win32Window::resize(UINT32 width, UINT32 height)
	{
		if (m->hWnd)
		{
			RECT rc = { 0, 0, (LONG)width, (LONG)height };
			AdjustWindowRect(&rc, GetWindowLong(m->hWnd, GWL_STYLE), false);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;

			m->width = width;
			m->height = height;

			SetWindowPos(m->hWnd, HWND_TOP, m->left, m->top, width, height, SWP_NOMOVE);
		}
	}

	void Win32Window::setActive(bool state)
	{
		if (m->hWnd)
		{
			if (state)
				ShowWindow(m->hWnd, SW_RESTORE);
			else
				ShowWindow(m->hWnd, SW_SHOWMINNOACTIVE);
		}
	}

	void Win32Window::setHidden(bool hidden)
	{
		if (hidden)
			ShowWindow(m->hWnd, SW_HIDE);
		else
			ShowWindow(m->hWnd, SW_SHOW);

		m->isHidden = hidden;
	}

	void Win32Window::minimize()
	{
		if (m->hWnd)
			ShowWindow(m->hWnd, SW_MINIMIZE);

		if(m->isHidden)
			ShowWindow(m->hWnd, SW_HIDE);
	}

	void Win32Window::maximize()
	{
		if (m->hWnd)
			ShowWindow(m->hWnd, SW_MAXIMIZE);

		if(m->isHidden)
			ShowWindow(m->hWnd, SW_HIDE);
	}

	void Win32Window::restore()
	{
		if (m->hWnd)
			ShowWindow(m->hWnd, SW_RESTORE);

		if(m->isHidden)
			ShowWindow(m->hWnd, SW_HIDE);
	}

	void Win32Window::_windowMovedOrResized()
	{
		if (!m->hWnd || IsIconic(m->hWnd))
			return;

		RECT rc;
		GetWindowRect(m->hWnd, &rc);
		m->top = rc.top;
		m->left = rc.left;

		GetClientRect(m->hWnd, &rc);
		m->width = rc.right - rc.left;
		m->height = rc.bottom - rc.top;
	}

	Vector2I Win32Window::screenToWindowPos(const Vector2I& screenPos) const
	{
		POINT pos;
		pos.x = screenPos.x;
		pos.y = screenPos.y;

		ScreenToClient(m->hWnd, &pos);
		return Vector2I(pos.x, pos.y);
	}

	Vector2I Win32Window::windowToScreenPos(const Vector2I& windowPos) const
	{
		POINT pos;
		pos.x = windowPos.x;
		pos.y = windowPos.y;

		ClientToScreen(m->hWnd, &pos);
		return Vector2I(pos.x, pos.y);
	}

	INT32 Win32Window::getLeft() const
	{
		return m->left;
	}

	INT32 Win32Window::getTop() const
	{
		return m->top;
	}

	UINT32 Win32Window::getWidth() const
	{
		return m->width;
	}

	UINT32 Win32Window::getHeight() const
	{
		return m->height;
	}

	HWND Win32Window::getHWnd() const
	{
		return m->hWnd;
	}

	DWORD Win32Window::getStyle() const
	{
		return m->style;
	}

	DWORD Win32Window::getStyleEx() const
	{
		return m->styleEx;
	}

	void Win32Window::_enableAllWindows()
	{
		Vector<HWND> windowsToEnable;

		{
			Lock lock(sWindowsMutex);
			for (auto& window : sAllWindows)
				windowsToEnable.push_back(window->m->hWnd);
		}

		for (auto& entry : windowsToEnable)
			EnableWindow(entry, TRUE);
	}

	void Win32Window::_restoreModalWindows()
	{
		FrameVector<HWND> windowsToDisable;
		HWND bringToFrontHwnd = 0;

		{
			Lock lock(sWindowsMutex);

			if (!sModalWindowStack.empty())
			{
				Win32Window* curModalWindow = sModalWindowStack.back();
				bringToFrontHwnd = curModalWindow->m->hWnd;

				for (auto& window : sAllWindows)
				{
					if (window != curModalWindow)
						windowsToDisable.push_back(window->m->hWnd);
				}
			}
		}

		for (auto& entry : windowsToDisable)
			EnableWindow(entry, FALSE);

		if (bringToFrontHwnd != nullptr)
			BringWindowToTop(bringToFrontHwnd);
	}
}
