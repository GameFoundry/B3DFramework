//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Threading/B3DThreading.h"
#include "Utility/B3DPool.h"

namespace b3d
{
	namespace render
	{
		class D3D12GpuDevice;
	}

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** References a native D3D12 heap used by placed resources. */
	struct D3D12GpuHeap : IGpuHeap
	{
		ComPtr<ID3D12Heap> Heap; /**< Backing native heap. */
		u64 Size = 0; /**< Total heap size in bytes. */
		D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT; /**< Native memory type. */
		D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE; /**< Resource classes accepted by the heap. */
	};

	/** Downcasts an opaque engine heap handle to the concrete D3D12 heap it must refer to. */
	inline D3D12GpuHeap& ToD3D12GpuHeap(IGpuHeap* heap)
	{
		B3D_ASSERT(heap != nullptr);
		return *static_cast<D3D12GpuHeap*>(heap);
	}

	/** Initializer struct for D3D12HeapBackend::CreateHeap. */
	struct D3D12HeapCreateInformation
	{
		D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT; /**< Native memory type. */
		D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE; /**< Resource classes accepted by the heap. */
		u64 Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; /**< Heap alignment. */
	};

	/** D3D12 implementation of the GpuHeapBackend trait. */
	class D3D12HeapBackend
	{
	public:
		using HeapHandle = IGpuHeap*;
		using HeapCreateInformation = D3D12HeapCreateInformation;

		explicit D3D12HeapBackend(render::D3D12GpuDevice& device);
		~D3D12HeapBackend();

		D3D12HeapBackend(const D3D12HeapBackend&) = delete;
		D3D12HeapBackend& operator=(const D3D12HeapBackend&) = delete;

		/** Allocates a native heap and returns its stable wrapper handle, or null on failure. */
		HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

		/** Releases the native heap and returns its wrapper to the pool. */
		void DestroyHeap(HeapHandle handle);

	private:
		ID3D12Device* mDevice = nullptr;
		TPool<D3D12GpuHeap> mHeapPool;
		Mutex mHeapPoolMutex;
	};

	/** @} */
} // namespace b3d
