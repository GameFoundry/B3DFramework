//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderWindow.h"
#include "CoreObject/BsRenderThread.h"
#include "Managers/BsRenderWindowManager.h"
#include "RenderAPI/BsViewport.h"
#include "Platform/BsPlatform.h"
#include "Private/RTTI/BsRenderTargetRTTI.h"

using namespace bs;

RenderWindowProperties::RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
{
	Width = desc.VideoMode.Width;
	Height = desc.VideoMode.Height;
	HwGamma = desc.Gamma;
	Vsync = desc.Vsync;
	VsyncInterval = desc.VsyncInterval;
	MultisampleCount = desc.MultisampleCount;
	Left = desc.Left;
	Top = desc.Top;
	IsFullScreen = desc.Fullscreen;
	IsHidden = desc.Hidden;
	IsModal = desc.Modal;
	IsWindow = true;
	RequiresTextureFlipping = false;
}

void RenderWindow::Destroy()
{
	RenderWindowManager::Instance().NotifyWindowDestroyed(this);

	RenderTarget::Destroy();
}

RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
	: mDesc(desc), mWindowId(windowId)
{
}

void RenderWindow::Resize(u32 width, u32 height)
{
	std::function<void(SPtr<ct::RenderWindow>, u32, u32)> resizeFunc =
		[](SPtr<ct::RenderWindow> renderWindow, u32 width, u32 height)
	{
		renderWindow->Resize(width, height);
	};

	GetRenderThread().PostCommand([core = B3DGetRenderProxy(this), resizeFunc, width, height] { resizeFunc(core, width, height); }, true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

void RenderWindow::Move(i32 left, i32 top)
{
	std::function<void(SPtr<ct::RenderWindow>, i32, i32)> moveFunc =
		[](SPtr<ct::RenderWindow> renderWindow, i32 left, i32 top)
	{
		renderWindow->Move(left, top);
	};

	GetRenderThread().PostCommand([core = B3DGetRenderProxy(this), moveFunc, left, top] { moveFunc(core, left, top); }, true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Left = syncedProps.Left;
		mutableProps.Top = syncedProps.Top;
	}
}

void RenderWindow::Hide()
{
	std::function<void(SPtr<ct::RenderWindow>)> hideFunc =
		[](SPtr<ct::RenderWindow> renderWindow)
	{
		renderWindow->SetHidden(true);
	};

	GetMutableProperties().IsHidden = true;

	GetRenderThread().PostCommand(std::bind(hideFunc, B3DGetRenderProxy(this)));
}

void RenderWindow::Show()
{
	std::function<void(SPtr<ct::RenderWindow>)> showFunc =
		[](SPtr<ct::RenderWindow> renderWindow)
	{
		renderWindow->SetHidden(false);
	};

	GetMutableProperties().IsHidden = false;

	GetRenderThread().PostCommand(std::bind(showFunc, B3DGetRenderProxy(this)));
}

void RenderWindow::Minimize()
{
	std::function<void(SPtr<ct::RenderWindow>)> minimizeFunc =
		[](SPtr<ct::RenderWindow> renderWindow)
	{
		renderWindow->Minimize();
	};

	GetMutableProperties().IsMaximized = false;

	GetRenderThread().PostCommand(std::bind(minimizeFunc, B3DGetRenderProxy(this)));
}

void RenderWindow::Maximize()
{
	std::function<void(SPtr<ct::RenderWindow>)> maximizeFunc =
		[](SPtr<ct::RenderWindow> renderWindow)
	{
		renderWindow->Maximize();
	};

	GetMutableProperties().IsMaximized = true;

	GetRenderThread().PostCommand(std::bind(maximizeFunc, B3DGetRenderProxy(this)), true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

void RenderWindow::Restore()
{
	std::function<void(SPtr<ct::RenderWindow>)> restoreFunc =
		[](SPtr<ct::RenderWindow> renderWindow)
	{
		renderWindow->Restore();
	};

	GetMutableProperties().IsMaximized = false;

	GetRenderThread().PostCommand(std::bind(restoreFunc, B3DGetRenderProxy(this)), true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

void RenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	std::function<void(SPtr<ct::RenderWindow>, u32, u32, float, u32)> fullscreenFunc =
		[](SPtr<ct::RenderWindow> renderWindow, u32 width, u32 height, float refreshRate, u32 monitorIdx)
	{
		renderWindow->SetFullscreen(width, height, refreshRate, monitorIdx);
	};

	GetRenderThread().PostCommand(std::bind(fullscreenFunc, B3DGetRenderProxy(this), width, height, refreshRate, monitorIdx), true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

void RenderWindow::SetFullscreen(const VideoMode& mode)
{
	std::function<void(SPtr<ct::RenderWindow>, const VideoMode&)> fullscreenFunc =
		[](SPtr<ct::RenderWindow> renderWindow, const VideoMode& mode)
	{
		renderWindow->SetFullscreen(mode);
	};

	GetRenderThread().PostCommand(std::bind(fullscreenFunc, B3DGetRenderProxy(this), std::cref(mode)), true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

void RenderWindow::SetWindowed(u32 width, u32 height)
{
	std::function<void(SPtr<ct::RenderWindow>, u32, u32)> windowedFunc =
		[](SPtr<ct::RenderWindow> renderWindow, u32 width, u32 height)
	{
		renderWindow->SetWindowed(width, height);
	};

	GetRenderThread().PostCommand(std::bind(windowedFunc, B3DGetRenderProxy(this), width, height), true);

	{
		ScopedSpinLock lock(B3DGetRenderProxy(this)->mLock);
		const RenderWindowProperties& syncedProps = B3DGetRenderProxy(this)->GetSyncedProperties();
		RenderWindowProperties& mutableProps = GetMutableProperties();

		mutableProps.Width = syncedProps.Width;
		mutableProps.Height = syncedProps.Height;
	}
}

SPtr<RenderWindow> RenderWindow::Create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
{
	return RenderWindowManager::Instance().Create(desc, parentWindow);
}

RenderWindowProperties& RenderWindow::GetMutableProperties()
{
	return const_cast<RenderWindowProperties&>(GetProperties());
}

const RenderWindowProperties& RenderWindow::GetProperties() const
{
	return static_cast<const RenderWindowProperties&>(GetPropertiesInternal());
}

void RenderWindow::NotifyWindowEventInternal(WindowEventType type)
{
	ASSERT_IF_RENDER_THREAD;

	ct::RenderWindow* coreWindow = B3DGetRenderProxy(this).get();
	RenderWindowProperties& syncProps = coreWindow->GetSyncedProperties();
	RenderWindowProperties& props = const_cast<RenderWindowProperties&>(GetProperties());

	switch(type)
	{
	case WindowEventType::Resized:
		{
			WindowMovedOrResizedInternal();

			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.Width = props.Width;
				syncProps.Height = props.Height;
			}

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			RenderWindowManager::Instance().NotifyMovedOrResized(coreWindow);

			break;
		}
	case WindowEventType::Moved:
		{
			WindowMovedOrResizedInternal();

			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.Top = props.Top;
				syncProps.Left = props.Left;
			}

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			RenderWindowManager::Instance().NotifyMovedOrResized(coreWindow);

			break;
		}
	case WindowEventType::FocusReceived:
		{
			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.HasFocus = true;
			}

			props.HasFocus = true;

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			RenderWindowManager::Instance().NotifyFocusReceived(coreWindow);
			break;
		}
	case WindowEventType::FocusLost:
		{
			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.HasFocus = false;
			}

			props.HasFocus = false;

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			RenderWindowManager::Instance().NotifyFocusLost(coreWindow);
			break;
		}
	case WindowEventType::Minimized:
		{
			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.IsMaximized = false;
			}

			props.IsMaximized = false;

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			break;
		}
	case WindowEventType::Maximized:
		{
			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.IsMaximized = true;
			}

			props.IsMaximized = true;

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			break;
		}
	case WindowEventType::Restored:
		{
			{
				ScopedSpinLock lock(coreWindow->mLock);
				syncProps.IsMaximized = false;
			}

			props.IsMaximized = false;

			ct::RenderWindowManager::Instance().NotifySyncDataDirty(coreWindow);
			break;
		}
	case WindowEventType::MouseLeft:
		{
			RenderWindowManager::Instance().NotifyMouseLeft(coreWindow);
			break;
		}
	case WindowEventType::CloseRequested:
		{
			RenderWindowManager::Instance().NotifyCloseRequested(coreWindow);
			break;
		}
	}
}

void RenderWindow::OnExternalResizeInternal(u32 width, u32 height)
{
	RenderWindowProperties& props = GetMutableProperties();
	props.Width = width;
	props.Height = height;
	NotifyWindowEventInternal(WindowEventType::Resized);
}

void RenderWindow::OnExternalMoveInternal(i32 top, i32 left)
{
	RenderWindowProperties& props = GetMutableProperties();
	props.Top = top;
	props.Left = left;
	NotifyWindowEventInternal(WindowEventType::Moved);
}

void RenderWindow::OnExternalFocusInternal(bool focused)
{
	if(focused)
	{
		NotifyWindowEventInternal(WindowEventType::FocusReceived);
	}
	else
	{
		NotifyWindowEventInternal(WindowEventType::FocusLost);
	}
}

void RenderWindow::OnExternalMaximizedInternal(bool maximized)
{
	if(maximized)
	{
		NotifyWindowEventInternal(WindowEventType::Maximized);
	}
	else
	{
		NotifyWindowEventInternal(WindowEventType::Restored);
	}
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* RenderWindow::GetRttiStatic()
{
	return RenderWindowRTTI::Instance();
}

RTTITypeBase* RenderWindow::GetRtti() const
{
	return RenderWindow::GetRttiStatic();
}

namespace bs { namespace ct
{
RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
	: mDesc(desc), mWindowId(windowId)
{
	RenderWindowManager::Instance().WindowCreated(this);
}

RenderWindow::~RenderWindow()
{
	RenderWindowManager::Instance().WindowDestroyed(this);
}

void RenderWindow::SetHidden(bool hidden)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	RenderWindowProperties& props = const_cast<RenderWindowProperties&>(GetProperties());

	props.IsHidden = hidden;
	{
		ScopedSpinLock lock(mLock);
		GetSyncedProperties().IsHidden = hidden;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
}

void RenderWindow::SetActive(bool state)
{
	ASSERT_IF_NOT_RENDER_THREAD;
}

void RenderWindow::NotifyWindowEventInternal(WindowEventType type)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	RenderWindowProperties& syncProps = GetSyncedProperties();
	RenderWindowProperties& props = const_cast<RenderWindowProperties&>(GetProperties());

	switch(type)
	{
	case WindowEventType::Resized:
		{
			WindowMovedOrResizedInternal();

			{
				ScopedSpinLock lock(mLock);
				syncProps.Width = props.Width;
				syncProps.Height = props.Height;
			}

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);

			break;
		}
	case WindowEventType::Moved:
		{
			WindowMovedOrResizedInternal();

			{
				ScopedSpinLock lock(mLock);
				syncProps.Top = props.Top;
				syncProps.Left = props.Left;
			}

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);

			break;
		}
	case WindowEventType::FocusReceived:
		{
			{
				ScopedSpinLock lock(mLock);
				syncProps.HasFocus = true;
			}

			props.HasFocus = true;

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			bs::RenderWindowManager::Instance().NotifyFocusReceived(this);
			break;
		}
	case WindowEventType::FocusLost:
		{
			{
				ScopedSpinLock lock(mLock);
				syncProps.HasFocus = false;
			}

			props.HasFocus = false;

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			bs::RenderWindowManager::Instance().NotifyFocusLost(this);
			break;
		}
	case WindowEventType::Minimized:
		{
			{
				ScopedSpinLock lock(mLock);
				syncProps.IsMaximized = false;
			}

			props.IsMaximized = false;

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			break;
		}
	case WindowEventType::Maximized:
		{
			{
				ScopedSpinLock lock(mLock);
				syncProps.IsMaximized = true;
			}

			props.IsMaximized = true;

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			break;
		}
	case WindowEventType::Restored:
		{
			{
				ScopedSpinLock lock(mLock);
				syncProps.IsMaximized = false;
			}

			props.IsMaximized = false;

			bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
			break;
		}
	case WindowEventType::MouseLeft:
		{
			bs::RenderWindowManager::Instance().NotifyMouseLeft(this);
			break;
		}
	case WindowEventType::CloseRequested:
		{
			bs::RenderWindowManager::Instance().NotifyCloseRequested(this);
			break;
		}
	}
}

const RenderWindowProperties& RenderWindow::GetProperties() const
{
	return static_cast<const RenderWindowProperties&>(GetPropertiesInternal());
}
}}
