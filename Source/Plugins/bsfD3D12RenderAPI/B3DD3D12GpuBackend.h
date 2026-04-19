//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "B3DD3D12GpuDevice.h"

namespace b3d
{
	/** @addtogroup D3D12RenderAPI
	 *  @{
	 */

	/** Handles initialization and shutdown of DirectX 12 GPU backend, and provides access to GPU device objects. */
	class D3D12GpuBackend : public GpuBackend
	{
		using Super = GpuBackend;
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		u32 GetDeviceCount() const override { return (u32)mDevices.size(); }
		SPtr<GpuDevice> GetDevice(u32 index) const override { return mDevices[index]; }

		/** Returns the DXGI factory used for device enumeration and swap chain creation. */
		IDXGIFactory6* GetDXGIFactory() const { return mDXGIFactory.Get(); }

		/** Returns a D3D12 device at the specified index. Must be in range [0, GetDeviceCount()) */
		const SPtr<render::D3D12GpuDevice>& GetD3D12Device(u32 index) const { return mDevices[index]; }

		/** Returns the primary device. */
		const SPtr<render::D3D12GpuDevice>& GetPrimaryDevice() const { return mPrimaryDevice; }

	private:
		ComPtr<IDXGIFactory6> mDXGIFactory;
		ComPtr<IDXGIAdapter4> mDXGIAdapter;

		TInlineArray<SPtr<render::D3D12GpuDevice>, 2> mDevices;
		SPtr<render::D3D12GpuDevice> mPrimaryDevice;

#if B3D_BUILD_TYPE_DEVELOPMENT
		ComPtr<ID3D12Debug> mDebugController;
#endif
	};

	/**	Provides easy access to the D3D12GpuBackend. */
	D3D12GpuBackend& GetD3D12GpuBackend();

	/** @} */
} // namespace b3d
