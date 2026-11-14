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

	/** Memory of a native heap. Every heap is a custom heap, so its memory is a CPU page property and a memory pool. */
	struct D3D12HeapMemory
	{
		D3D12HeapMemory() = default;
		D3D12HeapMemory(D3D12_CPU_PAGE_PROPERTY cpuPageProperty, D3D12_MEMORY_POOL memoryPool)
			: CpuPageProperty(cpuPageProperty), MemoryPool(memoryPool)
		{ }

		bool operator==(const D3D12HeapMemory& other) const { return CpuPageProperty == other.CpuPageProperty && MemoryPool == other.MemoryPool; }
		bool operator!=(const D3D12HeapMemory& other) const { return !(*this == other); }

		D3D12_CPU_PAGE_PROPERTY CpuPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE; /**< How the CPU accesses the memory, or NOT_AVAILABLE if it can't. */
		D3D12_MEMORY_POOL MemoryPool = D3D12_MEMORY_POOL_L1; /**< L0 for system memory (all memory on unified memory), L1 for the memory of a discrete GPU. */
	};

	/** References a native D3D12 heap used by placed resources. */
	struct D3D12GpuHeap : IGpuHeap
	{
		ComPtr<ID3D12Heap> Heap; /**< Backing native heap. */
		u64 Size = 0; /**< Total heap size in bytes. */
		D3D12HeapMemory Memory; /**< Memory the heap is in, which can be the fallback memory of the requested one (see D3D12HeapBackend::CreateHeap()). */
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
		D3D12HeapMemory Memory; /**< Memory of the heap. */

		/**
		 * Memory the heap is in instead when Memory is in memory pool L1 and GPU memory is close to its budget, or when the heap fails to
		 * create. No fallback if not set.
		 */
		TOptional<D3D12HeapMemory> FallbackMemory;

		D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE; /**< Resource classes accepted by the heap. */
		u64 Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; /**< Heap alignment. */
	};

	/**
	 * Class of native heaps that resources are placed in. Each class has its own persistent allocator, and buffer classes their own scratch
	 * page pool. GpuMemoryRequirements::MemoryType reported by render::D3D12GpuDevice is the class.
	 */
	enum class D3D12HeapClass : u32
	{
		DefaultBuffer,                  /**< GPU-local buffers. */
		DefaultTexture,                 /**< GPU-local, non-multisampled textures without RT/DS flags. */
		DefaultMsaaTexture,             /**< GPU-local multisampled textures without RT/DS flags. */
		DefaultRenderTargetTexture,     /**< GPU-local, non-multisampled RT/DS textures. */
		DefaultMsaaRenderTargetTexture, /**< GPU-local multisampled RT/DS textures. */
		GpuUploadBuffer,                /**< CPU-writable buffers in GPU memory, or in system memory once GPU memory is close to its budget. */
		GpuUploadOrDefaultBuffer,       /**< CPU-writable buffers in GPU memory, or unmapped in default memory once GPU memory is close to its budget. */
		CustomWriteBackBuffer,          /**< Mapped and staging buffers in system memory the CPU caches. */
		CustomWriteCombineBuffer,       /**< Mapped and staging buffers in write-combined system memory. */
		Count                           /**< Number of heap classes. */
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
		IDXGIAdapter3* mAdapter = nullptr;
		TPool<D3D12GpuHeap> mHeapPool;
		Mutex mHeapPoolMutex;
	};

	/** @} */
} // namespace b3d
