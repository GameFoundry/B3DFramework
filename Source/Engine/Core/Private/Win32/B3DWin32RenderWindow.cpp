//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DWin32RenderWindow.h"

#include "B3DApplication.h"
#include "B3DWin32VideoModeInfo.h"
#include "Private/Win32/B3DWin32Platform.h"
#include "Private/Win32/B3DWin32Window.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Math/B3DMath.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace b3d;

Win32RenderWindow::Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, parentWindow)
{}

void Win32RenderWindow::Initialize()
{
	// Create a window
	WindowCreateInformation windowCreateInformation;
	windowCreateInformation.ShowTitleBar = mCreateInformation.ShowTitleBar;
	windowCreateInformation.ShowBorder = mCreateInformation.ShowBorder;
	windowCreateInformation.AllowResize = mCreateInformation.AllowResize;
	windowCreateInformation.EnableDoubleClick = true;
	windowCreateInformation.Fullscreen = mCreateInformation.Fullscreen;
	windowCreateInformation.Size = Size2I((i32)mCreateInformation.VideoMode.Width, (i32)mCreateInformation.VideoMode.Height);
	windowCreateInformation.Hidden = mCreateInformation.Hidden || mCreateInformation.HideUntilSwap;
	windowCreateInformation.Position = Vector2I(mCreateInformation.Left, mCreateInformation.Top);
	windowCreateInformation.OuterDimensions = false;
	windowCreateInformation.Title = mCreateInformation.Title;
	windowCreateInformation.ToolWindow = mCreateInformation.ToolWindow;
	windowCreateInformation.CreationParams = this;
	windowCreateInformation.Modal = mCreateInformation.Modal;
	windowCreateInformation.WndProc = &Win32Platform::Win32WndProcInternal;

#ifdef B3D_STATIC_LIB
	windowCreateInformation.module = GetModuleHandle(NULL);
#else
	windowCreateInformation.Module = GetModuleHandle("bsfVulkanRenderAPI.dll");
#endif

	if(!B3DIsWeakUnassigned(mParentWindow))
	{
		const SPtr<Win32RenderWindow> parentWindow = std::static_pointer_cast<Win32RenderWindow>(mParentWindow.lock());
		if(B3D_ENSURE(parentWindow != nullptr))
			windowCreateInformation.Parent = (HWND)parentWindow->GetPlatformWindowHandle();
	}

	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	u32 outputCount = videoModeInfo.GetOutputCount();
	if(outputCount > 0)
	{
		u32 actualMonitorIdx = std::min(mCreateInformation.VideoMode.OutputIdx, outputCount - 1);
		const render::Win32VideoOutputInfo& outputInfo = static_cast<const render::Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));
		windowCreateInformation.Monitor = outputInfo.GetMonitorHandle();
	}

	// Must be set before creating a window, since wndProc will call ShowWindow if needed after creation
	if(!windowCreateInformation.External)
	{
		mRenderWindowProperties.IsHidden = mCreateInformation.Hidden;
	}

	mWindow = B3DNew<Win32Window>(windowCreateInformation);
	mWindow->Initialize();

	mIsChild = windowCreateInformation.Parent != nullptr;
	mDisplayFrequency = Math::RoundToI32(mCreateInformation.VideoMode.RefreshRate);

	// Update local properties
	const Area2I& clientArea = mWindow->GetClientArea();

	mRenderTargetProperties.Width = clientArea.Width;
	mRenderTargetProperties.Height = clientArea.Height;
	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = 1;
	mRenderTargetProperties.DPIScale = (float)mWindow->GetDPI() / (float)USER_DEFAULT_SCREEN_DPI;
	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen && !mIsChild;
	mRenderWindowProperties.Left = clientArea.X;
	mRenderWindowProperties.Top = clientArea.Y;

	// Make the window full screen if required
	if(!windowCreateInformation.External)
	{
		if(mRenderWindowProperties.IsFullScreen)
		{
			DEVMODE displayDeviceMode;

			memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
			displayDeviceMode.dmSize = sizeof(DEVMODE);
			displayDeviceMode.dmBitsPerPel = 32;
			displayDeviceMode.dmPelsWidth = mRenderTargetProperties.Width;
			displayDeviceMode.dmPelsHeight = mRenderTargetProperties.Height;
			displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if(mDisplayFrequency)
			{
				displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
				displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

				if(ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
					B3D_LOG(Error, LogPlatform, "ChangeDisplaySettings with user display frequency failed.");
			}

			if(ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
				B3D_LOG(Error, LogPlatform, "ChangeDisplaySettings failed");
		}
	}

	Super::Initialize();
}

void Win32RenderWindow::Destroy()
{
	GetRenderThread().PostCommand([renderProxy = GetRenderProxy()]
	{
		if(renderProxy != nullptr)
			renderProxy->Destroy();
	}, "DestroyRenderWindowRenderProxy", true);

	if(mWindow != nullptr)
	{
		B3DDelete(mWindow);
		mWindow = nullptr;
	}

	Platform::ResetNonClientAreas(*this);
	
	Super::Destroy();
}

Vector2I Win32RenderWindow::ScreenToWindowPosition(const Vector2I& screenPos) const
{
	POINT pos;
	pos.x = screenPos.X;
	pos.y = screenPos.Y;

	HWND hwnd = (HWND)GetPlatformWindowHandle();

	ScreenToClient(hwnd, &pos);
	return Vector2I(pos.x, pos.y);
}

Vector2I Win32RenderWindow::WindowToScreenPosition(const Vector2I& windowPos) const
{
	POINT pos;
	pos.x = windowPos.X;
	pos.y = windowPos.Y;

	HWND hwnd = (HWND)GetPlatformWindowHandle();

	ClientToScreen(hwnd, &pos);
	return Vector2I(pos.x, pos.y);
}

void Win32RenderWindow::Move(i32 left, i32 top)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		mWindow->Move(Vector2I(left, top));

		mRenderWindowProperties.Left = mWindow->GetClientArea().X;
		mRenderWindowProperties.Top = mWindow->GetClientArea().Y;

		MarkRenderProxyDataDirty();
	}
}

void Win32RenderWindow::Resize(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		mWindow->Resize(Size2I((i32)width, (i32)height));

		mRenderTargetProperties.Width = mWindow->GetClientArea().Width;
		mRenderTargetProperties.Height = mWindow->GetClientArea().Height;

		MarkRenderProxyDataDirty();
		OnResized();
	}
}

void Win32RenderWindow::Hide()
{
	mWindow->SetHidden(true);
	mRenderWindowProperties.IsHidden = true;

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Show()
{
	mWindow->SetHidden(false);
	mRenderWindowProperties.IsHidden = false;

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Minimize()
{
	mWindow->Minimize();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = true;

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Maximize()
{
	mWindow->Maximize();

	mRenderWindowProperties.IsMaximized = true;
	mRenderWindowProperties.IsMinimized = true;

	mRenderTargetProperties.Width = mWindow->GetClientArea().Width;
	mRenderTargetProperties.Height = mWindow->GetClientArea().Height;

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Restore()
{
	mWindow->Restore();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;

	mRenderTargetProperties.Width = mWindow->GetClientArea().Width;
	mRenderTargetProperties.Height = mWindow->GetClientArea().Height;

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	if(mIsChild)
		return;

	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();
	if(outputCount == 0)
		return;

	u32 actualMonitorIdx = std::min(monitorIdx, outputCount - 1);
	const render::Win32VideoOutputInfo& outputInfo = static_cast<const render::Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

	mDisplayFrequency = Math::RoundToI32(refreshRate);
	mRenderWindowProperties.IsFullScreen = true;

	DEVMODE displayDeviceMode;

	memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
	displayDeviceMode.dmSize = sizeof(DEVMODE);
	displayDeviceMode.dmBitsPerPel = 32;
	displayDeviceMode.dmPelsWidth = width;
	displayDeviceMode.dmPelsHeight = height;
	displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
	displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

	HMONITOR hMonitor = outputInfo.GetMonitorHandle();
	MONITORINFOEX monitorInfo;

	memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &monitorInfo);

	if(ChangeDisplaySettingsEx(monitorInfo.szDevice, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
		B3D_LOG(Error, LogPlatform, "ChangeDisplaySettings failed.");

	mRenderWindowProperties.Top = monitorInfo.rcMonitor.top;
	mRenderWindowProperties.Left = monitorInfo.rcMonitor.left;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;

	SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, 0);

	SetWindowPos(mWindow->GetHWnd(), HWND_TOP, mRenderWindowProperties.Left, mRenderWindowProperties.Top, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::SetWindowed(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
		return;

	mRenderWindowProperties.IsFullScreen = false;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;

	// Drop out of fullscreen
	ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);

	u32 winWidth = width;
	u32 winHeight = height;

	RECT rect;
	SetRect(&rect, 0, 0, winWidth, winHeight);

	UINT DPI = mWindow->GetDPI();

	AdjustWindowRectExForDpi(&rect, mWindow->GetStyle(), false, mWindow->GetStyleEx(), DPI);
	winWidth = rect.right - rect.left;
	winHeight = rect.bottom - rect.top;

	// Deal with centering when switching down to smaller resolution
	HMONITOR hMonitor = MonitorFromWindow(mWindow->GetHWnd(), MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo;
	memset(&monitorInfo, 0, sizeof(MONITORINFO));
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &monitorInfo);

	LONG screenw = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
	LONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

	i32 left = screenw > i32(winWidth) ? ((screenw - i32(winWidth)) / 2) : 0;
	i32 top = screenh > i32(winHeight) ? ((screenh - i32(winHeight)) / 2) : 0;

	SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, mWindow->GetStyle() | WS_VISIBLE);
	SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, mWindow->GetStyleEx());

	SetWindowPos(mWindow->GetHWnd(), HWND_NOTOPMOST, left, top, winWidth, winHeight, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::SetVSync(bool enabled, u32 interval)
{
	mRenderWindowProperties.Vsync = enabled;
	mRenderWindowProperties.VsyncInterval = interval;

	MarkRenderProxyDataDirty();
}

u64 Win32RenderWindow::GetPlatformWindowHandle() const
{
	return (u64)mWindow->GetHWnd();
}

SPtr<render::RenderProxy> Win32RenderWindow::CreateRenderProxy() const
{
	SPtr<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired()); // If parent window is assigned, it must not be expired

	RenderWindowCreateInformation createInformation = mCreateInformation;
	SPtr<render::RenderProxy> renderProxy = B3DMakeShared<render::Win32RenderWindow>(createInformation, mWindowId, GetPlatformWindowHandle(), B3DGetRenderProxy(parentWindow));
	renderProxy->SetShared(renderProxy);

	return renderProxy;
}

void Win32RenderWindow::DoOnWindowMovedOrResized()
{
	// mWindow will be null when this gets called during render window initialization
	if(mWindow == nullptr)
		return;

	// This will update internal window properties that we're about to retrieve below
	mWindow->DoOnWindowMovedOrResized();

	const Area2I& clientArea = mWindow->GetClientArea();

	mRenderWindowProperties.Left = clientArea.X;
	mRenderWindowProperties.Top = clientArea.Y;
	mRenderTargetProperties.Width = clientArea.Width;
	mRenderTargetProperties.Height = clientArea.Height;

	MarkRenderProxyDataDirty();

	Super::DoOnWindowMovedOrResized();
}

void Win32RenderWindow::DoOnDPIScaleChanged()
{
	const float oldDpiScale = mRenderTargetProperties.DPIScale;

	const UINT DPI = mWindow->GetDPI();
	mRenderTargetProperties.DPIScale = (float)DPI / (float)USER_DEFAULT_SCREEN_DPI;

	if(oldDpiScale == mRenderTargetProperties.DPIScale)
		return;

	Super::DoOnDPIScaleChanged();

	const float scaleRatio = mRenderTargetProperties.DPIScale / oldDpiScale;
	if(!mRenderWindowProperties.IsMaximized && !mRenderWindowProperties.IsFullScreen)
		Resize(Math::RoundToI32(mRenderTargetProperties.Width * scaleRatio), Math::RoundToI32(mRenderTargetProperties.Height * scaleRatio));
}

namespace b3d::render
{
Win32RenderWindow::Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 hWnd, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, hWnd, parentWindow)
{ }
} // namespace b3d::render
