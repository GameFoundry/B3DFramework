//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Win32/BsWin32RenderWindow.h"
#include "Win32/BsWin32Platform.h"
#include "Win32/BsWin32Window.h"
#include "Win32/BsWin32VideoModeInfo.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsRenderWindowManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanSwapChain.h"
#include "BsMath.h"

namespace BansheeEngine
{
	Win32RenderWindowProperties::Win32RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
		:RenderWindowProperties(desc)
	{ }

	Win32RenderWindowCore::Win32RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId, VulkanRenderAPI& renderAPI)
		: RenderWindowCore(desc, windowId), mProperties(desc), mSyncedProperties(desc), mWindow(nullptr), mIsChild(false)
		, mShowOnSwap(false), mDisplayFrequency(0), mRenderAPI(renderAPI)
	{ }

	Win32RenderWindowCore::~Win32RenderWindowCore()
	{ 
		Win32RenderWindowProperties& props = mProperties;
		props.mActive = false;

		if (mWindow != nullptr)
		{
			bs_delete(mWindow);
			mWindow = nullptr;
		}

		mSwapChain = nullptr;
		vkDestroySurfaceKHR(mRenderAPI._getInstance(), mSurface, gVulkanAllocator);
	}

	void Win32RenderWindowCore::initialize()
	{
		Win32RenderWindowProperties& props = mProperties;

		// Create a window
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
		
		const Win32VideoModeInfo& videoModeInfo = static_cast<const Win32VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs > 0)
		{
			UINT32 actualMonitorIdx = std::min(mDesc.videoMode.getOutputIdx(), numOutputs - 1);
			const Win32VideoOutputInfo& outputInfo = static_cast<const Win32VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));
			windowDesc.monitor = outputInfo.getMonitorHandle();
		}

		// Update local properties
		mWindow = bs_new<Win32Window>(windowDesc);

		mIsChild = windowDesc.parent != nullptr;
		mDisplayFrequency = Math::roundToInt(mDesc.videoMode.getRefreshRate());

		props.mIsFullScreen = mDesc.fullscreen && !mIsChild;
		props.mColorDepth = 32;
		props.mActive = true;
		props.mWidth = mWindow->getWidth();
		props.mHeight = mWindow->getHeight();
		props.mTop = mWindow->getTop();
		props.mLeft = mWindow->getLeft();

		if (!windowDesc.external)
		{
			mShowOnSwap = mDesc.hideUntilSwap;
			props.mHidden = mDesc.hideUntilSwap || mDesc.hidden;
		}

		// Create Vulkan surface
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.hwnd = mWindow->getHWnd();
		surfaceCreateInfo.hinstance = windowDesc.module;

		VkInstance instance = mRenderAPI._getInstance();
		VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &mSurface);
		assert(result == VK_SUCCESS);

		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		VkPhysicalDevice physicalDevice = presentDevice->getPhysical();

		UINT32 presentQueueIdx = presentDevice->getQueueFamily(VQT_GRAPHICS);
		
		VkBool32 supportsPresent;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, presentQueueIdx, mSurface, &supportsPresent);

		if(!supportsPresent)
		{
			// Note: Not supporting present only queues at the moment
			// Note: Also present device can only return one family of graphics queue, while there could be more (some of
			// which support present)
			BS_EXCEPT(RenderingAPIException, "Cannot find a graphics queue that also supports present operations.");
		}
		
		uint32_t numFormats;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &numFormats, nullptr);
		assert(result == VK_SUCCESS);
		assert(numFormats > 0);

		VkSurfaceFormatKHR* formats = bs_stack_alloc<VkSurfaceFormatKHR>(numFormats);

		Vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &numFormats, surfaceFormats.data());
		assert(result == VK_SUCCESS);

		// If there is no preferred format, use standard RGBA
		if ((numFormats == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			if (mDesc.gamma)
				mColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
			else
				mColorFormat = VK_FORMAT_B8G8R8A8_UNORM;

			mColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool foundFormat = false;

			VkFormat wantedFormats[] = 
			{
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_FORMAT_A8B8G8R8_SRGB_PACK32,
				VK_FORMAT_R8G8B8_SRGB,
			};

			UINT32 numWantedFormats = sizeof(wantedFormats) / sizeof(wantedFormats[0]);

			for(UINT32 i = 0; i < numWantedFormats; i++)
			{
				for(UINT32 j = 0; j < numFormats; j++)
				{
					if(surfaceFormats[j].format == wantedFormats[i])
					{
						mColorFormat = surfaceFormats[j].format;
						mColorSpace = surfaceFormats[j].colorSpace;

						foundFormat = true;
						break;
					}
				}
			}

			// If we haven't found anything, fall back to first available
			if(!foundFormat)
			{
				mColorFormat = surfaceFormats[0].format;
				mColorSpace = surfaceFormats[0].colorSpace;

				if (mDesc.gamma)
					LOGERR("Cannot find a valid sRGB format for a render window surface, falling back to a default format.");
			}
		}
		
		bs_stack_free(formats);

		// Create swap chain
		mSwapChain = bs_shared_ptr_new<VulkanSwapChain>();
		mSwapChain->rebuild(presentDevice, mSurface, props.mWidth, props.mHeight, props.mVSync, mColorFormat, mColorSpace);

		// TODO - Create a framebuffer for each swap chain buffer

		// Make the window full screen if required
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

					if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
					{
						BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings with user display frequency failed.");
					}
				}

				if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
				{
					BS_EXCEPT(RenderingAPIException, "ChangeDisplaySettings failed.");
				}
			}
		}

		// TODO - Set hwGamma and multisample properties

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

		if (mShowOnSwap)
			setHidden(false);

		// TODO - Swap buffers
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

		mShowOnSwap = false;
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

		SetWindowPos(mWindow->getHWnd(), HWND_TOP, props.mLeft, props.mTop, width, height, SWP_NOACTIVATE);
	}

	void Win32RenderWindowCore::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		setFullscreen(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode.getOutputIdx());
	}

	void Win32RenderWindowCore::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		Win32RenderWindowProperties& props = mProperties;

		if (!props.mIsFullScreen)
			return;

		props.mIsFullScreen = false;
		props.mWidth = width;
		props.mHeight = height;

		// Drop out of fullscreen
		ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);

		UINT32 winWidth = 0;
		UINT32 winHeight = 0;

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

		SetWindowLong(mWindow->getHWnd(), GWL_STYLE, mWindow->getStyle());
		SetWindowLong(mWindow->getHWnd(), GWL_EXSTYLE, mWindow->getStyleEx());
		SetWindowPos(mWindow->getHWnd(), HWND_NOTOPMOST, left, top, winWidth, winHeight,
			SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.mWidth = props.mWidth;
			mSyncedProperties.mHeight = props.mHeight;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
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

		// TODO - Retrieve renderpass/framebuffer

		RenderWindowCore::getCustomAttribute(name, pData);
	}

	void Win32RenderWindowCore::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

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

		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		mSwapChain->rebuild(presentDevice, mSurface, props.mWidth, props.mHeight, props.mVSync, mColorFormat, mColorSpace);

		// TODO - Update framebuffer?

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