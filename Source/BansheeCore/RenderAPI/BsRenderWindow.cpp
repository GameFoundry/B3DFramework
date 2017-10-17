//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "RenderAPI/BsRenderWindow.h"
#include "CoreThread/BsCoreThread.h"
#include "Managers/BsRenderWindowManager.h"
#include "RenderAPI/BsViewport.h"
#include "Platform/BsPlatform.h"
#include "Allocators/BsFrameAlloc.h"

namespace bs 
{
	RenderWindowProperties::RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
	{
		width = desc.videoMode.getWidth();
		height = desc.videoMode.getHeight();
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

		getMutableProperties().width = width;
		getMutableProperties().height = height;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().width = width;
			getCore()->getSyncedProperties().height = height;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		onResized();

		gCoreThread().queueCommand(std::bind(resizeFunc, getCore(), width, height));
	}

	void RenderWindow::move(INT32 left, INT32 top)
	{
		std::function<void(SPtr<ct::RenderWindow>, INT32, INT32)> moveFunc =
			[](SPtr<ct::RenderWindow> renderWindow, INT32 left, INT32 top)
		{
			renderWindow->move(left, top);
		};

		getMutableProperties().left = left;
		getMutableProperties().top = top;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().left = left;
			getCore()->getSyncedProperties().top = top;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(moveFunc, getCore(), left, top));
	}

	void RenderWindow::hide()
	{
		std::function<void(SPtr<ct::RenderWindow>)> hideFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->setHidden(true);
		};

		getMutableProperties().isHidden = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(maximizeFunc, getCore()));
	}

	void RenderWindow::restore()
	{
		std::function<void(SPtr<ct::RenderWindow>)> restoreFunc =
			[](SPtr<ct::RenderWindow> renderWindow)
		{
			renderWindow->restore();
		};

		getMutableProperties().isMaximized = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(restoreFunc, getCore()));
	}

	void RenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32, float, UINT32)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
		{
			renderWindow->setFullscreen(width, height, refreshRate, monitorIdx);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), width, height, refreshRate, monitorIdx));
	}

	void RenderWindow::setFullscreen(const VideoMode& mode)
	{
		std::function<void(SPtr<ct::RenderWindow>, const VideoMode&)> fullscreenFunc =
			[](SPtr<ct::RenderWindow> renderWindow, const VideoMode& mode)
		{
			renderWindow->setFullscreen(mode);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), std::cref(mode)));
	}

	void RenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindow>, UINT32, UINT32)> windowedFunc =
			[](SPtr<ct::RenderWindow> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->setWindowed(width, height);
		};

		gCoreThread().queueCommand(std::bind(windowedFunc, getCore(), width, height));
	}

	SPtr<ct::RenderWindow> RenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::RenderWindow>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> RenderWindow::createCore() const
	{
		RENDER_WINDOW_DESC desc = mDesc;
		return ct::RenderWindowManager::instance().createInternal(desc, mWindowId);
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

	namespace ct
	{
	RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{

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

	void RenderWindow::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().top = props.top;
			getSyncedProperties().left = props.left;
			getSyncedProperties().width = props.width;
			getSyncedProperties().height = props.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void RenderWindow::_windowFocusReceived()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& properties = const_cast<RenderWindowProperties&>(getProperties());
		properties.hasFocus = true;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().hasFocus = true;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyFocusReceived(this);
	}

	void RenderWindow::_windowFocusLost()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& properties = const_cast<RenderWindowProperties&>(getProperties());
		properties.hasFocus = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().hasFocus = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyFocusLost(this);
	}

	void RenderWindow::_notifyMaximized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.isMaximized = true;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().isMaximized = true;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindow::_notifyMinimized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.isMaximized = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().isMaximized = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindow::_notifyRestored()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.isMaximized = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().isMaximized = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindow::_notifyMouseLeft()
	{
		THROW_IF_NOT_CORE_THREAD;

		bs::RenderWindowManager::instance().notifyMouseLeft(this);
	}

	const RenderWindowProperties& RenderWindow::getProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}
	}
}
