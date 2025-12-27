//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DRenderWindow.h"

#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DRenderWindowManager.h"
#include "RenderAPI/B3DViewport.h"
#include "Platform/B3DPlatform.h"
#include "RTTI/B3DRenderTargetRTTI.h"

using namespace b3d;

static RenderWindowProperties CreateRenderWindowProperties(const RenderWindowCreateInformation& createInformation)
{
	RenderWindowProperties output;

	output.Vsync = createInformation.Vsync;
	output.VsyncInterval = createInformation.VsyncInterval;
	output.Left = createInformation.Left;
	output.Top = createInformation.Top;
	output.IsFullScreen = createInformation.Fullscreen;
	output.IsHidden = createInformation.Hidden;
	output.IsModal = createInformation.Modal;

	return output;
}

static RenderTargetProperties CreateRenderTargetProperties(const RenderWindowCreateInformation& createInformation)
{
	RenderTargetProperties output;

	output.Width = createInformation.VideoMode.Width;
	output.Height = createInformation.VideoMode.Height;
	output.HwGamma = createInformation.Gamma;
	output.MultisampleCount = createInformation.MultisampleCount;
	output.IsWindow = true;
	output.RequiresTextureFlipping = false;

	return output;
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(RenderWindow, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mRenderTargetProperties)
		B3D_SYNC_BLOCK_ENTRY(mRenderWindowProperties)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* RenderWindow::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

void RenderWindow::Destroy()
{
	RenderWindowManager::Instance().NotifyWindowDestroyed(*this);

	RenderTarget::Destroy();
}

RenderWindow::RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	: mCreateInformation(createInformation), mWindowId(windowId), mParentWindow(parentWindow), mRenderWindowProperties(CreateRenderWindowProperties(createInformation))
{
	mRenderTargetProperties = CreateRenderTargetProperties(createInformation);
}

void RenderWindow::SetFullscreen(const VideoMode& mode)
{
	SetFullscreen(mode.Width, mode.Height, mode.RefreshRate, mode.OutputIdx);
}

SPtr<RenderWindow> RenderWindow::Create(const RenderWindowCreateInformation& createInformation, const SPtr<RenderWindow>& parentWindow)
{
	return RenderWindowManager::Instance().CreateRenderWindow(createInformation, parentWindow);
}

void RenderWindow::NotifyWindowEvent(WindowEventType type)
{
	switch(type)
	{
	case WindowEventType::Resized:
		{
			DoOnWindowMovedOrResized();
			OnResized();

			break;
		}
	case WindowEventType::Moved:
		{
			DoOnWindowMovedOrResized();

			break;
		}
	case WindowEventType::DPIScaleChanged:
		{
			DoOnDPIScaleChanged();
			OnDPIScaleChanged();

			break;
		}
	case WindowEventType::FocusReceived:
		{
			mRenderWindowProperties.HasFocus = true;

			RenderWindowManager::Instance().NotifyFocusReceived(*this);
			MarkRenderProxyDataDirty();

			break;
		}
	case WindowEventType::FocusLost:
		{
			mRenderWindowProperties.HasFocus = false;

			RenderWindowManager::Instance().NotifyFocusLost(*this);
			MarkRenderProxyDataDirty();

			break;
		}
	case WindowEventType::Minimized:
		{
			mRenderWindowProperties.IsMaximized = false;
			mRenderWindowProperties.IsMinimized = true;
			MarkRenderProxyDataDirty();

			break;
		}
	case WindowEventType::Maximized:
		{
			mRenderWindowProperties.IsMaximized = true;
			mRenderWindowProperties.IsMinimized = false;
			MarkRenderProxyDataDirty();

			break;
		}
	case WindowEventType::Restored:
		{
			mRenderWindowProperties.IsMaximized = false;
			mRenderWindowProperties.IsMinimized = false;
			MarkRenderProxyDataDirty();

			break;
		}
	case WindowEventType::Redraw:
	{
		MarkRenderProxyDataDirty((u32)RenderWindowDirtyFlag::Redraw);
		break;
	}
	case WindowEventType::MouseLeft:
		{
			RenderWindowManager::Instance().NotifyMouseLeft(*this);
			break;
		}
	case WindowEventType::CloseRequested:
		{
			const SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();

			// Default behaviour for primary window is to quit the app on close
			if(this == primaryWindow.get() && OnCloseRequested.Empty())
			{
				GetApplication().NotifyQuitRequested();
				return;
			}

			OnCloseRequested();
			break;
		}
	}
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* RenderWindow::GetRttiStatic()
{
	return RenderWindowRTTI::Instance();
}

RTTIType* RenderWindow::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d { namespace render
{
RenderWindow::RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const SPtr<RenderWindow>& parentWindow)
	:  mCreateInformation(createInformation), mWindowId(windowId), mPlatformWindowHandle(platformWindowHandle), mParentWindow(parentWindow), mRenderWindowProperties(CreateRenderWindowProperties(createInformation))
{
	mRenderTargetProperties = CreateRenderTargetProperties(createInformation);
	mShowOnSwap = mCreateInformation.HideUntilSwap && !mCreateInformation.Hidden;
}

void RenderWindow::Initialize()
{
	if(mCreateInformation.CreateRenderSurface)
	{
		RenderWindowSurfaceCreateInformation renderWindowSurfaceCreateInformation;
		renderWindowSurfaceCreateInformation.Width = mRenderTargetProperties.Width;
		renderWindowSurfaceCreateInformation.Height = mRenderTargetProperties.Height;
		renderWindowSurfaceCreateInformation.CreateDepthBuffer = mCreateInformation.DepthBuffer;
		renderWindowSurfaceCreateInformation.UseHardwareSRGB = mCreateInformation.Gamma;
		renderWindowSurfaceCreateInformation.VSync = mCreateInformation.Vsync;
		renderWindowSurfaceCreateInformation.PlatformWindowHandle = mPlatformWindowHandle;

		mRenderWindowSurface = b3d::RenderWindowManager::Instance().CreateRenderWindowSurface(renderWindowSurfaceCreateInformation);
	}

	Super::Initialize();
}

void RenderWindow::Destroy()
{
	if(mRenderWindowSurface != nullptr)
	{
		mRenderWindowSurface->Destroy();
		mRenderWindowSurface = nullptr;
	}

	Super::Destroy();
}

void RenderWindow::NotifySwapBuffersRequested()
{
	if(mShowOnSwap)
	{
		b3d::RenderWindowManager::Instance().RequestShowWindow(mWindowId, true);
		mShowOnSwap = false;
	}

	mIsRedrawRequested = false;
}

void RenderWindow::RebuildSwapChain()
{
	if(mRenderWindowSurface != nullptr)
	{
		mRenderWindowSurface->RebuildSwapChain(mRenderTargetProperties.Width, mRenderTargetProperties.Height, mRenderWindowProperties.Vsync);
		OnSwapChainDidRebuild();
	}
}

void RenderWindow::DoOnSwapChainPropertiesModified()
{
	if(mRenderWindowSurface != nullptr)
		mRenderWindowSurface->MarkSwapChainAsInvalid();
}

void RenderWindow::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::RenderWindow::SyncPacket>();
	if(syncPacket == nullptr)
		return;

	if(syncPacket->Flags == (u32)RenderWindowDirtyFlag::Redraw)
	{
		mIsRedrawRequested = true;
		return;
	}

	const u32 oldWidth = mRenderTargetProperties.Width;
	const u32 oldHeight = mRenderTargetProperties.Height;
	const bool oldVSync = mRenderWindowProperties.Vsync;
	const u32 oldVSyncInterval = mRenderWindowProperties.VsyncInterval;
	const bool oldIsHidden = mRenderWindowProperties.IsHidden;

	syncPacket->ApplySyncData(this);

	if(oldWidth != mRenderTargetProperties.Width || oldHeight != mRenderTargetProperties.Height || oldVSync != mRenderWindowProperties.Vsync || oldVSyncInterval != mRenderWindowProperties.VsyncInterval)
		DoOnSwapChainPropertiesModified();

	// Reset show on swap if user had explicitly shown the window
	if(oldIsHidden != mRenderWindowProperties.IsHidden)
		mShowOnSwap = false;

	Super::SyncFromCoreObject(data, allocator);
}
}}
