//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#define BS_COCOA_INTERNALS
#include "MacOS/BsMacOSVideoModeInfo.h"
#include "Private/MacOS/BsMacOSWindow.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"
#include "MacOS/BsMacOSRenderWindow.h"
#include "MacOS/BsMacOSGLSupport.h"
#include "MacOS/BsMacOSContext.h"
#include "BsGLRenderWindowManager.h"
#include "BsGLPixelFormat.h"

namespace bs
{
	MacOSRenderWindow::MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::MacOSGLSupport& glSupport)
			:RenderWindow(desc, windowId), mProperties(desc), mGLSupport(glSupport)
	{ }

	void MacOSRenderWindow::initialize()
	{
		RenderWindowProperties& props = mProperties;

		props.isFullScreen = mDesc.fullscreen;
		mIsChild = false;

		WINDOW_DESC windowDesc;
		windowDesc.x = mDesc.left;
		windowDesc.y = mDesc.top;
		windowDesc.width = mDesc.videoMode.getWidth();
		windowDesc.height = mDesc.videoMode.getHeight();
		windowDesc.title = mDesc.title;
		windowDesc.showDecorations = mDesc.showTitleBar;
		windowDesc.allowResize = mDesc.allowResize;
		windowDesc.modal = mDesc.modal;

		auto iter = mDesc.platformSpecific.find("parentWindowHandle");
		mIsChild = iter != mDesc.platformSpecific.end();

		props.isFullScreen = mDesc.fullscreen && !mIsChild;

		props.isHidden = mDesc.hideUntilSwap || mDesc.hidden;

		mWindow = bs_new<CocoaWindow>(windowDesc);
		mWindow->_setUserData(this);

		Rect2I area = mWindow->getArea();
		props.width = area.width;
		props.height = area.height;
		props.top = area.y;
		props.left = area.x;

		props.hwGamma = mDesc.gamma;
		props.multisampleCount = mDesc.multisampleCount;

		mContext = mGLSupport.createContext(mDesc.depthBuffer, mDesc.multisampleCount);

		if(mDesc.fullscreen && !mIsChild)
			setFullscreen(mDesc.videoMode);

		RenderWindow::initialize();

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->mSyncedProperties = props;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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
				desc, mWindowId, mWindow->_getWindowId(), mContext);
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

			bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
			onResized();
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

			bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		}
	}

	void MacOSRenderWindow::hide()
	{
		getMutableProperties().isHidden = true;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = true;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		mWindow->hide();
	}

	void MacOSRenderWindow::show()
	{
		getMutableProperties().isHidden = false;
		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().isHidden = false;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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

		UINT32 outputIdx = videoMode.getOutputIdx();
		if(outputIdx >= videoModeInfo.getNumOutputs())
		{
			LOGERR("Invalid output device index.")
			return;
		}

		const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo (outputIdx);

		if(!videoMode.isCustom())
			setDisplayMode(outputInfo, videoMode);
		else
		{
			// Look for mode matching the requested resolution
			UINT32 foundMode = (UINT32)-1;
			UINT32 numModes = outputInfo.getNumVideoModes();
			for (UINT32 i = 0; i < numModes; i++)
			{
				const VideoMode& currentMode = outputInfo.getVideoMode(i);

				if (currentMode.getWidth() == videoMode.getWidth() && currentMode.getHeight() == videoMode.getHeight())
				{
					foundMode = i;

					if (Math::approxEquals(currentMode.getRefreshRate(), videoMode.getRefreshRate()))
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
		props.width = videoMode.getWidth();
		props.height = videoMode.getHeight();

		{
			ScopedSpinLock lock(getCore()->mLock);
			getCore()->getSyncedProperties().top = props.top;
			getCore()->getSyncedProperties().left = props.left;
			getCore()->getSyncedProperties().width = props.width;
			getCore()->getSyncedProperties().height = props.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
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

		bs::RenderWindowManager::instance().notifySyncDataDirty(getCore().get());
		_windowMovedOrResized();
	}

	void MacOSRenderWindow::setDisplayMode(const VideoOutputInfo& output, const VideoMode& mode)
	{
		CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
		if (CGAcquireDisplayFadeReservation(5.0f, &fadeToken))
			CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);

		auto& destOutput = static_cast<const ct::MacOSVideoOutputInfo&>(output);
		auto& newMode = static_cast<const ct::MacOSVideoMode&>(mode);
		auto& desktopMode = static_cast<const ct::MacOSVideoMode&>(output.getDesktopVideoMode());

		CGDirectDisplayID displayID = destOutput._getDisplayID();

		if (desktopMode._getModeRef() == newMode._getModeRef())
		{
			CGDisplaySetDisplayMode(displayID, newMode._getModeRef(), nullptr);

			if (CGDisplayIsMain(displayID))
				CGReleaseAllDisplays();
			else
				CGDisplayRelease(displayID);
		}
		else
		{
			CGError status;
			if (CGDisplayIsMain(displayID))
				status = CGCaptureAllDisplays();
			else
				status = CGDisplayCapture(displayID);

			if (status != kCGErrorSuccess)
				goto UNFADE;

			status = CGDisplaySetDisplayMode(displayID, newMode._getModeRef(), nullptr);
			if(status != kCGErrorSuccess)
			{
				if (CGDisplayIsMain(displayID))
					CGReleaseAllDisplays();
				else
					CGDisplayRelease(displayID);

				goto UNFADE;
			}
		}

		UNFADE:
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

		mContext->markAsDirty();
	}

	void MacOSRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->_getPropertiesLock());
		mProperties = getCore()->mSyncedProperties;
	}

	namespace ct
	{
		MacOSRenderWindow::MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 renderWindowId, UINT32 cocoaWindowId,
			const SPtr<MacOSContext>& context)
			: RenderWindow(desc, renderWindowId), mShowOnSwap(desc.hideUntilSwap), mCocoaWindowId(cocoaWindowId)
			, mProperties(desc), mSyncedProperties(desc)
		{
			mContext = context;
		}

		void MacOSRenderWindow::initialize()
		{
			if(mDesc.vsync && mDesc.vsyncInterval > 0)
				setVSync(true, mDesc.vsyncInterval);
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

			mContext->setVSync(interval);

			mProperties.vsync = enabled;
			mProperties.vsyncInterval = interval;

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties.vsync = enabled;
				mSyncedProperties.vsyncInterval = interval;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		}

		void MacOSRenderWindow::swapBuffers(UINT32 syncMask)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mShowOnSwap)
				setHidden(false);

			mContext->swapBuffers();
		}

		void MacOSRenderWindow::copyToMemory(PixelData &dst, FrameBuffer buffer)
		{
			THROW_IF_NOT_CORE_THREAD;

			if ((dst.getRight() > getProperties().width) ||
				(dst.getBottom() > getProperties().height) ||
				(dst.getFront() != 0) || (dst.getBack() != 1))
			{
				BS_EXCEPT(InvalidParametersException, "Invalid box.");
			}

			if (buffer == FB_AUTO)
			{
				buffer = mProperties.isFullScreen ? FB_FRONT : FB_BACK;
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

		void MacOSRenderWindow::getCustomAttribute(const String& name, void* data) const
		{
			if(name == "GLCONTEXT")
			{
				SPtr<GLContext>* contextPtr = static_cast<SPtr<GLContext>*>(data);
				*contextPtr = mContext;
				return;
			}
			else if(name == "WINDOW_ID")
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
	}
}

