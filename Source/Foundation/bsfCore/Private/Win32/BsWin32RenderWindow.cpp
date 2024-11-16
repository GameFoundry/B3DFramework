//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsWin32RenderWindow.h"

#include "BsCoreApplication.h"
#include "BsWin32VideoModeInfo.h"
#include "Private/Win32/BsWin32Platform.h"
#include "Private/Win32/BsWin32Window.h"
#include "CoreObject/BsRenderThread.h"
#include "Managers/BsRenderWindowManager.h"
#include "Math/BsMath.h"
#include "RenderAPI/BsGpuDevice.h"

using namespace bs;

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
	windowCreateInformation.Width = mCreateInformation.VideoMode.Width;
	windowCreateInformation.Height = mCreateInformation.VideoMode.Height;
	windowCreateInformation.Hidden = mCreateInformation.Hidden || mCreateInformation.HideUntilSwap;
	windowCreateInformation.Left = mCreateInformation.Left;
	windowCreateInformation.Top = mCreateInformation.Top;
	windowCreateInformation.OuterDimensions = false;
	windowCreateInformation.Title = mCreateInformation.Title;
	windowCreateInformation.ToolWindow = mCreateInformation.ToolWindow;
	windowCreateInformation.CreationParams = this;
	windowCreateInformation.Modal = mCreateInformation.Modal;
	windowCreateInformation.WndProc = &Win32Platform::Win32WndProcInternal;

#ifdef BS_STATIC_LIB
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

	const ct::Win32VideoModeInfo& videoModeInfo = static_cast<const ct::Win32VideoModeInfo&>(GetCoreApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	u32 outputCount = videoModeInfo.GetOutputCount();
	if(outputCount > 0)
	{
		u32 actualMonitorIdx = std::min(mCreateInformation.VideoMode.OutputIdx, outputCount - 1);
		const ct::Win32VideoOutputInfo& outputInfo = static_cast<const ct::Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));
		windowCreateInformation.Monitor = outputInfo.GetMonitorHandle();
	}

	// Must be set before creating a window, since wndProc will call ShowWindow if needed after creation
	if(!windowCreateInformation.External)
	{
		//mShowOnSwap = mCreateInformation.HideUntilSwap && !mCreateInformation.Hidden; // TODO
		mRenderWindowProperties.IsHidden = mCreateInformation.HideUntilSwap || mCreateInformation.Hidden;
	}

	mWindow = B3DNew<Win32Window>(windowCreateInformation);

	mIsChild = windowCreateInformation.Parent != nullptr;
	mDisplayFrequency = Math::RoundToI32(mCreateInformation.VideoMode.RefreshRate);

	// Update local properties
	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();
	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = 1;
	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen && !mIsChild;
	mRenderWindowProperties.Top = mWindow->GetTop();
	mRenderWindowProperties.Left = mWindow->GetLeft();

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
					B3D_LOG(Error, Platform, "ChangeDisplaySettings with user display frequency failed.");
			}

			if(ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
				B3D_LOG(Error, Platform, "ChangeDisplaySettings failed");
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
		mWindow->Move(left, top);

		mRenderWindowProperties.Top = mWindow->GetTop();
		mRenderWindowProperties.Left = mWindow->GetLeft();

		MarkRenderProxyDataDirty();
	}
}

void Win32RenderWindow::Resize(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		mWindow->Resize(width, height);

		mRenderTargetProperties.Width = mWindow->GetWidth();
		mRenderTargetProperties.Height = mWindow->GetHeight();

		MarkRenderProxyDataDirty();
	}
}

void Win32RenderWindow::Hide()
{
	mWindow->SetHidden(true);

	mRenderWindowProperties.IsHidden = true;
	//mShowOnSwap = false; // TODO

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Show()
{
	mWindow->SetHidden(false);

	mRenderWindowProperties.IsHidden = false;
	//mShowOnSwap = false; // TODO

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

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::Restore()
{
	mWindow->Restore();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void Win32RenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	if(mIsChild)
		return;

	const ct::Win32VideoModeInfo& videoModeInfo = static_cast<const ct::Win32VideoModeInfo&>(GetCoreApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();
	if(outputCount == 0)
		return;

	u32 actualMonitorIdx = std::min(monitorIdx, outputCount - 1);
	const ct::Win32VideoOutputInfo& outputInfo = static_cast<const ct::Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

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
		B3D_LOG(Error, Platform, "ChangeDisplaySettings failed.");

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

	AdjustWindowRect(&rect, mWindow->GetStyle(), false);
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

SPtr<ct::RenderProxy> Win32RenderWindow::CreateRenderProxy() const
{
	SPtr<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired()); // If parent window is assigned, it must not be expired

	RenderWindowCreateInformation createInformation = mCreateInformation;
	SPtr<ct::RenderProxy> renderProxy = B3DMakeShared<ct::Win32RenderWindow>(createInformation, mWindowId, GetPlatformWindowHandle(), B3DGetRenderProxy(parentWindow));
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

	mRenderWindowProperties.Top = mWindow->GetTop();
	mRenderWindowProperties.Left = mWindow->GetLeft();
	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();

	Super::DoOnWindowMovedOrResized();
}

namespace bs {
namespace ct {

Win32RenderWindow::Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 hWnd, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, hWnd, parentWindow)
{ }

void Win32RenderWindow::SwapBuffers(u32 syncMask)
{
	ASSERT_IF_NOT_RENDER_THREAD

	// TODO - Implement show on swap
	//if(mShowOnSwap)
	//	SetHidden(false);
}
}} // namespace bs::ct
