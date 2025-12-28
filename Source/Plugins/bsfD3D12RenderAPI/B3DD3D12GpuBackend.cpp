//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Managers/B3DD3D12GpuBackendFactory.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#else
	static_assert(false, "DirectX 12 is only supported on Windows.");
#endif

namespace b3d
{
	/** When enabled the D3D12 backend will prefer an integrated GPU over a discrete one. */
	static const bool kD3D12PreferIntegratedGPU = false;

	/** Enables D3D12 debug layer. */
	static const bool kEnableD3D12DebugLayer = B3D_DEBUG;

	/** Enables GPU-based validation (requires debug layer). More thorough but slower. */
	static const bool kEnableD3D12GPUBasedValidation = false;

	/** If specified, allows you to select which is the primary GPU to use. If ~0u system will pick the best GPU according to other options. */
	static const u32 kPreferredGPUIndex = ~0u;

} // namespace b3d

using namespace b3d;
using namespace b3d::render;

void D3D12GpuBackend::OnStartUp()
{
	HRESULT hr;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Enable the debug layer
	if (kEnableD3D12DebugLayer)
	{
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController));
		if (SUCCEEDED(hr))
		{
			mDebugController->EnableDebugLayer();

			// Enable GPU-based validation if requested
			if (kEnableD3D12GPUBasedValidation)
			{
				ComPtr<ID3D12Debug1> debugController1;
				if (SUCCEEDED(mDebugController.As(&debugController1)))
				{
					debugController1->SetEnableGPUBasedValidation(true);
				}
			}

			B3D_LOG(Info, LogRenderBackend, "D3D12 debug layer enabled.");
		}
		else
		{
			B3D_LOG(Warning, LogRenderBackend, "Failed to enable D3D12 debug layer. Install the Graphics Tools feature.");
		}
	}
#endif

	// Create DXGI factory
	UINT dxgiFactoryFlags = 0;
#if B3D_BUILD_TYPE_DEVELOPMENT
	if (kEnableD3D12DebugLayer)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mDXGIFactory));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create DXGI factory");

	// Enumerate adapters
	ComPtr<IDXGIAdapter1> adapter;
	Vector<ComPtr<IDXGIAdapter4>> availableAdapters;

	for (UINT adapterIndex = 0;
		mDXGIFactory->EnumAdapterByGpuPreference(
			adapterIndex,
			kD3D12PreferIntegratedGPU ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
		++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// Skip software adapter
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// Check if the adapter supports D3D12
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			ComPtr<IDXGIAdapter4> adapter4;
			if (SUCCEEDED(adapter.As(&adapter4)))
			{
				availableAdapters.push_back(adapter4);
			}
		}
	}

	if (availableAdapters.empty())
	{
		B3D_LOG(Error, LogRenderBackend, "No compatible D3D12 adapters found!");
		return;
	}

	// Select primary adapter
	u32 primaryAdapterIndex = 0;
	if (kPreferredGPUIndex != ~0u && kPreferredGPUIndex < availableAdapters.size())
		primaryAdapterIndex = kPreferredGPUIndex;

	mDXGIAdapter = availableAdapters[primaryAdapterIndex];

	// For now, always initialize a single device
	mDevices.resize(1);

	mDevices[0] = B3DMakeShared<D3D12GpuDevice>(mDXGIAdapter.Get());
	mDevices[0]->SetIsPrimary();

	mPrimaryDevice = mDevices[0];

	// Set GPU info for platform
	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = std::min(5U, (u32)mDevices.size());

	for (u32 i = 0; i < gpuInfo.NumGpUs; i++)
	{
		DXGI_ADAPTER_DESC3 desc;
		if (SUCCEEDED(mDXGIAdapter->GetDesc3(&desc)))
		{
			// Convert wide string to regular string
			char deviceName[128];
			wcstombs(deviceName, desc.Description, sizeof(deviceName));
			gpuInfo.Names[i] = deviceName;
		}
	}

	PlatformUtility::SetGPUInfoInternal(gpuInfo);

	// TODO: Create texture manager
	// TextureManager::StartUp<D3D12TextureManager>();
	// render::TextureManager::StartUp<render::D3D12TextureManager>(*mDevices[0]);

	// TODO: Create render window manager
	// RenderWindowManager::StartUp<D3D12RenderWindowManager>();

	// TODO: Set up frame capture (PIX, RenderDoc)
	// mFrameCapture = B3DMakeShared<D3D12FrameCapture>();

	Super::OnStartUp();
}

void D3D12GpuBackend::OnShutDown()
{
	// Wait for all devices to finish
	for (const auto& device : mDevices)
	{
		if (!device->IsInitialized())
			continue;

		device->WaitUntilIdle();
	}

	// TODO: Shutdown managers
	// RenderWindowManager::ShutDown();
	// render::TextureManager::ShutDown();
	// TextureManager::ShutDown();

	mPrimaryDevice = nullptr;
	mDevices.clear();
	mDXGIAdapter.Reset();
	mDXGIFactory.Reset();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Report live objects if debug layer is enabled
	if (mDebugController)
	{
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}

		mDebugController.Reset();
	}
#endif

	Super::OnShutDown();
}

namespace b3d
{
	D3D12GpuBackend& GetD3D12GpuBackend()
	{
		return static_cast<D3D12GpuBackend&>(D3D12GpuBackend::Instance());
	}
} // namespace b3d
