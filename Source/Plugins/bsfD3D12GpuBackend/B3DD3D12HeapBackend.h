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

	/** Kind of memory a heap handle of the D3D12 backend refers to. */
	enum class D3D12HeapKind : u8
	{
		Heap, /**< A native heap, in which resources are placed. See D3D12GpuHeap. */
		BufferPage /**< A native buffer resource, of which buffers are slices. See render::D3D12BufferPage. */
	};

	/** Base of every heap handle of the D3D12 backend. Identifies the kind of memory the handle refers to. */
	struct D3D12HeapBase : IGpuHeap
	{
		explicit D3D12HeapBase(D3D12HeapKind kind)
			: Kind(kind)
		{ }

		D3D12HeapKind Kind;
	};

	/** Returns the kind of memory @p heap, a heap handle of the D3D12 backend, refers to. */
	inline D3D12HeapKind GetD3D12HeapKind(IGpuHeap* heap)
	{
		B3D_ASSERT(heap != nullptr);
		return static_cast<D3D12HeapBase*>(heap)->Kind;
	}

	/** References a native D3D12 heap used by placed resources. */
	struct D3D12GpuHeap : D3D12HeapBase
	{
		D3D12GpuHeap()
			: D3D12HeapBase(D3D12HeapKind::Heap)
		{ }

		ComPtr<ID3D12Heap> Heap; /**< Backing native heap. */
		u64 Size = 0; /**< Total heap size in bytes. */
		D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT; /**< Native memory type, which can differ from the requested one (see D3D12HeapBackend::CreateHeap()). */
		D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE; /**< Resource classes accepted by the heap. */
	};

	/** Downcasts an opaque engine heap handle to the concrete D3D12 heap it must refer to. */
	inline D3D12GpuHeap& ToD3D12GpuHeap(IGpuHeap* heap)
	{
		B3D_ASSERT(GetD3D12HeapKind(heap) == D3D12HeapKind::Heap);
		return *static_cast<D3D12GpuHeap*>(heap);
	}

	/** Memory a GPU upload heap is created in instead where GPU memory is close to its budget, or where the heap fails to create. */
	enum class D3D12HeapFallback
	{
		/**
		 * A write-combined custom heap in system memory, the same memory as an upload heap, so its resources stay mapped. Unlike an upload
		 * heap, GPU copies can write it.
		 */
		SystemMemory,

		/** A default heap in GPU memory, whose resources aren't mapped. The CPU writes them through staging buffers. */
		UnmappedGpuMemory
	};

	/** Memory of a native heap resources are placed in. */
	struct D3D12HeapProperties
	{
		D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT; /**< Native memory type. */
		D3D12HeapFallback Fallback = D3D12HeapFallback::SystemMemory; /**< Memory a GPU upload heap falls back to. Ignored by other types. */
		D3D12_CPU_PAGE_PROPERTY CpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; /**< CPU page property of a custom heap, which is in memory pool L0. Ignored by other types. */
	};

	/** Initializer struct for D3D12HeapBackend::CreateHeap. */
	struct D3D12HeapCreateInformation
	{
		D3D12HeapProperties Properties; /**< Memory of the heap. */
		D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE; /**< Resource classes accepted by the heap. */
		u64 Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; /**< Heap alignment. */
	};

	/** Class of buffer pages that buffers are slices of. Each class has its own persistent allocator and scratch page pool. */
	enum class D3D12PageClass : u32
	{
		Default,                /**< Device-local slices without unordered access. */
		DefaultUnorderedAccess, /**< Device-local slices supporting unordered access. */
		GpuUpload,              /**< Persistently mapped CPU-to-GPU slices in GPU memory, or in system memory once GPU memory is close to its budget. */
		GpuUploadOrDefault,     /**< Persistently mapped CPU-to-GPU slices in GPU memory, or unmapped in default heaps once GPU memory is close to its budget. */
		CustomWriteBack,        /**< Persistently mapped slices in custom heaps the CPU caches, of mapped and staging buffers. See render::D3D12GpuDevice::GetBufferHeapProperties(). */
		CustomWriteCombine,     /**< Persistently mapped slices in write-combined custom heaps, of mapped and staging buffers. See render::D3D12GpuDevice::GetBufferHeapProperties(). */
		Count                   /**< Number of page classes. */
	};

	/** Class of native heaps that resources are placed in. Each class has its own persistent allocator. */
	enum class D3D12HeapClass : u32
	{
		DefaultBuffer,                  /**< GPU-local buffers. */
		DefaultTexture,                 /**< GPU-local, non-multisampled textures without RT/DS flags. */
		DefaultMsaaTexture,             /**< GPU-local multisampled textures without RT/DS flags. */
		DefaultRenderTargetTexture,     /**< GPU-local, non-multisampled RT/DS textures. */
		DefaultMsaaRenderTargetTexture, /**< GPU-local multisampled RT/DS textures. */
		GpuUploadBuffer,                /**< CPU-writable buffers in GPU memory, or in system memory once GPU memory is close to its budget. */
		GpuUploadOrDefaultBuffer,       /**< CPU-writable buffers in GPU memory, or unmapped in default heaps once GPU memory is close to its budget. */
		CustomWriteBackBuffer,          /**< Mapped and staging buffers in custom heaps the CPU caches. */
		CustomWriteCombineBuffer,       /**< Mapped and staging buffers in write-combined custom heaps. */
		Count                           /**< Number of heap classes. */
	};

	/**
	 * What a GpuMemoryRequirements::MemoryType reported by render::D3D12GpuDevice stands for: a class of buffer pages, or a class of heaps
	 * that resources are placed in. Exactly one of PageClass and HeapClass is valid; the other is Count.
	 */
	struct D3D12MemoryType
	{
		explicit D3D12MemoryType(D3D12PageClass pageClass)
			: PageClass(pageClass)
		{ }

		explicit D3D12MemoryType(D3D12HeapClass heapClass)
			: HeapClass(heapClass)
		{ }

		/** Returns the memory type @p memoryType stands for, a GpuMemoryRequirements::MemoryType reported by render::D3D12GpuDevice. */
		static D3D12MemoryType Decode(u32 memoryType)
		{
			constexpr u32 kPageClassCount = (u32)D3D12PageClass::Count;
			return memoryType < kPageClassCount ? D3D12MemoryType((D3D12PageClass)memoryType) : D3D12MemoryType((D3D12HeapClass)(memoryType - kPageClassCount));
		}

		/** Returns the GpuMemoryRequirements::MemoryType reported for this memory type. */
		u32 Encode() const { return PageClass != D3D12PageClass::Count ? (u32)PageClass : (u32)D3D12PageClass::Count + (u32)HeapClass; }

		/** Number of distinct values Encode() can return. */
		static constexpr u32 kEncodedCount = (u32)D3D12PageClass::Count + (u32)D3D12HeapClass::Count;

		D3D12PageClass PageClass = D3D12PageClass::Count; /**< Class of buffer pages, or Count if the memory type is a heap class. */
		D3D12HeapClass HeapClass = D3D12HeapClass::Count; /**< Class of heaps, or Count if the memory type is a page class. */
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
