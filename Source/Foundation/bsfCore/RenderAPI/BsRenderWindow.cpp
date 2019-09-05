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

	void RenderWindow::destroy()
	{
		RenderWindowManager::instance().notifyWindowDestroyed(this);

		RenderTarget::destroy();
	}

	RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{

	}

	void RenderWindow::resize(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32)> resizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->resize(width, height);
		};

		gCoreThread().queueCommand(std::bind(resizeFunc, getCore(), width, height));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::move(INT32 left, INT32 top)
	{
		std::function<void(SPtr<ct::RenderWindow>, INT32, INT32)> moveFunc =
			[](SPtr<ct::RenderWindow> renderWindow, INT32 left, INT32 top)
		{
			renderWindow->move(left, top);
		};

		gCoreThread().queueCommand(std::bind(moveFunc, getCore(), left, top));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.left = syncedProps.left;
			mutableProps.top = syncedProps.top;
		}
	}

	void RenderWindow::hide()
	{
		std::function<void(SPtr<ct::RenderWindow>)> hideFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->setHidden(true);
		};

		getMutableProperties().isHidden = true;

		gCoreThread().queueCommand(std::bind(hideFunc, getCore()));
	}

	void RenderWindow::show()
	{
		std::function<void(SPtr<ct::RenderWindow>)> showFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->setHidden(false);
		};

		getMutableProperties().isHidden = false;

		gCoreThread().queueCommand(std::bind(showFunc, getCore()));
	}

	void RenderWindow::minimize()
	{
		std::function<void(SPtr<ct::RenderWindow>)> minimizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->minimize();
		};

		getMutableProperties().isMaximized = false;

		gCoreThread().queueCommand(std::bind(minimizeFunc, getCore()));
	}

	void RenderWindow::maximize()
	{
		std::function<void(SPtr<ct::RenderWindow>)> maximizeFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->maximize();
		};

		getMutableProperties().isMaximized = true;

		gCoreThread().queueCommand(std::bind(maximizeFunc, getCore()));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::restore()
	{
		std::function<void(SPtr<ct::RenderWindow>)> restoreFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->restore();
		};

		getMutableProperties().isMaximized = false;

		gCoreThread().queueCommand(std::bind(restoreFunc, getCore()));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32, float, UINT32)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
		{
			renderWindow->setFullscreen(width, height, refreshRate, monitorIdx);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), width, height, refreshRate, monitorIdx));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::setFullscreen(const VideoMode& mode)
	{
		std::function<void(SPtr<ct::RenderWindow>, const VideoMode&)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, const VideoMode& mode)
		{
			renderWindow->setFullscreen(mode);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), std::cref(mode)));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	void RenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32)> windowedFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->setWindowed(width, height);
		};

		gCoreThread().queueCommand(std::bind(windowedFunc, getCore(), width, height));
		gCoreThread().submit(true);

		{
			ScopedSpinLock lock(getCore()->mLock);
			const RenderWindowProperties& syncedProps = getCore()->getSyncedProperties();
			RenderWindowProperties& mutableProps = getMutableProperties();

			mutableProps.width = syncedProps.width;
			mutableProps.height = syncedProps.height;
		}
	}

	SPtr<ct::RenderWindow> RenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::RenderWindow>(mCoreSpecific);
	}

	SPtr<RenderWindow> RenderWindow::create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
	{
		return RenderWindowManager::instance().create(desc, parentWindow);
	}

	RenderWindowProperties& RenderWindow::getMutableProperties()
	{
		return const_cast<RenderWindowProperties&>(getProperties());
	}

	const RenderWindowProperties& RenderWindow::getProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}

	void RenderWindow::_notifyWindowEvent(WindowEventType type)
	{
		THROW_IF_CORE_THREAD;

		ct::RenderWindow* coreWindow = getCore().get();
		RenderWindowProperties& syncProps = coreWindow->getSyncedProperties();
		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		switch(type)
		{
			case WindowEventType::Resized:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.width = props.width;
					syncProps.height = props.height;
				}

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().notifyMovedOrResized(coreWindow);

				break;
			}
			case WindowEventType::Moved:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.top = props.top;
					syncProps.left = props.left;
				}

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().notifyMovedOrResized(coreWindow);

				break;
			}
			case WindowEventType::FocusReceived:
			{
				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.hasFocus = true;
				}

				props.hasFocus = true;

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().notifyFocusReceived(coreWindow);
				break;
			}
			case WindowEventType::FocusLost:
			{
				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.hasFocus = false;
				}

				props.hasFocus = false;

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				RenderWindowManager::instance().notifyFocusLost(coreWindow);
				break;
			}
			case WindowEventType::Minimized:
			{
				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::Maximized:
			{
				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.isMaximized = true;
				}

				props.isMaximized = true;

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::Restored:
			{
				{
					ScopedSpinLock lock(coreWindow->mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				ct::RenderWindowManager::instance().notifySyncDataDirty(coreWindow);
				break;
			}
			case WindowEventType::MouseLeft:
			{
				RenderWindowManager::instance().notifyMouseLeft(coreWindow);
				break;
			}
			case WindowEventType::CloseRequested:
			{
				RenderWindowManager::instance().notifyCloseRequested(coreWindow);
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
		return RenderWindowRTTI::instance();
	}

	RTTITypeBase* RenderWindow::getRTTI() const
	{
		return RenderWindow::getRTTIStatic();
	}

	namespace ct
	{
	RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{
		RenderWindowManager::instance().windowCreated(this);
	}

	RenderWindow::~RenderWindow()
	{
		RenderWindowManager::instance().windowDestroyed(this);
	}

	void RenderWindow::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.isHidden = hidden;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().isHidden = hidden;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindow::setActive(bool state)
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
					ScopedSpinLock lock(mLock);
					syncProps.width = props.width;
					syncProps.height = props.height;
				}

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				bs::RenderWindowManager::instance().notifyMovedOrResized(this);

				break;
			}
			case WindowEventType::Moved:
			{
				_windowMovedOrResized();

				{
					ScopedSpinLock lock(mLock);
					syncProps.top = props.top;
					syncProps.left = props.left;
				}

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				bs::RenderWindowManager::instance().notifyMovedOrResized(this);

				break;
			}
			case WindowEventType::FocusReceived:
			{
				{
					ScopedSpinLock lock(mLock);
					syncProps.hasFocus = true;
				}

				props.hasFocus = true;

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				bs::RenderWindowManager::instance().notifyFocusReceived(this);
				break;
			}
			case WindowEventType::FocusLost:
			{
				{
					ScopedSpinLock lock(mLock);
					syncProps.hasFocus = false;
				}

				props.hasFocus = false;

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				bs::RenderWindowManager::instance().notifyFocusLost(this);
				break;
			}
			case WindowEventType::Minimized:
			{
				{
					ScopedSpinLock lock(mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				break;
			}
			case WindowEventType::Maximized:
			{
				{
					ScopedSpinLock lock(mLock);
					syncProps.isMaximized = true;
				}

				props.isMaximized = true;

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				break;
			}
			case WindowEventType::Restored:
			{
				{
					ScopedSpinLock lock(mLock);
					syncProps.isMaximized = false;
				}

				props.isMaximized = false;

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
				break;
			}
			case WindowEventType::MouseLeft:
			{
				bs::RenderWindowManager::instance().notifyMouseLeft(this);
				break;
			}
			case WindowEventType::CloseRequested:
			{
				bs::RenderWindowManager::instance().notifyCloseRequested(this);
				break;
			}
		}
	}

	const RenderWindowProperties& RenderWindow::getProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}
	}
}
