//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

// Windows headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace b3d
{
	class D3D12GpuBackend;

	namespace render
	{
		class D3D12GpuDevice;
		class D3D12GpuQueue;
		class D3D12GpuCommandBuffer;
		class D3D12GpuCommandBufferPool;
		struct D3D12GpuCommandBufferSubmitInformation;
		class D3D12GpuBuffer;
		class D3D12Texture;
		class D3D12GpuProgram;
		class D3D12GpuParameters;
		class D3D12GpuGraphicsPipelineState;
		class D3D12GpuComputePipelineState;
		class D3D12GpuPipelineParameterLayout;
		class D3D12SamplerState;
		class D3D12EventQuery;
		class D3D12GpuQueryPool;
		class D3D12Framebuffer;
		class D3D12RenderTexture;
		class D3D12SwapChain;
		class D3D12Image;
		class D3D12ImageSubresource;
		class D3D12Buffer;
		class D3D12BufferPage;
		class D3D12BufferPool;
		class D3D12ResourceManager;
		class D3D12ResourceTracker;
		class D3D12BarrierHelper;
		class D3D12GpuTimelineFence;
		class D3D12DescriptorHeap;
		class D3D12DescriptorManager;
		class D3D12QueryHeap;

		/** Maximum number of back buffers reserved by D3D12 render-window surface storage. */
		constexpr u32 kD3D12MaximumBackBufferCount = 3;

		/** Returns the configured number of back buffers cycled by D3D12 render-window surfaces. */
		u32 GetD3D12BackBufferCount();
	}

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** Reference counted pointer to a COM object, releasing it once the last reference goes out of scope. */
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	/** @} */
} // namespace b3d
