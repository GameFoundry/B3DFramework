//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "Win32/BsWin32RenderWindow.h"
#include "BsInput.h"
#include "BsRenderAPI.h"
#include "BsCoreThread.h"
#include "BsException.h"
#include "Win32/BsWin32GLSupport.h"
#include "Win32/BsWin32Context.h"
#include "Win32/BsWin32Platform.h"
#include "Win32/BsWin32VideoModeInfo.h"
#include "BsGLPixelFormat.h"
#include "BsRenderWindowManager.h"
#include "Win32/BsWin32Window.h"
#include "BsMath.h"

GLenum GLEWAPIENTRY wglewContextInit(bs::ct::GLSupport* glSupport);

namespace bs
{
	#define _MAX_CLASS_NAME_ 128

	Win32RenderWindowProperties::Win32RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
		:RenderWindowProperties(desc)
	{ }

	Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::Win32GLSupport &glsupport)
		:RenderWindow(desc, windowId), mGLSupport(glsupport), mProperties(desc)
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

	SPtr<ct::Win32RenderWindow> Win32RenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::Win32RenderWindow>(mCoreSpecific);
	}

	void Win32RenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->mLock);
		mProperties = getCore()->mSyncedProperties;
	}

	HWND Win32RenderWindow::getHWnd() const
	{
		blockUntilCoreInitialized();
		return getCore()->_getHWnd();
	}

	namespace ct
	{
		Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, Win32GLSupport& glsupport)
		: RenderWindowCore(desc, windowId), mWindow(nullptr), mGLSupport(glsupport), mHDC(nullptr), mIsChild(false)
		, mDeviceName(nullptr), mDisplayFrequency(0), mShowOnSwap(false), mContext(nullptr), mProperties(desc)
		, mSyncedProperties(desc)
	{ }

		Win32RenderWindow::~Win32RenderWindow()
	{ 
		Win32RenderWindowProperties& props = mProperties;

		if (mWindow != nullptr)
		{
			ReleaseDC(mWindow->getHWnd(), mHDC);

			bs_delete(mWindow);
			mWindow = nullptr;
		}

		props.mActive = false;
		mHDC = nullptr;

		if (mDeviceName != nullptr)
		{
			bs_free(mDeviceName);
			mDeviceName = nullptr;
		}
	}

	void Win32RenderWindow::initialize()
	{
		Win32RenderWindowProperties& props = mProperties;

		props.mIsFullScreen = mDesc.fullscreen;
		mIsChild = false;
		mDisplayFrequency = Math::roundToInt(mDesc.videoMode.getRefreshRate());
		props.mColorDepth = 32;

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
		windowDesc.module = GetModuleHandle(MODULE_NAME);
#endif

		NameValuePairList::const_iterator opt;
		opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (HWND)parseUINT64(opt->second);

		opt = mDesc.platformSpecific.find("externalWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.external = (HWND)parseUINT64(opt->second);
		
		const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs > 0)
		{
			UINT32 actualMonitorIdx = std::min(mDesc.videoMode.getOutputIdx(), numOutputs - 1);
			const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));
			windowDesc.monitor = outputInfo.getMonitorHandle();
		}

		mIsChild = windowDesc.parent != nullptr;
		props.mIsFullScreen = mDesc.fullscreen && !mIsChild;
		props.mColorDepth = 32;
		props.mActive = true;

		if (!windowDesc.external)
		{
			mShowOnSwap = mDesc.hideUntilSwap;
			props.mHidden = mDesc.hideUntilSwap || mDesc.hidden;
		}

		mWindow = bs_new<Win32Window>(windowDesc);

		props.mWidth = mWindow->getWidth();
		props.mHeight = mWindow->getHeight();
		props.mTop = mWindow->getTop();
		props.mLeft = mWindow->getLeft();
		
		if (!windowDesc.external)
		{
			if (props.mIsFullScreen)
			{
				DEVMODE displayDeviceMode;

				memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
				displayDeviceMode.dmSize = sizeof(DEVMODE);
				displayDeviceMode.dmBitsPerPel = props.mColorDepth;
				displayDeviceMode.dmPelsWidth = props.mWidth;
				displayDeviceMode.dmPelsHeight = props.mHeight;
				displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if (mDisplayFrequency)
				{
					displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
					displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

					if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
					{
						BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings with user display frequency failed.");
					}
				}

				if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
				{
					BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed.");
				}
			}
		}

		mHDC = GetDC(mWindow->getHWnd());

		int testMultisample = props.mMultisampleCount;
		bool testHwGamma = mDesc.gamma;
		bool formatOk = mGLSupport.selectPixelFormat(mHDC, props.mColorDepth, testMultisample, testHwGamma, mDesc.depthBuffer);
		if (!formatOk)
		{
			if (props.mMultisampleCount > 0)
			{
				// Try without multisampling
				testMultisample = 0;
				formatOk = mGLSupport.selectPixelFormat(mHDC, props.mColorDepth, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk && mDesc.gamma)
			{
				// Try without sRGB
				testHwGamma = false;
				testMultisample = props.mMultisampleCount;
				formatOk = mGLSupport.selectPixelFormat(mHDC, props.mColorDepth, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk && mDesc.gamma && (props.mMultisampleCount > 0))
			{
				// Try without both
				testHwGamma = false;
				testMultisample = 0;
				formatOk = mGLSupport.selectPixelFormat(mHDC, props.mColorDepth, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk)
				BS_EXCEPT(RenderingAPIException, "Failed selecting pixel format.");

		}

		// Record what gamma option we used in the end
		// this will control enabling of sRGB state flags when used
		props.mHwGamma = testHwGamma;
		props.mMultisampleCount = testMultisample;

		mContext = mGLSupport.createContext(mHDC, nullptr);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowCore::initialize();
	}

	void Win32RenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs == 0)
			return;

		Win32RenderWindowProperties& props = mProperties;

		UINT32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
		const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));

		mDisplayFrequency = Math::roundToInt(refreshRate);
		props.mIsFullScreen = true;

		DEVMODE displayDeviceMode;

		memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
		displayDeviceMode.dmSize = sizeof(DEVMODE);
		displayDeviceMode.dmBitsPerPel = props.mColorDepth;
		displayDeviceMode.dmPelsWidth = width;
		displayDeviceMode.dmPelsHeight = height;
		displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
		displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

		HMONITOR hMonitor = outputInfo.getMonitorHandle();
		MONITORINFOEX monitorInfo;

		memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &monitorInfo);

		if (ChangeDisplaySettingsEx(monitorInfo.szDevice, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
		{
			BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed");
		}

		props.mTop = monitorInfo.rcMonitor.top;
		props.mLeft = monitorInfo.rcMonitor.left;
		props.mWidth = width;
		props.mHeight = height;

		SetWindowLong(mWindow->getHWnd(), GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		SetWindowLong(mWindow->getHWnd(), GWL_EXSTYLE, 0);

		SetWindowPos(mWindow->getHWnd(), HWND_TOP, props.mLeft, props.mTop, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		_windowMovedOrResized();
	}

	void Win32RenderWindow::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		setFullscreen(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode.getOutputIdx());
	}

	void Win32RenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		Win32RenderWindowProperties& props = mProperties;

		if (!props.mIsFullScreen)
			return;

		props.mIsFullScreen = false;
		props.mWidth = width;
		props.mHeight = height;

		// Drop out of fullscreen
		ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

		UINT32 winWidth = width;
		UINT32 winHeight = height;

		RECT rect;
		SetRect(&rect, 0, 0, winWidth, winHeight);

		AdjustWindowRect(&rect, mWindow->getStyle(), false);
		winWidth = rect.right - rect.left;
		winHeight = rect.bottom - rect.top;

		// Deal with centering when switching down to smaller resolution
		HMONITOR hMonitor = MonitorFromWindow(mWindow->getHWnd(), MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		memset(&monitorInfo, 0, sizeof(MONITORINFO));
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);

		LONG screenw = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
		LONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

		INT32 left = screenw > INT32(winWidth) ? ((screenw - INT32(winWidth)) / 2) : 0;
		INT32 top = screenh > INT32(winHeight) ? ((screenh - INT32(winHeight)) / 2) : 0;

		SetWindowLong(mWindow->getHWnd(), GWL_STYLE, mWindow->getStyle() | WS_VISIBLE);
		SetWindowLong(mWindow->getHWnd(), GWL_EXSTYLE, mWindow->getStyleEx());
		SetWindowPos(mWindow->getHWnd(), HWND_NOTOPMOST, left, top, winWidth, winHeight,
			SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.mWidth = props.mWidth;
			mSyncedProperties.mHeight = props.mHeight;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		_windowMovedOrResized();
	}

	void Win32RenderWindow::move(INT32 left, INT32 top)
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

	void Win32RenderWindow::resize(UINT32 width, UINT32 height)
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

	void Win32RenderWindow::minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->minimize();
	}

	void Win32RenderWindow::maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->maximize();
	}

	void Win32RenderWindow::restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->restore();
	}

	void Win32RenderWindow::swapBuffers(UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		SwapBuffers(mHDC);
	}

	void Win32RenderWindow::copyToMemory(PixelData &dst, FrameBuffer buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if ((dst.getRight() > getProperties().getWidth()) ||
			(dst.getBottom() > getProperties().getHeight()) ||
			(dst.getFront() != 0) || (dst.getBack() != 1))
		{
			BS_EXCEPT(InvalidParametersException, "Invalid box.");
		}

		if (buffer == FB_AUTO)
		{
			buffer = mProperties.isFullScreen() ? FB_FRONT : FB_BACK;
		}

		GLenum format = GLPixelUtil::getGLOriginFormat(dst.getFormat());
		GLenum type = GLPixelUtil::getGLOriginDataType(dst.getFormat());

		if ((format == GL_NONE) || (type == 0))
		{
			BS_EXCEPT(InvalidParametersException, "Unsupported format.");
		}

		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
		glReadPixels((GLint)dst.getLeft(), (GLint)dst.getTop(),
					 (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
					 format, type, dst.getData());

		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		//vertical flip
		{
			size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.getFormat());
			size_t height = dst.getHeight();
			UINT8* tmpData = (UINT8*)bs_alloc((UINT32)(rowSpan * height));
			UINT8* srcRow = (UINT8 *)dst.getData(), *tmpRow = tmpData + (height - 1) * rowSpan;

			while (tmpRow >= tmpData)
			{
				memcpy(tmpRow, srcRow, rowSpan);
				srcRow += rowSpan;
				tmpRow -= rowSpan;
			}
			memcpy(dst.getData(), tmpData, rowSpan * height);

			bs_free(tmpData);
		}
	}

	void Win32RenderWindow::getCustomAttribute(const String& name, void* pData) const
	{
		if(name == "GLCONTEXT") 
		{
			SPtr<GLContext>* contextPtr = static_cast<SPtr<GLContext>*>(pData);
			*contextPtr = mContext;
			return;
		} 
		else if(name == "WINDOW")
		{
			UINT64 *pHwnd = (UINT64*)pData;
			*pHwnd = (UINT64)_getHWnd();
			return;
		} 
	}

	void Win32RenderWindow::setActive(bool state)
	{	
		THROW_IF_NOT_CORE_THREAD;

		mWindow->setActive(state);

		RenderWindowCore::setActive(state);
	}

	void Win32RenderWindow::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		mShowOnSwap = false;
		mWindow->setHidden(hidden);

		RenderWindowCore::setHidden(hidden);
	}

	void Win32RenderWindow::_windowMovedOrResized()
	{
		if (!mWindow)
			return;

		mWindow->_windowMovedOrResized();

		Win32RenderWindowProperties& props = mProperties;
		if (!props.mIsFullScreen) // Fullscreen is handled directly by this object
		{
			props.mTop = mWindow->getTop();
			props.mLeft = mWindow->getLeft();
			props.mWidth = mWindow->getWidth();
			props.mHeight = mWindow->getHeight();
		}

		RenderWindowCore::_windowMovedOrResized();
	}

	HWND Win32RenderWindow::_getHWnd() const
	{
		return mWindow->getHWnd();
	}

	void Win32RenderWindow::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}		
	}
}