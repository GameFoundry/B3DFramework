//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"

using namespace b3d;
using namespace b3d::render;

D3D12BufferPage::D3D12BufferPage(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuResourceLocation backingAllocation, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags, void* mappedData)
	: D3D12BufferResource(owner, "D3D12 buffer page"), mResource(std::move(resource)), mBackingAllocation(backingAllocation), mHeapType(heapType), mFlags(flags), mMappedData(mappedData)
{ }

D3D12BufferPage::~D3D12BufferPage()
{
	if(mMappedData != nullptr)
		mResource->Unmap(0, nullptr);

	mMappedData = nullptr;
	mResource.Reset();
	GetDevice().FreeMemory(mBackingAllocation);
}

IGpuHeap* D3D12BufferPageBackend::CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
{
	D3D12_RESOURCE_DESC resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescription.Width = sizeInBytes;
	resourceDescription.Height = 1;
	resourceDescription.DepthOrArraySize = 1;
	resourceDescription.MipLevels = 1;
	resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescription.SampleDesc.Count = 1;
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescription.Flags = createInformation.Flags;

	// TODO - We're using two allocator layers here. We could be allocating the memory directly here, since the buffer heaps are regularly sized,
	// probably no need to call into the generic TLSF allocator, and then have our own TLSF on top.
	ComPtr<ID3D12Resource> resource;
	GpuResourceLocation backingAllocation;
	HRESULT result = mDevice.CreateResource(resourceDescription, createInformation.HeapType, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, resource, backingAllocation);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create a {0}-byte buffer pool page (hr={1}, heapType={2}, flags={3}).", sizeInBytes, (u32)result, (u32)createInformation.HeapType, (u32)createInformation.Flags);
		return nullptr;
	}

	void* mappedData = nullptr;
	if(createInformation.HeapType == D3D12_HEAP_TYPE_UPLOAD || createInformation.HeapType == D3D12_HEAP_TYPE_READBACK)
	{
		D3D12_RANGE readRange = {};
		if(createInformation.HeapType == D3D12_HEAP_TYPE_READBACK)
			readRange.End = (SIZE_T)sizeInBytes;

		result = resource->Map(0, &readRange, &mappedData);
		if(FAILED(result))
		{
			resource.Reset();
			mDevice.FreeMemory(backingAllocation);
			B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to map a buffer pool page (hr={0}).", (u32)result);
			return nullptr;
		}
	}

	resource->SetName(L"D3D12 buffer pool page");
	return mDevice.GetResourceManager().Create<D3D12BufferPage>(std::move(resource), backingAllocation, createInformation.HeapType, createInformation.Flags, mappedData);
}

void D3D12BufferPageBackend::DestroyHeap(IGpuHeap* handle)
{
	if(handle != nullptr)
		static_cast<D3D12BufferPage*>(handle)->Destroy();
}

D3D12BufferPool::D3D12BufferPool(D3D12GpuDevice& device)
	: mBackend(device)
{ }

D3D12BufferPool::~D3D12BufferPool()
{
	for(TUnique<Allocator>& allocator : mAllocators)
	{
		if(allocator != nullptr)
		{
			allocator->ReclaimUnused(true);
			allocator.reset();
		}
	}
}

bool D3D12BufferPool::TryAllocate(u64 size, u32 alignment, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS resourceFlags, GpuResourceLocation& outAllocation)
{
	PoolType poolType = PoolType::Count;
	switch(heapType)
	{
	case D3D12_HEAP_TYPE_DEFAULT:
		poolType = (resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 ? PoolType::DefaultUnorderedAccess : PoolType::Default;
		break;
	case D3D12_HEAP_TYPE_UPLOAD:
		poolType = resourceFlags == D3D12_RESOURCE_FLAG_NONE ? PoolType::Upload : PoolType::Count;
		break;
	case D3D12_HEAP_TYPE_READBACK:
		poolType = resourceFlags == D3D12_RESOURCE_FLAG_NONE ? PoolType::Readback : PoolType::Count;
		break;
	default:
		break;
	}

	if(poolType == PoolType::Count)
		return false;

	Allocator* allocator;
	{
		Lock lock(mAllocatorMutex);
		TUnique<Allocator>& allocatorEntry = mAllocators[(u32)poolType];
		if(allocatorEntry == nullptr)
		{
			Allocator::Configuration configuration;
			configuration.InitialHeapSize = poolType == PoolType::Default || poolType == PoolType::DefaultUnorderedAccess ? 16ull * 1024 * 1024 : 4ull * 1024 * 1024;
			configuration.MaxHeapSize = 64ull * 1024 * 1024;
			configuration.GrowthFactor = 2;
			configuration.MaxEmptyHeapCount = 1;
			configuration.MinAllocationSize = 64;
			configuration.BufferImageGranularity = 1;
			configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

			switch(poolType)
			{
			case PoolType::Default:
				configuration.HeapCreateInfo.HeapType = D3D12_HEAP_TYPE_DEFAULT;
				break;
			case PoolType::DefaultUnorderedAccess:
				configuration.HeapCreateInfo.HeapType = D3D12_HEAP_TYPE_DEFAULT;
				configuration.HeapCreateInfo.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				break;
			case PoolType::Upload:
				configuration.HeapCreateInfo.HeapType = D3D12_HEAP_TYPE_UPLOAD;
				break;
			case PoolType::Readback:
				configuration.HeapCreateInfo.HeapType = D3D12_HEAP_TYPE_READBACK;
				break;
			default:
				B3D_ASSERT(false);
				break;
			}

			allocatorEntry = B3DMakeUnique<Allocator>(&mBackend, nullptr, configuration);
		}

		allocator = allocatorEntry.get();
	}

	return allocator->TryAllocate(size, alignment, GpuResourceKind::Linear, outAllocation);
}
