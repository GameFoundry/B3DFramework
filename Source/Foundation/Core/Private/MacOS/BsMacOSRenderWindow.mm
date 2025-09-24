//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS

#include "Private/MacOS/B3DMacOSRenderWindow.h"
#include "Private/MacOS/B3DMacOSVideoModeInfo.h"
#include "Private/MacOS/B3DMacOSWindow.h"
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Math/B3DMath.h"
#include "CoreThread/B3DCoreThread.h"

#import <QuartzCore/QuartzCore.h>

using namespace b3d;
MacOSRenderWindow::MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
		:RenderWindow(createInformation, windowId, parentWindow)
{ }

void MacOSRenderWindow::Initialize()
{
	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen;
	mIsChild = false;

	WINDOW_DESC windowCreateInformation;
	windowCreateInformation.x = mCreateInformation.left;
	windowCreateInformation.y = mCreateInformation.top;
	windowCreateInformation.width = mCreateInformation.VideoMode.Width;
	windowCreateInformation.height = mCreateInformation.VideoMode.Height;
	windowCreateInformation.title = mCreateInformation.Title;
	windowCreateInformation.showDecorations = mCreateInformation.ShowTitleBar;
	windowCreateInformation.allowResize = mCreateInformation.AllowResize;
	windowCreateInformation.modal = mCreateInformation.Modal;
	windowCreateInformation.floating = mCreateInformation.ToolWindow;

	mIsChild = false;
	if(!B3DIsWeakUnassigned(mParentWindow))
	{
		const SPtr<RenderWindow> parentWindow = mParentWindow.lock();
		if(B3D_ENSURE(parentWindow != nullptr))
			mIsChild = true;
	}

	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen && !mIsChild;
	mRenderWindowProperties.IsHidden = mCreateInformation.Hidden;

	mWindow = B3DNew<CocoaWindow>(windowCreateInformation);
	mWindow->_setUserData(this);

	Rect2I area = mWindow->getArea();
	mRenderTargetProperties.Width = area.width;
	mRenderTargetProperties.Height = area.height;
	mRenderWindowProperties.Top = area.y;
	mRenderWindowProperties.Left = area.x;
	mRenderWindowProperties.HasFocus = true;

	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = mCreateInformation.MultisampleCount;

	if(mCreateInformation.Fullscreen && !mIsChild && !windowCreateInformation.externalNSView)
		SetFullscreen(mCreateInformation.videoMode);

	if(mRenderWindowProperties.IsHidden)
		mWindow->Hide();

	CAMetalLayer* layer = [[CAMetalLayer alloc] init];
	mWindow->_setLayer((__bridge void *)layer);

	// New windows always receive focus, but we don't receive an initial event from the OS, so trigger one manually
	NotifyWindowEvent(WindowEventType::FocusReceived);

	Super::Initialize();
}

MacOSRenderWindow::Destroy()
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

Vector2I MacOSRenderWindow::ScreenToWindowPosition(const Vector2I& screenPosition) const
{
	return mWindow->ScreenToWindowPos(screenPosition);
}

Vector2I MacOSRenderWindow::WindowToScreenPosition(const Vector2I& windowPosition) const
{
	return mWindow->WindowToScreenPos(windowPosition);
}

void MacOSRenderWindow::Move(i32 left, i32 top)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		mWindow->Move(left, top);

		mRenderWindowProperties.Top = mWindow->GetTop();
		mRenderWindowProperties.Left = mWindow->GetLeft();

		MarkRenderProxyDataDirty();
	}
}

void MacOSRenderWindow::Resize(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
	{
		mWindow->Resize(width, height);

		mRenderTargetProperties.Width = mWindow->GetWidth();
		mRenderTargetProperties.Height = mWindow->GetHeight();

		MarkRenderProxyDataDirty();
	}
}

void MacOSRenderWindow::Hide()
{
	mWindow->SetHidden(true);
	mRenderWindowProperties.IsHidden = true;

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::Show()
{
	mWindow->SetHidden(false);

	mRenderWindowProperties.IsHidden = false;
	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::Minimize()
{
	mWindow->Minimize();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = true;

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::Maximize()
{
	mWindow->Maximize();

	mRenderWindowProperties.IsMaximized = true;
	mRenderWindowProperties.IsMinimized = true;

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::Restore()
{
	mWindow->Restore();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;

	mRenderTargetProperties.Width = mWindow->GetWidth();
	mRenderTargetProperties.Height = mWindow->GetHeight();

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	VideoMode videoMode(width, height, refreshRate, monitorIdx);
	SetFullscreen(videoMode);
}

void MacOSRenderWindow::SetFullscreen(const VideoMode& videoMode)
{
	if (mIsChild)
		return;

	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetCoreApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = videoMode.outputIdx;
	if(outputIdx >= outputCount)
	{
		BS_LOG(Error, Platform, "Invalid output device index.");
		return;
	}

	const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo(outputIdx);

	if(!videoMode.isCustom)
		SetDisplayMode(outputInfo, videoMode);
	else
	{
		// Look for mode matching the requested resolution
		u32 foundMode = ~0u;
		u32 numModes = outputInfo.getNumVideoModes();
		for (u32 i = 0; i < numModes; i++)
		{
			const VideoMode& currentMode = outputInfo.getVideoMode(i);

			if (currentMode.width == videoMode.width && currentMode.height == videoMode.height)
			{
				foundMode = i;

				if (Math::ApproxEquals(currentMode.refreshRate, videoMode.refreshRate))
					break;
			}
		}

		if (foundMode == ~0u)
		{
			BS_LOG(Error, Platform, "Unable to enter fullscreen, unsupported video mode requested.");
			return;
		}

		SetDisplayMode(outputInfo, outputInfo.GetVideoMode(foundMode));
	}

	mWindow->SetFullscreen();

	mRenderWindowProperties.IsFullScreen = true;

	mRenderWindowProperties.Top = 0;
	mRenderWindowProperties.Left = 0;
	mRenderTargetProperties.Width = mode.width;
	mRenderTargetProperties.Height = mode.height;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::SetWindowed(UINT32 width, UINT32 height)
{
	if (!mRenderWindowProperties.IsFullScreen)
		return;

	// Restore original display mode
	const render::Win32VideoModeInfo& videoModeInfo = static_cast<const render::Win32VideoModeInfo&>(GetCoreApplication().GetPrimaryGpuDevice()->GetVideoModeInfo());
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = 0; // 0 is always primary
	if(outputIdx >= outputCount)
	{
		BS_LOG(Error, Platform, "Invalid output device index.");
		return;
	}

	const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo(outputIdx);
	SetDisplayMode(outputInfo, outputInfo.getDesktopVideoMode());

	mWindow->SetWindowed();

	mRenderWindowProperties.isFullScreen = false;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::SetDisplayMode(const VideoOutputInfo& output, const VideoMode& mode)
{
	CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
	if (CGAcquireDisplayFadeReservation(5.0f, &fadeToken))
		CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);

	auto& destOutput = static_cast<const render::MacOSVideoOutputInfo&>(output);
	auto& newMode = static_cast<const render::MacOSVideoMode&>(mode);

	// Note: An alternative to changing display resolution would be to only change the back-buffer size. But that doesn't
	// account for refresh rate, so it's questionable how useful it would be.
	CGDirectDisplayID displayID = destOutput._getDisplayID();
	CGDisplaySetDisplayMode(displayID, newMode._getModeRef(), nullptr);

	if (fadeToken != kCGDisplayFadeReservationInvalidToken)
	{
		CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, FALSE);
		CGReleaseDisplayFadeReservation(fadeToken);
	}
}

void MacOSRenderWindow::SetVSync(bool enabled, u32 interval)
{
	if(!enabled)
		interval = 0;

	mRenderWindowProperties.Vsync = enabled;
	mRenderWindowProperties.VsyncInterval = interval;

	MarkRenderProxyDataDirty();
}

u64 MacOSRenderWindow::GetPlatformWindowHandle() const
{
	return mWindow->_getWindowId();
}

SPtr<render::RenderProxy> MacOSRenderWindow::CreateRenderProxy() const
{
	SPtr<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired()); // If parent window is assigned, it must not be expired

	RenderWindowCreateInformation createInformation = mCreateInformation;
	SPtr<render::RenderProxy> renderProxy = B3DMakeShared<render::MacOSRenderWindow>(createInformation, mWindowId, GetPlatformWindowHandle(), B3DGetRenderProxy(parentWindow));
	renderProxy->SetShared(renderProxy);

	return renderProxy;
}

void MacOSRenderWindow::DoOnWindowMovedOrResized()
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

MacOSRenderWindow::MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, platformWindowHandle, parentWindow)
{ }