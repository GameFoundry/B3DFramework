//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderWindow.h"
#include "CoreThread/BsCoreThread.h"
#include "Private/Win32/BsWin32Platform.h"
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
#include "Private/Win32/BsWin32Window.h"

using namespace bs;

D3D11RenderWindow::D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
	: RenderWindow(desc, windowId), mProperties(desc)
{
}

void D3D11RenderWindow::GetCustomAttribute(const String& name, void* pData) const
{
	if(name == "WINDOW")
	{
		u64* pHwnd = (u64*)pData;
		*pHwnd = (u64)GetHWnd();
		return;
	}
}

Vector2I D3D11RenderWindow::ScreenToWindowPos(const Vector2I& screenPos) const
{
	POINT pos;
	pos.x = screenPos.X;
	pos.y = screenPos.Y;

	ScreenToClient(GetHWnd(), &pos);
	return Vector2I(pos.x, pos.y);
}

Vector2I D3D11RenderWindow::WindowToScreenPos(const Vector2I& windowPos) const
{
	POINT pos;
	pos.x = windowPos.X;
	pos.y = windowPos.Y;

	ClientToScreen(GetHWnd(), &pos);
	return Vector2I(pos.x, pos.y);
}

SPtr<ct::D3D11RenderWindow> D3D11RenderWindow::GetCore() const
{
	return std::static_pointer_cast<ct::D3D11RenderWindow>(mCoreSpecific);
}

HWND D3D11RenderWindow::GetHWnd() const
{
	BlockUntilCoreInitialized();
	return GetCore()->GetWindowHandleInternal();
}

void D3D11RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(GetCore()->mLock);
	mProperties = GetCore()->mSyncedProperties;
}

SPtr<ct::CoreObject> D3D11RenderWindow::CreateCore() const
{
	ct::RenderAPI* rs = ct::RenderAPI::InstancePtr();
	auto d3d11rs = static_cast<ct::D3D11RenderAPI*>(rs);

	// Create the window
	RENDER_WINDOW_DESC desc = mDesc;
	SPtr<ct::CoreObject> coreObj = bs_shared_ptr_new<ct::D3D11RenderWindow>(desc, mWindowId, d3d11rs->GetPrimaryDevice(), d3d11rs->GetDxgiFactory());
	coreObj->SetThisPtrInternal(coreObj);

	return coreObj;
}

namespace bs { namespace ct {
D3D11RenderWindow::D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, D3D11Device& device, IDXGIFactory1* DXGIFactory)
	: RenderWindow(desc, windowId), mProperties(desc), mSyncedProperties(desc), mDevice(device), mDXGIFactory(DXGIFactory)
{}

D3D11RenderWindow::~D3D11RenderWindow()
{
	RenderWindowProperties& props = mProperties;

	if(props.IsFullScreen)
		mSwapChain->SetFullscreenState(false, nullptr);

	SAFE_RELEASE(mSwapChain);
	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_SwapChain);

	if(mWindow != nullptr)
	{
		bs_delete(mWindow);
		mWindow = nullptr;
	}

	DestroySizeDependedD3DResources();
	Platform::ResetNonClientAreas(*this);
}

void D3D11RenderWindow::Initialize()
{
	ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	RenderWindowProperties& props = mProperties;

	mMultisampleType.Count = 1;
	mMultisampleType.Quality = 0;

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
	windowDesc.Module = GetModuleHandle("bsfD3D11RenderAPI.dll");
#endif

	auto opt = mDesc.PlatformSpecific.find("parentWindowHandle");
	if(opt != mDesc.PlatformSpecific.end())
		windowDesc.Parent = (HWND)parseu64(opt->second);

	opt = mDesc.PlatformSpecific.find("externalWindowHandle");
	if(opt != mDesc.PlatformSpecific.end())
		windowDesc.External = (HWND)parseu64(opt->second);

	mIsChild = windowDesc.Parent != nullptr;
	props.IsFullScreen = mDesc.Fullscreen && !mIsChild;

	if(mDesc.VideoMode.IsCustom)
	{
		mRefreshRateNumerator = Math::RoundToInt(mDesc.VideoMode.RefreshRate);
		mRefreshRateDenominator = 1;
	}
	else
	{
		const D3D11VideoMode& d3d11videoMode = static_cast<const D3D11VideoMode&>(mDesc.VideoMode);
		mRefreshRateNumerator = d3d11videoMode.GetRefreshRateNumerator();
		mRefreshRateDenominator = d3d11videoMode.GetRefreshRateDenominator();
	}

	const D3D11VideoOutputInfo* outputInfo = nullptr;

	const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::Instance().GetVideoModeInfo());
	u32 numOutputs = videoModeInfo.GetNumOutputs();
	if(numOutputs > 0)
	{
		u32 actualMonitorIdx = std::min(mDesc.VideoMode.OutputIdx, numOutputs - 1);
		outputInfo = static_cast<const D3D11VideoOutputInfo*>(&videoModeInfo.GetOutputInfo(actualMonitorIdx));

		DXGI_OUTPUT_DESC desc;
		outputInfo->GetDxgiOutput()->GetDesc(&desc);

		windowDesc.Monitor = desc.Monitor;
	}

	if(!windowDesc.External)
	{
		mShowOnSwap = mDesc.HideUntilSwap && !mDesc.Hidden;
		props.IsHidden = mDesc.HideUntilSwap || mDesc.Hidden;
	}

	mWindow = bs_new<Win32Window>(windowDesc);

	props.Width = mWindow->GetWidth();
	props.Height = mWindow->GetHeight();
	props.Top = mWindow->GetTop();
	props.Left = mWindow->GetLeft();

	CreateSwapChain();

	if(props.IsFullScreen)
	{
		if(outputInfo != nullptr)
			mSwapChain->SetFullscreenState(true, outputInfo->GetDxgiOutput());
		else
			mSwapChain->SetFullscreenState(true, nullptr);
	}

	CreateSizeDependedD3DResources();
	mDXGIFactory->MakeWindowAssociation(mWindow->GetHWnd(), NULL);

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties = props;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	RenderWindow::Initialize();
}

void D3D11RenderWindow::SwapBuffers(u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mShowOnSwap)
		SetHidden(false);

	if(mDevice.GetD3D11Device() != nullptr)
	{
		HRESULT hr = mSwapChain->Present(GetProperties().Vsync ? GetProperties().VsyncInterval : 0, 0);

		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error Presenting surfaces");
	}
}

void D3D11RenderWindow::Move(i32 left, i32 top)
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

void D3D11RenderWindow::Resize(u32 width, u32 height)
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

void D3D11RenderWindow::SetActive(bool state)
{
	THROW_IF_NOT_CORE_THREAD;

	RenderWindowProperties& props = mProperties;
	mWindow->SetActive(state);

	if(mSwapChain)
	{
		if(state)
			mSwapChain->SetFullscreenState(props.IsFullScreen, nullptr);
		else
			mSwapChain->SetFullscreenState(FALSE, nullptr);
	}

	RenderWindow::SetActive(state);
}

void D3D11RenderWindow::SetHidden(bool hidden)
{
	THROW_IF_NOT_CORE_THREAD;

	mShowOnSwap = false;
	mWindow->SetHidden(hidden);

	RenderWindow::SetHidden(hidden);
}

void D3D11RenderWindow::Minimize()
{
	THROW_IF_NOT_CORE_THREAD;

	mWindow->Minimize();
}

void D3D11RenderWindow::Maximize()
{
	THROW_IF_NOT_CORE_THREAD;

	mWindow->Maximize();
}

void D3D11RenderWindow::Restore()
{
	THROW_IF_NOT_CORE_THREAD;

	mWindow->Restore();
}

void D3D11RenderWindow::SetFullscreen(u32 width, u32 height, float refreshRate, u32 monitorIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mIsChild)
		return;

	const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::Instance().GetVideoModeInfo());
	u32 numOutputs = videoModeInfo.GetNumOutputs();
	if(numOutputs == 0)
		return;

	u32 actualMonitorIdx = std::min(monitorIdx, numOutputs - 1);
	const D3D11VideoOutputInfo& outputInfo = static_cast<const D3D11VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

	DXGI_MODE_DESC modeDesc;
	ZeroMemory(&modeDesc, sizeof(modeDesc));

	modeDesc.Width = width;
	modeDesc.Height = height;
	modeDesc.RefreshRate.Numerator = Math::RoundToInt(refreshRate);
	modeDesc.RefreshRate.Denominator = 1;
	modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_MODE_DESC nearestMode;
	ZeroMemory(&nearestMode, sizeof(nearestMode));

	outputInfo.GetDxgiOutput()->FindClosestMatchingMode(&modeDesc, &nearestMode, nullptr);

	mProperties.IsFullScreen = true;
	mProperties.Width = width;
	mProperties.Height = height;

	mSwapChain->ResizeTarget(&nearestMode);
	mSwapChain->SetFullscreenState(true, outputInfo.GetDxgiOutput());

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Top = mProperties.Top;
		mSyncedProperties.Left = mProperties.Left;
		mSyncedProperties.Width = mProperties.Width;
		mSyncedProperties.Height = mProperties.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);
}

void D3D11RenderWindow::SetFullscreen(const VideoMode& mode)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mIsChild)
		return;

	if(mode.IsCustom)
	{
		SetFullscreen(mode.Width, mode.Height, mode.RefreshRate, mode.OutputIdx);
		return;
	}

	const D3D11VideoModeInfo& videoModeInfo = static_cast<const D3D11VideoModeInfo&>(RenderAPI::Instance().GetVideoModeInfo());
	u32 numOutputs = videoModeInfo.GetNumOutputs();
	if(numOutputs == 0)
		return;

	u32 actualMonitorIdx = std::min(mode.OutputIdx, numOutputs - 1);
	const D3D11VideoOutputInfo& outputInfo = static_cast<const D3D11VideoOutputInfo&>(videoModeInfo.GetOutputInfo(actualMonitorIdx));

	const D3D11VideoMode& videoMode = static_cast<const D3D11VideoMode&>(mode);

	mProperties.IsFullScreen = true;
	mProperties.Width = mode.Width;
	mProperties.Height = mode.Height;

	mSwapChain->ResizeTarget(&videoMode.GetDxgiModeDesc());
	mSwapChain->SetFullscreenState(true, outputInfo.GetDxgiOutput());

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Top = mProperties.Top;
		mSyncedProperties.Left = mProperties.Left;
		mSyncedProperties.Width = mProperties.Width;
		mSyncedProperties.Height = mProperties.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);
}

void D3D11RenderWindow::SetWindowed(u32 width, u32 height)
{
	THROW_IF_NOT_CORE_THREAD;

	mProperties.Width = width;
	mProperties.Height = height;
	mProperties.IsFullScreen = false;

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
		mSyncedProperties.Top = mProperties.Top;
		mSyncedProperties.Left = mProperties.Left;
		mSyncedProperties.Width = mProperties.Width;
		mSyncedProperties.Height = mProperties.Height;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
	bs::RenderWindowManager::Instance().NotifyMovedOrResized(this);
}

void D3D11RenderWindow::SetVSync(bool enabled, u32 interval)
{
	mProperties.Vsync = enabled;
	mProperties.VsyncInterval = interval;

	{
		ScopedSpinLock lock(mLock);
		mSyncedProperties.Vsync = enabled;
		mSyncedProperties.VsyncInterval = interval;
	}

	bs::RenderWindowManager::Instance().NotifySyncDataDirty(this);
}

HWND D3D11RenderWindow::GetWindowHandleInternal() const
{
	return mWindow->GetHWnd();
}

void D3D11RenderWindow::GetCustomAttribute(const String& name, void* pData) const
{
	if(name == "WINDOW")
	{
		u64* pWnd = (u64*)pData;
		*pWnd = (u64)mWindow->GetHWnd();
		return;
	}

	if(name == "RTV")
	{
		*static_cast<ID3D11RenderTargetView**>(pData) = mRenderTargetView;
		return;
	}
	else if(name == "DSV")
	{
		if(mDepthStencilView != nullptr)
		{
			D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
			*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->GetDsv(false, false);
		}
		else
		{
			*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
		}

		return;
	}
	else if(name == "RODSV")
	{
		if(mDepthStencilView != nullptr)
		{
			D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
			*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->GetDsv(true, true);
		}
		else
		{
			*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
		}

		return;
	}
	else if(name == "RODWSV")
	{
		if(mDepthStencilView != nullptr)
		{
			D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
			*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->GetDsv(true, false);
		}
		else
		{
			*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
		}

		return;
	}
	else if(name == "WDROSV")
	{
		if(mDepthStencilView != nullptr)
		{
			D3D11TextureView* d3d11TextureView = static_cast<D3D11TextureView*>(mDepthStencilView.get());
			*static_cast<ID3D11DepthStencilView**>(pData) = d3d11TextureView->GetDsv(false, true);
		}
		else
		{
			*static_cast<ID3D11DepthStencilView**>(pData) = nullptr;
		}

		return;
	}

	RenderWindow::GetCustomAttribute(name, pData);
}

void D3D11RenderWindow::CopyToMemory(PixelData& dst, FrameBuffer buffer)
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

		HRESULT hr = mDevice.GetD3D11Device()->CreateTexture2D(&desc, nullptr, &backbuffer);

		if(FAILED(hr) || mDevice.HasError())
		{
			String errorDescription = mDevice.GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
		}

		mDevice.GetImmediateContext()->ResolveSubresource(backbuffer, D3D11CalcSubresource(0, 0, 1), mBackBuffer, D3D11CalcSubresource(0, 0, 1), desc.Format);
	}

	// Change the parameters of the texture so we can read it
	BBDesc.Usage = D3D11_USAGE_STAGING;
	BBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	BBDesc.BindFlags = 0;
	BBDesc.SampleDesc.Quality = 0;
	BBDesc.SampleDesc.Count = 1;

	// Create a temp buffer to copy to
	ID3D11Texture2D* tempTexture;
	HRESULT hr = mDevice.GetD3D11Device()->CreateTexture2D(&BBDesc, nullptr, &tempTexture);

	if(FAILED(hr) || mDevice.HasError())
	{
		String errorDescription = mDevice.GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
	}

	// Copy the back buffer
	mDevice.GetImmediateContext()->CopyResource(tempTexture, backbuffer != NULL ? backbuffer : mBackBuffer);

	// Map the copied texture
	D3D11_MAPPED_SUBRESOURCE mappedTex2D;
	mDevice.GetImmediateContext()->Map(tempTexture, 0, D3D11_MAP_READ, 0, &mappedTex2D);

	// Copy the the texture to the dest
	PixelData src(GetProperties().Width, GetProperties().Height, 1, PF_RGBA8);
	src.SetExternalBuffer((u8*)mappedTex2D.pData);
	PixelUtil::BulkPixelConversion(src, dst);

	// Unmap the temp buffer
	mDevice.GetImmediateContext()->Unmap(tempTexture, 0);

	// Release the temp buffer
	SAFE_RELEASE(tempTexture);
	SAFE_RELEASE(backbuffer);
}

void D3D11RenderWindow::WindowMovedOrResizedInternal()
{
	THROW_IF_NOT_CORE_THREAD;

	if(!mWindow)
		return;

	mWindow->WindowMovedOrResizedInternal();

	RenderWindowProperties& props = mProperties;

	if(props.IsFullScreen) // Fullscreen is handled directly by this object
	{
		ResizeSwapChainBuffers(props.Width, props.Height);
	}
	else
	{
		ResizeSwapChainBuffers(mWindow->GetWidth(), mWindow->GetHeight());
		props.Width = mWindow->GetWidth();
		props.Height = mWindow->GetHeight();
		props.Top = mWindow->GetTop();
		props.Left = mWindow->GetLeft();
	}
}

void D3D11RenderWindow::CreateSwapChain()
{
	ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	RenderWindowProperties& props = mProperties;
	IDXGIDevice* pDXGIDevice = QueryDxgiDevice();

	ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	mSwapChainDesc.OutputWindow = mWindow->GetHWnd();
	mSwapChainDesc.BufferDesc.Width = props.Width;
	mSwapChainDesc.BufferDesc.Height = props.Height;
	mSwapChainDesc.BufferDesc.Format = format;

	if(props.IsFullScreen)
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
	mSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	mSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	mSwapChainDesc.BufferCount = 1;
	mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	mSwapChainDesc.Windowed = true;

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	rs->DetermineMultisampleSettings(props.MultisampleCount, format, &mMultisampleType);
	mSwapChainDesc.SampleDesc.Count = mMultisampleType.Count;
	mSwapChainDesc.SampleDesc.Quality = mMultisampleType.Quality;

	HRESULT hr;

	// Create swap chain
	hr = mDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mSwapChain);

	if(FAILED(hr))
	{
		// Try a second time, may fail the first time due to back buffer count,
		// which will be corrected by the runtime
		hr = mDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mSwapChain);
	}

	SAFE_RELEASE(pDXGIDevice);

	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Unable to create swap chain. Error code: " + toString(hr));

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_SwapChain);
}

void D3D11RenderWindow::CreateSizeDependedD3DResources()
{
	SAFE_RELEASE(mBackBuffer);

	HRESULT hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBuffer);
	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Unable to Get Back Buffer for swap chain");

	assert(mBackBuffer && !mRenderTargetView);

	D3D11_TEXTURE2D_DESC BBDesc;
	mBackBuffer->GetDesc(&BBDesc);

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));

	RTVDesc.Format = BBDesc.Format;
	RTVDesc.ViewDimension = GetProperties().MultisampleCount > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	hr = mDevice.GetD3D11Device()->CreateRenderTargetView(mBackBuffer, &RTVDesc, &mRenderTargetView);

	if(FAILED(hr))
	{
		String errorDescription = mDevice.GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Unable to create rendertagert view\nError Description:" + errorDescription);
	}

	mDepthStencilView = nullptr;

	if(mDesc.DepthBuffer)
	{
		TEXTURE_DESC texDesc;
		texDesc.Type = TEX_TYPE_2D;
		texDesc.Width = BBDesc.Width;
		texDesc.Height = BBDesc.Height;
		texDesc.Format = PF_D32_S8X24;
		texDesc.Usage = TU_DEPTHSTENCIL;
		texDesc.NumSamples = GetProperties().MultisampleCount;

		mDepthStencilBuffer = Texture::Create(texDesc);
		mDepthStencilView = mDepthStencilBuffer->RequestView(0, 1, 0, 1, GVU_DEPTHSTENCIL);
	}
	else
		mDepthStencilBuffer = nullptr;
}

void D3D11RenderWindow::DestroySizeDependedD3DResources()
{
	SAFE_RELEASE(mBackBuffer);
	SAFE_RELEASE(mRenderTargetView);

	mDepthStencilBuffer = nullptr;
}

void D3D11RenderWindow::ResizeSwapChainBuffers(u32 width, u32 height)
{
	DestroySizeDependedD3DResources();

	UINT Flags = mProperties.IsFullScreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
	HRESULT hr = mSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, mSwapChainDesc.BufferDesc.Format, Flags);

	if(hr != S_OK)
		BS_EXCEPT(InternalErrorException, "Call to ResizeBuffers failed.");

	mSwapChain->GetDesc(&mSwapChainDesc);
	mProperties.Width = mSwapChainDesc.BufferDesc.Width;
	mProperties.Height = mSwapChainDesc.BufferDesc.Height;
	mProperties.IsFullScreen = (0 == mSwapChainDesc.Windowed); // Alt-Enter together with SetWindowAssociation() can change this state

	CreateSizeDependedD3DResources();

	mDevice.GetImmediateContext()->OMSetRenderTargets(0, 0, 0);
}

IDXGIDevice* D3D11RenderWindow::QueryDxgiDevice()
{
	if(mDevice.GetD3D11Device() == nullptr)
	{
		BS_EXCEPT(RenderingAPIException, "D3D11Device is null.");
	}

	IDXGIDevice* pDXGIDevice = nullptr;
	HRESULT hr = mDevice.GetD3D11Device()->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Unable to query a DXGIDevice.");

	return pDXGIDevice;
}

void D3D11RenderWindow::SyncProperties()
{
	ScopedSpinLock lock(mLock);
	mProperties = mSyncedProperties;
}
}} // namespace bs::ct
