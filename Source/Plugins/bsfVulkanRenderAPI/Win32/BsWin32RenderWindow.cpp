//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Win32/BsWin32RenderWindow.h"
#include "Private/Win32/BsWin32Platform.h"
#include "Private/Win32/BsWin32Window.h"
#include "Win32/BsWin32VideoModeInfo.h"
#include "Corethread/BsCoreThread.h"
#include "Profiling/BsRenderStats.h"
#include "Managers/BsRenderWindowManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanDevice.h"
#include "BsVulkanCommandBuffer.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanQueue.h"
#include "BsVulkanSubmitThread.h"
#include "Math/BsMath.h"

using namespace bs;

Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
	: RenderWindow(desc, windowId), mProperties(desc)
{}

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

HWND Win32RenderWindow::GetHWnd() const
{
	BlockUntilCoreInitialized();
	return GetCore()->GetWindowHandleInternal();
}

void Win32RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(GetCore()->mLock);
	mProperties = GetCore()->mSyncedProperties;
}

SPtr<ct::CoreObject> Win32RenderWindow::CreateCore() const
{
	ct::VulkanRenderAPI& rapi = static_cast<ct::VulkanRenderAPI&>(ct::RenderAPI::Instance());

	RENDER_WINDOW_DESC desc = mDesc;
	SPtr<ct::CoreObject> coreObj = B3DMakeShared<ct::Win32RenderWindow>(desc, mWindowId, rapi);
	coreObj->SetThisPtrInternal(coreObj);

	return coreObj;
}

namespace bs {
namespace ct {
Win32RenderWindow::Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, VulkanRenderAPI& renderAPI)
	: RenderWindow(desc, windowId), mProperties(desc), mSyncedProperties(desc), mWindow(nullptr), mIsChild(false), mShowOnSwap(false), mDisplayFrequency(0), mRenderAPI(renderAPI)
{}

Win32RenderWindow::~Win32RenderWindow()
{
	GetVulkanSubmitThread().WaitUntilIdle();

	if(mWindow != nullptr)
	{
		B3DDelete(mWindow);
		mWindow = nullptr;
	}

	mSwapChain->Destroy();

	Platform::ResetNonClientAreas(*this);
}

void Win32RenderWindow::Initialize()
{
	RenderWindowProperties& props = mProperties;

	// Create a window
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
	windowDesc.Module = GetModuleHandle("bsfVulkanRenderAPI.dll");
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

	// Must be set before creating a window, since wndProc will call ShowWindow if needed after creation
	if(!windowDesc.External)
	{
		mShowOnSwap = mDesc.HideUntilSwap && !mDesc.Hidden;
		props.IsHidden = mDesc.HideUntilSwap || mDesc.Hidden;
	}

	mWindow = B3DNew<Win32Window>(windowDesc);

	mIsChild = windowDesc.Parent != nullptr;
	mDisplayFrequency = Math::RoundToI32(mDesc.VideoMode.RefreshRate);

	// Update local properties
	props.IsFullScreen = mDesc.Fullscreen && !mIsChild;
	props.Width = mWindow->GetWidth();
	props.Height = mWindow->GetHeight();
	props.Top = mWindow->GetTop();
	props.Left = mWindow->GetLeft();
	props.HwGamma = mDesc.Gamma;
	props.MultisampleCount = 1;

	// Create Vulkan surface
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hwnd = mWindow->GetHWnd();
	surfaceCreateInfo.hinstance = windowDesc.Module;

	VkInstance instance = mRenderAPI.GetVkInstance();
	VkSurfaceKHR vkSurface;
	VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);

	mSurface = B3DMakeShared<VulkanSurface>(vkSurface);

	SPtr<VulkanDevice> presentDevice = mRenderAPI.GetPresentDevice();
	VkPhysicalDevice physicalDevice = presentDevice->GetPhysical();

	mPresentQueueFamily = presentDevice->GetQueueFamily(GQT_GRAPHICS);

	VkBool32 supportsPresent;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mPresentQueueFamily, vkSurface, &supportsPresent);

	if(!supportsPresent)
	{
		// Note: Not supporting present only queues at the moment
		// Note: Also present device can only return one family of graphics queue, while there could be more (some of
		// which support present)
		B3D_EXCEPT(RenderingAPIException, "Cannot find a graphics queue that also supports present operations.");
	}

	SurfaceFormat format = presentDevice->GetSurfaceFormat(vkSurface, mDesc.Gamma);
	mColorFormat = format.ColorFormat;
	mColorSpace = format.ColorSpace;
	mDepthFormat = format.DepthFormat;

	// Create swap chain
	mSwapChain = presentDevice->GetResourceManager().Create<VulkanSwapChain>(mSurface, props.Width, props.Height, props.Vsync, mColorFormat, mColorSpace, mDesc.DepthBuffer, mDepthFormat);

	// Make the window full screen if required
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

				if(ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
				{
					B3D_EXCEPT(RenderingAPIException, "ChangeDisplaySettings with user display frequency failed.");
				}
			}

			if(ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
			{
				B3D_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed.");
			}
		}
	}

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties = props;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	RenderWindow::Initialize();
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
	ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);

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

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Width = props.Width;
		mSyncedProperties.Height = props.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
}

void Win32RenderWindow::SetVSync(bool enabled, u32 interval)
{
	mProperties.Vsync = enabled;
	mProperties.VsyncInterval = interval;

	if(mSwapChain != nullptr)
		mSwapChain->MarkAsInvalid();

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Vsync = enabled;
		mSyncedProperties.VsyncInterval = interval;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
}

HWND Win32RenderWindow::GetWindowHandleInternal() const
{
	return mWindow->GetHWnd();
}

void Win32RenderWindow::GetCustomAttribute(const String& name, void* data) const
{
	if(name == "WINDOW")
	{
		u64* pWnd = (u64*)data;
		*pWnd = (u64)mWindow->GetHWnd();
		return;
	}
}

void Win32RenderWindow::WindowMovedOrResizedInternal()
{
	THROW_IF_NOT_CORE_THREAD;

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

void Win32RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(mLock);
	mProperties = mSyncedProperties;
}

void Win32RenderWindow::RebuildSwapChain()
{
	GetVulkanSubmitThread().WaitUntilIdle();

	SPtr<VulkanDevice> presentDevice = mRenderAPI.GetPresentDevice();
	VulkanSwapChain* oldSwapChain = mSwapChain;
	oldSwapChain->MarkAsRetired();

	mSwapChain = presentDevice->GetResourceManager().Create<VulkanSwapChain>(mSurface, mProperties.Width, mProperties.Height, mProperties.Vsync, mColorFormat, mColorSpace, mDesc.DepthBuffer, mDepthFormat, oldSwapChain);
	oldSwapChain->Destroy();

	OnSwapChainDidRebuild();
}
}} // namespace bs::ct
