//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12DescriptorManager.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Sets @p count bits of @p bits, starting at @p first, to @p value. */
	void SetBits(TBitfield<>& bits, u32 first, u32 count, bool value)
	{
		for(u32 index = first; index < first + count; index++)
			bits[index] = value;
	}
}

D3D12BindlessHeap::D3D12BindlessHeap(D3D12ResourceManager* owner, u32 capacity, D3D12DescriptorHeap heap)
	: TD3D12Resource<GpuBindlessHeap>(owner, capacity), mHeap(std::move(heap))
{
	// A new heap lacks every table
	mDirtyTableDescriptors.Resize(D3D12DescriptorManager::kResourceTableDescriptorCount, true);
}

void D3D12BindlessHeap::MarkTableDescriptorsDirty(u32 offset, u32 count)
{
	SetBits(mDirtyTableDescriptors, offset, count, true);
}

D3D12DescriptorTable::D3D12DescriptorTable(D3D12ResourceManager* owner, D3D12DescriptorManager& descriptorManager, D3D12DescriptorHeapType type, u32 offset, u32 descriptorCount)
	: D3D12Resource(owner, "D3D12 descriptor table"), mDescriptorManager(descriptorManager), mType(type), mOffset(offset), mDescriptorCount(descriptorCount)
{ }

D3D12DescriptorTable::~D3D12DescriptorTable()
{
	mDescriptorManager.FreeGPUDescriptorTable(*this);
}

D3D12DescriptorManager::D3D12DescriptorManager(D3D12GpuDevice& device, bool supportsBindlessResources, u32 bindlessSamplerCount)
	: mDevice(device)
{
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	CreateHeaps(bindlessSamplerCount);

	mDefaultSamplerHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::Sampler);
	if (mDefaultSamplerHandle.ptr != 0)
	{
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		d3d12Device->CreateSampler(&samplerDesc, mDefaultSamplerHandle);
	}

	CreateNullDescriptors();

	if(!supportsBindlessResources)
	{
		D3D12BindlessHeap* heap = CreateResourceHeap(0);
		B3D_ASSERT(heap != nullptr && "Failed to create the shader-visible CBV/SRV/UAV descriptor heap");

		mFallbackResourceHeapPool = B3DMakeUnique<GpuBindlessHeapPool>(1);
		mFallbackResourceHeapPool->Initialize(heap);
		mResourceHeapPool = mFallbackResourceHeapPool.get();
	}
}

D3D12DescriptorManager::~D3D12DescriptorManager() = default;

void D3D12DescriptorManager::CreateNullDescriptors()
{
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

	// The formats are arbitrary (null descriptors read as zero regardless) but must be valid for the view dimension
	mNullCBVHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
	if (mNullCBVHandle.ptr != 0)
		d3d12Device->CreateConstantBufferView(nullptr, mNullCBVHandle);

	const D3D12_SRV_DIMENSION srvDimensions[] = {
		D3D12_SRV_DIMENSION_BUFFER, D3D12_SRV_DIMENSION_TEXTURE1D, D3D12_SRV_DIMENSION_TEXTURE1DARRAY,
		D3D12_SRV_DIMENSION_TEXTURE2D, D3D12_SRV_DIMENSION_TEXTURE2DARRAY, D3D12_SRV_DIMENSION_TEXTURE2DMS,
		D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY, D3D12_SRV_DIMENSION_TEXTURE3D, D3D12_SRV_DIMENSION_TEXTURECUBE,
		D3D12_SRV_DIMENSION_TEXTURECUBEARRAY
	};

	for (D3D12_SRV_DIMENSION dimension : srvDimensions)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if (handle.ptr == 0)
			continue;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dimension == D3D12_SRV_DIMENSION_BUFFER ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = dimension;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		d3d12Device->CreateShaderResourceView(nullptr, &srvDesc, handle);
		mNullSRVHandles[dimension] = handle;
	}

	const D3D12_UAV_DIMENSION uavDimensions[] = {
		D3D12_UAV_DIMENSION_BUFFER, D3D12_UAV_DIMENSION_TEXTURE1D, D3D12_UAV_DIMENSION_TEXTURE1DARRAY,
		D3D12_UAV_DIMENSION_TEXTURE2D, D3D12_UAV_DIMENSION_TEXTURE2DARRAY, D3D12_UAV_DIMENSION_TEXTURE3D
	};

	for (D3D12_UAV_DIMENSION dimension : uavDimensions)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if (handle.ptr == 0)
			continue;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = dimension == D3D12_UAV_DIMENSION_BUFFER ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = dimension;

		d3d12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, handle);
		mNullUAVHandles[dimension] = handle;
	}

	// Structured and raw buffer views differ from typed ones in their format, stride and flags
	auto fnCreateNullBufferViews = [this, d3d12Device](DXGI_FORMAT format, u32 stride, bool isRaw, D3D12_CPU_DESCRIPTOR_HANDLE& outSRVHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE& outUAVHandle)
	{
		outSRVHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(outSRVHandle.ptr != 0)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.StructureByteStride = stride;
			srvDesc.Buffer.Flags = isRaw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

			d3d12Device->CreateShaderResourceView(nullptr, &srvDesc, outSRVHandle);
		}

		outUAVHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(outUAVHandle.ptr != 0)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.StructureByteStride = stride;
			uavDesc.Buffer.Flags = isRaw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

			d3d12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, outUAVHandle);
		}
	};

	fnCreateNullBufferViews(DXGI_FORMAT_UNKNOWN, sizeof(u32), false, mNullStructuredBufferSRVHandle, mNullStructuredBufferUAVHandle);
	fnCreateNullBufferViews(DXGI_FORMAT_R32_TYPELESS, 0, true, mNullRawBufferSRVHandle, mNullRawBufferUAVHandle);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::GetNullSRVHandle(GpuParameterObjectType type) const
{
	switch(type)
	{
	case GPOT_TEXTURE1D:			return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE1D];
	case GPOT_TEXTURE1DARRAY:		return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE1DARRAY];
	case GPOT_TEXTURE2D:			return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE2D];
	case GPOT_TEXTURE2DARRAY:		return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE2DARRAY];
	case GPOT_TEXTURE2DMS:			return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE2DMS];
	case GPOT_TEXTURE2DMSARRAY:		return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY];
	case GPOT_TEXTURE3D:			return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE3D];
	case GPOT_TEXTURECUBE:			return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURECUBE];
	case GPOT_TEXTURECUBEARRAY:		return mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURECUBEARRAY];
	case GPOT_STRUCTURED_BUFFER:	return mNullStructuredBufferSRVHandle;
	case GPOT_BYTE_ADDRESS_BUFFER:	return mNullRawBufferSRVHandle;
	default:
		return GpuObjectParameterTypeInformation::IsBuffer(type) ? mNullSRVHandles[D3D12_SRV_DIMENSION_BUFFER] : mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURE2D];
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::GetNullUAVHandle(GpuParameterObjectType type) const
{
	switch(type)
	{
	case GPOT_RWTEXTURE1D:						return mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE1D];
	case GPOT_RWTEXTURE1DARRAY:					return mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE1DARRAY];
	case GPOT_RWTEXTURE2D:
	case GPOT_RWTEXTURE2DMS:					return mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE2D];
	case GPOT_RWTEXTURE2DARRAY:
	case GPOT_RWTEXTURE2DMSARRAY:				return mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE2DARRAY];
	case GPOT_RWTEXTURE3D:						return mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE3D];
	case GPOT_RWSTRUCTURED_BUFFER:
	case GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER:
	case GPOT_RWAPPEND_BUFFER:
	case GPOT_RWCONSUME_BUFFER:					return mNullStructuredBufferUAVHandle;
	case GPOT_RWBYTE_ADDRESS_BUFFER:			return mNullRawBufferUAVHandle;
	default:
		return GpuObjectParameterTypeInformation::IsBuffer(type) ? mNullUAVHandles[D3D12_UAV_DIMENSION_BUFFER] : mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE2D];
	}
}

void D3D12DescriptorManager::CreateHeaps(u32 bindlessSamplerCount)
{
	ID3D12Device* device = mDevice.GetD3D12Device();

	auto fnCreateHeap = [device](D3D12DescriptorHeap& outHeap, D3D12_DESCRIPTOR_HEAP_TYPE nativeType, u32 descriptorCount, bool shaderVisible, const char* debugName, u32 bindlessCount = 0)
	{
		const bool isCreated = outHeap.Create(device, nativeType, descriptorCount, shaderVisible, bindlessCount);
		B3D_ASSERT(isCreated && debugName);
		(void)isCreated;
	};

	// Every shader-visible CBV/SRV/UAV heap shares the table region allocation
	TableRegion& resourceTableRegion = mTableRegions[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV];
	resourceTableRegion.FreeRanges = RangeAllocator(kResourceTableDescriptorCount);
	resourceTableRegion.LiveDescriptors.Resize(kResourceTableDescriptorCount);
	fnCreateHeap(mTableMirror, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kResourceTableDescriptorCount, false, "Failed to create the table mirror descriptor heap");

	// Bindless descriptors are at the start of the heaps, so a bindless index equals its heap index
	fnCreateHeap(mSamplerHeap, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE, true, "Failed to create Sampler descriptor heap", bindlessSamplerCount);
	TableRegion& samplerTableRegion = mTableRegions[(u32)D3D12DescriptorHeapType::Sampler];
	samplerTableRegion.FreeRanges = RangeAllocator(D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE - bindlessSamplerCount);
	samplerTableRegion.LiveDescriptors.Resize(D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE - bindlessSamplerCount);

	// Shader-visible heaps cannot be descriptor-copy sources, so resource views live in separate staging heaps.
	fnCreateHeap(mStagingAllocators[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].Heap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 32 * 1024, false, "Failed to create CBV/SRV/UAV staging descriptor heap");
	fnCreateHeap(mStagingAllocators[(u32)D3D12DescriptorHeapType::Sampler].Heap, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048, false, "Failed to create Sampler staging descriptor heap");
	fnCreateHeap(mStagingAllocators[(u32)D3D12DescriptorHeapType::RTV].Heap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024, false, "Failed to create RTV descriptor heap");
	// A depth-stencil framebuffer takes four views (one per read-only combination), so this fits as many framebuffers as the RTV heap
	fnCreateHeap(mStagingAllocators[(u32)D3D12DescriptorHeapType::DSV].Heap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 4096, false, "Failed to create DSV descriptor heap");
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::AllocateCPUDescriptor(D3D12DescriptorHeapType type)
{
	// Potential future improvements:
	//  - Might be too slow to allocate these one by one.
	//  - Using a linked list might be better than a Vector<T> free list

	StagingAllocator& allocator = mStagingAllocators[(u32)type];
	Lock lock(allocator.AllocationMutex);

	if (!allocator.FreeList.empty())
	{
		const u32 index = allocator.FreeList.back();
		allocator.FreeList.pop_back();

		return allocator.Heap.GetCPUHandle(index);
	}

	if (!B3D_ENSURE(allocator.NextFreeIndex < allocator.Heap.GetCapacity()))
	{
		B3D_LOG(Error, LogRenderBackend, "Descriptor heap exhausted for type {0}", (u32)type);
		return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
	}

	return allocator.Heap.GetCPUHandle(allocator.NextFreeIndex++);
}

void D3D12DescriptorManager::FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	StagingAllocator& allocator = mStagingAllocators[(u32)type];
	Lock lock(allocator.AllocationMutex);

	const SIZE_T offset = handle.ptr - allocator.Heap.GetCPUHandle(0).ptr;
	allocator.FreeList.push_back((u32)(offset / allocator.Heap.GetDescriptorSize()));
}

D3D12DescriptorTable* D3D12DescriptorManager::AllocateGPUDescriptorTable(D3D12DescriptorHeapType type, u32 count)
{
	B3D_ASSERT(type == D3D12DescriptorHeapType::CBV_SRV_UAV || type == D3D12DescriptorHeapType::Sampler);

	TableRegion& region = mTableRegions[(u32)type];

	// Note: This allocator could get very fragmented, a potential improvement to use a better allocator. Ideally also to handle transient allocations separately using a linear allocator.
	u32 startIndex = RangeAllocator::kInvalidIndex;
	{
		Lock lock(region.RegionMutex);
		startIndex = region.FreeRanges.Allocate(count);
		if(startIndex != RangeAllocator::kInvalidIndex)
			SetBits(region.LiveDescriptors, startIndex, count, true);
	}

	if (startIndex == RangeAllocator::kInvalidIndex)
	{
		B3D_LOG(Error, LogRenderBackend, "Shader-visible descriptor heap exhausted for type {0}", (u32)type);
		return nullptr;
	}

	return mDevice.GetResourceManager().Create<D3D12DescriptorTable>(*this, type, startIndex, count);
}

void D3D12DescriptorManager::FreeGPUDescriptorTable(const D3D12DescriptorTable& table)
{
	TableRegion& region = mTableRegions[(u32)table.GetType()];
	Lock lock(region.RegionMutex);

	region.FreeRanges.Free(table.GetOffset(), table.GetDescriptorCount());
	SetBits(region.LiveDescriptors, table.GetOffset(), table.GetDescriptorCount(), false);
}

void D3D12DescriptorManager::WriteTable(const D3D12DescriptorTable& table, TArrayView<const D3D12_CPU_DESCRIPTOR_HANDLE> sources)
{
	B3D_ASSERT(sources.Size() == table.GetDescriptorCount());

	ID3D12Device* device = mDevice.GetD3D12Device();
	const UINT descriptorCount = table.GetDescriptorCount();
	if(table.GetType() == D3D12DescriptorHeapType::Sampler)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE destination = mSamplerHeap.GetCPUHandle(mSamplerHeap.GetTableRegionStart() + table.GetOffset());
		device->CopyDescriptors(1, &destination, &descriptorCount, descriptorCount, sources.Data(), nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		return;
	}

	Lock lock(mTableRegions[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].RegionMutex);

	const D3D12_CPU_DESCRIPTOR_HANDLE mirrorStart = mTableMirror.GetCPUHandle(table.GetOffset());
	device->CopyDescriptors(1, &mirrorStart, &descriptorCount, descriptorCount, sources.Data(), nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Command buffers bind the heap that is current when they record, so other heaps only need the table before they are current again
	const D3D12BindlessHeap* currentHeap = GetCurrentResourceHeap();
	const D3D12DescriptorHeap& currentDescriptorHeap = currentHeap->GetDescriptorHeap();
	device->CopyDescriptorsSimple(descriptorCount, currentDescriptorHeap.GetCPUHandle(currentDescriptorHeap.GetTableRegionStart() + table.GetOffset()), mirrorStart,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mResourceHeapPool->ForEachHeap([currentHeap, &table](GpuBindlessHeap& heap)
	{
		if(&heap != currentHeap)
			static_cast<D3D12BindlessHeap&>(heap).MarkTableDescriptorsDirty(table.GetOffset(), table.GetDescriptorCount());
	});
}

void D3D12DescriptorManager::CopyDirtyTables(D3D12BindlessHeap& heap)
{
	const TableRegion& region = mTableRegions[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV];
	const D3D12DescriptorHeap& descriptorHeap = heap.GetDescriptorHeap();
	TBitfield<>& dirtyDescriptors = heap.GetDirtyTableDescriptors();

	FrameAllocatorScope frameScope;
	FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> destinationStarts;
	FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> sourceStarts;
	FrameVector<UINT> rangeSizes;
	dirtyDescriptors.ForEachSetRange(region.LiveDescriptors, kResourceTableDescriptorCount, [&](const TIndexRange<u64>& range)
	{
		destinationStarts.push_back(descriptorHeap.GetCPUHandle(descriptorHeap.GetTableRegionStart() + (u32)range.First));
		sourceStarts.push_back(mTableMirror.GetCPUHandle((u32)range.First));
		rangeSizes.push_back((UINT)range.Count);
	});

	if(!rangeSizes.empty())
	{
		mDevice.GetD3D12Device()->CopyDescriptors((UINT)rangeSizes.size(), destinationStarts.data(), rangeSizes.data(), (UINT)rangeSizes.size(), sourceStarts.data(),
			rangeSizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	dirtyDescriptors.Reset(false);
}

D3D12BindlessHeap* D3D12DescriptorManager::GetAndTrackResourceHeap(GpuResourceTracker& resourceTracker)
{
	return static_cast<D3D12BindlessHeap*>(mResourceHeapPool->GetAndTrack(resourceTracker));
}

D3D12BindlessHeap* D3D12DescriptorManager::GetCurrentResourceHeap() const
{
	return static_cast<D3D12BindlessHeap*>(mResourceHeapPool->GetCurrent());
}

void D3D12DescriptorManager::DestroyFallbackResourceHeap()
{
	B3D_ASSERT(mFallbackResourceHeapPool == nullptr || mResourceHeapPool == mFallbackResourceHeapPool.get());

	mResourceHeapPool = nullptr;
	mFallbackResourceHeapPool = nullptr;
}

D3D12BindlessHeap* D3D12DescriptorManager::CreateResourceHeap(u32 bindlessCapacity)
{
	const u32 descriptorCount = bindlessCapacity + kResourceTableDescriptorCount;

	D3D12DescriptorHeap heap;
	if(!heap.Create(mDevice.GetD3D12Device(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorCount, true, bindlessCapacity))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create a shader-visible CBV/SRV/UAV descriptor heap of {0} descriptors.", descriptorCount);
		return nullptr;
	}

	return mDevice.GetResourceManager().Create<D3D12BindlessHeap>(bindlessCapacity, std::move(heap));
}

void D3D12DescriptorManager::WriteBindlessSamplerSlot(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source)
{
	B3D_ASSERT(slotIndex < mSamplerHeap.GetTableRegionStart());
	mDevice.GetD3D12Device()->CopyDescriptorsSimple(1, mSamplerHeap.GetCPUHandle(slotIndex), source, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}
