//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// This file provides a wrapper for the D3D12 Memory Allocator library.
// The actual D3D12MemAlloc.h should be obtained from: https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator
//
// If D3D12MA is not available, we fall back to CreateCommittedResource for compatibility.

#ifdef B3D_USE_D3D12MA
	// Include the real D3D12 Memory Allocator if available
	#include <D3D12MemAlloc.h>
#else
	// Provide stub implementation for projects without D3D12MA
	#include <d3d12.h>
	#include <dxgi.h>
	#include <wrl/client.h>

	using Microsoft::WRL::ComPtr;

	namespace D3D12MA
	{
		// Forward declarations
		class Allocation;
		class Allocator;

		/** Flags for allocation */
		enum ALLOCATION_FLAGS
		{
			ALLOCATION_FLAG_NONE = 0,
			ALLOCATION_FLAG_COMMITTED = 0x1,
		};

		/** Describes properties of allocated memory block */
		struct ALLOCATION_DESC
		{
			D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
			ALLOCATION_FLAGS Flags = ALLOCATION_FLAG_NONE;
			D3D12_HEAP_FLAGS ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
		};

		/** Represents single memory allocation */
		class Allocation
		{
		public:
			virtual ~Allocation() = default;
			virtual ID3D12Resource* GetResource() const = 0;
			virtual void Release() = 0;
		};

		/** Parameters for creating an allocator */
		struct ALLOCATOR_DESC
		{
			ID3D12Device* pDevice = nullptr;
			IDXGIAdapter* pAdapter = nullptr;
			const D3D12_FEATURE_DATA_D3D12_OPTIONS* pD3D12Options = nullptr;
		};

		/** Custom memory allocator for D3D12 */
		class Allocator
		{
		public:
			virtual ~Allocator() = default;

			virtual HRESULT CreateResource(
				const ALLOCATION_DESC* pAllocDesc,
				const D3D12_RESOURCE_DESC* pResourceDesc,
				D3D12_RESOURCE_STATES InitialResourceState,
				const D3D12_CLEAR_VALUE* pOptimizedClearValue,
				Allocation** ppAllocation,
				REFIID riidResource,
				void** ppvResource) = 0;

			virtual void Release() = 0;
		};

		HRESULT CreateAllocator(const ALLOCATOR_DESC* pDesc, Allocator** ppAllocator);

	} // namespace D3D12MA
#endif
