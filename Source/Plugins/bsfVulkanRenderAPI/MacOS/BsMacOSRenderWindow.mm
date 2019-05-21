//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS

#include "MacOS/BsMacOSVideoModeInfo.h"
#include "Private/MacOS/BsMacOSWindow.h"
#include "Private/MacOS/BsMacOSPlatform.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"
#include "MacOS/BsMacOSRenderWindow.h"
#include "Managers/BsVulkanRenderWindowManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanQueue.h"
#include "Managers/BsVulkanCommandBufferManager.h"

#import <QuartzCore/QuartzCore.h>

namespace bs
{
	MacOSRenderWindow::MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
			:RenderWindow(desc, windowId), mProperties(desc)
	{ }

	void MacOSRenderWindow::initialize()
	{
		RenderWindowProperties& props = mProperties;

		props.isFullScreen = mDesc.fullscreen;
		mIsChild = false;

		WINDOW_DESC windowDesc;
		windowDesc.x = mDesc.left;
		windowDesc.y = mDesc.top;
		windowDesc.width = mDesc.videoMode.width;
		windowDesc.height = mDesc.videoMode.height;
		windowDesc.title = mDesc.title;
		windowDesc.showDecorations = mDesc.showTitleBar;
		windowDesc.allowResize = mDesc.allowResize;
		windowDesc.modal = mDesc.modal;
		windowDesc.floating = mDesc.toolWindow;

		auto iter = mDesc.platformSpecific.find("parentWindowHandle");
		mIsChild = iter != mDesc.platformSpecific.end();

		props.isFullScreen = mDesc.fullscreen && !mIsChild;
		props.isHidden = mDesc.hidden;

		mWindow = bs_new<CocoaWindow>(windowDesc);
		mWindow->_setUserData(this);

		Rect2I area = mWindow->getArea();
		props.width = area.width;
		props.height = area.height;
		props.top = area.y;
		props.left = area.x;
		props.hasFocus = true;

		props.hwGamma = mDesc.gamma;
		props.multisampleCount = mDesc.multisampleCount;

		if(mDesc.fullscreen && !mIsChild)
			setFullscreen(mDesc.videoMode);

		RenderWindow::initialize();

		if(props.isHidden)
			mWindow->hide();

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->mSyncedProperties = props;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());

		// New windows always receive focus, but we don't receive an initial event from the OS, so trigger one manually
		RenderWindowManager::instance().notifyFocusReceived(getCore().get());
	}

	void MacOSRenderWindow::destroy()
	{
		// Make sure to set the original desktop video mode before we exit
		if(mProperties.isFullScreen)
			setWindowed(50, 50);

		if (mWindow != nullptr)
		{
			bs_delete(mWindow);
			mWindow = nullptr;
		}

		RenderWindow::destroy();
	}

	SPtr<ct::CoreObject> MacOSRenderWindow::createCore() const
	{
		RENDER_WINDOW_DESC desc = mDesc;
		SPtr<ct::CoreObject> obj = bs_shared_ptr_new<ct::MacOSRenderWindow>(
				desc, mWindowId, mWindow->_getWindowId());
		obj->_setThisPtr(obj);
		return obj;
	}

	void MacOSRenderWindow::resize(UINT32 width, UINT32 height)
	{
		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			mWindow->resize(width, height);

			Rect2I area = mWindow->getArea();
			props.width = area.width;
			props.height = area.height;

			{
				ScopedSpinLock lock(getCore()->mLock);
				getCore()->getSyncedProperties().width = width;
				getCore()->getSyncedProperties().height = height;
			}

			ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		}
	}

	void MacOSRenderWindow::move(INT32 left, INT32 top)
	{
		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen)
		{
			mWindow->move(left, top);

			Rect2I area = mWindow->getArea();
			props.top = area.y;
			props.left = area.x;

			{
				ScopedSpinLock lock(getCore()->mLock);
				getCore()->getSyncedProperties().top = props.top;
				getCore()->getSyncedProperties().left = props.left;
			}

			ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		}
	}

	void MacOSRenderWindow::hide()
	{
		getMutableProperties().isHidden = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->hide();
	}

	void MacOSRenderWindow::show()
	{
		getMutableProperties().isHidden = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->show();
	}

	void MacOSRenderWindow::minimize()
	{
		RenderWindowProperties& props = mProperties;
		props.isMaximized = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->minimize();
	}

	void MacOSRenderWindow::maximize()
	{
		RenderWindowProperties& props = mProperties;
		props.isMaximized = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = true;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->maximize();
	}

	void MacOSRenderWindow::restore()
	{
		RenderWindowProperties& props = mProperties;
		props.isMaximized = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isMaximized = false;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->restore();
	}

	void MacOSRenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		VideoMode videoMode(width, height, refreshRate, monitorIdx);
		setFullscreen(videoMode);
	}

	void MacOSRenderWindow::setFullscreen(const VideoMode& videoMode)
	{
		if (mIsChild)
			return;

		const VideoModeInfo& videoModeInfo = ct::RenderAPI::instance().getVideoModeInfo();

		UINT32 outputIdx = videoMode.outputIdx;
		if(outputIdx >= videoModeInfo.getNumOutputs())
		{
			LOGERR("Invalid output device index.")
			return;
		}

		const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo (outputIdx);

		if(!videoMode.isCustom)
			setDisplayMode(outputInfo, videoMode);
		else
		{
			// Look for mode matching the requested resolution
			UINT32 foundMode = (UINT32)-1;
			UINT32 numModes = outputInfo.getNumVideoModes();
			for (UINT32 i = 0; i < numModes; i++)
			{
				const VideoMode& currentMode = outputInfo.getVideoMode(i);

				if (currentMode.width == videoMode.width && currentMode.height == videoMode.height)
				{
					foundMode = i;

					if (Math::approxEquals(currentMode.refreshRate, videoMode.refreshRate))
						break;
				}
			}

			if (foundMode == (UINT32)-1)
			{
				LOGERR("Unable to enter fullscreen, unsupported video mode requested.");
				return;
			}

			setDisplayMode(outputInfo, outputInfo.getVideoMode(foundMode));
		}

		mWindow->setFullscreen();

		RenderWindowProperties& props = mProperties;
		props.isFullScreen = true;

		props.top = 0;
		props.left = 0;
		props.width = videoMode.width;
		props.height = videoMode.height;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().top = props.top;
			getCore()->getSyncedProperties().left = props.left;
			getCore()->getSyncedProperties().width = props.width;
			getCore()->getSyncedProperties().height = props.height;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		_windowMovedOrResized();
	}

	void MacOSRenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
			return;

		// Restore original display mode
		const VideoModeInfo& videoModeInfo = ct::RenderAPI::instance().getVideoModeInfo();

		UINT32 outputIdx = 0; // 0 is always primary
		if(outputIdx >= videoModeInfo.getNumOutputs())
		{
			LOGERR("Invalid output device index.")
			return;
		}

		const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo(outputIdx);
		setDisplayMode(outputInfo, outputInfo.getDesktopVideoMode());

		mWindow->setWindowed();

		props.isFullScreen = false;
		props.width = width;
		props.height = height;

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().width = props.width;
			getCore()->getSyncedProperties().height = props.height;
		}

		ct::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		_windowMovedOrResized();
	}

	void MacOSRenderWindow::setDisplayMode(const VideoOutputInfo& output, const VideoMode& mode)
	{
		CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
		if (CGAcquireDisplayFadeReservation(5.0f, &fadeToken))
			CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);

		auto& destOutput = static_cast<const ct::MacOSVideoOutputInfo&>(output);
		auto& newMode = static_cast<const ct::MacOSVideoMode&>(mode);

		// Note: An alternative to changing display resolution would be to only change the back-buffer size. But that doesn't
		// account for refresh rate, so it's questionable how useful it would be.
		CGDirectDisplayID displayID = destOutput._getDisplayID();
		CGDisplaySetDisplayMode(displayID, newMode._getModeRef(), nullptr);

		if (fadeToken != kCGDisplayFadeReservationInvalidToken)
		{
			CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, FALSE);
			CGReleaseDisplayFadeReservation(fadeToken);
		}
	}

	void MacOSRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "COCOA_WINDOW")
		{
			CocoaWindow** window = (CocoaWindow**)data;
			*window = mWindow;
			return;
		}
		else if(name == "WINDOW_ID")
		{
			UINT32* windowId = (UINT32*)data;
			*windowId = mWindow->_getWindowId();
		}
	}

	Vector2I MacOSRenderWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		return mWindow->screenToWindowPos(screenPos);
	}

	Vector2I MacOSRenderWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		return mWindow->windowToScreenPos(windowPos);
	}

	SPtr<ct::MacOSRenderWindow> MacOSRenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::MacOSRenderWindow>(mCoreSpecific);
	}

	void MacOSRenderWindow::_windowMovedOrResized()
	{
		if (!mWindow)
			return;

		RenderWindowProperties& props = mProperties;
		if (!props.isFullScreen) // Fullscreen is handled directly by this object
		{
			Rect2I area = mWindow->getArea();

			props.top = area.y;
			props.left = area.x;
			props.width = area.width;
			props.height = area.height;
		}

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().top = props.top;
			getCore()->getSyncedProperties().left = props.left;
			getCore()->getSyncedProperties().width = props.width;
			getCore()->getSyncedProperties().height = props.height;
		}
	}

	void MacOSRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->_getPropertiesLock());
		mProperties = getCore()->mSyncedProperties;
	}

	namespace ct
	{
		MacOSRenderWindow::MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 renderWindowId, UINT32 cocoaWindowId)
			: RenderWindow(desc, renderWindowId)
			, mRenderAPI(static_cast<VulkanRenderAPI&>(RenderAPI::instance()))
			, mShowOnSwap(false), mCocoaWindowId(cocoaWindowId), mProperties(desc), mSyncedProperties(desc)
		{ }

		MacOSRenderWindow::~MacOSRenderWindow()
		{
			// TODO - Careful to detach this from the window/view in a thread-safe way first?

			SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
			presentDevice->waitIdle();

			mSwapChain->destroy();
			vkDestroySurfaceKHR(mRenderAPI._getInstance(), mSurface, gVulkanAllocator);
		}

		void MacOSRenderWindow::initialize()
		{
			MacOSPlatform::lockWindows();

			CocoaWindow* window = MacOSPlatform::getWindow(mCocoaWindowId);
			if(!window)
			{
				MacOSPlatform::unlockWindows();
				return;
			}

			CAMetalLayer* layer = [[CAMetalLayer alloc] init];
			window->_setLayer((__bridge void *)layer);

			// Create Vulkan surface
			VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
			surfaceCreateInfo.pNext = nullptr;
			surfaceCreateInfo.flags = 0;
			surfaceCreateInfo.pView = (__bridge const void *)(layer);

			VkResult result = vkCreateMacOSSurfaceMVK(mRenderAPI._getInstance(), &surfaceCreateInfo,
					bs::ct::gVulkanAllocator, &mSurface);
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
			mSwapChain = presentDevice->getResourceManager().create<VulkanSwapChain>(mSurface,
					mProperties.width, mProperties.height, mProperties.vsync, mColorFormat, mColorSpace, mDesc.depthBuffer,
					mDepthFormat);

			if(mDesc.vsync && mDesc.vsyncInterval > 0)
				setVSync(true, mDesc.vsyncInterval);

			MacOSPlatform::unlockWindows();
		}

		void MacOSRenderWindow::move(INT32 left, INT32 top)
		{
			// Do nothing
		}

		void MacOSRenderWindow::resize(UINT32 width, UINT32 height)
		{
			// Do nothing
		}

		void MacOSRenderWindow::setVSync(bool enabled, UINT32 interval)
		{
			THROW_IF_NOT_CORE_THREAD;

			if(!enabled)
				interval = 0;

			rebuildSwapChain();

			mProperties.vsync = enabled;
			mProperties.vsyncInterval = interval;

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.vsync = enabled;
				mSyncedProperties.vsyncInterval = interval;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		}

		void MacOSRenderWindow::acquireBackBuffer()
		{
			// We haven't presented the current back buffer yet, so just use that one
			if (!mRequiresNewBackBuffer)
				return;

			VkResult acquireResult = mSwapChain->acquireBackBuffer();
			if(acquireResult == VK_SUBOPTIMAL_KHR || acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
			{
				rebuildSwapChain();
				mSwapChain->acquireBackBuffer();
			}

			mRequiresNewBackBuffer = false;
		}

		void MacOSRenderWindow::swapBuffers(UINT32 syncMask)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mShowOnSwap)
				setHidden(false);

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

			VkResult presentResult = queue->present(mSwapChain, mSemaphoresTemp, numSemaphores);
			if(presentResult == VK_SUBOPTIMAL_KHR || presentResult == VK_ERROR_OUT_OF_DATE_KHR)
				rebuildSwapChain();

			mRequiresNewBackBuffer = true;
		}

		void MacOSRenderWindow::getCustomAttribute(const String& name, void* data) const
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
				*sc = mSwapChain;
				return;
			}

			if(name == "WINDOW_ID")
			{
				UINT32* windowId = (UINT32*)data;
				*windowId = mCocoaWindowId;
			}
		}

		void MacOSRenderWindow::syncProperties()
		{
			ScopedSpinLock lock(mLock);
			mProperties = mSyncedProperties;
		}

		void MacOSRenderWindow::rebuildSwapChain()
		{
			//// Need to make sure nothing is using the swap buffer before we re-create it
			// Note: Optionally I can detect exactly on which queues (if any) are the swap chain images used on, and only wait
			// on those
			SPtr<VulkanDevice> presentDevice = mRenderAPI._getPresentDevice();
			presentDevice->waitIdle();

			VulkanSwapChain* oldSwapChain = mSwapChain;

			mSwapChain = presentDevice->getResourceManager().create<VulkanSwapChain>(
					mSurface, mProperties.width, mProperties.height, mProperties.vsync, mColorFormat, mColorSpace,
					mDesc.depthBuffer, mDepthFormat, oldSwapChain);

			oldSwapChain->destroy();
		}
	}
}

