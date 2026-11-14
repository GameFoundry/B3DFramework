//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "B3DD3D12HeapBackend.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

namespace b3d::render
{
	/** Properties shared by every resource page in one buffer-pool class. */
	struct D3D12BufferPageCreateInformation
	{
		/** Memory of the heap the page resource is placed in. */
		D3D12HeapProperties HeapProperties;

		/** Flags used to create the page resource. */
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
	};

	/** A persistently allocated native buffer resource suballocated into logical D3D12Buffer slices. */
	class D3D12BufferPage : public D3D12BufferResource, public D3D12HeapBase
	{
	public:
		/**
		 * Takes ownership of @p resource and its @p backingAllocation. @p createInformation describes the pool's pages, and @p heapType is the type
		 * of the heap the page is in.
		 */
		D3D12BufferPage(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuAllocation backingAllocation, const D3D12BufferPageCreateInformation& createInformation,
			D3D12_HEAP_TYPE heapType, void* mappedData);
		~D3D12BufferPage() override;

		/** Returns the native resource backing every slice on this page. */
		ID3D12Resource* GetD3D12Resource() const override { return mResource.Get(); }

		/** Returns this physical page. */
		D3D12BufferPage* GetPage() const override { return const_cast<D3D12BufferPage*>(this); }

		/** Returns the base GPU virtual address of the page. */
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mResource->GetGPUVirtualAddress(); }

		/**
		 * Returns the type of the native heap the page is in. A page that asked for a GPU upload heap can be in a custom or a default heap
		 * (see D3D12HeapBackend::CreateHeap()).
		 */
		D3D12_HEAP_TYPE GetHeapType() const override { return mHeapType; }

		/** Returns the properties the pool created the page with, which the page's class follows from. */
		const D3D12BufferPageCreateInformation& GetCreateInformation() const { return mCreateInformation; }

		/** Returns the flags used to create the native resource. */
		D3D12_RESOURCE_FLAGS GetFlags() const { return mCreateInformation.Flags; }

		/** Returns the persistent CPU mapping, or null for device-local pages. */
		void* GetMappedData() const { return mMappedData; }

	private:
		ComPtr<ID3D12Resource> mResource;
		GpuAllocation mBackingAllocation;
		D3D12BufferPageCreateInformation mCreateInformation;
		D3D12_HEAP_TYPE mHeapType = D3D12_HEAP_TYPE_DEFAULT;
		void* mMappedData = nullptr;
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

	/** Owns persistent and scratch buffer-page allocators, partitioned by native heap and resource flags. */
	class D3D12BufferPool
	{
	public:
		explicit D3D12BufferPool(D3D12GpuDevice& device);
		~D3D12BufferPool();

		/** Lazily creates and returns the persistent TLSF allocator of pages of @p pageClass. */
		IGpuAllocator& GetOrCreateGpuMemoryAllocator(D3D12PageClass pageClass);

		/** Appends the persistent allocators created so far to @p outAllocators. */
		void GetPersistentAllocators(TArray<IGpuAllocator*>& outAllocators);

		/** Creates a context-owned linear allocator of pages of @p pageClass, backed by the shared page pool of that class. */
		TUnique<IGpuAllocator> CreateScratchAllocator(D3D12PageClass pageClass, IGpuCompletionTracker& completionTracker);

		/** Returns the class of pages compatible with @p heapProperties and @p resourceFlags, or Count when unsupported. */
		static D3D12PageClass GetPageClass(const D3D12HeapProperties& heapProperties, D3D12_RESOURCE_FLAGS resourceFlags);

	private:
		using PersistentAllocator = TGpuTlsfAllocator<D3D12BufferPageBackend>;
		using LinearPagePool = TGpuLinearPagePool<D3D12BufferPageBackend>;
		using ScratchAllocator = TGpuLinearAllocator<D3D12BufferPageBackend>;

		/** Lazily creates and returns the shared scratch page pool of pages of @p pageClass. */
		LinearPagePool& GetOrCreateLinearPagePool(D3D12PageClass pageClass);

		/** Returns the native properties of pages of @p pageClass. */
		static D3D12BufferPageCreateInformation GetPageCreateInformation(D3D12PageClass pageClass);

		/** Returns the initial persistent TLSF page size of @p pageClass. */
		static u64 GetPersistentPageSize(D3D12PageClass pageClass);

		/** Returns the scratch linear page size of @p pageClass. */
		static u64 GetScratchPageSize(D3D12PageClass pageClass);

		D3D12BufferPageBackend mBackend;
		TUnique<PersistentAllocator> mPersistentAllocators[(u32)D3D12PageClass::Count];
		TUnique<LinearPagePool> mLinearPagePools[(u32)D3D12PageClass::Count];

		/** Guards lazy creation of persistent allocators and scratch page pools. */
		Mutex mAllocatorMutex;
	};
}

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(b3d::render::D3D12BufferPageBackend);
