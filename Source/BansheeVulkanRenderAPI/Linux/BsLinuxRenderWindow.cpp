//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "CoreThread/BsCoreThread.h"
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxRenderWindow.h"
#include "Linux/BsLinuxWindow.h"
#include "Linux/BsLinuxVideoModeInfo.h"
#include "Math/BsMath.h"
#include "Managers/BsRenderWindowManager.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanQueue.h"
#include <X11/Xutil.h>

#define XRANDR_ROTATION_LEFT    (1 << 1)
#define XRANDR_ROTATION_RIGHT   (1 << 3)

namespace bs
{
	LinuxRenderWindow::LinuxRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:RenderWindow(desc, windowId), mProperties(desc)
	{ }

	void LinuxRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "WINDOW" || name == "LINUX_WINDOW")
		{
			blockUntilCoreInitialized();
			getCore()->getCustomAttribute(name, data);
			return;
		}
	}

	Vector2I LinuxRenderWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		blockUntilCoreInitialized();

		LinuxPlatform::lockX();
		Vector2I pos = getCore()->_getInternal()->screenToWindowPos(screenPos);
		LinuxPlatform::unlockX();

		return pos;
	}

	Vector2I LinuxRenderWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		blockUntilCoreInitialized();

		LinuxPlatform::lockX();
		Vector2I pos = getCore()->_getInternal()->windowToScreenPos(windowPos);
		LinuxPlatform::unlockX();

		return pos;
	}

	SPtr<ct::LinuxRenderWindow> LinuxRenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::LinuxRenderWindow>(mCoreSpecific);
	}

	void LinuxRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->_getPropertiesLock());
		mProperties = getCore()->mSyncedProperties;
	}

	namespace ct
	{
	LinuxRenderWindow::LinuxRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, VulkanRenderAPI& renderAPI)
		: RenderWindow(desc, windowId), mRenderAPI(renderAPI), mRequiresNewBackBuffer(true), mWindow(nullptr)
		, mProperties(desc), mSyncedProperties(desc), mIsChild(false), mShowOnSwap(false)
	{ }

	LinuxRenderWindow::~LinuxRenderWindow()
	{
		// Make sure to set the original desktop video mode before we exit
		if(mProperties.isFullScreen)
			setWindowed(50, 50);

		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		presentDevice->waitIdle();

		if (mWindow != nullptr)
		{
			Platform::resetNonClientAreas(*this);

			LinuxPlatform::lockX();

			bs_delete(mWindow);
			mWindow = nullptr;

			LinuxPlatform::unlockX();
		}

		mSwapChain = nullptr;
		vkDestroySurfaceKHR(mRenderAPI._getInstance(), mSurface, gVulkanAllocator);
	}

	void LinuxRenderWindow::initialize()
	{
		LinuxPlatform::lockX();

		RenderWindowProperties& props = mProperties;

		props.isFullScreen = mDesc.fullscreen;
		mIsChild = false;

		XVisualInfo visualInfoTempl = {};
		visualInfoTempl.screen = XDefaultScreen(LinuxPlatform::getXDisplay());
		visualInfoTempl.depth = 24;
		visualInfoTempl.c_class = TrueColor;

		int32_t numVisuals;
		XVisualInfo* visualInfo = XGetVisualInfo(LinuxPlatform::getXDisplay(),
				VisualScreenMask | VisualDepthMask | VisualClassMask, &visualInfoTempl, &numVisuals);

		WINDOW_DESC windowDesc;
		windowDesc.x = mDesc.left;
		windowDesc.y = mDesc.top;
		windowDesc.width = mDesc.videoMode.getWidth();
		windowDesc.height = mDesc.videoMode.getHeight();
		windowDesc.title = mDesc.title;
		windowDesc.showDecorations = mDesc.showTitleBar;
		windowDesc.allowResize = mDesc.allowResize;
		windowDesc.showOnTaskBar = !mDesc.toolWindow;
		windowDesc.modal = mDesc.modal;
		windowDesc.visualInfo = *visualInfo;
		windowDesc.screen = mDesc.videoMode.getOutputIdx();
		windowDesc.hidden = mDesc.hideUntilSwap || mDesc.hidden;

		NameValuePairList::const_iterator opt;
		opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (::Window)parseUINT64(opt->second);
		else
			windowDesc.parent = 0;

		mIsChild = windowDesc.parent != 0;
		props.isFullScreen = mDesc.fullscreen && !mIsChild;

		mShowOnSwap = mDesc.hideUntilSwap;
		props.isHidden = mDesc.hideUntilSwap || mDesc.hidden;

		mWindow = bs_new<LinuxWindow>(windowDesc);
		mWindow->_setUserData(this);

		props.width = mWindow->getWidth();
		props.height = mWindow->getHeight();
		props.top = mWindow->getTop();
		props.left = mWindow->getLeft();

		props.hwGamma = mDesc.gamma;
		props.multisampleCount = mDesc.multisampleCount;

		XWindowAttributes windowAttributes;
		XGetWindowAttributes(LinuxPlatform::getXDisplay(), mWindow->_getXWindow(), &windowAttributes);

		// Create Vulkan surface
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.window = mWindow->_getXWindow();
		surfaceCreateInfo.dpy = LinuxPlatform::getXDisplay();

		// Note: I manually lock Xlib, while Vulkan spec says XInitThreads should be called, since Vulkan
		// surely calls Xlib under the hood as well. I've tried to guess which calls use Xlib and lock them
		// externally, but XInitThreads might be required if problems occur.
		VkInstance instance = mRenderAPI._getInstance();
		VkResult result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &mSurface);
		assert(result == VK_SUCCESS);

		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		VkPhysicalDevice physicalDevice = presentDevice->getPhysical();

		mPresentQueueFamily = presentDevice->getQueueFamily(GQT_GRAPHICS);

		VkBool32 supportsPresent;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mPresentQueueFamily, mSurface, &supportsPresent);

		if(!supportsPresent)
		{
			// Note: Not supporting present only queues at the moment
			// Note: Also present device can only return one family of graphics queue, while there could be more (some of
			// which support present)
			BS_EXCEPT(RenderingAPIException, "Cannot find a graphics queue that also supports present operations.");
		}

		SurfaceFormat format = presentDevice->getSurfaceFormat(mSurface, mDesc.gamma);
		mColorFormat = format.colorFormat;
		mColorSpace = format.colorSpace;
		mDepthFormat = format.depthFormat;

		// Create swap chain
		mSwapChain = bs_shared_ptr_new<VulkanSwapChain>();
		mSwapChain->rebuild(presentDevice, mSurface, props.width, props.height, props.vsync, mColorFormat, mColorSpace,
				mDesc.depthBuffer, mDepthFormat);

		LinuxPlatform::unlockX(); // Calls below have their own locking mechanisms

		if(mDesc.fullscreen && !mIsChild)
			setFullscreen(mDesc.videoMode);

		if(mDesc.vsync && mDesc.vsyncInterval > 0)
			setVSync(true, mDesc.vsyncInterval);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindow::initialize();
	}

	void LinuxRenderWindow::acquireBackBuffer()
	{
		// We haven't presented the current back buffer yet, so just use that one
		if (!mRequiresNewBackBuffer)
			return;

		mSwapChain->acquireBackBuffer();
		mRequiresNewBackBuffer = false;
	}

	void LinuxRenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		VideoMode videoMode(width, height, refreshRate, monitorIdx);
		setFullscreen(videoMode);
	}

	void LinuxRenderWindow::setVideoMode(INT32 screen, RROutput output, RRMode mode)
	{
		::Display* display = LinuxPlatform::getXDisplay();
		::Window rootWindow = RootWindow(display, screen);

		XRRScreenResources* screenRes = XRRGetScreenResources (display, rootWindow);
		if(screenRes == nullptr)
		{
			LOGERR("XRR: Failed to retrieve screen resources. ");
			return;
		}

		XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, output);
		if(outputInfo == nullptr)
		{
			XRRFreeScreenResources(screenRes);

			LOGERR("XRR: Failed to retrieve output info for output: " + toString((UINT32)output));
			return;
		}

		XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
		if(crtcInfo == nullptr)
		{
			XRRFreeScreenResources(screenRes);
			XRRFreeOutputInfo(outputInfo);

			LOGERR("XRR: Failed to retrieve CRTC info for output: " + toString((UINT32)output));
			return;
		}

		// Note: This changes the user's desktop resolution permanently, even when the app exists, make sure to revert
		// (Sadly there doesn't appear to be a better way)
		Status status = XRRSetCrtcConfig (display, screenRes, outputInfo->crtc, CurrentTime,
			crtcInfo->x, crtcInfo->y, mode, crtcInfo->rotation, &output, 1);

		if(status != Success)
			LOGERR("XRR: XRRSetCrtcConfig failed.");

		XRRFreeCrtcInfo(crtcInfo);
		XRRFreeOutputInfo(outputInfo);
		XRRFreeScreenResources(screenRes);
	}

	void LinuxRenderWindow::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const LinuxVideoModeInfo& videoModeInfo =
				static_cast<const LinuxVideoModeInfo&>(RenderAPI::instance().getVideoModeInfo());

		UINT32 outputIdx = mode.getOutputIdx();
		if(outputIdx >= videoModeInfo.getNumOutputs())
		{
			LOGERR("Invalid output device index.")
			return;
		}

		const LinuxVideoOutputInfo& outputInfo =
				static_cast<const LinuxVideoOutputInfo&>(videoModeInfo.getOutputInfo (outputIdx));

		INT32 screen = outputInfo._getScreen();
		RROutput outputID = outputInfo._getOutputID();

		RRMode modeID = 0;
		if(!mode.isCustom())
		{
			const LinuxVideoMode& videoMode = static_cast<const LinuxVideoMode&>(mode);
			modeID = videoMode._getModeID();
		}
		else
		{
			LinuxPlatform::lockX();

			// Look for mode matching the requested resolution
			::Display* display = LinuxPlatform::getXDisplay();
			::Window rootWindow = RootWindow(display, screen);

			XRRScreenResources* screenRes = XRRGetScreenResources(display, rootWindow);
			if (screenRes == nullptr)
			{
				LOGERR("XRR: Failed to retrieve screen resources. ");
				return;
			}

			XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, outputID);
			if (outputInfo == nullptr)
			{
				XRRFreeScreenResources(screenRes);

				LOGERR("XRR: Failed to retrieve output info for output: " + toString((UINT32)outputID));
				return;
			}

			XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
			if (crtcInfo == nullptr)
			{
				XRRFreeScreenResources(screenRes);
				XRRFreeOutputInfo(outputInfo);

				LOGERR("XRR: Failed to retrieve CRTC info for output: " + toString((UINT32)outputID));
				return;
			}

			bool foundMode = false;
			for (INT32 i = 0; i < screenRes->nmode; i++)
			{
				const XRRModeInfo& modeInfo = screenRes->modes[i];

				UINT32 width, height;

				if (crtcInfo->rotation & (XRANDR_ROTATION_LEFT | XRANDR_ROTATION_RIGHT))
				{
					width = modeInfo.height;
					height = modeInfo.width;
				}
				else
				{
					width = modeInfo.width;
					height = modeInfo.height;
				}

				float refreshRate;
				if (modeInfo.hTotal != 0 && modeInfo.vTotal != 0)
					refreshRate = (float) (modeInfo.dotClock / (double) (modeInfo.hTotal * modeInfo.vTotal));
				else
					refreshRate = 0.0f;

				if (width == mode.getWidth() && height == mode.getHeight())
				{
					modeID = modeInfo.id;
					foundMode = true;

					if (Math::approxEquals(refreshRate, mode.getRefreshRate()))
						break;
				}
			}

			if (!foundMode)
			{
				LinuxPlatform::unlockX();

				LOGERR("Unable to enter fullscreen, unsupported video mode requested.");
				return;
			}

			LinuxPlatform::unlockX();
		}

		LinuxPlatform::lockX();

		setVideoMode(screen, outputID, modeID);
		mWindow->_setFullscreen(true);

		LinuxPlatform::unlockX();

		RenderWindowProperties& props = mProperties;
		props.isFullScreen = true;

		props.top = 0;
		props.left = 0;
		props.width = mode.getWidth();
		props.height = mode.getHeight();

		_windowMovedOrResized();
	}

	void LinuxRenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
			return;

		// Restore old screen config
		const LinuxVideoModeInfo& videoModeInfo =
				static_cast<const LinuxVideoModeInfo&>(RenderAPI::instance().getVideoModeInfo());

		UINT32 outputIdx = 0; // 0 is always primary
		if(outputIdx >= videoModeInfo.getNumOutputs())
		{
			LOGERR("Invalid output device index.")
			return;
		}

		const LinuxVideoOutputInfo& outputInfo =
				static_cast<const LinuxVideoOutputInfo&>(videoModeInfo.getOutputInfo (outputIdx));

		const LinuxVideoMode& desktopVideoMode = static_cast<const LinuxVideoMode&>(outputInfo.getDesktopVideoMode());

		LinuxPlatform::lockX();

		setVideoMode(outputInfo._getScreen(), outputInfo._getOutputID(), desktopVideoMode._getModeID());
		mWindow->_setFullscreen(false);

		LinuxPlatform::unlockX();

		props.isFullScreen = false;
		props.width = width;
		props.height = height;

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.width = props.width;
			mSyncedProperties.height = props.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		_windowMovedOrResized();
	}

	void LinuxRenderWindow::move(INT32 left, INT32 top)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			LinuxPlatform::lockX();
			mWindow->move(left, top);
			LinuxPlatform::unlockX();

			props.top = mWindow->getTop();
			props.left = mWindow->getLeft();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.top = props.top;
				mSyncedProperties.left = props.left;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		}
	}

	void LinuxRenderWindow::resize(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			LinuxPlatform::lockX();
			mWindow->resize(width, height);
			LinuxPlatform::unlockX();

			props.width = mWindow->getWidth();
			props.height = mWindow->getHeight();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.width = props.width;
				mSyncedProperties.height = props.height;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		}
	}

	void LinuxRenderWindow::minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		LinuxPlatform::lockX();
		mWindow->minimize();
		LinuxPlatform::unlockX();
	}

	void LinuxRenderWindow::maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		LinuxPlatform::lockX();
		mWindow->maximize();
		LinuxPlatform::unlockX();
	}

	void LinuxRenderWindow::restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		LinuxPlatform::lockX();
		mWindow->restore();
		LinuxPlatform::unlockX();
	}

	void LinuxRenderWindow::setVSync(bool enabled, UINT32 interval)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(!enabled)
			interval = 0;

		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		presentDevice->waitIdle();

		LinuxPlatform::lockX();

		mSwapChain->rebuild(presentDevice, mSurface, mProperties.width, mProperties.height, enabled, mColorFormat,
				mColorSpace, mDesc.depthBuffer, mDepthFormat);

		LinuxPlatform::unlockX();
		
		mProperties.vsync = enabled;
		mProperties.vsyncInterval = interval;

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.vsync = enabled;
			mSyncedProperties.vsyncInterval = interval;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);		
	}

	void LinuxRenderWindow::swapBuffers(UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		LinuxPlatform::lockX();

		// Get a command buffer on which we'll submit
		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();

		// Assuming present queue is always graphics
		assert(presentDevice->getQueueFamily(GQT_GRAPHICS) == mPresentQueueFamily);

		// Find an appropriate queue to execute on
		VulkanQueue* queue = presentDevice->getQueue(GQT_GRAPHICS, 0);
		UINT32 queueMask = presentDevice->getQueueMask(GQT_GRAPHICS, 0);

		// Ignore myself
		syncMask &= ~queueMask;

		UINT32 deviceIdx = presentDevice->getIndex();
		VulkanCommandBufferManager& cbm = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());

		UINT32 numSemaphores;
		cbm.getSyncSemaphores(deviceIdx, syncMask, mSemaphoresTemp, numSemaphores);

		// Wait on present (i.e. until the back buffer becomes available), if we haven't already done so
		const SwapChainSurface& surface = mSwapChain->getBackBuffer();
		if(surface.needsWait)
		{
			mSemaphoresTemp[numSemaphores] = mSwapChain->getBackBuffer().sync;
			numSemaphores++;

			mSwapChain->notifyBackBufferWaitIssued();
		}

		queue->present(mSwapChain.get(), mSemaphoresTemp, numSemaphores);
		mRequiresNewBackBuffer = true;

		LinuxPlatform::unlockX();
	}

	void LinuxRenderWindow::copyToMemory(PixelData &dst, FrameBuffer buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		assert(false && "Not implemented");
	}

	void LinuxRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "FB")
		{
			VulkanFramebuffer** fb = (VulkanFramebuffer**)data;
			*fb = mSwapChain->getBackBuffer().framebuffer;
			return;
		}

		if(name == "SC")
		{
			VulkanSwapChain** sc = (VulkanSwapChain**)data;
			*sc = mSwapChain.get();
			return;
		}

		if(name == "LINUX_WINDOW")
		{
			LinuxWindow** window = (LinuxWindow**)data;
			*window = mWindow;
			return;
		}

		if(name == "WINDOW")
		{
			::Window* window = (::Window*)data;
			*window = mWindow->_getXWindow();
			return;
		}
	}

	void LinuxRenderWindow::setActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;

		LinuxPlatform::lockX();

		if(state)
			mWindow->restore();
		else
			mWindow->minimize();

		LinuxPlatform::unlockX();

		RenderWindow::setActive(state);
	}

	void LinuxRenderWindow::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(!hidden)
			mShowOnSwap = false;

		LinuxPlatform::lockX();

		if(hidden)
			mWindow->hide();
		else
			mWindow->show();

		LinuxPlatform::unlockX();

		RenderWindow::setHidden(hidden);
	}

	void LinuxRenderWindow::_windowMovedOrResized()
	{
		if (!mWindow)
			return;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen) // Fullscreen is handled directly by this object
		{
			props.top = mWindow->getTop();
			props.left = mWindow->getLeft();
			props.width = mWindow->getWidth();
			props.height = mWindow->getHeight();
		}

		// Resize swap chain

		//// Need to make sure nothing is using the swap buffer before we re-create it
		// Note: Optionally I can detect exactly on which queues (if any) are the swap chain images used on, and only wait
		// on those
		SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
		presentDevice->waitIdle();

		// Note: This assumes that this method was called from the main message loop, which already acquires X locks,
		// so no need to lock here explicitly
		mSwapChain->rebuild(presentDevice, mSurface, props.width, props.height, props.vsync, mColorFormat, mColorSpace,
				mDesc.depthBuffer, mDepthFormat);

		RenderWindow::_windowMovedOrResized();
	}

	void LinuxRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}
}}

