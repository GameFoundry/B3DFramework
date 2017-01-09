//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderWindow.h"
#include "BsCoreThread.h"
#include "BsRenderWindowManager.h"
#include "BsViewport.h"
#include "BsPlatform.h"
#include "BsFrameAlloc.h"

namespace bs 
{
	RenderWindowProperties::RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
	{
		mWidth = desc.videoMode.getWidth();
		mHeight = desc.videoMode.getHeight();
		mHwGamma = desc.gamma;
		mVSync = desc.vsync;
		mVSyncInterval = desc.vsyncInterval;
		mMultisampleCount = desc.multisampleCount;
		mLeft = desc.left;
		mTop = desc.top;
		mIsFullScreen = desc.fullscreen;
		mHidden = desc.hidden;
		mIsModal = desc.modal;
		mIsWindow = true;
		mRequiresTextureFlipping = false;
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
		std::function<void(SPtr<ct::RenderWindowCore>, UINT32, UINT32)> resizeFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->resize(width, height);
		};

		getMutableProperties().mWidth = width;
		getMutableProperties().mHeight = height;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mWidth = width;
			getCore()->getSyncedProperties().mHeight = height;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		onResized();

		gCoreThread().queueCommand(std::bind(resizeFunc, getCore(), width, height));
	}

	void RenderWindow::move(INT32 left, INT32 top)
	{
		std::function<void(SPtr<ct::RenderWindowCore>, INT32, INT32)> moveFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow, INT32 left, INT32 top)
		{
			renderWindow->move(left, top);
		};

		getMutableProperties().mLeft = left;
		getMutableProperties().mTop = top;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mLeft = left;
			getCore()->getSyncedProperties().mTop = top;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(moveFunc, getCore(), left, top));
	}

	void RenderWindow::hide()
	{
		std::function<void(SPtr<ct::RenderWindowCore>)> hideFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow)
		{
			renderWindow->setHidden(true);
		};

		getMutableProperties().mHidden = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mHidden = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(hideFunc, getCore()));
	}

	void RenderWindow::show()
	{
		std::function<void(SPtr<ct::RenderWindowCore>)> showFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow)
		{
			renderWindow->setHidden(false);
		};

		getMutableProperties().mHidden = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mHidden = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(showFunc, getCore()));
	}

	void RenderWindow::minimize()
	{
		std::function<void(SPtr<ct::RenderWindowCore>)> minimizeFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow)
		{
			renderWindow->minimize();
		};

		getMutableProperties().mIsMaximized = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mIsMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(minimizeFunc, getCore()));
	}

	void RenderWindow::maximize()
	{
		std::function<void(SPtr<ct::RenderWindowCore>)> maximizeFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow)
		{
			renderWindow->maximize();
		};

		getMutableProperties().mIsMaximized = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mIsMaximized = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(maximizeFunc, getCore()));
	}

	void RenderWindow::restore()
	{
		std::function<void(SPtr<ct::RenderWindowCore>)> restoreFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow)
		{
			renderWindow->restore();
		};

		getMutableProperties().mIsMaximized = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().mIsMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		gCoreThread().queueCommand(std::bind(restoreFunc, getCore()));
	}

	void RenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		std::function<void(SPtr<ct::RenderWindowCore>, UINT32, UINT32, float, UINT32)> fullscreenFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow, UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
		{
			renderWindow->setFullscreen(width, height, refreshRate, monitorIdx);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), width, height, refreshRate, monitorIdx));
	}

	void RenderWindow::setFullscreen(const VideoMode& mode)
	{
		std::function<void(SPtr<ct::RenderWindowCore>, const VideoMode&)> fullscreenFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow, const VideoMode& mode)
		{
			renderWindow->setFullscreen(mode);
		};

		gCoreThread().queueCommand(std::bind(fullscreenFunc, getCore(), std::cref(mode)));
	}

	void RenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		std::function<void(SPtr<ct::RenderWindowCore>, UINT32, UINT32)> windowedFunc =
			[](SPtr<ct::RenderWindowCore> renderWindow, UINT32 width, UINT32 height)
		{
			renderWindow->setWindowed(width, height);
		};

		gCoreThread().queueCommand(std::bind(windowedFunc, getCore(), width, height));
	}

	SPtr<ct::RenderWindowCore> RenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::RenderWindowCore>(mCoreSpecific);
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
	RenderWindowCore::RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:mDesc(desc), mWindowId(windowId)
	{

	}

	RenderWindowCore::~RenderWindowCore()
	{
		Platform::resetNonClientAreas(*this);

		RenderWindowManager::instance().windowDestroyed(this);
	}

	void RenderWindowCore::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.mHidden = hidden;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mHidden = hidden;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindowCore::setActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.mActive = state;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mActive = state;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindowCore::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mTop = props.mTop;
			getSyncedProperties().mLeft = props.mLeft;
			getSyncedProperties().mWidth = props.mWidth;
			getSyncedProperties().mHeight = props.mHeight;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void RenderWindowCore::_windowFocusReceived()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& properties = const_cast<RenderWindowProperties&>(getProperties());
		properties.mHasFocus = true;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mHasFocus = true;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyFocusReceived(this);
	}

	void RenderWindowCore::_windowFocusLost()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& properties = const_cast<RenderWindowProperties&>(getProperties());
		properties.mHasFocus = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mHasFocus = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyFocusLost(this);
	}

	void RenderWindowCore::_notifyMaximized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.mIsMaximized = true;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mIsMaximized = true;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindowCore::_notifyMinimized()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.mIsMaximized = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mIsMaximized = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	void RenderWindowCore::_notifyRestored()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = const_cast<RenderWindowProperties&>(getProperties());

		props.mIsMaximized = false;
		{
			ScopedSpinLock lock(mLock);
			getSyncedProperties().mIsMaximized = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	const RenderWindowProperties& RenderWindowCore::getProperties() const
	{
		return static_cast<const RenderWindowProperties&>(getPropertiesInternal());
	}
	}
}
