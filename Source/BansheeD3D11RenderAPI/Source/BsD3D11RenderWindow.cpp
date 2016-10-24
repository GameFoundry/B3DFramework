//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderWindow.h"
#include "BsCoreThread.h"
#include "Win32/BsWin32Platform.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderTexture.h"
#include "BsD3D11TextureView.h"
#include "BsTextureManager.h"
#include "BsD3D11DriverList.h"
#include "BsD3D11Driver.h"
#include "BsD3D11VideoModeInfo.h"
#include "BsRenderStats.h"
#include "BsInput.h"
#include "BsException.h"
#include "BsRenderWindowManager.h"
#include "BsMath.h"
#include "Win32/BsWin32Window.h"

namespace BansheeEngine
{
	D3D11RenderWindowProperties::D3D11RenderWindowProperties(const RENDER_WINDOW_DESC& desc)
		:RenderWindowProperties(desc)
	{ }

	D3D11RenderWindowCore::D3D11RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId, D3D11Device& device, IDXGIFactory* DXGIFactory)
		: RenderWindowCore(desc, windowId), mProperties(desc), mSyncedProperties(desc), mDevice(device), mDXGIFactory(DXGIFactory), 
		mSizing(false), mRenderTargetView(nullptr), mBackBuffer(nullptr), mSwapChain(nullptr), mDepthStencilView(nullptr), mIsChild(false), 
		mRefreshRateNumerator(0), mRefreshRateDenominator(0), mShowOnSwap(false), mWindow(nullptr)
	{ }

	D3D11RenderWindowCore::~D3D11RenderWindowCore()
	{ 
		D3D11RenderWindowProperties& props = mProperties;

		props.mActive = false;

		if (props.isFullScreen())
			mSwapChain->SetFullscreenState(false, nullptr);

		SAFE_RELEASE(mSwapChain);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_SwapChain);

		if (mWindow != nullptr)
		{
			bs_delete(mWindow);
			mWindow = nullptr;
		}

		if (mDepthStencilView != nullptr)
			TextureCore::releaseView(mDepthStencilView);

		destroySizeDependedD3DResources();
	}

	void D3D11RenderWindowCore::initialize()
	{
		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		D3D11RenderWindowProperties& props = mProperties;

		mMultisampleType.Count = 1;
		mMultisampleType.Quality = 0;

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
		windowDesc.module = GetModuleHandle("BansheeD3D11RenderAPI.dll");
#endif

		NameValuePairList::const_iterator opt;
		opt = mDesc.platformSpecific.find("parentWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.parent = (HWND)parseUINT64(opt->second);

		opt = mDesc.platformSpecific.find("externalWindowHandle");
		if (opt != mDesc.platformSpecific.end())
			windowDesc.external = (HWND)parseUINT64(opt->second);

		mIsChild = windowDesc.parent != nullptr;
		props.mIsFullScreen = mDesc.fullscreen && !mIsChild;
		props.mColorDepth = 32;
		props.mActive = true;

		if (mDesc.videoMode.isCustom())
		{
			mRefreshRateNumerator = Math::roundToInt(mDesc.videoMode.getRefreshRate());
			mRefreshRateDenominator = 1;
		}
		else
		{
			const D3D11VideoMode& d3d11videoMode = static_cast<const D3D11VideoMode&>(mDesc.videoMode);
			mRefreshRateNumerator = d3d11videoMode.getRefreshRateNumerator();
			mRefreshRateDenominator = d3d11videoMode.getRefreshRateDenominator();
		}

		const D3D11VideoOutputInfo* outputInfo = nullptr;

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs > 0)
		{
			UINT32 actualMonitorIdx = std::min(mDesc.videoMode.getOutputIdx(), numOutputs - 1);
			outputInfo = static_cast<const D3D11VideoOutputInfo*>(&videoModeInfo.getOutputInfo(actualMonitorIdx));

			DXGI_OUTPUT_DESC desc;
			outputInfo->getDXGIOutput()->GetDesc(&desc);

			windowDesc.monitor = desc.Monitor;
		}

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

		createSwapChain();

		if (props.isFullScreen())
		{
			if (outputInfo != nullptr)
				mSwapChain->SetFullscreenState(true, outputInfo->getDXGIOutput());
			else
				mSwapChain->SetFullscreenState(true, nullptr);
		}

		createSizeDependedD3DResources();
		mDXGIFactory->MakeWindowAssociation(mWindow->getHWnd(), NULL);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties = props;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowCore::initialize();
	}

	void D3D11RenderWindowCore::swapBuffers()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		if(mDevice.getD3D11Device() != nullptr)
		{
			HRESULT hr = mSwapChain->Present(getProperties().getVSync() ? getProperties().getVSyncInterval() : 0, 0);

			if( FAILED(hr) )
				BS_EXCEPT(RenderingAPIException, "Error Presenting surfaces");
		}
	}

	void D3D11RenderWindowCore::move(INT32 left, INT32 top)
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D11RenderWindowProperties& props = mProperties;

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

	void D3D11RenderWindowCore::resize(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D11RenderWindowProperties& props = mProperties;

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

	void D3D11RenderWindowCore::setActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D11RenderWindowProperties& props = mProperties;
		mWindow->setActive(state);

		if (mSwapChain)
		{
			if (state)
				mSwapChain->SetFullscreenState(props.mIsFullScreen, nullptr);
			else
				mSwapChain->SetFullscreenState(FALSE, nullptr);
		}

		RenderWindowCore::setActive(state);
	}

	void D3D11RenderWindowCore::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		mShowOnSwap = false;
		mWindow->setHidden(hidden);

		RenderWindowCore::setHidden(hidden);
	}

	void D3D11RenderWindowCore::minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->minimize();
	}

	void D3D11RenderWindowCore::maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->maximize();
	}

	void D3D11RenderWindowCore::restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->restore();
	}

	void D3D11RenderWindowCore::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs == 0)
			return;

		UINT32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
		const D3D11VideoOutputInfo& outputInfo = static_cast<const D3D11VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));

		DXGI_MODE_DESC modeDesc;
		ZeroMemory(&modeDesc, sizeof(modeDesc));

		modeDesc.Width = width;
		modeDesc.Height = height;
		modeDesc.RefreshRate.Numerator = Math::roundToInt(refreshRate);
		modeDesc.RefreshRate.Denominator = 1;
		modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_MODE_DESC nearestMode;
		ZeroMemory(&nearestMode, sizeof(nearestMode));

		outputInfo.getDXGIOutput()->FindClosestMatchingMode(&modeDesc, &nearestMode, nullptr);

		mProperties.mIsFullScreen = true;
		mProperties.mWidth = width;
		mProperties.mHeight = height;

		mSwapChain->ResizeTarget(&nearestMode);
		mSwapChain->SetFullscreenState(true, outputInfo.getDXGIOutput()); 

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.mTop = mProperties.mTop;
			mSyncedProperties.mLeft = mProperties.mLeft;
			mSyncedProperties.mWidth = mProperties.mWidth;
			mSyncedProperties.mHeight = mProperties.mHeight;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void D3D11RenderWindowCore::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		if (mode.isCustom())
		{
			setFullscreen(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode.getOutputIdx());
			return;
		}

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPICore::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs == 0)
			return;

		UINT32 actualMonitorIdx = std::min(mode.getOutputIdx(), numOutputs - 1);
		const D3D11VideoOutputInfo& outputInfo = static_cast<const D3D11VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));

		const D3D11VideoMode& videoMode = static_cast<const D3D11VideoMode&>(mode);

		mProperties.mIsFullScreen = true;
		mProperties.mWidth = mode.getWidth();
		mProperties.mHeight = mode.getHeight();

		mSwapChain->ResizeTarget(&videoMode.getDXGIModeDesc());
		mSwapChain->SetFullscreenState(true, outputInfo.getDXGIOutput());

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.mTop = mProperties.mTop;
			mSyncedProperties.mLeft = mProperties.mLeft;
			mSyncedProperties.mWidth = mProperties.mWidth;
			mSyncedProperties.mHeight = mProperties.mHeight;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void D3D11RenderWindowCore::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		mProperties.mWidth = width;
		mProperties.mHeight = height;
		mProperties.mIsFullScreen = false;

		mSwapChainDesc.Windowed = true;
		mSwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		mSwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		mSwapChainDesc.BufferDesc.Width = width;
		mSwapChainDesc.BufferDesc.Height = height;

		DXGI_MODE_DESC modeDesc;
		ZeroMemory(&modeDesc, sizeof(modeDesc));

		modeDesc.Width = width;
		modeDesc.Height = height;
		modeDesc.RefreshRate.Numerator = 0;
		modeDesc.RefreshRate.Denominator = 0;
		modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		mSwapChain->SetFullscreenState(false, nullptr);
		mSwapChain->ResizeTarget(&modeDesc);

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.mTop = mProperties.mTop;
			mSyncedProperties.mLeft = mProperties.mLeft;
			mSyncedProperties.mWidth = mProperties.mWidth;
			mSyncedProperties.mHeight = mProperties.mHeight;
		}

		RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	HWND D3D11RenderWindowCore::_getWindowHandle() const
	{
		return mWindow->getHWnd();
	}

	void D3D11RenderWindowCore::getCustomAttribute(const String& name, void* pData) const
	{
		if(name == "WINDOW")
		{
			UINT64 *pWnd = (UINT64*)pData;
			*pWnd = (UINT64)mWindow->getHWnd();
			return;
		}

		if(name == "RTV")
		{
			*static_cast<ID3D11RenderTargetView**>(pData) = mRenderTargetView;
			return;
		}
		else if(name == "DSV")
		{
			if (mDepthStencilView != nullptr)
			{
				D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(false);
			}
			else
			{
				*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
			}

			return;
		}
		else if (name == "RODSV")
		{
			if (mDepthStencilView != nullptr)
			{
				D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(true);
			}
			else
			{
				*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
			}

			return;
		}

		RenderWindowCore::getCustomAttribute(name, pData);
	}

	void D3D11RenderWindowCore::copyToMemory(PixelData &dst, FrameBuffer buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mBackBuffer == nullptr)
			return;

		// Get the backbuffer desc
		D3D11_TEXTURE2D_DESC BBDesc;
		mBackBuffer->GetDesc(&BBDesc);

		ID3D11Texture2D* backbuffer = nullptr;

		if(BBDesc.SampleDesc.Quality > 0)
		{
			D3D11_TEXTURE2D_DESC desc = BBDesc;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
			desc.BindFlags = 0;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;

			HRESULT hr = mDevice.getD3D11Device()->CreateTexture2D(&desc, nullptr, &backbuffer);

			if (FAILED(hr) || mDevice.hasError())
			{
				String errorDescription = mDevice.getErrorDescription();
				BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
			}

			mDevice.getImmediateContext()->ResolveSubresource(backbuffer, D3D11CalcSubresource(0, 0, 1), mBackBuffer, D3D11CalcSubresource(0, 0, 1), desc.Format);
		}

		// Change the parameters of the texture so we can read it
		BBDesc.Usage = D3D11_USAGE_STAGING;
		BBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		BBDesc.BindFlags = 0;
		BBDesc.SampleDesc.Quality = 0;
		BBDesc.SampleDesc.Count = 1;

		// Create a temp buffer to copy to
		ID3D11Texture2D* tempTexture;
		HRESULT hr = mDevice.getD3D11Device()->CreateTexture2D(&BBDesc, nullptr, &tempTexture);

		if (FAILED(hr) || mDevice.hasError())
		{
			String errorDescription = mDevice.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
		}

		// Copy the back buffer
		mDevice.getImmediateContext()->CopyResource(tempTexture, backbuffer != NULL ? backbuffer : mBackBuffer);

		// Map the copied texture
		D3D11_MAPPED_SUBRESOURCE mappedTex2D;
		mDevice.getImmediateContext()->Map(tempTexture, 0,D3D11_MAP_READ, 0, &mappedTex2D);

		// Copy the the texture to the dest
		PixelData src(getProperties().getWidth(), getProperties().getHeight(), 1, PF_A8B8G8R8);
		src.setExternalBuffer((UINT8*)mappedTex2D.pData);
		PixelUtil::bulkPixelConversion(src, dst);

		// Unmap the temp buffer
		mDevice.getImmediateContext()->Unmap(tempTexture, 0);

		// Release the temp buffer
		SAFE_RELEASE(tempTexture);
		SAFE_RELEASE(backbuffer);
	}

	void D3D11RenderWindowCore::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (!mWindow)
			return;

		mWindow->_windowMovedOrResized();

		D3D11RenderWindowProperties& props = mProperties;

		if (props.isFullScreen()) // Fullscreen is handled directly by this object
		{
			resizeSwapChainBuffers(props.getWidth(), props.getHeight());
		}
		else
		{
			resizeSwapChainBuffers(mWindow->getWidth(), mWindow->getHeight());
			props.mWidth = mWindow->getWidth();
			props.mHeight = mWindow->getHeight();
			props.mTop = mWindow->getTop();
			props.mLeft = mWindow->getLeft();
		}

		RenderWindowCore::_windowMovedOrResized();
	}

	void D3D11RenderWindowCore::createSwapChain()
	{
		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		D3D11RenderWindowProperties& props = mProperties;
		IDXGIDevice* pDXGIDevice = queryDxgiDevice();

		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mSwapChainDesc.OutputWindow = mWindow->getHWnd();
		mSwapChainDesc.BufferDesc.Width = props.mWidth;
		mSwapChainDesc.BufferDesc.Height = props.mHeight;
		mSwapChainDesc.BufferDesc.Format = format;

		if (props.mIsFullScreen)
		{
			mSwapChainDesc.BufferDesc.RefreshRate.Numerator = mRefreshRateNumerator;
			mSwapChainDesc.BufferDesc.RefreshRate.Denominator = mRefreshRateDenominator;
		}
		else
		{
			mSwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			mSwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		}

		mSwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mSwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		mSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ;

		mSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		mSwapChainDesc.BufferCount = 1;
		mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD ;

		mSwapChainDesc.Windowed	= true;

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		rs->determineMultisampleSettings(props.mMultisampleCount, format, &mMultisampleType);
		mSwapChainDesc.SampleDesc.Count = mMultisampleType.Count;
		mSwapChainDesc.SampleDesc.Quality = mMultisampleType.Quality;
		
		HRESULT hr;

		// Create swap chain			
		hr = mDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mSwapChain);

		if (FAILED(hr))
		{
			// Try a second time, may fail the first time due to back buffer count,
			// which will be corrected by the runtime
			hr = mDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mSwapChain);
		}

		SAFE_RELEASE(pDXGIDevice);

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Unable to create swap chain. Error code: " + toString(hr));

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_SwapChain);
	}

	void D3D11RenderWindowCore::createSizeDependedD3DResources()
	{
		SAFE_RELEASE(mBackBuffer);

		HRESULT hr = mSwapChain->GetBuffer(0,  __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBuffer);
		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Unable to Get Back Buffer for swap chain");

		assert(mBackBuffer && !mRenderTargetView);

		D3D11_TEXTURE2D_DESC BBDesc;
		mBackBuffer->GetDesc(&BBDesc);

		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
		ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

		RTVDesc.Format = BBDesc.Format;
		RTVDesc.ViewDimension = getProperties().getMultisampleCount() > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		hr = mDevice.getD3D11Device()->CreateRenderTargetView(mBackBuffer, &RTVDesc, &mRenderTargetView);

		if(FAILED(hr))
		{
			String errorDescription = mDevice.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Unable to create rendertagert view\nError Description:" + errorDescription);
		}

		if (mDepthStencilView != nullptr)
		{
			TextureCore::releaseView(mDepthStencilView);
			mDepthStencilView = nullptr;
		}

		if (mDesc.depthBuffer)
		{
			TEXTURE_DESC texDesc;
			texDesc.type = TEX_TYPE_2D;
			texDesc.width = BBDesc.Width;
			texDesc.height = BBDesc.Height;
			texDesc.format = PF_D24S8;
			texDesc.usage = TU_DEPTHSTENCIL;
			texDesc.numSamples = getProperties().getMultisampleCount();

			mDepthStencilBuffer = TextureCore::create(texDesc);
			mDepthStencilView = TextureCore::requestView(mDepthStencilBuffer, 0, 1, 0, 1, GVU_DEPTHSTENCIL);
		}
		else
			mDepthStencilBuffer = nullptr;
	}

	void D3D11RenderWindowCore::destroySizeDependedD3DResources()
	{
		SAFE_RELEASE(mBackBuffer);
		SAFE_RELEASE(mRenderTargetView);

		mDepthStencilBuffer = nullptr;
	}

	void D3D11RenderWindowCore::resizeSwapChainBuffers(UINT32 width, UINT32 height)
	{
		destroySizeDependedD3DResources();

		UINT Flags = mProperties.isFullScreen() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
		HRESULT hr = mSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, mSwapChainDesc.BufferDesc.Format, Flags);

		if(hr != S_OK)
			BS_EXCEPT(InternalErrorException, "Call to ResizeBuffers failed.");

		mSwapChain->GetDesc(&mSwapChainDesc);
		mProperties.mWidth = mSwapChainDesc.BufferDesc.Width;
		mProperties.mHeight = mSwapChainDesc.BufferDesc.Height;
		mProperties.mIsFullScreen = (0 == mSwapChainDesc.Windowed); // Alt-Enter together with SetWindowAssociation() can change this state

		createSizeDependedD3DResources();

		mDevice.getImmediateContext()->OMSetRenderTargets(0, 0, 0);
	}

	IDXGIDevice* D3D11RenderWindowCore::queryDxgiDevice()
	{
		if (mDevice.getD3D11Device() == nullptr)
		{
			BS_EXCEPT(RenderingAPIException, "D3D11Device is null.");
		}

		IDXGIDevice* pDXGIDevice = nullptr;
		HRESULT hr = mDevice.getD3D11Device()->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Unable to query a DXGIDevice.");

		return pDXGIDevice;
	}

	void D3D11RenderWindowCore::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}

	D3D11RenderWindow::D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, D3D11Device& device, IDXGIFactory* DXGIFactory)
		:RenderWindow(desc, windowId), mProperties(desc), mDevice(device), mDXGIFactory(DXGIFactory)
	{

	}

	void D3D11RenderWindow::getCustomAttribute(const String& name, void* pData) const
	{
		if (name == "WINDOW")
		{
			UINT64 *pHwnd = (UINT64*)pData;
			*pHwnd = (UINT64)getHWnd();
			return;
		}
	}

	Vector2I D3D11RenderWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		POINT pos;
		pos.x = screenPos.x;
		pos.y = screenPos.y;

		ScreenToClient(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	Vector2I D3D11RenderWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		POINT pos;
		pos.x = windowPos.x;
		pos.y = windowPos.y;

		ClientToScreen(getHWnd(), &pos);
		return Vector2I(pos.x, pos.y);
	}

	SPtr<D3D11RenderWindowCore> D3D11RenderWindow::getCore() const
	{
		return std::static_pointer_cast<D3D11RenderWindowCore>(mCoreSpecific);
	}

	HWND D3D11RenderWindow::getHWnd() const
	{
		blockUntilCoreInitialized();
		return getCore()->_getWindowHandle();
	}

	void D3D11RenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->mLock);
		mProperties = getCore()->mSyncedProperties;
	}
}