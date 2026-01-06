//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DHeadlessRenderWindow.h"

#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DRenderWindowManager.h"

using namespace b3d;

HeadlessRenderWindow::HeadlessRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, parentWindow)
{}

void HeadlessRenderWindow::Initialize()
{
	// No OS window to create - just set up the properties based on VideoMode
	mRenderTargetProperties.Width = mCreateInformation.VideoMode.Width;
	mRenderTargetProperties.Height = mCreateInformation.VideoMode.Height;
	mRenderTargetProperties.HwGamma = mCreateInformation.Gamma;
	mRenderTargetProperties.MultisampleCount = 1;
	mRenderTargetProperties.DPIScale = 1.0f;

	mRenderWindowProperties.IsFullScreen = mCreateInformation.Fullscreen;
	mRenderWindowProperties.Left = 0;
	mRenderWindowProperties.Top = 0;
	mRenderWindowProperties.IsHidden = mCreateInformation.Hidden;

	Super::Initialize();
}

void HeadlessRenderWindow::Destroy()
{
	GetRenderThread().PostCommand([renderProxy = GetRenderProxy()]
	{
		if(renderProxy != nullptr)
			renderProxy->Destroy();
	}, "DestroyHeadlessRenderWindowRenderProxy", true);

	Super::Destroy();
}

void HeadlessRenderWindow::Resize(u32 width, u32 height)
{
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;

	MarkRenderProxyDataDirty();
	OnResized();
}

void HeadlessRenderWindow::Move(i32 left, i32 top)
{
	// No-op: headless window has no position
	mRenderWindowProperties.Left = left;
	mRenderWindowProperties.Top = top;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::Hide()
{
	mRenderWindowProperties.IsHidden = true;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::Show()
{
	mRenderWindowProperties.IsHidden = false;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::Minimize()
{
	// No-op: headless window cannot be minimized
	mRenderWindowProperties.IsMinimized = true;
	mRenderWindowProperties.IsMaximized = false;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::Maximize()
{
	// No-op: headless window cannot be maximized
	mRenderWindowProperties.IsMaximized = true;
	mRenderWindowProperties.IsMinimized = false;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::Restore()
{
	mRenderWindowProperties.IsMaximized = false;
	mRenderWindowProperties.IsMinimized = false;
	MarkRenderProxyDataDirty();
}

void HeadlessRenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIndex)
{
	// Just update properties without changing display mode
	mRenderWindowProperties.IsFullScreen = true;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;
	MarkRenderProxyDataDirty();
	OnResized();
}

void HeadlessRenderWindow::SetWindowed(u32 width, u32 height)
{
	mRenderWindowProperties.IsFullScreen = false;
	mRenderTargetProperties.Width = width;
	mRenderTargetProperties.Height = height;
	MarkRenderProxyDataDirty();
	OnResized();
}

void HeadlessRenderWindow::SetVSync(bool enabled, u32 interval)
{
	mRenderWindowProperties.Vsync = enabled;
	mRenderWindowProperties.VsyncInterval = interval;
	MarkRenderProxyDataDirty();
}

SPtr<render::RenderProxy> HeadlessRenderWindow::CreateRenderProxy() const
{
	SPtr<RenderWindow> parentWindow = mParentWindow.lock();
	B3D_ENSURE(B3DIsWeakUnassigned(mParentWindow) || !mParentWindow.expired());

	SPtr<render::RenderProxy> renderProxy = B3DMakeShared<render::HeadlessRenderWindow>(mCreateInformation, mWindowId, B3DGetRenderProxy(parentWindow));
	renderProxy->SetShared(renderProxy);

	return renderProxy;
}

namespace b3d::render
{
HeadlessRenderWindow::HeadlessRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	: RenderWindow(createInformation, windowId, 0, parentWindow) // 0 for platform window handle
{}
} // namespace b3d::render
