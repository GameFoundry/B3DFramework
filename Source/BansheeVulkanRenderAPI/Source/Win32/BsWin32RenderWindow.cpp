//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Win32/BsWin32RenderWindow.h"
#include "Win32/BsWin32Platform.h"
#include "Win32/BsWin32Window.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsRenderWindowManager.h"

namespace BansheeEngine
{
	Win32RenderWindowProperties::Win32RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
		:RenderWindowProperties(desc)
	{ }

	Win32RenderWindowCore::Win32RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		: RenderWindowCore(desc, windowId), mProperties(desc), mSyncedProperties(desc), mWindow(nullptr)
	{ }

	Win32RenderWindowCore::~Win32RenderWindowCore()
	{ 
		if (mWindow != nullptr)
		{
			bs_delete(mWindow);
			mWindow = nullptr;
		}
	}

	void Win32RenderWindowCore::initialize()
	{
		Win32RenderWindowProperties& props = mProperties;

		WINDOW_DESC windowDesc;
		windowDesc.border = mDesc.border;
		windowDesc.enableDoubleClick = mDesc.enableDoubleClick;
		windowDesc.fullscreen = mDesc.fullscreen;
		windowDesc.width = mDesc.videoMode.getWidth();
		windowDesc.height = mDesc.videoMode.getHeight();
		windowDesc.hidden = mDesc.hidden || mDesc.hideUntilSwap;
		windowDesc.left = mDesc.left;
		windowDesc.top = mDesc.top;
		windowDesc.outerDimensions = mDesc.outerDimensions;
		windowDesc.title = mDesc.title;
		windowDesc.toolWindow = mDesc.toolWindow;
		windowDesc.creationParams = this;
		windowDesc.modal = mDesc.modal;
		windowDesc.wndProc = &Win32Platform::_win32WndProc;

#ifdef BS_STATIC_LIB
		windowDesc.module = GetModuleHandle(NULL);
#else
		windowDesc.module = GetModuleHandle("BansheeVulkanRenderAPI.dll");
#endif

		NameValuePairList::const_iterator opt;
		opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (HWND)parseUINT64(opt->second);

		opt = mDesc.platformSpecific.find("externalWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.external = (HWND)parseUINT64(opt->second);
		
		mWindow = bs_new<Win32Window>(windowDesc);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowCore::initialize();
	}

	void Win32RenderWindowCore::swapBuffers()
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void Win32RenderWindowCore::move(INT32 left, INT32 top)
	{
		THROW_IF_NOT_CORE_THREAD;

		Win32RenderWindowProperties& props = mProperties;

		if (!props.mIsFullScreen)
		{
			mWindow->move(left, top);

			props.mTop = mWindow->getTop();
			props.mLeft = mWindow->getLeft();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.mTop = props.mTop;
				mSyncedProperties.mLeft = props.mLeft;
			}

			RenderWindowManager::instance().notifySyncDataDirty(this);
		}
	}

	void Win32RenderWindowCore::resize(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		Win32RenderWindowProperties& props = mProperties;

		if (!props.mIsFullScreen)
		{
			mWindow->resize(width, height);

			props.mWidth = mWindow->getWidth();
			props.mHeight = mWindow->getHeight();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.mWidth = props.mWidth;
				mSyncedProperties.mHeight = props.mHeight;
			}

			RenderWindowManager::instance().notifySyncDataDirty(this);
		}
	}

	void Win32RenderWindowCore::setActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->setActive(state);

		RenderWindowCore::setActive(state);
	}

	void Win32RenderWindowCore::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->setHidden(hidden);

		RenderWindowCore::setHidden(hidden);
	}

	void Win32RenderWindowCore::minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->minimize();
	}

	void Win32RenderWindowCore::maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->maximize();
	}

	void Win32RenderWindowCore::restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->restore();
	}

	void Win32RenderWindowCore::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void Win32RenderWindowCore::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void Win32RenderWindowCore::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	HWND Win32RenderWindowCore::_getWindowHandle() const
	{
		return mWindow->getHWnd();
	}

	void Win32RenderWindowCore::getCustomAttribute(const String& name, void* pData) const
	{
		if(name == "WINDOW")
		{
			UINT64 *pWnd = (UINT64*)pData;
			*pWnd = (UINT64)mWindow->getHWnd();
			return;
		}

		RenderWindowCore::getCustomAttribute(name, pData);
	}

	void Win32RenderWindowCore::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (!mWindow)
			return;

		mWindow->_windowMovedOrResized();

		RenderWindowCore::_windowMovedOrResized();
	}

	void Win32RenderWindowCore::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}

	Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:RenderWindow(desc, windowId), mProperties(desc)
	{

	}

	void Win32RenderWindow::getCustomAttribute(const String& name, void* pData) const
	{
		if (name == "WINDOW")
		{
			UINT64 *pHwnd = (UINT64*)pData;
			*pHwnd = (UINT64)getHWnd();
			return;
		}
	}

	Vector2I Win32RenderWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		POINT pos;
		pos.x = screenPos.x;
		pos.y = screenPos.y;

		ScreenToClient(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	Vector2I Win32RenderWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		POINT pos;
		pos.x = windowPos.x;
		pos.y = windowPos.y;

		ClientToScreen(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	SPtr<Win32RenderWindowCore> Win32RenderWindow::getCore() const
	{
		return std::static_pointer_cast<Win32RenderWindowCore>(mCoreSpecific);
	}

	HWND Win32RenderWindow::getHWnd() const
	{
		blockUntilCoreInitialized();
		return getCore()->_getWindowHandle();
	}

	void Win32RenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->mLock);
		mProperties = getCore()->mSyncedProperties;
	}
}