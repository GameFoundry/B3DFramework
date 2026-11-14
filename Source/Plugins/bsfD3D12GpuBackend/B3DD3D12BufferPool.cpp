//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"

using namespace b3d;
using namespace b3d::render;

D3D12BufferPage::D3D12BufferPage(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuAllocation backingAllocation, const D3D12BufferPageCreateInformation& createInformation,
	D3D12_HEAP_TYPE heapType, void* mappedData)
	: D3D12BufferResource(owner, "D3D12 buffer page"), D3D12HeapBase(D3D12HeapKind::BufferPage), mResource(std::move(resource)), mBackingAllocation(backingAllocation)
	, mCreateInformation(createInformation), mHeapType(heapType), mMappedData(mappedData)
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
	GpuAllocation backingAllocation;
	HRESULT result = mDevice.CreateResource(resourceDescription, createInformation.HeapProperties, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, resource, backingAllocation);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create a {0}-byte buffer pool page (hr={1}, heapType={2}, flags={3}).", sizeInBytes, (u32)result, (u32)createInformation.HeapProperties.Type, (u32)createInformation.Flags);
		return nullptr;
	}

	// The heap can differ from the requested one (see D3D12HeapBackend::CreateHeap())
	const D3D12_HEAP_TYPE heapType = ToD3D12GpuHeap(backingAllocation.Heap).Type;

	void* mappedData = nullptr;
	if(heapType != D3D12_HEAP_TYPE_DEFAULT)
	{
		D3D12_RANGE readRange = {};
		if(heapType == D3D12_HEAP_TYPE_CUSTOM)
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
	return mDevice.GetResourceManager().Create<D3D12BufferPage>(std::move(resource), backingAllocation, createInformation, heapType, mappedData);
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
	for(TUnique<PersistentAllocator>& allocator : mPersistentAllocators)
	{
		if(allocator != nullptr)
		{
			allocator->ReclaimUnused(true);
			allocator.reset();
		}
	}

	for(TUnique<LinearPagePool>& pagePool : mLinearPagePools)
		pagePool.reset();
}

D3D12PageClass D3D12BufferPool::GetPageClass(const D3D12HeapProperties& heapProperties, D3D12_RESOURCE_FLAGS resourceFlags)
{
	switch(heapProperties.Type)
	{
	case D3D12_HEAP_TYPE_DEFAULT:
		if(resourceFlags == D3D12_RESOURCE_FLAG_NONE)
			return D3D12PageClass::Default;

		if(resourceFlags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			return D3D12PageClass::DefaultUnorderedAccess;

		return D3D12PageClass::Count;
	case D3D12_HEAP_TYPE_GPU_UPLOAD:
		if(resourceFlags != D3D12_RESOURCE_FLAG_NONE)
			return D3D12PageClass::Count;

		return heapProperties.Fallback == D3D12HeapFallback::UnmappedGpuMemory ? D3D12PageClass::GpuUploadOrDefault : D3D12PageClass::GpuUpload;
	case D3D12_HEAP_TYPE_CUSTOM:
		if(resourceFlags != D3D12_RESOURCE_FLAG_NONE)
			return D3D12PageClass::Count;

		return heapProperties.CpuPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE ? D3D12PageClass::CustomWriteCombine : D3D12PageClass::CustomWriteBack;
	default:
		return D3D12PageClass::Count;
	}
}

D3D12BufferPageCreateInformation D3D12BufferPool::GetPageCreateInformation(D3D12PageClass pageClass)
{
	D3D12BufferPageCreateInformation createInformation;
	switch(pageClass)
	{
	case D3D12PageClass::Default:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case D3D12PageClass::DefaultUnorderedAccess:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		createInformation.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		break;
	case D3D12PageClass::GpuUpload:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_GPU_UPLOAD;
		break;
	case D3D12PageClass::GpuUploadOrDefault:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_GPU_UPLOAD;
		createInformation.HeapProperties.Fallback = D3D12HeapFallback::UnmappedGpuMemory;
		break;
	case D3D12PageClass::CustomWriteBack:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		createInformation.HeapProperties.CpuPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		break;
	case D3D12PageClass::CustomWriteCombine:
		createInformation.HeapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		createInformation.HeapProperties.CpuPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		break;
	default:
		B3D_ASSERT(false);
		break;
	}

	return createInformation;
}

u64 D3D12BufferPool::GetPersistentPageSize(D3D12PageClass pageClass)
{
	return pageClass == D3D12PageClass::Default || pageClass == D3D12PageClass::DefaultUnorderedAccess ? 16ull * 1024 * 1024 : 4ull * 1024 * 1024;
}

u64 D3D12BufferPool::GetScratchPageSize(D3D12PageClass pageClass)
{
	return pageClass == D3D12PageClass::Default || pageClass == D3D12PageClass::DefaultUnorderedAccess ? 8ull * 1024 * 1024 : 4ull * 1024 * 1024;
}

void D3D12BufferPool::GetPersistentAllocators(TArray<IGpuAllocator*>& outAllocators)
{
	Lock lock(mAllocatorMutex);
	for(u32 pageClassIndex = 0; pageClassIndex < (u32)D3D12PageClass::Count; pageClassIndex++)
	{
		if(mPersistentAllocators[pageClassIndex] != nullptr)
			outAllocators.Add(mPersistentAllocators[pageClassIndex].get());
	}
}

IGpuAllocator& D3D12BufferPool::GetOrCreateGpuMemoryAllocator(D3D12PageClass pageClass)
{
	B3D_ASSERT(pageClass < D3D12PageClass::Count);

	Lock lock(mAllocatorMutex);
	TUnique<PersistentAllocator>& allocator = mPersistentAllocators[(u32)pageClass];
	if(allocator == nullptr)
	{
		PersistentAllocator::Configuration configuration;
		configuration.InitialHeapSize = GetPersistentPageSize(pageClass);
		configuration.MaxHeapSize = 64ull * 1024 * 1024;
		configuration.GrowthFactor = 2;
		configuration.MaxEmptyHeapCount = 1;
		configuration.MinAllocationSize = 64;
		configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
		configuration.HeapCreateInfo = GetPageCreateInformation(pageClass);

		allocator = B3DMakeUnique<PersistentAllocator>(&mBackend, nullptr, configuration);
	}

	return *allocator;
}

D3D12BufferPool::LinearPagePool& D3D12BufferPool::GetOrCreateLinearPagePool(D3D12PageClass pageClass)
{
	B3D_ASSERT(pageClass < D3D12PageClass::Count);

	Lock lock(mAllocatorMutex);
	TUnique<LinearPagePool>& pagePool = mLinearPagePools[(u32)pageClass];
	if(pagePool == nullptr)
	{
		LinearPagePool::Configuration configuration;
		configuration.PageSize = GetScratchPageSize(pageClass);
		configuration.MaxRetainedPages = 4;
		configuration.HeapCreateInfo = GetPageCreateInformation(pageClass);

		pagePool = B3DMakeUnique<LinearPagePool>(&mBackend, configuration);
	}

	return *pagePool;
}

TUnique<IGpuAllocator> D3D12BufferPool::CreateScratchAllocator(D3D12PageClass pageClass, IGpuCompletionTracker& completionTracker)
{
	LinearPagePool& pagePool = GetOrCreateLinearPagePool(pageClass);

	ScratchAllocator::Configuration configuration;
	configuration.PageSize = pagePool.GetPageSize();
	configuration.HeapCreateInfo = GetPageCreateInformation(pageClass);

	return B3DMakeUnique<ScratchAllocator>(&mBackend, &completionTracker, configuration, &pagePool);
}
