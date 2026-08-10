	mResourceManager = B3DNew<D3D12ResourceManager>(*this);
	mHeapBackend = B3DMakeUnique<D3D12HeapBackend>(*this);
	mBufferPool = B3DMakeUnique<D3D12BufferPool>(*this);
D3D12GpuDevice::MemoryPoolType D3D12GpuDevice::GetMemoryPoolType(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType)
{
	if(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		switch(heapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT: return MemoryPoolType::DefaultBuffer;
		case D3D12_HEAP_TYPE_UPLOAD: return MemoryPoolType::UploadBuffer;
		case D3D12_HEAP_TYPE_READBACK: return MemoryPoolType::ReadbackBuffer;
		default: return MemoryPoolType::Count;
		}
	}

	if(heapType != D3D12_HEAP_TYPE_DEFAULT)
		return MemoryPoolType::Count;

	const bool isRenderTarget = (resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0;
	if(isRenderTarget)
		return MemoryPoolType::Count;

	return resourceDesc.SampleDesc.Count > 1 ? MemoryPoolType::DefaultMsaaTexture : MemoryPoolType::DefaultTexture;
}

D3D12GpuDevice::GpuMemoryAllocator& D3D12GpuDevice::GetOrCreateGpuMemoryAllocator(MemoryPoolType poolType)
{
	B3D_ASSERT(poolType < MemoryPoolType::Count);

	Lock lock(mGpuMemoryAllocatorMutex);
	TUnique<GpuMemoryAllocator>& slot = mGpuMemoryAllocators[(u32)poolType];
	if(slot != nullptr)
		return *slot;

	D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
	u64 heapAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	u64 initialHeapSize = 64ull * 1024 * 1024;

	switch(poolType)
	{
	case MemoryPoolType::DefaultBuffer:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		break;
	case MemoryPoolType::DefaultTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		break;
	case MemoryPoolType::DefaultMsaaTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		heapAlignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		break;
	case MemoryPoolType::UploadBuffer:
		heapType = D3D12_HEAP_TYPE_UPLOAD;
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		initialHeapSize = 16ull * 1024 * 1024;
		break;
	case MemoryPoolType::ReadbackBuffer:
		heapType = D3D12_HEAP_TYPE_READBACK;
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		initialHeapSize = 16ull * 1024 * 1024;
		break;
	default:
		B3D_ASSERT(false && "Invalid D3D12 memory pool type.");
		break;
	}

	GpuMemoryAllocator::Configuration configuration;
	configuration.InitialHeapSize = initialHeapSize;
	configuration.MaxHeapSize = 256ull * 1024 * 1024;
	configuration.GrowthFactor = 2;
	configuration.MaxEmptyHeapCount = 1;
	configuration.MinAllocationSize = 16;
	configuration.BufferImageGranularity = 1;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	configuration.HeapCreateInfo.Type = heapType;
	configuration.HeapCreateInfo.Flags = heapFlags;
	configuration.HeapCreateInfo.Alignment = heapAlignment;

	slot = B3DMakeUnique<GpuMemoryAllocator>(mHeapBackend.get(), nullptr, configuration);
	return *slot;
}

HRESULT D3D12GpuDevice::CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuResourceLocation& outAllocation)
{
	B3D_ASSERT(!outAllocation.IsValid());
	outResource.Reset();

	D3D12_RESOURCE_DESC1 enhancedResourceDescription = {};
	enhancedResourceDescription.Dimension = resourceDesc.Dimension;
	enhancedResourceDescription.Alignment = resourceDesc.Alignment;
	enhancedResourceDescription.Width = resourceDesc.Width;
	enhancedResourceDescription.Height = resourceDesc.Height;
	enhancedResourceDescription.DepthOrArraySize = resourceDesc.DepthOrArraySize;
	enhancedResourceDescription.MipLevels = resourceDesc.MipLevels;
	enhancedResourceDescription.Format = resourceDesc.Format;
	enhancedResourceDescription.SampleDesc = resourceDesc.SampleDesc;
	enhancedResourceDescription.Layout = resourceDesc.Layout;
	enhancedResourceDescription.Flags = resourceDesc.Flags;

	// TODO - Placed RT/DS resources begin uninitialized and must receive a full-subresource Clear, Discard, or Copy before
	// any other operation. The renderer supports rectangular clears and may render without a preceding full clear,
	// so keep these resources committed (and therefore zero-initialized) until first-use initialization is explicit.
	if((resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0)
	{
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = heapType;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0;
		heapProperties.VisibleNodeMask = 0;

		return mEnhancedDevice->CreateCommittedResource3(&heapProperties, D3D12_HEAP_FLAG_NONE, &enhancedResourceDescription, initialLayout, optimizedClearValue, nullptr, 0, nullptr, IID_PPV_ARGS(&outResource));
	}

	const MemoryPoolType poolType = GetMemoryPoolType(resourceDesc, heapType);
	if(poolType == MemoryPoolType::Count)
		return E_INVALIDARG;

	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = mDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
	if(allocationInfo.SizeInBytes == UINT64_MAX || allocationInfo.Alignment == 0 || allocationInfo.Alignment > UINT32_MAX)
		return E_INVALIDARG;

	GpuMemoryAllocator& allocator = GetOrCreateGpuMemoryAllocator(poolType);
	const GpuResourceKind resourceKind = resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER || resourceDesc.Layout == D3D12_TEXTURE_LAYOUT_ROW_MAJOR ? GpuResourceKind::Linear : GpuResourceKind::NonLinear;

	if(!allocator.TryAllocate(allocationInfo.SizeInBytes, (u32)allocationInfo.Alignment, resourceKind, outAllocation))
		return E_OUTOFMEMORY;

	D3D12GpuHeap& heap = ToD3D12GpuHeap(outAllocation.Heap);
	const HRESULT hr = mEnhancedDevice->CreatePlacedResource2(heap.Heap.Get(), outAllocation.Offset,
		&enhancedResourceDescription, initialLayout, optimizedClearValue, 0, nullptr, IID_PPV_ARGS(&outResource));
	if(FAILED(hr))
		allocator.Free(outAllocation);

	return hr;
}

void D3D12GpuDevice::FreeMemory(GpuResourceLocation& allocation)
{
	if(!allocation.IsValid())
		return;

	allocation.Allocator->Free(allocation);
}
