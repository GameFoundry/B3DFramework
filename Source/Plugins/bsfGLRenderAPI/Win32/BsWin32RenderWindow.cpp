//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "Win32/BsWin32RenderWindow.h"
#include "Input/BsInput.h"
#include "Renderapi/BsRenderAPI.h"
#include "Corethread/BsCoreThread.h"
#include "Error/BsException.h"
#include "Win32/BsWin32GLSupport.h"
#include "Win32/BsWin32Context.h"
#include "Win32/BsWin32VideoModeInfo.h"
#include "BsGLPixelFormat.h"
#include "Managers/BsRenderWindowManager.h"
#include "Private/Win32/BsWin32Platform.h"
#include "Private/Win32/BsWin32Window.h"
#include "Math/BsMath.h"

GLenum GLEWAPIENTRY wglewContextInit(bs::ct::GLSupport* glSupport);

namespace bs
{
	#define _MAX_CLASS_NAME_ 128

	Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::Win32GLSupport &glsupport)
		:RenderWindow(desc, windowId), mGLSupport(glsupport), mProperties(desc)
	{

	}

	Win32RenderWindow::~Win32RenderWindow()
	{
	}

	void Win32RenderWindow::GetCustomAttribute(const String& name, void* pData) const
	{
		if (name == "WINDOW")
		{
			UINT64 *pHwnd = (UINT64*)pData;
			*pHwnd = (UINT64)getHWnd();
			return;
		}
	}

	Vector2I Win32RenderWindow::ScreenToWindowPos(const Vector2I& screenPos) const
	{
		POINT pos;
		pos.x = screenPos.x;
		pos.y = screenPos.y;

		ScreenToClient(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	Vector2I Win32RenderWindow::WindowToScreenPos(const Vector2I& windowPos) const
	{
		POINT pos;
		pos.x = windowPos.x;
		pos.y = windowPos.y;

		ClientToScreen(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	SPtr<ct::Win32RenderWindow> Win32RenderWindow::GetCore() const
	{
		return std::static_pointer_cast<ct::Win32RenderWindow>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Win32RenderWindow::CreateCore() const
	{
		RENDER_WINDOW_DESC desc = mDesc;
		SPtr<ct::Win32RenderWindow> coreObj = bs_shared_ptr_new<ct::Win32RenderWindow>(desc, mWindowId, mGLSupport);
		coreObj->_setThisPtr(coreObj);

		mGLSupport._notifyWindowCreated(coreObj.Get());
		return coreObj;
	}

	void Win32RenderWindow::SyncProperties()
	{
		ScopedSpinLock lock(getCore()->mLock);
		mProperties = getCore()->mSyncedProperties;
	}

	HWND Win32RenderWindow::GetHWnd() const
	{
		blockUntilCoreInitialized();
		return GetCore()->_getHWnd();
	}

	namespace ct
	{
	Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, Win32GLSupport& glsupport)
		: RenderWindow(desc, windowId), mWindow(nullptr), mGLSupport(glsupport), mHDC(nullptr), mIsChild(false)
		, MDeviceName(nullptr), mDisplayFrequency(0), mShowOnSwap(false), mContext(nullptr), mProperties(desc)
		, MSyncedProperties(desc)
	{ }

	Win32RenderWindow::~Win32RenderWindow()
	{
		RenderWindowProperties& props = mProperties;

		if (mWindow != nullptr)
		{
			ReleaseDC(mWindow->GetHWnd(), mHDC);

			bs_delete(mWindow);
			mWindow = nullptr;
		}

		mHDC = nullptr;

		if (mDeviceName != nullptr)
		{
			bs_free(mDeviceName);
			mDeviceName = nullptr;
		}

		Platform::resetNonClientAreas(*this);
	}

	void Win32RenderWindow::Initialize()
	{
		RenderWindowProperties& props = mProperties;

		props.isFullScreen = mDesc.fullscreen;
		mIsChild = false;
		mDisplayFrequency = Math::RoundToInt(mDesc.videoMode.refreshRate);

		WINDOW_DESC windowDesc;
		windowDesc.showTitleBar = mDesc.showTitleBar;
		windowDesc.showBorder = mDesc.showBorder;
		windowDesc.allowResize = mDesc.allowResize;
		windowDesc.enableDoubleClick = true;
		windowDesc.fullscreen = mDesc.fullscreen;
		windowDesc.width = mDesc.videoMode.width;
		windowDesc.height = mDesc.videoMode.height;
		windowDesc.hidden = mDesc.hidden || mDesc.hideUntilSwap;
		windowDesc.left = mDesc.left;
		windowDesc.top = mDesc.top;
		windowDesc.outerDimensions = false;
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

		auto opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (HWND)parseUINT64(opt->second);

		opt = mDesc.platformSpecific.find("externalWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.external = (HWND)parseUINT64(opt->second);
		
		const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPI::instance().GetVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.GetNumOutputs();
		if (numOutputs > 0)
		{
			UINT32 actualMonitorIdx = std::min(mDesc.videoMode.outputIdx, numOutputs - 1);
			const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));
			windowDesc.monitor = outputInfo.GetMonitorHandle();
		}

		mIsChild = windowDesc.parent != nullptr;
		props.isFullScreen = mDesc.fullscreen && !mIsChild;

		if (!windowDesc.external)
		{
			mShowOnSwap = mDesc.hideUntilSwap && !mDesc.hidden;
			props.isHidden = mDesc.hideUntilSwap || mDesc.hidden;
		}

		mWindow = bs_new<Win32Window>(windowDesc);

		props.width = mWindow->GetWidth();
		props.height = mWindow->GetHeight();
		props.top = mWindow->GetTop();
		props.left = mWindow->GetLeft();
		
		if (!windowDesc.external)
		{
			if (props.isFullScreen)
			{
				DEVMODE displayDeviceMode;

				memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
				displayDeviceMode.dmSize = sizeof(DEVMODE);
				displayDeviceMode.dmBitsPerPel = 32;
				displayDeviceMode.dmPelsWidth = props.width;
				displayDeviceMode.dmPelsHeight = props.height;
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

		mHDC = GetDC(mWindow->GetHWnd());

		int testMultisample = props.multisampleCount;
		bool testHwGamma = mDesc.gamma;
		bool formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.depthBuffer);
		if (!formatOk)
		{
			if (props.multisampleCount > 0)
			{
				// Try without multisampling
				testMultisample = 0;
				formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk && mDesc.gamma)
			{
				// Try without sRGB
				testHwGamma = false;
				testMultisample = props.multisampleCount;
				formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk && mDesc.gamma && (props.multisampleCount > 0))
			{
				// Try without both
				testHwGamma = false;
				testMultisample = 0;
				formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.depthBuffer);
			}

			if (!formatOk)
				BS_EXCEPT(RenderingAPIException, "Failed selecting pixel format.");

		}

		// Record what gamma option we used in the end
		// this will control enabling of sRGB state flags when used
		props.hwGamma = testHwGamma;
		props.multisampleCount = testMultisample;

		mContext = mGLSupport.CreateContext(mHDC, nullptr);

		if (props.vsync && props.vsyncInterval > 0)
			setVSync(true, props.vsyncInterval);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
		RenderWindow::initialize();
	}

	void Win32RenderWindow::SetFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPI::instance().GetVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.GetNumOutputs();
		if (numOutputs == 0)
			return;

		RenderWindowProperties& props = mProperties;

		UINT32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
		const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

		mDisplayFrequency = Math::RoundToInt(refreshRate);
		props.isFullScreen = true;

		DEVMODE displayDeviceMode;

		memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
		displayDeviceMode.dmSize = sizeof(DEVMODE);
		displayDeviceMode.dmBitsPerPel = 32;
		displayDeviceMode.dmPelsWidth = width;
		displayDeviceMode.dmPelsHeight = height;
		displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
		displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

		HMONITOR hMonitor = outputInfo.GetMonitorHandle();
		MONITORINFOEX monitorInfo;

		memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &monitorInfo);

		if (ChangeDisplaySettingsEx(monitorInfo.szDevice, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
		{
			BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed");
		}

		props.top = monitorInfo.rcMonitor.top;
		props.left = monitorInfo.rcMonitor.left;
		props.width = width;
		props.height = height;

		SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, 0);

		SetWindowPos(mWindow->GetHWnd(), HWND_TOP, props.left, props.top, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		_windowMovedOrResized();

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.top = props.top;
			mSyncedProperties.left = props.left;
			mSyncedProperties.width = props.width;
			mSyncedProperties.height = props.height;
		}

		bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
		bs::RenderWindowManager::instance().NotifyMovedOrResized(this);
	}

	void Win32RenderWindow::SetFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		setFullscreen(mode.width, mode.height, mode.refreshRate, mode.outputIdx);
	}

	void Win32RenderWindow::SetWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
			return;

		props.isFullScreen = false;
		props.width = width;
		props.height = height;

		// Drop out of fullscreen
		ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

		UINT32 winWidth = width;
		UINT32 winHeight = height;

		RECT rect;
		SetRect(&rect, 0, 0, winWidth, winHeight);

		AdjustWindowRect(&rect, mWindow->GetStyle(), false);
		winWidth = rect.right - rect.left;
		winHeight = rect.bottom - rect.top;

		// Deal with centering when switching down to smaller resolution
		HMONITOR hMonitor = MonitorFromWindow(mWindow->GetHWnd(), MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		memset(&monitorInfo, 0, sizeof(MONITORINFO));
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);

		LONG screenw = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
		LONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

		INT32 left = screenw > INT32(winWidth) ? ((screenw - INT32(winWidth)) / 2) : 0;
		INT32 top = screenh > INT32(winHeight) ? ((screenh - INT32(winHeight)) / 2) : 0;

		SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, mWindow->getStyle() | WS_VISIBLE);
		SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, mWindow->getStyleEx());
		SetWindowPos(mWindow->GetHWnd(), HWND_NOTOPMOST, left, top, winWidth, winHeight,
			SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

		_windowMovedOrResized();

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.top = props.top;
			mSyncedProperties.left = props.left;
			mSyncedProperties.width = props.width;
			mSyncedProperties.height = props.height;
		}

		bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
		bs::RenderWindowManager::instance().NotifyMovedOrResized(this);
	}

	void Win32RenderWindow::Move(INT32 left, INT32 top)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			mWindow->Move(left, top);

			props.top = mWindow->GetTop();
			props.left = mWindow->GetLeft();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.top = props.top;
				mSyncedProperties.left = props.left;
			}

			bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
		}
	}

	void Win32RenderWindow::Resize(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			mWindow->Resize(width, height);

			props.width = mWindow->GetWidth();
			props.height = mWindow->GetHeight();
			
			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.width = props.width;
				mSyncedProperties.height = props.height;
			}

			bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
		}
	}

	void Win32RenderWindow::Minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->Minimize();
	}

	void Win32RenderWindow::Maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->Maximize();
	}

	void Win32RenderWindow::Restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->Restore();
	}

	void Win32RenderWindow::SetVSync(bool enabled, UINT32 interval)
	{
		wglSwapIntervalEXT(interval);
		BS_CHECK_GL_ERROR();

		mProperties.vsync = enabled;
		mProperties.vsyncInterval = interval;

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.vsync = enabled;
			mSyncedProperties.vsyncInterval = interval;
		}

		bs::RenderWindowManager::instance().NotifySyncDataDirty(this);
	}

	void Win32RenderWindow::SwapBuffers(UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		SwapBuffers(mHDC);
	}

	void Win32RenderWindow::CopyToMemory(PixelData &dst, FrameBuffer buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if ((dst.GetRight() > getProperties().width) ||
			(dst.GetBottom() > getProperties().height) ||
			(dst.GetFront() != 0) || (dst.getBack() != 1))
		{
			BS_EXCEPT(InvalidParametersException, "Invalid box.");
		}

		if (buffer == FB_AUTO)
		{
			buffer = mProperties.isFullScreen ? FB_FRONT : FB_BACK;
		}

		GLenum format = GLPixelUtil::getGLOriginFormat(dst.GetFormat());
		GLenum type = GLPixelUtil::getGLOriginDataType(dst.GetFormat());

		if ((format == GL_NONE) || (type == 0))
		{
			BS_EXCEPT(InvalidParametersException, "Unsupported format.");
		}

		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		BS_CHECK_GL_ERROR();

		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
		BS_CHECK_GL_ERROR();

		glReadPixels((GLint)dst.GetLeft(), (GLint)dst.getTop(),
					 (GLsizei)dst.GetWidth(), (GLsizei)dst.getHeight(),
					 format, type, dst.GetData());
		BS_CHECK_GL_ERROR();

		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		BS_CHECK_GL_ERROR();

		//vertical flip
		{
			size_t rowSpan = dst.GetWidth() * PixelUtil::getNumElemBytes(dst.getFormat());
			size_t height = dst.GetHeight();
			UINT8* tmpData = (UINT8*)bs_alloc((UINT32)(rowSpan * height));
			UINT8* srcRow = (UINT8 *)dst.GetData(), *tmpRow = tmpData + (height - 1) * rowSpan;

			while (tmpRow >= tmpData)
			{
				memcpy(tmpRow, srcRow, rowSpan);
				srcRow += rowSpan;
				tmpRow -= rowSpan;
			}
			memcpy(dst.GetData(), tmpData, rowSpan * height);

			bs_free(tmpData);
		}
	}

	void Win32RenderWindow::GetCustomAttribute(const String& name, void* pData) const
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

	void Win32RenderWindow::SetActive(bool state)
	{	
		THROW_IF_NOT_CORE_THREAD;

		mWindow->SetActive(state);

		RenderWindow::setActive(state);
	}

	void Win32RenderWindow::SetHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		mShowOnSwap = false;
		mWindow->SetHidden(hidden);

		RenderWindow::setHidden(hidden);
	}

	void Win32RenderWindow::_windowMovedOrResized()
	{
		if (!mWindow)
			return;

		mWindow->_windowMovedOrResized();

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen) // Fullscreen is handled directly by this object
		{
			props.top = mWindow->GetTop();
			props.left = mWindow->GetLeft();
			props.width = mWindow->GetWidth();
			props.height = mWindow->GetHeight();
		}
	}

	HWND Win32RenderWindow::_getHWnd() const
	{
		return mWindow->GetHWnd();
	}

	void Win32RenderWindow::SyncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}		
	}
}
