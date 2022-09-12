//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderWindow.h"
#include "CoreThread/BsCoreThread.h"
#include "Managers/BsRenderWindowManager.h"
#include "RenderAPI/BsViewport.h"
#include "Platform/BsPlatform.h"
#include "Private/RTTI/BsRenderTargetRTTI.h"

namespace bs
{
	RenderWindowProperties::RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
	{
		width = desc.videoMode.width;
		height = desc.videoMode.height;
		hwGamma = desc.gamma;
		vsync = desc.vsync;
		vsyncInterval = desc.vsyncInterval;
		multisampleCount = desc.multisampleCount;
		left = desc.left;
		top = desc.top;
		isFullScreen = desc.fullscreen;
		isHidden = desc.hidden;
		isModal = desc.modal;
		isWindow = true;
		requiresTextureFlipping = false;
	}

	void RenderWindow::Destroy()
	{
		RenderWindowManager::instance().NotifyWindowDestroyed(this);

		RenderTarget::destroy();
	}

	RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{

	}

	void RenderWindow::Resize(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32)> resizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->Resize(width, height);
		};

		gCoreThread().QueueCommand(std::bind(resizeFunc, getCore(), width, height));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::Move(INT32 left, INT32 top)
	{
		std::function<void(SPtr<ct::RenderWindow>, INT32, INT32)> moveFunc =
			[](SPtr<ct::RenderWindow> renderWindow, INT32 left, INT32 top)
		{
			renderWindow->Move(left, top);
		};

		gCoreThread().QueueCommand(std::bind(moveFunc, getCore(), left, top));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.left = syncedProps.left;
			mutableProps.top = syncedProps.top;
		}
	}

	void RenderWindow::Hide()
	{
		std::function<void(SPtr<ct::RenderWindow>)> hideFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->SetHidden(true);
		};

		getMutableProperties().isHidden = true;

		gCoreThread().QueueCommand(std::bind(hideFunc, getCore()));
	}

	void RenderWindow::Show()
	{
		std::function<void(SPtr<ct::RenderWindow>)> showFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->SetHidden(false);
		};

		getMutableProperties().isHidden = false;

		gCoreThread().QueueCommand(std::bind(showFunc, getCore()));
	}

	void RenderWindow::Minimize()
	{
		std::function<void(SPtr<ct::RenderWindow>)> minimizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->Minimize();
		};

		getMutableProperties().isMaximized = false;

		gCoreThread().QueueCommand(std::bind(minimizeFunc, getCore()));
	}

	void RenderWindow::Maximize()
	{
		std::function<void(SPtr<ct::RenderWindow>)> maximizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->Maximize();
		};

		getMutableProperties().isMaximized = true;

		gCoreThread().QueueCommand(std::bind(maximizeFunc, getCore()));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::Restore()
	{
		std::function<void(SPtr<ct::RenderWindow>)> restoreFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->Restore();
		};

		getMutableProperties().isMaximized = false;

		gCoreThread().QueueCommand(std::bind(restoreFunc, getCore()));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::SetFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32, float, UINT32)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
		{
			renderWindow->SetFullscreen(width, height, refreshRate, monitorIdx);
		};

		gCoreThread().QueueCommand(std::bind(fullscreenFunc, getCore(), width, height, refreshRate, monitorIdx));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::SetFullscreen(const VideoMode& mode)
	{
		std::function<void(SPtr<ct::RenderWindow>, const VideoMode&)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, const VideoMode& mode)
		{
			renderWindow->SetFullscreen(mode);
		};

		gCoreThread().QueueCommand(std::bind(fullscreenFunc, getCore(), std::cref(mode)));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::SetWindowed(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32)> windowedFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->SetWindowed(width, height);
		};

		gCoreThread().QueueCommand(std::bind(windowedFunc, getCore(), width, height));
		gCoreThread().Submit(true);

		{
			ScopedSpinLock Lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->GetSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	SPtr<ct::RenderWindow> RenderWindow::GetCore() const
	{
		return std::static_pointer_cast<ct::RenderWindow>(mCoreSpecific);
	}

	SPtr<RenderWindow> RenderWindow::Create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
	{
		return RenderWindowManager::Instance().Create(desc, parentWindow);
	}

	RenderWindowProperties& RenderWindow::GetMutableProperties()
	{
		return const_cast<RenderWindowProperties&>(getProperties());
	}

	const RenderWindowProperties& RenderWindow::GetProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}

	void RenderWindow::_notifyWindowEvent(WindowEventType type)
	{
		THROW_IF_CORE_THREAD;

		ct::RenderWindow* coreWindow = getCore().Get();
		RenderWindowProperties& syncProps = coreWindow->GetSyncedProperties();
		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		switch(type)
		{
			case WindowEventType::Resized:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.width = props.width;
					syncProps.height = props.height;
				}

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().NotifyMovedOrResized(coreWindow);

				break;
			}
			case WindowEventType::Moved:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.top = props.top;
					syncProps.left = props.left;
				}

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().NotifyMovedOrResized(coreWindow);

				break;
			}
			case WindowEventType::FocusReceived:
			{
				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.hasFocus = true;
				}

				props.hasFocus = true;

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().NotifyFocusReceived(coreWindow);
				break;
			}
			case WindowEventType::FocusLost:
			{
				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.hasFocus = false;
				}

				props.hasFocus = false;

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().NotifyFocusLost(coreWindow);
				break;
			}
			case WindowEventType::Minimized:
			{
				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::Maximized:
			{
				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.isMaximized = true;
				}

				props.isMaximized = true;

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::Restored:
			{
				{
					ScopedSpinLock Lock(coreWindow->mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				ct::RenderWindowManager::instance().NotifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::MouseLeft:
			{
				RenderWindowManager::instance().NotifyMouseLeft(coreWindow);
				break;
			}
			case WindowEventType::CloseRequested:
			{
				RenderWindowManager::instance().NotifyCloseRequested(coreWindow);
				break;
			}
		}
	}

	void RenderWindow::_onExternalResize(UINT32 width, UINT32 height)
	{
		RenderWindowProperties& props = getMutableProperties();
		props.width = width;
		props.height = height;
		_notifyWindowEvent(WindowEventType::Resized);
	}

	void RenderWindow::_onExternalMove(INT32 top, INT32 left)
	{
		RenderWindowProperties& props = getMutableProperties();
		props.top = top;
		props.left = left;
		_notifyWindowEvent(WindowEventType::Moved);
	}

	void RenderWindow::_onExternalFocus(bool focused)
	{
		if(focused)
		{
			_notifyWindowEvent(WindowEventType::FocusReceived);
		}
		else
		{
			_notifyWindowEvent(WindowEventType::FocusLost);
		}
	}

	void RenderWindow::_onExternalMaximized(bool maximized)
	{
		if(maximized)
		{
			_notifyWindowEvent(WindowEventType::Maximized);
		}
		else
		{
			_notifyWindowEvent(WindowEventType::Restored);
		}
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* RenderWindow::getRTTIStatic()
	{
		return RenderWindowRTTI::Instance();
	}

	RTTITypeBase* RenderWindow::getRTTI() const
	{
		return RenderWindow::GetRTTIStatic();
	}

	namespace ct
	{
	RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{
		RenderWindowManager::instance().WindowCreated(this);
	}

	RenderWindow::~RenderWindow()
	{
		RenderWindowManager::instance().WindowDestroyed(this);
	}

	void RenderWindow::SetHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.isHidden = hidden;
		{
			ScopedSpinLock Lock(mLock);
			getSyncedProperties().isHidden = hidden;
		}

		bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
	}

	void RenderWindow::SetActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void RenderWindow::_notifyWindowEvent(WindowEventType type)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& syncProps = getSyncedProperties();
		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		switch(type)
		{
			case WindowEventType::Resized:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock Lock(mLock);
					syncProps.width = props.width;
					syncProps.height = props.height;
				}

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				bs::RenderWindowManager::instance().NotifyMovedOrResized(this);

				break;
			}
			case WindowEventType::Moved:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock Lock(mLock);
					syncProps.top = props.top;
					syncProps.left = props.left;
				}

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				bs::RenderWindowManager::instance().NotifyMovedOrResized(this);

				break;
			}
			case WindowEventType::FocusReceived:
			{
				{
					ScopedSpinLock Lock(mLock);
					syncProps.hasFocus = true;
				}

				props.hasFocus = true;

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				bs::RenderWindowManager::instance().NotifyFocusReceived(this);
				break;
			}
			case WindowEventType::FocusLost:
			{
				{
					ScopedSpinLock Lock(mLock);
					syncProps.hasFocus = false;
				}

				props.hasFocus = false;

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				bs::RenderWindowManager::instance().NotifyFocusLost(this);
				break;
			}
			case WindowEventType::Minimized:
			{
				{
					ScopedSpinLock Lock(mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				break;
			}
			case WindowEventType::Maximized:
			{
				{
					ScopedSpinLock Lock(mLock);
					syncProps.isMaximized = true;
				}

				props.isMaximized = true;

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				break;
			}
			case WindowEventType::Restored:
			{
				{
					ScopedSpinLock Lock(mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
				break;
			}
			case WindowEventType::MouseLeft:
			{
				bs::RenderWindowManager::instance().NotifyMouseLeft(this);
				break;
			}
			case WindowEventType::CloseRequested:
			{
				bs::RenderWindowManager::instance().NotifyCloseRequested(this);
				break;
			}
		}
	}

	const RenderWindowProperties& RenderWindow::GetProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}
	}
}
