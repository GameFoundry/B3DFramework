//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Size a constant buffer view must be a multiple of. */
	constexpr u32 kConstantBufferViewSizeAlignment = 256;

	/**
	 * Converts an engine buffer element format into the DXGI format used to type a typed (simple storage) buffer
	 * view. Returns DXGI_FORMAT_UNKNOWN for formats without a direct 1:1 mapping.
	 */
	DXGI_FORMAT GetBufferViewFormat(GpuBufferFormat format) // TODO - Move to D3D12Utility
	{
		switch(format)
		{
		case BF_16X1F: return DXGI_FORMAT_R16_FLOAT;
		case BF_16X2F: return DXGI_FORMAT_R16G16_FLOAT;
		case BF_16X4F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case BF_32X1F: return DXGI_FORMAT_R32_FLOAT;
		case BF_32X2F: return DXGI_FORMAT_R32G32_FLOAT;
		case BF_32X3F: return DXGI_FORMAT_R32G32B32_FLOAT;
		case BF_32X4F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case BF_8X1: return DXGI_FORMAT_R8_UNORM;
		case BF_8X2: return DXGI_FORMAT_R8G8_UNORM;
		case BF_8X4: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case BF_16X1: return DXGI_FORMAT_R16_UNORM;
		case BF_16X2: return DXGI_FORMAT_R16G16_UNORM;
		case BF_16X4: return DXGI_FORMAT_R16G16B16A16_UNORM;
		case BF_8X1S: return DXGI_FORMAT_R8_SINT;
		case BF_8X2S: return DXGI_FORMAT_R8G8_SINT;
		case BF_8X4S: return DXGI_FORMAT_R8G8B8A8_SINT;
		case BF_16X1S: return DXGI_FORMAT_R16_SINT;
		case BF_16X2S: return DXGI_FORMAT_R16G16_SINT;
		case BF_16X4S: return DXGI_FORMAT_R16G16B16A16_SINT;
		case BF_32X1S: return DXGI_FORMAT_R32_SINT;
		case BF_32X2S: return DXGI_FORMAT_R32G32_SINT;
		case BF_32X3S: return DXGI_FORMAT_R32G32B32_SINT;
		case BF_32X4S: return DXGI_FORMAT_R32G32B32A32_SINT;
		case BF_8X1U: return DXGI_FORMAT_R8_UINT;
		case BF_8X2U: return DXGI_FORMAT_R8G8_UINT;
		case BF_8X4U: return DXGI_FORMAT_R8G8B8A8_UINT;
		case BF_16X1U: return DXGI_FORMAT_R16_UINT;
		case BF_16X2U: return DXGI_FORMAT_R16G16_UINT;
		case BF_16X4U: return DXGI_FORMAT_R16G16B16A16_UINT;
		case BF_32X1U: return DXGI_FORMAT_R32_UINT;
		case BF_32X2U: return DXGI_FORMAT_R32G32_UINT;
		case BF_32X3U: return DXGI_FORMAT_R32G32B32_UINT;
		case BF_32X4U: return DXGI_FORMAT_R32G32B32A32_UINT;
		default: return DXGI_FORMAT_UNKNOWN;
		}
	}
}

D3D12Buffer::D3D12Buffer(D3D12ResourceManager* owner, GpuAllocation allocation, ComPtr<ID3D12Resource> resource, void* mappedData, const StringView& name)
	: TD3D12Resource<IGpuBufferResource>(owner, name), mAllocation(allocation), mResource(std::move(resource)), mMappedData(mappedData)
{ }

D3D12Buffer::~D3D12Buffer()
{
	// A placed resource is released before its memory
	if(mMappedData != nullptr)
		mResource->Unmap(0, nullptr);

	mMappedData = nullptr;
	mResource.Reset();
	GetDevice().FreeMemory(mAllocation);
}

D3D12GpuBuffer::D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device, const GpuAllocation& allocation)
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device), allocation)
{
}

u32 D3D12GpuBuffer::GetResourceSize(const GpuBufferInformation& information, u32 totalSize)
{
	// Not allowed to have size 0 buffer
	u32 bufferSize = Math::Max(totalSize, 64u);

	// Constant buffer views must be sized to a 256-byte multiple, and may not extend past the end of the
	// resource, so uniform buffers get their backing resource padded accordingly.
	if(information.Type == GpuBufferType::Uniform)
		bufferSize = Math::CeilToMultiple(bufferSize, kConstantBufferViewSizeAlignment);

	return bufferSize;
}

D3D12_RESOURCE_DESC D3D12GpuBuffer::GetResourceDescription(u64 size, D3D12_RESOURCE_FLAGS flags, bool useTightAlignment)
{
	D3D12_RESOURCE_DESC output = {};
	output.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	output.Width = size;
	output.Height = 1;
	output.DepthOrArraySize = 1;
	output.MipLevels = 1;
	output.Format = DXGI_FORMAT_UNKNOWN;
	output.SampleDesc.Count = 1;
	output.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	output.Flags = flags;

	if(useTightAlignment)
		output.Flags |= D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT;

	return output;
}

D3D12GpuBuffer::~D3D12GpuBuffer()
{
	ReleaseBuffer();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::ReleaseBuffer()
{
	D3D12DescriptorManager& descriptorManager = static_cast<D3D12GpuDevice&>(mDevice).GetDescriptorManager();
	{
		Lock lock(mViewMutex);
		for(BufferViews& views : mViews)
		{
			if(views.Cbv.ptr != 0)
				descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, views.Cbv);
			if(views.Srv.ptr != 0)
				descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, views.Srv);
			if(views.Uav.ptr != 0)
				descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, views.Uav);
		}

		mViews.clear();
	}

	if(mBuffer == nullptr)
		return;

	mMappedMemory = nullptr;

	SetCurrentAllocation(GpuAllocation());
	mBuffer->Destroy();
	mBuffer = nullptr;
}

void D3D12GpuBuffer::SwapInternalBuffer(GpuBuffer& other)
{
	D3D12GpuBuffer& d3d12Other = static_cast<D3D12GpuBuffer&>(other);

	Lock lock(mViewMutex);
	Lock otherLock(d3d12Other.mViewMutex);
	std::swap(mBuffer, d3d12Other.mBuffer);
	std::swap(mMappedMemory, d3d12Other.mMappedMemory);
	std::swap(mVertexBufferView, d3d12Other.mVertexBufferView);
	std::swap(mIndexBufferView, d3d12Other.mIndexBufferView);
	std::swap(mViews, d3d12Other.mViews);

	mResourceGeneration++;
	d3d12Other.mResourceGeneration++;
}

void D3D12GpuBuffer::Initialize()
{
	RecreateInternalBuffer();
	GpuBuffer::Initialize();
}

void D3D12GpuBuffer::RecreateInternalBuffer()
{
	ReleaseBuffer();
	mResourceGeneration++;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);
	const GpuBufferInformation& information = GetInformation();

	GpuAllocation allocation;
	if(!mRequestedAllocation.IsPending())
		allocation = mRequestedAllocation;
	else
	{
		const GpuMemoryRequirements memoryRequirements = device.GetMemoryRequirements(information);
		if(!mRequestedAllocation.Allocator->TryAllocate(memoryRequirements.Size, (u32)memoryRequirements.Alignment, allocation))
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to allocate a buffer (size={0}, alignment={1}, type={2}, memory={3}, flags={4}).",
				memoryRequirements.Size, memoryRequirements.Alignment, (u32)information.Type, (u32)information.Memory, (u32)information.Flags);
			return;
		}
	}

	const u32 resourceSize = GetResourceSize(information, mTotalSize);
	const D3D12_RESOURCE_DESC resourceDescription = GetResourceDescription(resourceSize, D3D12Utility::GetBufferResourceFlags(information.Flags),
		device.IsTightAlignmentSupported());

	ComPtr<ID3D12Resource> resource;
	GpuAllocation placedAllocation;
	HRESULT result = device.CreateResource(resourceDescription, allocation, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, resource, placedAllocation);

	// The memory can be the fallback of the requested one (see D3D12HeapBackend::CreateHeap())
	const D3D12HeapMemory& memory = ToD3D12GpuHeap(allocation.Heap).Memory;

	void* mappedData = nullptr;
	if(SUCCEEDED(result) && memory.CpuPageProperty != D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE)
	{
		// The CPU only reads memory it caches, since reading write-combined memory is slow
		D3D12_RANGE readRange = {};
		if(memory.CpuPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK)
			readRange.End = (SIZE_T)resourceSize;

		result = resource->Map(0, &readRange, &mappedData);
	}

	if(FAILED(result))
	{
		resource.Reset();
		if(mRequestedAllocation.IsPending())
			allocation.Allocator->Free(allocation);

		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create a buffer (hr={0}, size={1}, type={2}, memory={3}, flags={4}).", (u32)result, resourceSize,
			(u32)information.Type, (u32)information.Memory, (u32)information.Flags);
		return;
	}

	mBuffer = device.GetResourceManager().Create<D3D12Buffer>(allocation, std::move(resource), mappedData, mName);
	SetCurrentAllocation(allocation);
	mMappedMemory = (u8*)mBuffer->GetMappedData();
	mMappedMemory = (u8*)mBuffer->GetMappedData();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Initialize suballocation tracking for the new buffer; without it IsRangeBound/IsRangeInUse fall back to
	// whole-buffer counts and ValidateMap reports false positives for every write to a pooled suballocation
	if(mBuffer != nullptr)
		mBuffer->InitializeSuballocationTracking(information.SuballocationCount, mSuballocationSize);
#endif

	if(!mName.empty())
		SetName(mName);

	// Create vertex buffer view if this is a vertex buffer.
	if(information.Type == GpuBufferType::Vertex)
	{
		mVertexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = (UINT)mTotalSize;
		mVertexBufferView.StrideInBytes = (UINT)information.Vertex.ElementSize;
	}

	// Create index buffer view if this is an index buffer.
	if(information.Type == GpuBufferType::Index)
	{
		mIndexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = (UINT)mTotalSize;
		mIndexBufferView.Format = information.Index.Type == IT_32BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	}

	// Create the default shader-binding descriptors. Format and offset overrides remain lazy.
	if(information.Type == GpuBufferType::Uniform)
		GetOrCreateView(GpuBufferViewType::Typed, BF_UNKNOWN, ViewType::CBV);

	if(information.Type == GpuBufferType::SimpleStorage || information.Type == GpuBufferType::StructuredStorage || information.Type == GpuBufferType::Vertex)
	{
		GetOrCreateView(GpuBufferViewType::Typed, BF_UNKNOWN, ViewType::SRV);
		if(information.Flags.IsSet(GpuBufferFlag::GpuWrite))
			GetOrCreateView(GpuBufferViewType::Typed, BF_UNKNOWN, ViewType::UAV);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_VertexBuffer);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetCBVHandle(u32 offset) const
{
	return GetOrCreateView(GpuBufferViewType::Typed, BF_UNKNOWN, ViewType::CBV, offset);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetSRVHandle(GpuBufferViewType viewType, GpuBufferFormat format) const
{
	return GetOrCreateView(viewType, format, ViewType::SRV);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetUAVHandle(GpuBufferViewType viewType, GpuBufferFormat format) const
{
	return GetOrCreateView(viewType, format, ViewType::UAV);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetOrCreateView(GpuBufferViewType viewType, GpuBufferFormat format, ViewType type, u32 offset) const
{
	const GpuBufferInformation& information = GetInformation();
	if(mBuffer == nullptr)
		return {};

	const bool isRaw = viewType == GpuBufferViewType::Raw;
	if(isRaw && information.Type != GpuBufferType::StructuredStorage)
		return {};

	// Only typed views of simple storage buffers have an element format, which defaults to the buffer's own
	if(viewType == GpuBufferViewType::Typed && information.Type == GpuBufferType::SimpleStorage)
	{
		if(format == BF_UNKNOWN)
			format = information.SimpleStorage.Format;
	}
	else
		format = BF_UNKNOWN;

	if(type == ViewType::CBV && information.Type != GpuBufferType::Uniform)
		return {};

	if(type == ViewType::CBV && (offset % kConstantBufferViewSizeAlignment != 0 || offset > mTotalSize || mSuballocationSize > mTotalSize - offset))
		return {};

	const bool supportsShaderViews = information.Type == GpuBufferType::SimpleStorage || information.Type == GpuBufferType::StructuredStorage || information.Type == GpuBufferType::Vertex;
	if(type != ViewType::CBV && !supportsShaderViews)
		return {};

	if(type == ViewType::UAV && !information.Flags.IsSet(GpuBufferFlag::GpuWrite))
		return {};

	// Raw views address 32-bit words and have no structure stride
	const bool isStructured = information.Type != GpuBufferType::SimpleStorage && !isRaw;
	DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN;
	if(isRaw)
		viewFormat = DXGI_FORMAT_R32_TYPELESS;
	else if(!isStructured)
	{
		viewFormat = GetBufferViewFormat(format);
		if(viewFormat == DXGI_FORMAT_UNKNOWN)
			return {};
	}

	Lock lock(mViewMutex);

	BufferViews* matchingViews = nullptr;
	for(BufferViews& cachedViews : mViews)
	{
		if(cachedViews.Type == viewType && cachedViews.Format == format && cachedViews.Offset == offset)
		{
			matchingViews = &cachedViews;
			break;
		}
	}

	if(matchingViews == nullptr)
	{
		mViews.Add(BufferViews(viewType, format, offset));
		matchingViews = &mViews.back();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* viewHandle = nullptr;
	switch(type)
	{
	case ViewType::CBV: viewHandle = &matchingViews->Cbv; break;
	case ViewType::SRV: viewHandle = &matchingViews->Srv; break;
	case ViewType::UAV: viewHandle = &matchingViews->Uav; break;
	}

	if(viewHandle->ptr != 0)
		return *viewHandle;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	*viewHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
	if(viewHandle->ptr == 0)
		return *viewHandle;

	if(type == ViewType::CBV)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress() + offset;
		cbvDesc.SizeInBytes = Math::CeilToMultiple(mSuballocationSize, kConstantBufferViewSizeAlignment);

		device.GetD3D12Device()->CreateConstantBufferView(&cbvDesc, *viewHandle);
		return *viewHandle;
	}

	u32 elementCount;
	u32 elementStride;
	if(information.Type == GpuBufferType::StructuredStorage)
	{
		elementCount = information.StructuredStorage.Count;
		elementStride = information.StructuredStorage.ElementSize;
	}
	else if(information.Type == GpuBufferType::Vertex)
	{
		elementCount = information.Vertex.Count;
		elementStride = information.Vertex.ElementSize;
	}
	else
	{
		// Typed views cover the whole buffer, including format overrides that reinterpret its contents.
		const u32 byteSize = information.SimpleStorage.Count * b3d::GpuBuffer::GetFormatSize(information.SimpleStorage.Format);
		elementCount = byteSize / b3d::GpuBuffer::GetFormatSize(format);
		elementStride = b3d::GpuBuffer::GetFormatSize(format);
	}

	if(isRaw)
	{
		elementCount = (elementCount * elementStride) / sizeof(u32);
		elementStride = sizeof(u32);
	}

	if(type == ViewType::UAV)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Format = viewFormat;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = elementCount;
		uavDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = isRaw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

		device.GetD3D12Device()->CreateUnorderedAccessView(mBuffer->GetD3D12Resource(), nullptr, &uavDesc, *viewHandle);
	}
	else
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = viewFormat;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = elementCount;
		srvDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
		srvDesc.Buffer.Flags = isRaw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

		device.GetD3D12Device()->CreateShaderResourceView(mBuffer->GetD3D12Resource(), &srvDesc, *viewHandle);
	}

	return *viewHandle;
}

void D3D12GpuBuffer::SetName(const StringView& name)
{
	GpuBuffer::SetName(name);

	if(mBuffer != nullptr)
		mBuffer->SetDebugName(mName);
}

GpuQueueMask D3D12GpuBuffer::GetUseMask(GpuAccessFlags accessFlags)
{
	if(mBuffer == nullptr)
		return GpuQueueMask::kNone;

	return mBuffer->GetUseInfo(accessFlags);
}

u32 D3D12GpuBuffer::GetBoundCount() const
{
	return mBuffer != nullptr ? mBuffer->GetBoundCount() : 0;
}

u32 D3D12GpuBuffer::GetUseCount() const
{
	return mBuffer != nullptr ? mBuffer->GetUseCount() : 0;
}

#if B3D_BUILD_TYPE_DEVELOPMENT
bool D3D12GpuBuffer::IsRangeBound(u32 offset, u32 size) const
{
	return mBuffer != nullptr && mBuffer->IsRangeBound(offset, size);
}

bool D3D12GpuBuffer::IsRangeInUse(u32 offset, u32 size) const
{
	return mBuffer != nullptr && mBuffer->IsRangeInUse(offset, size);
}
#endif

D3D12_GPU_VIRTUAL_ADDRESS D3D12GpuBuffer::GetGPUVirtualAddress() const
{
	return mBuffer != nullptr ? mBuffer->GetGPUVirtualAddress() : 0;
}
