//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderWindow.h"
#include "CoreThread/BsCoreThread.h"
#include "Win32/BsWin32Platform.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderTexture.h"
#include "BsD3D11TextureView.h"
#include "Managers/BsTextureManager.h"
#include "BsD3D11DriverList.h"
#include "BsD3D11Driver.h"
#include "BsD3D11VideoModeInfo.h"
#include "Profiling/BsRenderStats.h"
#include "Input/BsInput.h"
#include "Error/BsException.h"
#include "Managers/BsRenderWindowManager.h"
#include "Math/BsMath.h"
#include "Win32/BsWin32Window.h"

namespace bs
{
	D3D11RenderWindow::D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::D3D11Device& device, 
		IDXGIFactory* DXGIFactory)
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

	SPtr<ct::D3D11RenderWindow> D3D11RenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::D3D11RenderWindow>(mCoreSpecific);
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

	namespace ct
	{
	D3D11RenderWindow::D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, D3D11Device& device, IDXGIFactory* DXGIFactory)
		: RenderWindow(desc, windowId), mProperties(desc), mSyncedProperties(desc), mDevice(device), mDXGIFactory(DXGIFactory), 
		mSizing(false), mRenderTargetView(nullptr), mBackBuffer(nullptr), mSwapChain(nullptr), mDepthStencilView(nullptr), mIsChild(false), 
		mRefreshRateNumerator(0), mRefreshRateDenominator(0), mShowOnSwap(false), mWindow(nullptr)
	{ }

	D3D11RenderWindow::~D3D11RenderWindow()
	{ 
		RenderWindowProperties& props = mProperties;

		if (props.isFullScreen)
			mSwapChain->SetFullscreenState(false, nullptr);

		SAFE_RELEASE(mSwapChain);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_SwapChain);

		if (mWindow != nullptr)
		{
			bs_delete(mWindow);
			mWindow = nullptr;
		}

		destroySizeDependedD3DResources();
		Platform::resetNonClientAreas(*this);
	}

	void D3D11RenderWindow::initialize()
	{
		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		RenderWindowProperties& props = mProperties;

		mMultisampleType.Count = 1;
		mMultisampleType.Quality = 0;

		WINDOW_DESC windowDesc;
		windowDesc.showTitleBar = mDesc.showTitleBar;
		windowDesc.showBorder = mDesc.showBorder;
		windowDesc.allowResize = mDesc.allowResize;
		windowDesc.enableDoubleClick = true;
		windowDesc.fullscreen = mDesc.fullscreen;
		windowDesc.width = mDesc.videoMode.getWidth();
		windowDesc.height = mDesc.videoMode.getHeight();
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
		props.isFullScreen = mDesc.fullscreen && !mIsChild;

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

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::instance().getVideoModeInfo());
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
			props.isHidden = mDesc.hideUntilSwap || mDesc.hidden;
		}

		mWindow = bs_new<Win32Window>(windowDesc);

		props.width = mWindow->getWidth();
		props.height = mWindow->getHeight();
		props.top = mWindow->getTop();
		props.left = mWindow->getLeft();

		createSwapChain();

		if (props.isFullScreen)
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

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		RenderWindow::initialize();
	}

	void D3D11RenderWindow::swapBuffers(UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mShowOnSwap)
			setHidden(false);

		if(mDevice.getD3D11Device() != nullptr)
		{
			HRESULT hr = mSwapChain->Present(getProperties().vsync ? getProperties().vsyncInterval : 0, 0);

			if( FAILED(hr) )
				BS_EXCEPT(RenderingAPIException, "Error Presenting surfaces");
		}
	}

	void D3D11RenderWindow::move(INT32 left, INT32 top)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
		{
			mWindow->move(left, top);

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

	void D3D11RenderWindow::resize(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;

		if (!props.isFullScreen)
		{
			mWindow->resize(width, height);

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

	void D3D11RenderWindow::setActive(bool state)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderWindowProperties& props = mProperties;
		mWindow->setActive(state);

		if (mSwapChain)
		{
			if (state)
				mSwapChain->SetFullscreenState(props.isFullScreen, nullptr);
			else
				mSwapChain->SetFullscreenState(FALSE, nullptr);
		}

		RenderWindow::setActive(state);
	}

	void D3D11RenderWindow::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;

		mShowOnSwap = false;
		mWindow->setHidden(hidden);

		RenderWindow::setHidden(hidden);
	}

	void D3D11RenderWindow::minimize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->minimize();
	}

	void D3D11RenderWindow::maximize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->maximize();
	}

	void D3D11RenderWindow::restore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mWindow->restore();
	}

	void D3D11RenderWindow::setFullscreen(UINT32 width, UINT32 height, float refreshRate, UINT32 monitorIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::instance().getVideoModeInfo());
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

		mProperties.isFullScreen = true;
		mProperties.width = width;
		mProperties.height = height;

		mSwapChain->ResizeTarget(&nearestMode);
		mSwapChain->SetFullscreenState(true, outputInfo.getDXGIOutput()); 

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.top = mProperties.top;
			mSyncedProperties.left = mProperties.left;
			mSyncedProperties.width = mProperties.width;
			mSyncedProperties.height = mProperties.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void D3D11RenderWindow::setFullscreen(const VideoMode& mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mIsChild)
			return;

		if (mode.isCustom())
		{
			setFullscreen(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode.getOutputIdx());
			return;
		}

		const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::instance().getVideoModeInfo());
		UINT32 numOutputs = videoModeInfo.getNumOutputs();
		if (numOutputs == 0)
			return;

		UINT32 actualMonitorIdx = std::min(mode.getOutputIdx(), numOutputs - 1);
		const D3D11VideoOutputInfo& outputInfo = static_cast<const D3D11VideoOutputInfo&>(videoModeInfo.getOutputInfo(actualMonitorIdx));

		const D3D11VideoMode& videoMode = static_cast<const D3D11VideoMode&>(mode);

		mProperties.isFullScreen = true;
		mProperties.width = mode.getWidth();
		mProperties.height = mode.getHeight();

		mSwapChain->ResizeTarget(&videoMode.getDXGIModeDesc());
		mSwapChain->SetFullscreenState(true, outputInfo.getDXGIOutput());

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.top = mProperties.top;
			mSyncedProperties.left = mProperties.left;
			mSyncedProperties.width = mProperties.width;
			mSyncedProperties.height = mProperties.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void D3D11RenderWindow::setWindowed(UINT32 width, UINT32 height)
	{
		THROW_IF_NOT_CORE_THREAD;

		mProperties.width = width;
		mProperties.height = height;
		mProperties.isFullScreen = false;

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
			mSyncedProperties.top = mProperties.top;
			mSyncedProperties.left = mProperties.left;
			mSyncedProperties.width = mProperties.width;
			mSyncedProperties.height = mProperties.height;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
		bs::RenderWindowManager::instance().notifyMovedOrResized(this);
	}

	void D3D11RenderWindow::setVSync(bool enabled, UINT32 interval)
	{
		mProperties.vsync = enabled;
		mProperties.vsyncInterval = interval;

		{
			ScopedSpinLock lock(mLock);
			mSyncedProperties.vsync = enabled;
			mSyncedProperties.vsyncInterval = interval;
		}

		bs::RenderWindowManager::instance().notifySyncDataDirty(this);
	}

	HWND D3D11RenderWindow::_getWindowHandle() const
	{
		return mWindow->getHWnd();
	}

	void D3D11RenderWindow::getCustomAttribute(const String& name, void* pData) const
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
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(false, false);
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
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(true, true);
			}
			else
			{
				*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
			}

			return;
		}
		else if (name == "RODWSV")
		{
			if (mDepthStencilView != nullptr)
			{
				D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(true, false);
			}
			else
			{
				*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
			}

			return;
		}
		else if (name == "WDROSV")
		{
			if (mDepthStencilView != nullptr)
			{
				D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
				*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->getDSV(false, true);
			}
			else
			{
				*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
			}

			return;
		}

		RenderWindow::getCustomAttribute(name, pData);
	}

	void D3D11RenderWindow::copyToMemory(PixelData &dst, FrameBuffer buffer)
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
		PixelData src(getProperties().width, getProperties().height, 1, PF_RGBA8);
		src.setExternalBuffer((UINT8*)mappedTex2D.pData);
		PixelUtil::bulkPixelConversion(src, dst);

		// Unmap the temp buffer
		mDevice.getImmediateContext()->Unmap(tempTexture, 0);

		// Release the temp buffer
		SAFE_RELEASE(tempTexture);
		SAFE_RELEASE(backbuffer);
	}

	void D3D11RenderWindow::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (!mWindow)
			return;

		mWindow->_windowMovedOrResized();

		RenderWindowProperties& props = mProperties;

		if (props.isFullScreen) // Fullscreen is handled directly by this object
		{
			resizeSwapChainBuffers(props.width, props.height);
		}
		else
		{
			resizeSwapChainBuffers(mWindow->getWidth(), mWindow->getHeight());
			props.width = mWindow->getWidth();
			props.height = mWindow->getHeight();
			props.top = mWindow->getTop();
			props.left = mWindow->getLeft();
		}

		RenderWindow::_windowMovedOrResized();
	}

	void D3D11RenderWindow::createSwapChain()
	{
		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		RenderWindowProperties& props = mProperties;
		IDXGIDevice* pDXGIDevice = queryDxgiDevice();

		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mSwapChainDesc.OutputWindow = mWindow->getHWnd();
		mSwapChainDesc.BufferDesc.Width = props.width;
		mSwapChainDesc.BufferDesc.Height = props.height;
		mSwapChainDesc.BufferDesc.Format = format;

		if (props.isFullScreen)
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

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		rs->determineMultisampleSettings(props.multisampleCount, format, &mMultisampleType);
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

	void D3D11RenderWindow::createSizeDependedD3DResources()
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
		RTVDesc.ViewDimension = getProperties().multisampleCount > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		hr = mDevice.getD3D11Device()->CreateRenderTargetView(mBackBuffer, &RTVDesc, &mRenderTargetView);

		if(FAILED(hr))
		{
			String errorDescription = mDevice.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Unable to create rendertagert view\nError Description:" + errorDescription);
		}

		mDepthStencilView = nullptr;

		if (mDesc.depthBuffer)
		{
			TEXTURE_DESC texDesc;
			texDesc.type = TEX_TYPE_2D;
			texDesc.width = BBDesc.Width;
			texDesc.height = BBDesc.Height;
			texDesc.format = PF_D32_S8X24;
			texDesc.usage = TU_DEPTHSTENCIL;
			texDesc.numSamples = getProperties().multisampleCount;

			mDepthStencilBuffer = Texture::create(texDesc);
			mDepthStencilView = mDepthStencilBuffer->requestView(0, 1, 0, 1, GVU_DEPTHSTENCIL);
		}
		else
			mDepthStencilBuffer = nullptr;
	}

	void D3D11RenderWindow::destroySizeDependedD3DResources()
	{
		SAFE_RELEASE(mBackBuffer);
		SAFE_RELEASE(mRenderTargetView);

		mDepthStencilBuffer = nullptr;
	}

	void D3D11RenderWindow::resizeSwapChainBuffers(UINT32 width, UINT32 height)
	{
		destroySizeDependedD3DResources();

		UINT Flags = mProperties.isFullScreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
		HRESULT hr = mSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, mSwapChainDesc.BufferDesc.Format, Flags);

		if(hr != S_OK)
			BS_EXCEPT(InternalErrorException, "Call to ResizeBuffers failed.");

		mSwapChain->GetDesc(&mSwapChainDesc);
		mProperties.width = mSwapChainDesc.BufferDesc.Width;
		mProperties.height = mSwapChainDesc.BufferDesc.Height;
		mProperties.isFullScreen = (0 == mSwapChainDesc.Windowed); // Alt-Enter together with SetWindowAssociation() can change this state

		createSizeDependedD3DResources();

		mDevice.getImmediateContext()->OMSetRenderTargets(0, 0, 0);
	}

	IDXGIDevice* D3D11RenderWindow::queryDxgiDevice()
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

	void D3D11RenderWindow::syncProperties()
	{
		ScopedSpinLock lock(mLock);
		mProperties = mSyncedProperties;
	}		
	}
}