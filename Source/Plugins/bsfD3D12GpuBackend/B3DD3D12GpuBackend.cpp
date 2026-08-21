//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Managers/B3DD3D12GpuBackendFactory.h"
#include "Managers/B3DD3D12RenderWindowManager.h"
#include "Managers/B3DD3D12TextureManager.h"
#include "Managers/B3DD3D12VertexInputManager.h"
#include "String/B3DUnicode.h"
#include "Utility/B3DConfigVariable.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#else
	static_assert(false, "DirectX 12 is only supported on Windows.");
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
namespace b3d
{
	static TConfigVariable<bool> gD3D12EnableDebugLayer("d3d12.EnableDebugLayer",
		"Enable the D3D12 debug layer, which validates API usage and reports errors to the debug output. Requires the "
		"Graphics Tools optional Windows feature to be installed. Development builds only.", B3D_DEBUG != 0,
		ConfigVariableFlag::ReadOnly);

	static TConfigVariable<bool> gD3D12EnableGpuBasedValidation("d3d12.EnableGpuBasedValidation",
		"Additionally validate shader-side resource accesses on the GPU. Much more thorough than the debug layer on "
		"its own, but slows execution down significantly. Requires d3d12.EnableDebugLayer. Development builds only.",
		false, ConfigVariableFlag::ReadOnly);
} // namespace b3d
#endif

namespace b3d::render
{
	static TConfigVariable<u32> gD3D12BackBufferCount("d3d12.BackBufferCount",
		"Number of back buffers cycled by D3D12 windowed and headless render-window surfaces. Values are limited to "
		"the supported range of two to three back buffers.", 2, ConfigVariableFlag::ReadOnly);

	u32 GetD3D12BackBufferCount()
	{
		return std::clamp(gD3D12BackBufferCount.Get(), 2u, kD3D12MaximumBackBufferCount);
	}
} // namespace b3d::render

using namespace b3d;
using namespace b3d::render;

namespace
{
	void LogD3D12Runtime()
	{
		HMODULE d3d12Core = GetModuleHandleW(L"D3D12Core.dll");
		if(d3d12Core == nullptr)
		{
			B3D_LOG(Warning, LogRenderBackend, "D3D12Core.dll is not loaded; the system D3D12 runtime may be active.");
			return;
		}

		WString modulePath(1024, L'\0');
		const DWORD pathLength = GetModuleFileNameW(d3d12Core, modulePath.data(), (DWORD)modulePath.size());
		if(pathLength == 0)
			modulePath = L"<unknown>";
		else
			modulePath.resize(pathLength);

		const UINT* sdkVersion = reinterpret_cast<const UINT*>(GetProcAddress(d3d12Core, "D3D12SDKVersion"));
		if(sdkVersion != nullptr)
			B3D_LOG(Info, LogRenderBackend, "D3D12 Agility SDK {0} loaded from '{1}'.", *sdkVersion, UTF8::FromWide(modulePath));
		else
			B3D_LOG(Warning, LogRenderBackend, "D3D12Core.dll loaded from '{0}', but its SDK version is unavailable.", UTF8::FromWide(modulePath));
	}
}

void D3D12GpuBackend::OnStartUp()
{
	HRESULT hr;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if (gD3D12EnableDebugLayer)
	{
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController));
		if (SUCCEEDED(hr))
		{
			mDebugController->EnableDebugLayer();

			if (gD3D12EnableGpuBasedValidation)
			{
				ComPtr<ID3D12Debug1> debugController1;
				if (SUCCEEDED(mDebugController.As(&debugController1)))
					debugController1->SetEnableGPUBasedValidation(true);
			}

			B3D_LOG(Info, LogRenderBackend, "D3D12 debug layer enabled.");
		}
		else
		{
			B3D_LOG(Warning, LogRenderBackend, "Failed to enable D3D12 debug layer. Install the Graphics Tools feature.");
		}

		// Enable DRED so device removals record which command hung (see D3D12GpuDevice::LogDeviceRemovalBreadcrumbs()).
		// Must be configured before any device is created.
		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings))))
		{
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
	}
#endif

	UINT dxgiFactoryFlags = 0;
#if B3D_BUILD_TYPE_DEVELOPMENT
	if (gD3D12EnableDebugLayer)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mDXGIFactory));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create DXGI factory");

	const DXGI_GPU_PREFERENCE gpuPreference = gGpuPreferIntegrated ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;

	ComPtr<IDXGIAdapter1> adapter;
	Vector<ComPtr<IDXGIAdapter4>> availableAdapters;

	for (UINT adapterIndex = 0; mDXGIFactory->EnumAdapterByGpuPreference(adapterIndex, gpuPreference, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		ComPtr<IDXGIAdapter4> adapter4;
		if (SUCCEEDED(adapter.As(&adapter4)))
			availableAdapters.push_back(adapter4);
	}

	LogD3D12Runtime();

	if (availableAdapters.empty())
	{
		B3D_LOG(Error, LogRenderBackend, "No compatible D3D12 hardware adapter was found.");
		return;
	}

	// Select primary adapter
	u32 primaryAdapterIndex = 0;
	if (gGpuPreferredDeviceIndex >= 0 && (u32)gGpuPreferredDeviceIndex < availableAdapters.size())
		primaryAdapterIndex = (u32)gGpuPreferredDeviceIndex;

	const Vector<ComPtr<IDXGIAdapter4>> deviceAdapters = { availableAdapters[primaryAdapterIndex] };

	mDevices.resize(deviceAdapters.size());
	mDevices[0] = B3DMakeShared<D3D12GpuDevice>(deviceAdapters[0].Get());

	mDevices[0]->SetIsPrimary();

	mPrimaryDevice = mDevices[0];

	// Set GPU info for platform
	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = std::min(5U, (u32)mDevices.size());

	for (u32 deviceIndex = 0; deviceIndex < gpuInfo.NumGpUs; deviceIndex++)
	{
		DXGI_ADAPTER_DESC3 adapterDescription;
		if (SUCCEEDED(deviceAdapters[deviceIndex]->GetDesc3(&adapterDescription)))
		{
			// Convert wide string to regular string
			char deviceName[128];
			wcstombs(deviceName, adapterDescription.Description, sizeof(deviceName));
			gpuInfo.Names[deviceIndex] = deviceName;
		}
	}

	PlatformUtility::SetGPUInfo(gpuInfo);

	TextureManager::StartUp<D3D12TextureManager>();
	render::TextureManager::StartUp<render::D3D12TextureManager>(*mDevices[0]);

	RenderWindowManager::StartUp<D3D12RenderWindowManager>();
	D3D12VertexInputManager::StartUp();

	// TODO(d3d12-port): Set up GPU command capture with PIX.

	Super::OnStartUp();
}

const char* D3D12GpuBackend::GetBackendName() const
{
	return D3D12GpuBackendFactory::SystemName;
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

	D3D12VertexInputManager::ShutDown();
	RenderWindowManager::ShutDown();
	render::TextureManager::ShutDown();
	TextureManager::ShutDown();

	mPrimaryDevice = nullptr;
	mDevices.clear();
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
