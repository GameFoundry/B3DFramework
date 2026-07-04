//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

// Windows headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

// D3D12 Memory Allocator
#include "ThirdParty/D3D12MemAlloc.h"

namespace b3d
{
	class D3D12GpuBackend;

	namespace render
	{
		class D3D12GpuDevice;
		class D3D12GpuQueue;
		class D3D12GpuCommandBuffer;
		class D3D12GpuCommandBufferPool;
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
		class D3D12ResourceManager;
		class D3D12ResourceTracker;
		class D3D12BarrierHelper;
		class D3D12GpuTimelineFence;
		class D3D12DescriptorHeap;
		class D3D12DescriptorManager;
		class D3D12RootSignature;
		class D3D12QueryHeap;
	}

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** Maximum number of command buffers that another command buffer can be dependent on (via a sync mask) */
	#define BS_MAX_D3D12_CB_DEPENDENCIES 2

	/** Maximum number of queues per type */
	constexpr u32 D3D12_MAX_QUEUES_PER_TYPE = 8;

	/** @} */

	// Template aliases for COM smart pointers
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
} // namespace b3d
