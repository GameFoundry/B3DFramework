//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

namespace b3d::render
{
	/** A persistently allocated native buffer resource suballocated into logical D3D12Buffer slices. */
	class D3D12BufferPage : public D3D12Resource, public IGpuHeap
	{
	public:
		/** Takes ownership of @p resource and its @p backingAllocation. */
		D3D12BufferPage(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuResourceLocation backingAllocation, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags, void* mappedData);
		~D3D12BufferPage() override;

		/** Returns the native resource backing every slice on this page. */
		ID3D12Resource* GetD3D12Resource() const { return mResource.Get(); }

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

	/** Thread-safe persistent buffer pool, partitioned by native heap and resource flags. */
	class D3D12BufferPool
	{
	public:
		explicit D3D12BufferPool(D3D12GpuDevice& device);
		~D3D12BufferPool();

		/** Allocates a compatible buffer slice, returning false for unsupported heap/flag combinations or allocation failure. */
		bool TryAllocate(u64 size, u32 alignment, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS resourceFlags, GpuResourceLocation& outAllocation);

	private:
		enum class PoolType : u32
		{
			Default,                /**< Device-local slices without unordered access. */
			DefaultUnorderedAccess, /**< Device-local slices supporting unordered access. */
			Upload,                 /**< Persistently mapped CPU-to-GPU slices. */
			Readback,               /**< Persistently mapped GPU-to-CPU slices. */
			Count                   /**< Number of pool types. */
		};

		using Allocator = TGpuTlsfAllocator<D3D12BufferPageBackend>;

		D3D12BufferPageBackend mBackend;
		TUnique<Allocator> mAllocators[(u32)PoolType::Count];

		/** Guards lazy creation of the allocators; each allocator provides its own operation-level locking. */
		Mutex mAllocatorMutex;
	};
}

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(b3d::render::D3D12BufferPageBackend);
