//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "CoreThread/BsCoreThread.h"
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxRenderWindow.h"
#include "Linux/BsLinuxWindow.h"
#include "Linux/BsLinuxVideoModeInfo.h"
#include "Linux/BsLinuxGLSupport.h"
#include "BsGLPixelFormat.h"
#include "BsGLRenderWindowManager.h"

namespace bs
{
	LinuxRenderWindow::LinuxRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::LinuxGLSupport& glSupport)
		:RenderWindow(desc, windowId), mGLSupport(glSupport), mProperties(desc)
	{ }

	void LinuxRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "WINDOW")
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
	LinuxRenderWindow::LinuxRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, LinuxGLSupport& glsupport)
			: RenderWindow(desc, windowId), mWindow(nullptr), mGLSupport(glsupport), mContext(nullptr), mProperties(desc)
			, mSyncedProperties(desc), mIsChild(false), mShowOnSwap(false), mOldScreenConfig(nullptr)
	{ }

	LinuxRenderWindow::~LinuxRenderWindow()
	{
		RenderWindowProperties& props = mProperties;

		if (mWindow != nullptr)
		{
			LinuxPlatform::lockX();

			if(mOldScreenConfig)
			{
				XRRFreeScreenConfigInfo(mOldScreenConfig);
				mOldScreenConfig = nullptr;
			}

			mWindow->close();

			bs_delete(mWindow);
			mWindow = nullptr;

			LinuxPlatform::unlockX();
		}
	}

	void LinuxRenderWindow::initialize()
	{
		LinuxPlatform::lockX();

		RenderWindowProperties& props = mProperties;

		props.isFullScreen = mDesc.fullscreen;
		mIsChild = false;

		GLVisualConfig visualConfig = mGLSupport.findBestVisual(LinuxPlatform::getXDisplay(), mDesc.depthBuffer,
				mDesc.multisampleCount, mDesc.gamma);

		WINDOW_DESC windowDesc;
		windowDesc.x = mDesc.left;
		windowDesc.y = mDesc.top;
		windowDesc.width = mDesc.videoMode.getWidth();
		windowDesc.height = mDesc.videoMode.getHeight();
		windowDesc.title = mDesc.title;
		windowDesc.showDecorations = !mDesc.toolWindow;
		windowDesc.modal = mDesc.modal;
		windowDesc.visualInfo = visualConfig.visualInfo;
		windowDesc.screen = mDesc.videoMode.getOutputIdx();

		NameValuePairList::const_iterator opt;
		opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (::Window)parseUINT64(opt->second);

		mIsChild = windowDesc.parent != nullptr;
		props.isFullScreen = mDesc.fullscreen && !mIsChild;

		mShowOnSwap = mDesc.hideUntilSwap;
		props.isHidden = mDesc.hideUntilSwap || mDesc.hidden;

		mWindow = bs_new<LinuxWindow>(windowDesc);

		props.width = mWindow->getWidth();
		props.height = mWindow->getHeight();
		props.top = mWindow->getTop();
		props.left = mWindow->getLeft();

		props.hwGamma = visualConfig.caps.srgb;
		props.multisampleCount = visualConfig.caps.numSamples > 1;

		XWindowAttributes windowAttributes;
		XGetWindowAttributes(LinuxPlatform::getXDisplay(), mWindow->_getXWindow(), &windowAttributes);

		XVisualInfo requestVI;
		requestVI.screen = windowDesc.screen;
		requestVI.visualid = XVisualIDFromVisual(windowAttributes.visual);

		LinuxPlatform::unlockX(); // Calls below have their own locking mechanisms

		mContext = mGLSupport.createContext(LinuxPlatform::getXDisplay(), requestVI);

		if(mDesc.fullscreen && !mIsChild)
			setFullscreen(mDesc.videoMode);

		if(mDesc.hideUntilSwap || mDesc.hidden)
			setHidden(true);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindow::initialize();
	}

	void LinuxRenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const LinuxVideoModeInfo& videoModeInfo = static_cast<const LinuxVideoModeInfo&>(RenderAPI::instance() .getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs == 0)
			return;

		RenderWindowProperties& props = mProperties;

		UINT32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
		const LinuxVideoOutputInfo& outputInfo = static_cast<const LinuxVideoOutputInfo&>(videoModeInfo.getOutputInfo (actualMonitorIdx));

		LinuxPlatform::lockX();
		::Display* display = LinuxPlatform::getXDisplay();

		// Change video mode if required
		bool changeVideoMode = false;

		XRRScreenConfiguration* screenConfig = XRRGetScreenInfo(display, XRootWindow(display, DefaultScreen (display)));
		Rotation currentRotation;
		SizeID currentSizeID = XRRConfigCurrentConfiguration(screenConfig, &currentRotation);
		short currentRate = XRRConfigCurrentRate(screenConfig);

		int numSizes;
		XRRScreenSize* screenSizes = XRRConfigSizes(screenConfig, &numSizes);

		if(width != screenSizes[currentSizeID].width || height != screenSizes[currentSizeID].height ||
			currentRate != (short)refreshRate)
			changeVideoMode = true;

		// If provided mode matches current mode, avoid making the video mode change
		if(changeVideoMode)
		{
			// Remember the old config so we can restore it when exiting fullscreen
			if(mOldScreenConfig)
			{
				XRRFreeScreenConfigInfo(mOldScreenConfig);
				mOldScreenConfig = nullptr;
			}

			mOldScreenConfig = screenConfig;
			mOldConfigSizeID = currentSizeID;
			mOldConfigRate = currentRate;

			// Look for size that best matches our requested video mode
			bool foundSize = false;
			SizeID foundSizeID = 0;
			for(int i = 0; i < numSizes; i++)
			{
				UINT32 curWidth, curHeight;
				if(currentRotation == RR_Rotate_90 || currentRotation == RR_Rotate_270)
				{
					curWidth = screenSizes[i].height;
					curHeight = screenSizes[i].width;
				}
				else
				{
					curWidth = screenSizes[i].width;
					curHeight = screenSizes[i].height;
				}

				if(curWidth == width && curHeight == height)
				{
					foundSizeID = i;
					foundSize = true;
					break;
				}
			}

			if(!foundSize)
				LOGERR("Cannot change video mode, requested resolution not supported.");

			// Find refresh rate closest to the requested one, or fall back to 60
			if(foundSize)
			{
				int numRates;
				short* rates = XRRConfigRates(screenConfig, foundSizeID, &numRates);

				short bestRate = 60;
				for(int i = 0; i < numRates; i++)
				{
					if(rates[i] == (short)refreshRate)
					{
						bestRate = rates[i];
						break;
					}
					else
					{
						short diffNew = abs((short)refreshRate - rates[i]);
						short diffOld = abs((short)refreshRate - bestRate);

						if(diffNew < diffOld)
							bestRate = rates[i];
					}
				}

				XRRSetScreenConfigAndRate(display, screenConfig, XRootWindow(display, DefaultScreen(display)),
						foundSizeID, currentRotation, bestRate, CurrentTime);
			}
		}

		mWindow->_setFullscreen(true);

		LinuxPlatform::unlockX();

		props.isFullScreen = true;

		props.top = 0;
		props.left = 0;
		props.width = width;
		props.height = height;

		_windowMovedOrResized();
	}

	void LinuxRenderWindow::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		setFullscreen(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode.getOutputIdx());
	}

	void LinuxRenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
			return;

		props.isFullScreen = false;
		props.width = width;
		props.height = height;

		LinuxPlatform::lockX();

		// Restore old screen config
		if(mOldScreenConfig)
		{
			::Display* display = LinuxPlatform::getXDisplay();
			XRRSetScreenConfigAndRate(display, mOldScreenConfig, XRootWindow(display, DefaultScreen(display)),
					mOldConfigSizeID, mOldConfigRotation, mOldConfigRate, CurrentTime);
			XRRFreeScreenConfigInfo(mOldScreenConfig);
		}
		mWindow->_setFullscreen(false);

		LinuxPlatform::unlockX();

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

	void LinuxRenderWindow::swapBuffers(UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		LinuxPlatform::lockX();
		glXSwapBuffers(LinuxPlatform::getXDisplay(), mWindow->_getXWindow());
		LinuxPlatform::unlockX();
	}

	void LinuxRenderWindow::copyToMemory(PixelData &dst, FrameBuffer buffer)
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

	void LinuxRenderWindow::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "GLCONTEXT")
		{
			SPtr<GLContext>* contextPtr = static_cast<SPtr<GLContext>*>(data);
			*contextPtr = mContext;
			return;
		}
		else if(name == "WINDOW")
		{
			LinuxWindow** window = (LinuxWindow**)data;
			*window = mWindow;
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

		RenderWindow::_windowMovedOrResized();
	}

	void LinuxRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}
}}

