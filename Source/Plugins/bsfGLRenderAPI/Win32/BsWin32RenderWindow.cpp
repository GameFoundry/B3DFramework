//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0500
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

GLenum GLEWAPIENTRY WglewContextInit(bs::ct::GLSupport* glSupport);

using namespace bs;

#define _MAX_CLASS_NAME_ 128

Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, ct::Win32GLSupport& glsupport)
	: RenderWindow(desc, windowId), mGLSupport(glsupport), mProperties(desc)
{
}

Win32RenderWindow::~Win32RenderWindow()
{
}

void Win32RenderWindow::GetCustomAttribute(const String& name, void* pData) const
{
	if(name == "WINDOW")
	{
		u64* pHwnd = (u64*)pData;
		*pHwnd = (u64)GetHWnd();
		return;
	}
}

Vector2I Win32RenderWindow::ScreenToWindowPos(const Vector2I& screenPos) const
{
	POINT pos;
	pos.x = screenPos.X;
	pos.y = screenPos.Y;

	ScreenToClient(GetHWnd(), &pos);
	return Vector2I(pos.x, pos.y);
}

Vector2I Win32RenderWindow::WindowToScreenPos(const Vector2I& windowPos) const
{
	POINT pos;
	pos.x = windowPos.X;
	pos.y = windowPos.Y;

	ClientToScreen(GetHWnd(), &pos);
	return Vector2I(pos.x, pos.y);
}

SPtr<ct::Win32RenderWindow> Win32RenderWindow::GetCore() const
{
	return std::static_pointer_cast<ct::Win32RenderWindow>(mCoreSpecific);
}

SPtr<ct::CoreObject> Win32RenderWindow::CreateCore() const
{
	RENDER_WINDOW_DESC desc = mDesc;
	SPtr<ct::Win32RenderWindow> coreObj = B3DMakeShared<ct::Win32RenderWindow>(desc, mWindowId, mGLSupport);
	coreObj->SetThisPtrInternal(coreObj);

	mGLSupport.NotifyWindowCreatedInternal(coreObj.get());
	return coreObj;
}

void Win32RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(GetCore()->mLock);
	mProperties = GetCore()->mSyncedProperties;
}

HWND Win32RenderWindow::GetHWnd() const
{
	BlockUntilCoreInitialized();
	return GetCore()->GetHWndInternal();
}

namespace bs {
namespace ct {
Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, Win32GLSupport& glsupport)
	: RenderWindow(desc, windowId), mWindow(nullptr), mGLSupport(glsupport), mHDC(nullptr), mIsChild(false), mDeviceName(nullptr), mDisplayFrequency(0), mShowOnSwap(false), mContext(nullptr), mProperties(desc), mSyncedProperties(desc)
{}

Win32RenderWindow::~Win32RenderWindow()
{
	RenderWindowProperties& props = mProperties;

	if(mWindow != nullptr)
	{
		ReleaseDC(mWindow->GetHWnd(), mHDC);

		B3DDelete(mWindow);
		mWindow = nullptr;
	}

	mHDC = nullptr;

	if(mDeviceName != nullptr)
	{
		B3DFree(mDeviceName);
		mDeviceName = nullptr;
	}

	Platform::ResetNonClientAreas(*this);
}

void Win32RenderWindow::Initialize()
{
	RenderWindowProperties& props = mProperties;

	props.IsFullScreen = mDesc.Fullscreen;
	mIsChild = false;
	mDisplayFrequency = Math::RoundToI32(mDesc.VideoMode.RefreshRate);

	WINDOW_DESC windowDesc;
	windowDesc.ShowTitleBar = mDesc.ShowTitleBar;
	windowDesc.ShowBorder = mDesc.ShowBorder;
	windowDesc.AllowResize = mDesc.AllowResize;
	windowDesc.EnableDoubleClick = true;
	windowDesc.Fullscreen = mDesc.Fullscreen;
	windowDesc.Width = mDesc.VideoMode.Width;
	windowDesc.Height = mDesc.VideoMode.Height;
	windowDesc.Hidden = mDesc.Hidden || mDesc.HideUntilSwap;
	windowDesc.Left = mDesc.Left;
	windowDesc.Top = mDesc.Top;
	windowDesc.OuterDimensions = false;
	windowDesc.Title = mDesc.Title;
	windowDesc.ToolWindow = mDesc.ToolWindow;
	windowDesc.CreationParams = this;
	windowDesc.Modal = mDesc.Modal;
	windowDesc.WndProc = &Win32Platform::Win32WndProcInternal;

#ifdef BS_STATIC_LIB
	windowDesc.module = GetModuleHandle(NULL);
#else
	windowDesc.Module = GetModuleHandle(MODULE_NAME);
#endif

	auto opt = mDesc.PlatformSpecific.find("parentWindowHandle");
	if(opt != mDesc.PlatformSpecific.end())
		windowDesc.Parent = (HWND)Parseu64(opt->second);

	opt = mDesc.PlatformSpecific.find("externalWindowHandle");
	if(opt != mDesc.PlatformSpecific.end())
		windowDesc.External = (HWND)Parseu64(opt->second);

	const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPI::Instance().GetVideoModeInfo());
	u32 numOutputs = videoModeInfo.GetNumOutputs();
	if(numOutputs > 0)
	{
		u32 actualMonitorIdx = std::min(mDesc.VideoMode.OutputIdx, numOutputs - 1);
		const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));
		windowDesc.Monitor = outputInfo.GetMonitorHandle();
	}

	mIsChild = windowDesc.Parent != nullptr;
	props.IsFullScreen = mDesc.Fullscreen && !mIsChild;

	if(!windowDesc.External)
	{
		mShowOnSwap = mDesc.HideUntilSwap && !mDesc.Hidden;
		props.IsHidden = mDesc.HideUntilSwap || mDesc.Hidden;
	}

	mWindow = B3DNew<Win32Window>(windowDesc);

	props.Width = mWindow->GetWidth();
	props.Height = mWindow->GetHeight();
	props.Top = mWindow->GetTop();
	props.Left = mWindow->GetLeft();

	if(!windowDesc.External)
	{
		if(props.IsFullScreen)
		{
			DEVMODE displayDeviceMode;

			memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
			displayDeviceMode.dmSize = sizeof(DEVMODE);
			displayDeviceMode.dmBitsPerPel = 32;
			displayDeviceMode.dmPelsWidth = props.Width;
			displayDeviceMode.dmPelsHeight = props.Height;
			displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if(mDisplayFrequency)
			{
				displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
				displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

				if(ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
				{
					B3D_EXCEPT(RenderingAPIException, "ChangeDisplaySettings with user display frequency failed.");
				}
			}

			if(ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
			{
				B3D_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed.");
			}
		}
	}

	mHDC = GetDC(mWindow->GetHWnd());

	int testMultisample = props.MultisampleCount;
	bool testHwGamma = mDesc.Gamma;
	bool formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.DepthBuffer);
	if(!formatOk)
	{
		if(props.MultisampleCount > 0)
		{
			// Try without multisampling
			testMultisample = 0;
			formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.DepthBuffer);
		}

		if(!formatOk && mDesc.Gamma)
		{
			// Try without sRGB
			testHwGamma = false;
			testMultisample = props.MultisampleCount;
			formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.DepthBuffer);
		}

		if(!formatOk && mDesc.Gamma && (props.MultisampleCount > 0))
		{
			// Try without both
			testHwGamma = false;
			testMultisample = 0;
			formatOk = mGLSupport.SelectPixelFormat(mHDC, 32, testMultisample, testHwGamma, mDesc.DepthBuffer);
		}

		if(!formatOk)
			B3D_EXCEPT(RenderingAPIException, "Failed selecting pixel format.");
	}

	// Record what gamma option we used in the end
	// this will control enabling of sRGB state flags when used
	props.HwGamma = testHwGamma;
	props.MultisampleCount = testMultisample;

	mContext = mGLSupport.CreateContext(mHDC, nullptr);

	if(props.Vsync && props.VsyncInterval > 0)
		SetVSync(true, props.VsyncInterval);

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties = props;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	RenderWindow::Initialize();
}

void Win32RenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mIsChild)
		return;

	const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPI::Instance().GetVideoModeInfo());
	u32 numOutputs = videoModeInfo.GetNumOutputs();
	if(numOutputs == 0)
		return;

	RenderWindowProperties& props = mProperties;

	u32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
	const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

	mDisplayFrequency = Math::RoundToI32(refreshRate);
	props.IsFullScreen = true;

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

	if(ChangeDisplaySettingsEx(monitorInfo.szDevice, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
	{
		B3D_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed");
	}

	props.Top = monitorInfo.rcMonitor.top;
	props.Left = monitorInfo.rcMonitor.left;
	props.Width = width;
	props.Height = height;

	SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, 0);

	SetWindowPos(mWindow->GetHWnd(), HWND_TOP, props.Left, props.Top, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	WindowMovedOrResizedInternal();

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Top = props.Top;
		mSyncedProperties.Left = props.Left;
		mSyncedProperties.Width = props.Width;
		mSyncedProperties.Height = props.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);
}

void Win32RenderWindow::SetFullscreen(const VideoMode& mode)
{
	THROW_IF_NOT_CORE_THREAD;

	SetFullscreen(mode.Width, mode.Height, mode.RefreshRate, mode.OutputIdx);
}

void Win32RenderWindow::SetWindowed(u32 width, u32 height)
{
	THROW_IF_NOT_CORE_THREAD;

	RenderWindowProperties& props = mProperties;

	if(!props.IsFullScreen)
		return;

	props.IsFullScreen = false;
	props.Width = width;
	props.Height = height;

	// Drop out of fullscreen
	ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

	u32 winWidth = width;
	u32 winHeight = height;

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

	i32 left = screenw > i32(winWidth) ? ((screenw - i32(winWidth)) / 2) : 0;
	i32 top = screenh > i32(winHeight) ? ((screenh - i32(winHeight)) / 2) : 0;

	SetWindowLong(mWindow->GetHWnd(), GWL_STYLE, mWindow->GetStyle() | WS_VISIBLE);
	SetWindowLong(mWindow->GetHWnd(), GWL_EXSTYLE, mWindow->GetStyleEx());
	SetWindowPos(mWindow->GetHWnd(), HWND_NOTOPMOST, left, top, winWidth, winHeight, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

	WindowMovedOrResizedInternal();

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Top = props.Top;
		mSyncedProperties.Left = props.Left;
		mSyncedProperties.Width = props.Width;
		mSyncedProperties.Height = props.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);
}

void Win32RenderWindow::Move(i32 left, i32 top)
{
	THROW_IF_NOT_CORE_THREAD;

	RenderWindowProperties& props = mProperties;
	if(!props.IsFullScreen)
	{
		mWindow->Move(left, top);

		props.Top = mWindow->GetTop();
		props.Left = mWindow->GetLeft();

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.Top = props.Top;
			mSyncedProperties.Left = props.Left;
		}

		bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	}
}

void Win32RenderWindow::Resize(u32 width, u32 height)
{
	THROW_IF_NOT_CORE_THREAD;

	RenderWindowProperties& props = mProperties;
	if(!props.IsFullScreen)
	{
		mWindow->Resize(width, height);

		props.Width = mWindow->GetWidth();
		props.Height = mWindow->GetHeight();

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.Width = props.Width;
			mSyncedProperties.Height = props.Height;
		}

		bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
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

void Win32RenderWindow::SetVSync(bool enabled, u32 interval)
{
	wglSwapIntervalEXT(interval);
	B3D_CHECK_GL_ERROR();

	mProperties.Vsync = enabled;
	mProperties.VsyncInterval = interval;

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Vsync = enabled;
		mSyncedProperties.VsyncInterval = interval;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
}

void Win32RenderWindow::SwapBuffers(u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mShowOnSwap)
		SetHidden(false);

	::SwapBuffers(mHDC);
}

void Win32RenderWindow::CopyToMemory(PixelData& dst, FrameBuffer buffer)
{
	THROW_IF_NOT_CORE_THREAD;

	if((dst.GetRight() > GetProperties().Width) ||
	   (dst.GetBottom() > GetProperties().Height) ||
	   (dst.GetFront() != 0) || (dst.GetBack() != 1))
	{
		B3D_EXCEPT(InvalidParametersException, "Invalid box.");
	}

	if(buffer == FB_AUTO)
	{
		buffer = mProperties.IsFullScreen ? FB_FRONT : FB_BACK;
	}

	GLenum format = GLPixelUtil::GetGlOriginFormat(dst.GetFormat());
	GLenum type = GLPixelUtil::GetGlOriginDataType(dst.GetFormat());

	if((format == GL_NONE) || (type == 0))
	{
		B3D_EXCEPT(InvalidParametersException, "Unsupported format.");
	}

	// Must change the packing to ensure no overruns!
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	B3D_CHECK_GL_ERROR();

	glReadBuffer((buffer == FB_FRONT) ? GL_FRONT : GL_BACK);
	B3D_CHECK_GL_ERROR();

	glReadPixels((GLint)dst.GetLeft(), (GLint)dst.GetTop(), (GLsizei)dst.GetWidth(), (GLsizei)dst.GetHeight(), format, type, dst.GetData());
	B3D_CHECK_GL_ERROR();

	// restore default alignment
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	B3D_CHECK_GL_ERROR();

	// vertical flip
	{
		size_t rowSpan = dst.GetWidth() * PixelUtil::GetNumElemBytes(dst.GetFormat());
		size_t height = dst.GetHeight();
		u8* tmpData = (u8*)B3DAllocate((u32)(rowSpan * height));
		u8 *srcRow = (u8*)dst.GetData(), *tmpRow = tmpData + (height - 1) * rowSpan;

		while(tmpRow >= tmpData)
		{
			memcpy(tmpRow, srcRow, rowSpan);
			srcRow += rowSpan;
			tmpRow -= rowSpan;
		}
		memcpy(dst.GetData(), tmpData, rowSpan * height);

		B3DFree(tmpData);
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
		u64* pHwnd = (u64*)pData;
		*pHwnd = (u64)GetHWndInternal();
		return;
	}
}

void Win32RenderWindow::SetActive(bool state)
{
	THROW_IF_NOT_CORE_THREAD;

	mWindow->SetActive(state);

	RenderWindow::SetActive(state);
}

void Win32RenderWindow::SetHidden(bool hidden)
{
	THROW_IF_NOT_CORE_THREAD;

	mShowOnSwap = false;
	mWindow->SetHidden(hidden);

	RenderWindow::SetHidden(hidden);
}

void Win32RenderWindow::WindowMovedOrResizedInternal()
{
	if(!mWindow)
		return;

	mWindow->WindowMovedOrResizedInternal();

	RenderWindowProperties& props = mProperties;
	if(!props.IsFullScreen) // Fullscreen is handled directly by this object
	{
		props.Top = mWindow->GetTop();
		props.Left = mWindow->GetLeft();
		props.Width = mWindow->GetWidth();
		props.Height = mWindow->GetHeight();
	}
}

HWND Win32RenderWindow::GetHWndInternal() const
{
	return mWindow->GetHWnd();
}

void Win32RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(mLock);
	mProperties = mSyncedProperties;
}
}} // namespace bs::ct
