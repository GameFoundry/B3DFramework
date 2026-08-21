//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

namespace b3d::render
{
	/** A persistently allocated native buffer resource suballocated into logical D3D12Buffer slices. */
	class D3D12BufferPage : public D3D12BufferResource, public IGpuHeap
	{
	public:
		/** Takes ownership of @p resource and its @p backingAllocation. */
		D3D12BufferPage(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuResourceLocation backingAllocation, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags, void* mappedData);
		~D3D12BufferPage() override;

		/** Returns the native resource backing every slice on this page. */
		ID3D12Resource* GetD3D12Resource() const override { return mResource.Get(); }

		/** Returns this physical page. */
		D3D12BufferPage* GetPage() const override { return const_cast<D3D12BufferPage*>(this); }

		/** Returns the base GPU virtual address of the page. */
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mResource->GetGPUVirtualAddress(); }

		/** Returns the native heap type used by the page. */
		D3D12_HEAP_TYPE GetHeapType() const { return mHeapType; }

		/** Returns the flags used to create the native resource. */
		D3D12_RESOURCE_FLAGS GetFlags() const { return mFlags; }

		/** Returns the persistent CPU mapping, or null for device-local pages. */
		void* GetMappedData() const { return mMappedData; }

	private:
		ComPtr<ID3D12Resource> mResource;
		GpuResourceLocation mBackingAllocation;
		D3D12_HEAP_TYPE mHeapType = D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_FLAGS mFlags = D3D12_RESOURCE_FLAG_NONE;
		void* mMappedData = nullptr;
	};

	/** Properties shared by every resource page in one buffer-pool class. */
	struct D3D12BufferPageCreateInformation
	{
		/** Native heap type used for the page resource. */
		D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;

		/** Flags used to create the page resource. */
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
	};

	/** Creates the native resource pages managed by the buffer pool's TLSF allocators. */
	class D3D12BufferPageBackend
	{
	public:
		using HeapHandle = IGpuHeap*;
		using HeapCreateInformation = D3D12BufferPageCreateInformation;

		/** Creates a page backend for @p device. */
		explicit D3D12BufferPageBackend(D3D12GpuDevice& device) : mDevice(device)
		{ }

		/** Creates a buffer page, returning null when native resource allocation or mapping fails. */
		HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

		/** Queues @p handle for destruction after its tracked GPU uses complete. */
		void DestroyHeap(HeapHandle handle);

	private:
		D3D12GpuDevice& mDevice;
	};

	/** Owns persistent and transient buffer-page allocators, partitioned by native heap and resource flags. */
	class D3D12BufferPool
	{
	public:
		/** Native page compatibility classes exposed as backend memory types. */
		enum class MemoryType : u32
		{
			Default,                /**< Device-local slices without unordered access. */
			DefaultUnorderedAccess, /**< Device-local slices supporting unordered access. */
			Upload,                 /**< Persistently mapped CPU-to-GPU slices. */
			Readback,               /**< Persistently mapped GPU-to-CPU slices. */
			Count                   /**< Number of buffer memory types. */
		};

		explicit D3D12BufferPool(D3D12GpuDevice& device);
		~D3D12BufferPool();

		/** Lazily creates and returns the persistent TLSF allocator for @p memoryType. */
		IGpuAllocator& GetOrCreatePersistentAllocator(MemoryType memoryType);

		/** Creates a context-owned linear allocator for @p memoryType, backed by the shared page pool for that type. */
		TUnique<IGpuAllocator> CreateTransientAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker);

		/** Returns the memory type compatible with @p heapType and @p resourceFlags, or Count when unsupported. */
		static MemoryType GetMemoryType(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS resourceFlags);

	private:
		using PersistentAllocator = TGpuTlsfAllocator<D3D12BufferPageBackend>;
		using LinearPagePool = TGpuLinearPagePool<D3D12BufferPageBackend>;
		using TransientAllocator = TGpuLinearAllocator<D3D12BufferPageBackend>;

		/** Lazily creates and returns the shared transient page pool for @p memoryType. */
		LinearPagePool& GetOrCreateLinearPagePool(MemoryType memoryType);

		/** Returns the native page properties for @p memoryType. */
		static D3D12BufferPageCreateInformation GetPageCreateInformation(MemoryType memoryType);

		/** Returns the initial persistent TLSF page size for @p memoryType. */
		static u64 GetPersistentPageSize(MemoryType memoryType);

		/** Returns the transient linear page size for @p memoryType. */
		static u64 GetTransientPageSize(MemoryType memoryType);

		D3D12BufferPageBackend mBackend;
		TUnique<PersistentAllocator> mPersistentAllocators[(u32)MemoryType::Count];
		TUnique<LinearPagePool> mLinearPagePools[(u32)MemoryType::Count];

		/** Guards lazy creation of persistent allocators and transient page pools. */
		Mutex mAllocatorMutex;
	};
}

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(b3d::render::D3D12BufferPageBackend);
