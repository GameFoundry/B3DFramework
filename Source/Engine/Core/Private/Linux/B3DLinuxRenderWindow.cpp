//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/B3DCoreThread.h"
#include "Private/Linux/B3DLinuxPlatform.h"
#include "Private/Linux/B3DLinuxWindow.h"
#include "Private/Linux/B3DLinuxRenderWindow.h"
#include "Private/Linux/B3DLinuxVideoModeInfo.h"
#include "Math/B3DMath.h"
#include "Managers/B3DRenderWindowManager.h"
#include <X11/Xutil.h>

#define XRANDR_ROTATION_LEFT (1 << 1)
#define XRANDR_ROTATION_RIGHT (1 << 3)

using namespace b3d;

LinuxRenderWindow::LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, parentWindow)
{}

void LinuxRenderWindow::Initialize()
{
	LinuxPlatform::lockX();

	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen;
	mIsChild = false;

	XVisualInfo visualInfoTempl = {};
	visualInfoTempl.screen = XDefaultScreen(LinuxPlatform::getXDisplay());
	visualInfoTempl.depth = 24;
	visualInfoTempl.c_class = TrueColor;

	int32_t numVisuals;
	XVisualInfo* visualInfo = XGetVisualInfo(LinuxPlatform::getXDisplay(), VisualScreenMask | VisualDepthMask | VisualClassMask, &visualInfoTempl, &numVisuals);

	WINDOW_DESC windowCreateInformation;
	windowCreateInformation.x = mCreateInformation.Left;
	windowCreateInformation.y = mCreateInformation.Top;
	windowCreateInformation.width = mCreateInformation.VideoMode.Width;
	windowCreateInformation.height = mCreateInformation.VideoMode.Height;
	windowCreateInformation.title = mCreateInformation.title;
	windowCreateInformation.showDecorations = mCreateInformation.ShowTitleBar;
	windowCreateInformation.allowResize = mCreateInformation.AllowResize;
	windowCreateInformation.showOnTaskBar = !mCreateInformation.ToolWindow;
	windowCreateInformation.modal = mCreateInformation.Modal;
	windowCreateInformation.visualInfo = *visualInfo;
	windowCreateInformation.screen = mCreateInformation.VideoMode.OutputIdx;
	windowCreateInformation.hidden = mCreateInformation.HideUntilSwap || mCreateInformation.Hidden;

	windowCreateInformation.parent = 0;
	if(!B3DIsWeakUnassigned(mParentWindow))
	{
		const SPtr<LinuxRenderWindow> parentWindow = std::static_pointer_cast<LinuxRenderWindow>(mParentWindow.lock());
		if(B3D_ENSURE(parentWindow != nullptr))
			windowCreateInformation.parent = (::Window)parentWindow->GetPlatformWindowHandle();
	}

	mIsChild = windowCreateInformation.parent != 0;
	mWindowProperties.IsFullScreen = mCreateInformation.Fullscreen && !mIsChild;

	mShowOnSwap = mCreateInformation.HideUntilSwap && !mCreateInformation.Hidden;
	mRenderWindowProperties.IsHidden = mCreateInformation.HideUntilSwap || mCreateInformation.Hidden;

	mWindow = B3DNew<LinuxWindow>(windowCreateInformation);
	mWindow->SetUserDataInternal(this);

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();
	mRenderWindowProperties.Top = mWindow->GetTop();
	mRenderWindowProperties.Left = mWindow->GetLeft();

	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = mCreateInformation.MultisampleCount;

	XWindowAttributes windowAttributes;
	XGetWindowAttributes(LinuxPlatform::getXDisplay(), mWindow->GetXWindowInternal(), &windowAttributes);

	LinuxPlatform::unlockX(); // Calls below have their own locking mechanisms

	if(mCreateInformation.Fullscreen && !mIsChild)
		SetFullscreen(mCreateInformation.VideoMode);

	if(mCreateInformation.Vsync && mCreateInformation.VsyncInterval > 0)
		SetVSync(true, mCreateInformation.vsyncInterval);

	Super::Initialize();
}

LinuxRenderWindow::Destroy()
{
	// Make sure to set the original desktop video mode before we exit
	if(mRenderWindowProperties.IsFullScreen)
		SetWindowed(50, 50);

	GetRenderThread().PostCommand([renderProxy = GetRenderProxy()]
								  {
									if(renderProxy != nullptr)
										renderProxy->Destroy();
								  }, "DestroyRenderWindowRenderProxy", true);

	Platform::ResetNonClientAreas(*this);

	if(mWindow != nullptr)
	{
		B3DDelete(mWindow);
		mWindow = nullptr;
	}

	Super::Destroy();
}

Vector2I LinuxRenderWindow::ScreenToWindowPosition(const Vector2I& screenPosition) const
{
	LinuxPlatform::lockX();
	Vector2I pos = mWindow->ScreenToWindowPos(screenPosition);
	LinuxPlatform::unlockX();

	return pos;
}

Vector2I LinuxRenderWindow::WindowToScreenPosition(const Vector2I& windowPosition) const
{
	LinuxPlatform::lockX();
	Vector2I pos = mWindow->WindowToScreenPos(windowPosition);
	LinuxPlatform::unlockX();

	return pos;
}

void LinuxRenderWindow::Move(i32 left, i32 top)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		LinuxPlatform::lockX();
		mWindow->Move(left, top);
		LinuxPlatform::unlockX();

		mRenderWindowProperties.Top = mWindow->GetTop();
		mRenderWindowProperties.Left = mWindow->GetLeft();

		MarkRenderProxyDataDirty();
	}
}

void LinuxRenderWindow::Resize(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		LinuxPlatform::lockX();
		mWindow->Resize(width, height);
		LinuxPlatform::unlockX();

		mRenderTargetProperties.Width = mWindow->GetWidth();
		mRenderTargetProperties.Height = mWindow->GetHeight();

		MarkRenderProxyDataDirty();
	}
}

void LinuxRenderWindow::Hide()
{
	LinuxPlatform::lockX();
	mWindow->SetHidden(true);
	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsHidden = true;

	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::Show()
{
	LinuxPlatform::lockX();
	mWindow->SetHidden(false);
	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsHidden = false;

	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::Minimize()
{
	LinuxPlatform::lockX();
	mWindow->Minimize();
	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = true;

	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::Maximize()
{
	LinuxPlatform::lockX();
	mWindow->Maximize();
	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsMaximized = true;
	mRenderWindowProperties.IsMinimized = true;

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::Restore()
{
	LinuxPlatform::lockX();
	mWindow->Restore();
	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::SetVideoMode(i32 screen, RROutput output, RRMode mode)
{
	::Display* display = LinuxPlatform::getXDisplay();
	::Window rootWindow = RootWindow(display, screen);

	XRRScreenResources* screenRes = XRRGetScreenResources(display, rootWindow);
	if(screenRes == nullptr)
	{
		B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve screen resources. ");
		return;
	}

	XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, output);
	if(outputInfo == nullptr)
	{
		XRRFreeScreenResources(screenRes);

		B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve output info for output: {0}", (u32)output);
		return;
	}

	XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
	if(crtcInfo == nullptr)
	{
		XRRFreeScreenResources(screenRes);
		XRRFreeOutputInfo(outputInfo);

		B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve CRTC info for output: {0}", (u32)output);
		return;
	}

	// Note: This changes the user's desktop resolution permanently, even when the app exists, make sure to revert
	// (Sadly there doesn't appear to be a better way)
	Status status = XRRSetCrtcConfig(display, screenRes, outputInfo->crtc, CurrentTime, crtcInfo->x, crtcInfo->y, mode, crtcInfo->rotation, &output, 1);

	if(status != Success)
		B3D_LOG(Error, LogPlatform, "XRR: XRRSetCrtcConfig failed.");

	XRRFreeCrtcInfo(crtcInfo);
	XRRFreeOutputInfo(outputInfo);
	XRRFreeScreenResources(screenRes);
}

void LinuxRenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	VideoMode videoMode(width, height, refreshRate, monitorIdx);
	SetFullscreen(videoMode);
}

void LinuxRenderWindow::SetFullscreen(const VideoMode& mode)
{
	if(mIsChild)
		return;

	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = mode.outputIdx;
	if(outputIdx >= outputCount)
	{
		B3D_LOG(Error, LogPlatform, "Invalid output device index.");
		return;
	}

	const LinuxVideoOutputInfo& outputInfo = static_cast<const LinuxVideoOutputInfo&>(videoModeInfo.GetOutputInfo(outputIdx));

	i32 screen = outputInfo.GetScreenInternal();
	RROutput outputID = outputInfo.GetOutputIDInternal();

	RRMode modeID = 0;
	if(!mode.IsCustom)
	{
		const LinuxVideoMode& videoMode = static_cast<const LinuxVideoMode&>(mode);
		modeID = videoMode.GetModeIDInternal();
	}
	else
	{
		LinuxPlatform::lockX();

		// Look for mode matching the requested resolution
		::Display* display = LinuxPlatform::getXDisplay();
		::Window rootWindow = RootWindow(display, screen);

		XRRScreenResources* screenRes = XRRGetScreenResources(display, rootWindow);
		if(screenRes == nullptr)
		{
			B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve screen resources. ");
			return;
		}

		XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, outputID);
		if(outputInfo == nullptr)
		{
			XRRFreeScreenResources(screenRes);

			B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve output info for output: {0}", (u32)outputID);
			return;
		}

		XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
		if(crtcInfo == nullptr)
		{
			XRRFreeScreenResources(screenRes);
			XRRFreeOutputInfo(outputInfo);

			B3D_LOG(Error, LogPlatform, "XRR: Failed to retrieve CRTC info for output: {0}", (u32)outputID);
			return;
		}

		bool foundMode = false;
		for(i32 i = 0; i < screenRes->nmode; i++)
		{
			const XRRModeInfo& modeInfo = screenRes->modes[i];

			u32 width, height;

			if(crtcInfo->rotation & (XRANDR_ROTATION_LEFT | XRANDR_ROTATION_RIGHT))
			{
				width = modeInfo.height;
				height = modeInfo.width;
			}
			else
			{
				width = modeInfo.width;
				height = modeInfo.height;
			}

			float refreshRate;
			if(modeInfo.hTotal != 0 && modeInfo.vTotal != 0)
				refreshRate = (float)(modeInfo.DotClock / (double)(modeInfo.hTotal * modeInfo.vTotal));
			else
				refreshRate = 0.0f;

			if(width == mode.width && height == mode.height)
			{
				modeID = modeInfo.id;
				foundMode = true;

				if(Math::ApproxEquals(refreshRate, mode.refreshRate))
					break;
			}
		}

		if(!foundMode)
		{
			LinuxPlatform::unlockX();

			B3D_LOG(Error, LogPlatform, "Unable to enter fullscreen, unsupported video mode requested.");
			return;
		}

		LinuxPlatform::unlockX();
	}

	LinuxPlatform::lockX();

	SetVideoMode(screen, outputID, modeID);
	mWindow->SetFullscreenInternal(true);

	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsFullScreen = true;

	mRenderWindowProperties.Top = 0;
	mRenderWindowProperties.Left = 0;
	mRenderTargetProperties.Width = mode.width;
	mRenderTargetProperties.Height = mode.height;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::SetWindowed(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
		return;

	// Restore old screen config
	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = 0; // 0 is always primary
	if(outputIdx >= outputCount)
	{
		B3D_LOG(Error, LogPlatform, "Invalid output device index.");
		return;
	}

	const LinuxVideoOutputInfo& outputInfo =
		static_cast<const LinuxVideoOutputInfo&>(videoModeInfo.GetOutputInfo(outputIdx));

	const LinuxVideoMode& desktopVideoMode = static_cast<const LinuxVideoMode&>(outputInfo.GetDesktopVideoMode());

	LinuxPlatform::lockX();

	SetVideoMode(outputInfo.GetScreenInternal(), outputInfo._getOutputID(), desktopVideoMode._getModeID());
	mWindow->SetFullscreenInternal(false);

	LinuxPlatform::unlockX();

	mRenderWindowProperties.IsFullScreen = false;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void LinuxRenderWindow::SetVSync(bool enabled, u32 interval)
{
	if(!enabled)
		interval = 0;

	mRenderWindowProperties.Vsync = enabled;
	mRenderWindowProperties.VsyncInterval = interval;

	MarkRenderProxyDataDirty();
}

u64 LinuxRenderWindow::GetPlatformWindowHandle() const
{
	return (u64)mWindow->GetXWindowInternal();
}

SPtr<render::RenderProxy> LinuxRenderWindow::CreateRenderProxy() const
{
	SPtr<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired()); // If parent window is assigned, it must not be expired

	RenderWindowCreateInformation createInformation = mCreateInformation;
	SPtr<render::RenderProxy> renderProxy = B3DMakeShared<render::LinuxRenderWindow>(createInformation, mWindowId, GetPlatformWindowHandle(), B3DGetRenderProxy(parentWindow));
	renderProxy->SetShared(renderProxy);

	return renderProxy;
}

void LinuxRenderWindow::DoOnWindowMovedOrResized()
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

using namespace b3d::render;

LinuxRenderWindow::LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, platformWindowHandle, parentWindow)
{}