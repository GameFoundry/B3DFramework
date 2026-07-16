//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS

#include "Private/MacOS/B3DMacOSRenderWindow.h"

#include "B3DApplication.h"
#include "Private/MacOS/B3DMacOSWindow.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Math/B3DMath.h"
#include "Platform/B3DPlatform.h"
#include "GpuBackend/B3DGpuDevice.h"

#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>

using namespace b3d;

/**
 * Returns the Core Graphics display ID of the output device with the specified index, or kCGNullDirectDisplay if no
 * such output exists. Output indices match those reported by the GPU device video mode info (mirrored displays are
 * skipped and the primary display is always first).
 */
static CGDirectDisplayID GetDisplayId(u32 outputIdx)
{
	CGDisplayCount displayCount;
	CGGetOnlineDisplayList(0, nullptr, &displayCount);

	auto displays = (CGDirectDisplayID*)B3DStackAllocate((u32)(sizeof(CGDirectDisplayID) * displayCount));
	auto orderedDisplays = (CGDirectDisplayID*)B3DStackAllocate((u32)(sizeof(CGDirectDisplayID) * displayCount));
	CGGetOnlineDisplayList(displayCount, displays, &displayCount);

	u32 orderedCount = 0;
	for(u32 displayIdx = 0; displayIdx < displayCount; displayIdx++)
	{
		if(CGDisplayMirrorsDisplay(displays[displayIdx]) != kCGNullDirectDisplay)
			continue;

		CGDisplayModeRef modeRef = CGDisplayCopyDisplayMode(displays[displayIdx]);
		if(!modeRef)
			continue;

		CGDisplayModeRelease(modeRef);

		if(CGDisplayIsMain(displays[displayIdx]))
		{
			for(u32 moveIdx = orderedCount; moveIdx > 0; moveIdx--)
				orderedDisplays[moveIdx] = orderedDisplays[moveIdx - 1];

			orderedDisplays[0] = displays[displayIdx];
		}
		else
			orderedDisplays[orderedCount] = displays[displayIdx];

		orderedCount++;
	}

	CGDirectDisplayID displayId = kCGNullDirectDisplay;
	if(outputIdx < orderedCount)
		displayId = orderedDisplays[outputIdx];

	B3DStackFree(orderedDisplays);
	B3DStackFree(displays);

	return displayId;
}

/**
 * Finds a Core Graphics display mode of the specified display that matches the resolution and refresh rate of the
 * provided video mode. Returns null if no matching mode exists. Caller must release the returned mode.
 */
static CGDisplayModeRef FindDisplayMode(CGDirectDisplayID displayId, const VideoMode& mode)
{
	CFArrayRef modes = CGDisplayCopyAllDisplayModes(displayId, nullptr);
	if(modes == nullptr)
		return nullptr;

	CGDisplayModeRef foundModeRef = nullptr;

	// Look for a mode matching the requested resolution, preferring one that also matches the refresh rate
	CFIndex modeCount = CFArrayGetCount(modes);
	for(CFIndex modeIdx = 0; modeIdx < modeCount; modeIdx++)
	{
		auto modeRef = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, modeIdx);

		u32 width = (u32)CGDisplayModeGetPixelWidth(modeRef);
		u32 height = (u32)CGDisplayModeGetPixelHeight(modeRef);

		if(width == mode.Width && height == mode.Height)
		{
			foundModeRef = modeRef;

			float refreshRate = (float)CGDisplayModeGetRefreshRate(modeRef);
			if(Math::ApproxEquals(refreshRate, mode.RefreshRate))
				break;
		}
	}

	if(foundModeRef != nullptr)
		CGDisplayModeRetain(foundModeRef);

	CFRelease(modes);

	return foundModeRef;
}

MacOSRenderWindow::MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const TShared<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, parentWindow)
{ }

void MacOSRenderWindow::Initialize()
{
	WindowCreateInformation windowCreateInformation;
	windowCreateInformation.X = mCreateInformation.Left;
	windowCreateInformation.Y = mCreateInformation.Top;
	windowCreateInformation.Width = mCreateInformation.VideoMode.Width;
	windowCreateInformation.Height = mCreateInformation.VideoMode.Height;
	windowCreateInformation.Title = mCreateInformation.Title;
	windowCreateInformation.ShowDecorations = mCreateInformation.ShowTitleBar;
	windowCreateInformation.AllowResize = mCreateInformation.AllowResize;
	windowCreateInformation.Modal = mCreateInformation.Modal;
	windowCreateInformation.Floating = mCreateInformation.ToolWindow;

	mIsChild = false;
	if(!B3DIsWeakUnassigned(mParentWindow))
	{
		const TShared<RenderWindow> parentWindow = mParentWindow.lock();
		if(B3D_ENSURE(parentWindow != nullptr))
			mIsChild = true;
	}

	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen && !mIsChild;
	mRenderWindowProperties.IsHidden = mCreateInformation.Hidden;

	mWindow = B3DNew<CocoaWindow>(windowCreateInformation);
	mWindow->SetUserDataInternal(this);
	mWindow->Resize(mCreateInformation.VideoMode.Width, mCreateInformation.VideoMode.Height);

	Area2I area = mWindow->GetArea();
	const Vector2I framebufferSize = mWindow->GetFramebufferSizeInternal();
	mRenderTargetProperties.Width = (u32)framebufferSize.X;
	mRenderTargetProperties.Height = (u32)framebufferSize.Y;
	mRenderWindowProperties.Top = area.Y;
	mRenderWindowProperties.Left = area.X;
	mRenderWindowProperties.HasFocus = true;

	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = mCreateInformation.MultisampleCount;

	if(mCreateInformation.Fullscreen && !mIsChild)
		SetFullscreen(mCreateInformation.VideoMode);

	if(mRenderWindowProperties.IsHidden)
		mWindow->SetHidden(true);

	CAMetalLayer* layer = [CAMetalLayer layer];
	layer.contentsScale = mWindow->GetPrivateDataInternal()->Window.backingScaleFactor;
	layer.drawableSize = CGSizeMake(mRenderTargetProperties.Width, mRenderTargetProperties.Height);
	mWindow->SetLayerInternal((__bridge void*)layer);

	// New windows always receive focus, but we don't receive an initial event from the OS, so trigger one manually
	NotifyWindowEvent(WindowEventType::FocusReceived);

	Super::Initialize();
}

void MacOSRenderWindow::Destroy()
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
		mWindow->SetLayerInternal(nullptr);
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

		const Vector2I framebufferSize = mWindow->GetFramebufferSizeInternal();
		mRenderTargetProperties.Width = (u32)framebufferSize.X;
		mRenderTargetProperties.Height = (u32)framebufferSize.Y;

		MarkRenderProxyDataDirty();
		OnResized();
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
	mRenderWindowProperties.IsMinimized = false;

	const Vector2I framebufferSize = mWindow->GetFramebufferSizeInternal();
	mRenderTargetProperties.Width = (u32)framebufferSize.X;
	mRenderTargetProperties.Height = (u32)framebufferSize.Y;

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::Restore()
{
	mWindow->Restore();

	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;

	const Vector2I framebufferSize = mWindow->GetFramebufferSizeInternal();
	mRenderTargetProperties.Width = (u32)framebufferSize.X;
	mRenderTargetProperties.Height = (u32)framebufferSize.Y;

	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	VideoMode videoMode(width, height, refreshRate, monitorIdx);
	SetFullscreen(videoMode);
}

void MacOSRenderWindow::SetFullscreen(const VideoMode& mode)
{
	if(mIsChild)
		return;

	const VideoModeInfo& videoModeInfo = GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo();
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = mode.OutputIdx;
	if(outputIdx >= outputCount)
	{
		B3D_LOG(Error, LogPlatform, "Invalid output device index.");
		return;
	}

	CGDirectDisplayID displayId = GetDisplayId(outputIdx);
	if(displayId == kCGNullDirectDisplay)
	{
		B3D_LOG(Error, LogPlatform, "Unable to find a display for output device index {0}.", outputIdx);
		return;
	}

	if(!SetDisplayMode(displayId, mode))
	{
		B3D_LOG(Error, LogPlatform, "Unable to enter fullscreen, unsupported video mode requested.");
		return;
	}

	mWindow->SetFullscreen();

	mRenderWindowProperties.IsFullScreen = true;

	mRenderWindowProperties.Top = 0;
	mRenderWindowProperties.Left = 0;
	mRenderTargetProperties.Width = mode.Width;
	mRenderTargetProperties.Height = mode.Height;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

void MacOSRenderWindow::SetWindowed(u32 width, u32 height)
{
	if(!mRenderWindowProperties.IsFullScreen)
		return;

	// Restore original display mode
	const VideoModeInfo& videoModeInfo = GetApplication().GetPrimaryGpuDevice()->GetVideoModeInfo();
	const u32 outputCount = videoModeInfo.GetOutputCount();

	u32 outputIdx = 0; // 0 is always primary
	if(outputIdx >= outputCount)
	{
		B3D_LOG(Error, LogPlatform, "Invalid output device index.");
		return;
	}

	CGDirectDisplayID displayId = GetDisplayId(outputIdx);
	if(displayId != kCGNullDirectDisplay)
	{
		if(!SetDisplayMode(displayId, videoModeInfo.GetOutputInfo(outputIdx).GetDesktopVideoMode()))
			B3D_LOG(Error, LogPlatform, "Unable to restore the original desktop video mode.");
	}

	mWindow->SetWindowed();
	mWindow->Resize(width, height);

	mRenderWindowProperties.IsFullScreen = false;

	DoOnWindowMovedOrResized();
	MarkRenderProxyDataDirty();
}

bool MacOSRenderWindow::SetDisplayMode(u32 displayId, const VideoMode& mode)
{
	CGDisplayModeRef modeRef = FindDisplayMode((CGDirectDisplayID)displayId, mode);
	if(modeRef == nullptr)
		return false;

	CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
	if(CGAcquireDisplayFadeReservation(5.0f, &fadeToken) == kCGErrorSuccess)
		CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);

	// Note: An alternative to changing display resolution would be to only change the back-buffer size. But that doesn't
	// account for refresh rate, so it's questionable how useful it would be.
	CGDisplaySetDisplayMode((CGDirectDisplayID)displayId, modeRef, nullptr);

	if(fadeToken != kCGDisplayFadeReservationInvalidToken)
	{
		CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, FALSE);
		CGReleaseDisplayFadeReservation(fadeToken);
	}

	CGDisplayModeRelease(modeRef);

	return true;
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
	return (u64)mWindow->GetWindowIdInternal();
}

TShared<render::RenderProxy> MacOSRenderWindow::CreateRenderProxy() const
{
	TShared<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired()); // If parent window is assigned, it must not be expired

	RenderWindowCreateInformation createInformation = mCreateInformation;
	TShared<render::RenderProxy> renderProxy = B3DMakeShared<render::MacOSRenderWindow>(createInformation, mWindowId, GetPlatformWindowHandle(), B3DGetRenderProxy(parentWindow));
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
	const Vector2I framebufferSize = mWindow->GetFramebufferSizeInternal();
	mRenderTargetProperties.Width = (u32)framebufferSize.X;
	mRenderTargetProperties.Height = (u32)framebufferSize.Y;

	MarkRenderProxyDataDirty();

	Super::DoOnWindowMovedOrResized();
}

namespace b3d::render
{
MacOSRenderWindow::MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const TShared<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, platformWindowHandle, parentWindow)
{ }
} // namespace b3d::render
