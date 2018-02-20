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

	void MacOSRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "WINDOW" || name == "COCOA_WINDOW")
		{
			blockUntilCoreInitialized();
			getCore()->getCustomAttribute(name, data);
			return;
		}
	}

	Vector2I MacOSRenderWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		blockUntilCoreInitialized();

		Vector2I point;
		point.x = screenPos.x - mProperties.left;
		point.y = screenPos.y - mProperties.top;

		return point;
	}

	Vector2I MacOSRenderWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		blockUntilCoreInitialized();

		Vector2I point;
		point.x = windowPos.x + mProperties.left;
		point.y = windowPos.y + mProperties.top;

		return point;
	}

	SPtr<ct::MacOSRenderWindow> MacOSRenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::MacOSRenderWindow>(mCoreSpecific);
	}

	void MacOSRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->_getPropertiesLock());
		mProperties = getCore()->mSyncedProperties;
	}

	namespace ct
	{
		MacOSRenderWindow::MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, MacOSGLSupport& glsupport)
			: RenderWindow(desc, windowId), mWindow(nullptr), mContext(nullptr), mGLSupport(glsupport), mProperties(desc)
			, mSyncedProperties(desc), mIsChild(false), mShowOnSwap(false)
		{ }

		MacOSRenderWindow::~MacOSRenderWindow()
		{
			// Make sure to set the original desktop video mode before we exit
			if(mProperties.isFullScreen)
				setWindowed(50, 50);

			if (mWindow != nullptr)
			{
				bs_delete(mWindow);
				mWindow = nullptr;
			}
		}

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

			mShowOnSwap = mDesc.hideUntilSwap;
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

			if(mDesc.vsync && mDesc.vsyncInterval > 0)
				setVSync(true, mDesc.vsyncInterval);

			mScreenArea = mWindow->_getScreenArea();
			mNativeWindowArea = mWindow->_getScreenArea();

			{
				ScopedSpinLock lock(mLock);
				mSyncedProperties = props;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
			RenderWindow::initialize();
		}

		void MacOSRenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
		{
			THROW_IF_NOT_CORE_THREAD;

			VideoMode videoMode(width, height, refreshRate, monitorIdx);
			setFullscreen(videoMode);
		}

		void MacOSRenderWindow::setFullscreen(const VideoMode& mode)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mIsChild)
				return;

			const VideoModeInfo& videoModeInfo = RenderAPI::instance().getVideoModeInfo();

			UINT32 outputIdx = mode.getOutputIdx();
			if(outputIdx >= videoModeInfo.getNumOutputs())
			{
				LOGERR("Invalid output device index.")
				return;
			}

			const VideoOutputInfo& outputInfo = videoModeInfo.getOutputInfo (outputIdx);

			if(!mode.isCustom())
				setDisplayMode(outputInfo, mode);
			else
			{
				// Look for mode matching the requested resolution
				UINT32 foundMode = (UINT32)-1;
				UINT32 numModes = outputInfo.getNumVideoModes();
				for (UINT32 i = 0; i < numModes; i++)
				{
					const VideoMode& currentMode = outputInfo.getVideoMode(i);

					if (currentMode.getWidth() == mode.getWidth() && currentMode.getHeight() == mode.getHeight())
					{
						foundMode = i;

						if (Math::approxEquals(currentMode.getRefreshRate(), mode.getRefreshRate()))
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
			props.width = mode.getWidth();
			props.height = mode.getHeight();

			_windowMovedOrResized();
		}

		void MacOSRenderWindow::setWindowed(UINT32 width, UINT32 height)
		{
			THROW_IF_NOT_CORE_THREAD;

			RenderWindowProperties& props = mProperties;

			if (!props.isFullScreen)
				return;

			// Restore original display mode
			const VideoModeInfo& videoModeInfo = RenderAPI::instance().getVideoModeInfo();

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
				ScopedSpinLock lock(mLock);
				mSyncedProperties.width = props.width;
				mSyncedProperties.height = props.height;
			}

			bs::RenderWindowManager::instance().notifySyncDataDirty(this);
			_windowMovedOrResized();
		}

		void MacOSRenderWindow::setDisplayMode(const VideoOutputInfo& output, const VideoMode& mode)
		{
			CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
			if (CGAcquireDisplayFadeReservation(5.0f, &fadeToken))
				CGDisplayFade(fadeToken, 0.3f, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);

			auto& destOutput = static_cast<const MacOSVideoOutputInfo&>(output);
			auto& newMode = static_cast<const MacOSVideoMode&>(mode);
			auto& desktopMode = static_cast<const MacOSVideoMode&>(output.getDesktopVideoMode());

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

		void MacOSRenderWindow::move(INT32 left, INT32 top)
		{
			THROW_IF_NOT_CORE_THREAD;

			RenderWindowProperties& props = mProperties;
			if (!props.isFullScreen)
			{
				mWindow->move(left, top);

				Rect2I area = mWindow->getArea();
				props.top = area.y;
				props.left = area.x;

				{
					ScopedSpinLock lock(mLock);
					mSyncedProperties.top = props.top;
					mSyncedProperties.left = props.left;
				}

				mScreenArea = mWindow->_getScreenArea();
				mNativeWindowArea = mWindow->_getScreenArea();

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
			}
		}

		void MacOSRenderWindow::resize(UINT32 width, UINT32 height)
		{
			THROW_IF_NOT_CORE_THREAD;

			RenderWindowProperties& props = mProperties;
			if (!props.isFullScreen)
			{
				mWindow->resize(width, height);

				Rect2I area = mWindow->getArea();
				props.width = area.width;
				props.height = area.height;

				{
					ScopedSpinLock lock(mLock);
					mSyncedProperties.width = props.width;
					mSyncedProperties.height = props.height;
				}

				mScreenArea = mWindow->_getScreenArea();
				mNativeWindowArea = mWindow->_getScreenArea();

				bs::RenderWindowManager::instance().notifySyncDataDirty(this);
			}
		}

		void MacOSRenderWindow::minimize()
		{
			THROW_IF_NOT_CORE_THREAD;

			mWindow->minimize();
		}

		void MacOSRenderWindow::maximize()
		{
			THROW_IF_NOT_CORE_THREAD;

			mWindow->maximize();
		}

		void MacOSRenderWindow::restore()
		{
			THROW_IF_NOT_CORE_THREAD;

			mWindow->restore();
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
			else if(name == "COCOA_WINDOW")
			{
				CocoaWindow** window = (CocoaWindow**)data;
				*window = mWindow;
				return;
			}
		}

		void MacOSRenderWindow::setActive(bool state)
		{
			THROW_IF_NOT_CORE_THREAD;

			if(state)
				mWindow->restore();
			else
				mWindow->minimize();

			RenderWindow::setActive(state);
		}

		void MacOSRenderWindow::setHidden(bool hidden)
		{
			THROW_IF_NOT_CORE_THREAD;

			if(!hidden)
				mShowOnSwap = false;

			if(hidden)
				mWindow->hide();
			else
				mWindow->show();

			RenderWindow::setHidden(hidden);
		}

		void MacOSRenderWindow::_windowMovedOrResized()
		{
			if (!mWindow)
				return;

			Lock lock(mDimensionsMutex);

			RenderWindowProperties& props = mProperties;
			if (!props.isFullScreen) // Fullscreen is handled directly by this object
			{
				Rect2I area = mWindow->getArea();

				props.top = area.y;
				props.left = area.x;
				props.width = area.width;
				props.height = area.height;
			}

			mScreenArea = mWindow->_getScreenArea();
			mNativeWindowArea = mWindow->getArea(false);

			RenderWindow::_windowMovedOrResized();
		}

		void MacOSRenderWindow::syncProperties()
		{
			ScopedSpinLock lock(mLock);
			mProperties = mSyncedProperties;
		}
	}
}

