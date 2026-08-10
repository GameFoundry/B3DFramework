//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"

#include <numeric>

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Size a constant buffer view must be a multiple of. */
	constexpr u32 kConstantBufferViewSizeAlignment = 256;

	/** Least common multiple of every typed-buffer element size accepted by the backend. */
	constexpr u32 kTypedBufferViewAlignment = 48;

	/**
	 * Returns an alignment that keeps the pooled slice valid for every native view and copy footprint the logical
	 * buffer may use. The copy alignment is included for every buffer so a later buffer-to-image operation never
	 * depends on how the buffer was originally classified.
	 */
	u32 GetBufferSliceAlignment(const GpuBufferInformation& information)
	{
		u32 viewAlignment = 4;
		switch(information.Type)
		{
		case GpuBufferType::Uniform:
			viewAlignment = kConstantBufferViewSizeAlignment;
			break;
		case GpuBufferType::SimpleStorage:
			viewAlignment = kTypedBufferViewAlignment;
			break;
		case GpuBufferType::StructuredStorage:
			viewAlignment = information.StructuredStorage.ElementSize;
			break;
		case GpuBufferType::Vertex:
			viewAlignment = information.Vertex.ElementSize;
			break;
		case GpuBufferType::Index:
			viewAlignment = information.Index.Type == IT_32BIT ? 4 : 2;
			break;
		default:
			break;
		}

		return std::lcm(viewAlignment, (u32)D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
	}

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

D3D12Buffer::D3D12Buffer(D3D12ResourceManager* owner, GpuResourceLocation allocation, const StringView& name)
	: TD3D12Resource<IGpuBufferResource>(owner, name), mAllocation(allocation)
{ }

D3D12Buffer::~D3D12Buffer()
{
	GetDevice().FreeMemory(mAllocation);
}

ID3D12Resource* D3D12Buffer::GetD3D12Resource() const
{
	D3D12BufferPage* const page = GetPage();
	return page != nullptr ? page->GetD3D12Resource() : nullptr;
}

D3D12_GPU_VIRTUAL_ADDRESS D3D12Buffer::GetGPUVirtualAddress() const
{
	D3D12BufferPage* const page = GetPage();
	return page != nullptr ? page->GetGPUVirtualAddress() + mAllocation.Offset : 0;
}

D3D12BufferPage* D3D12Buffer::GetPage() const
{
	return mAllocation.Heap != nullptr ? static_cast<D3D12BufferPage*>(mAllocation.Heap) : nullptr;
}

D3D12_HEAP_TYPE D3D12Buffer::GetHeapType() const
{
	D3D12BufferPage* const page = GetPage();
	return page != nullptr ? page->GetHeapType() : D3D12_HEAP_TYPE_DEFAULT;
}

D3D12GpuBuffer::D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device)
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device))
{
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

	mBuffer->Destroy();
	mBuffer = nullptr;
}

void D3D12GpuBuffer::Initialize()
{
	RecreateInternalBuffer();
}

void D3D12GpuBuffer::RecreateInternalBuffer()
{
	ReleaseBuffer();

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);

	const GpuBufferInformation& information = GetInformation();
	const D3D12_HEAP_TYPE heapType = D3D12Utility::GetHeapType(information.Type, information.Flags);

	// D3D12 cannot place a UAV on an upload heap, so this combination is rejected with E_INVALIDARG below and leaves the buffer without a resource.
	// TODO - Should probably just fall back to a different heap instead
	B3D_ENSURE_LOG(!(heapType == D3D12_HEAP_TYPE_UPLOAD && information.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU)), "D3D12: Buffer '{0}' requests AllowUnorderedAccessOnTheGPU with CPU-visible storage (StoreOnCPUWithGPUAccess); unordered access requires StoreOnGPU.", mName);

	// Not allowed to have size 0 buffer
	u32 bufferSize = Math::Max(mTotalSize, 64u);

	// Constant buffer views must be sized to a 256-byte multiple, and may not extend past the end of the
	// resource, so uniform buffers get their backing resource padded accordingly.
	if(information.Type == GpuBufferType::Uniform)
		bufferSize = Math::CeilToMultiple(bufferSize, kConstantBufferViewSizeAlignment);

	GpuResourceLocation allocation;
	const D3D12_RESOURCE_FLAGS resourceFlags = D3D12Utility::GetBufferResourceFlags(information.Flags);
	const u32 alignment = GetBufferSliceAlignment(information);
	if(!device.GetBufferPool().TryAllocate(bufferSize, alignment, heapType, resourceFlags, allocation))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to allocate a pooled buffer slice (size={0}, alignment={1}, type={2}, heapType={3}, resourceFlags={4}).",
			bufferSize, alignment, (u32)information.Type, (u32)heapType, (u32)resourceFlags);
		return;
	}

	mBuffer = device.GetResourceManager().Create<D3D12Buffer>(allocation, mName);
	D3D12BufferPage* const page = mBuffer->GetPage();
	mMappedMemory = page->GetMappedData() != nullptr ? (u8*)page->GetMappedData() + allocation.Offset : nullptr;

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

	// Create the default shader-binding descriptors. Typed format overrides remain lazy.
	if(information.Type == GpuBufferType::Uniform)
	{
		// TODO(d3d12-port): Per-suballocation CBVs for SuballocationCount > 1 (binding currently supports only index 0).
		GetOrCreateView(BF_UNKNOWN, ViewType::CBV);
	}

	if(information.Type == GpuBufferType::SimpleStorage || information.Type == GpuBufferType::StructuredStorage || information.Type == GpuBufferType::Vertex)
	{
		const GpuBufferFormat format = information.Type == GpuBufferType::SimpleStorage ? information.SimpleStorage.Format : BF_UNKNOWN;
		GetOrCreateView(format, ViewType::SRV);
		if(information.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
			GetOrCreateView(format, ViewType::UAV);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_VertexBuffer);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetCBVHandle(u32 suballocationIndex) const
{
	// TODO(d3d12-port): Create a dedicated CBV for each suballocation instead of returning the default view.
	(void)suballocationIndex;
	return GetOrCreateView(BF_UNKNOWN, ViewType::CBV);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetSRVHandle(GpuBufferFormat format) const
{
	const GpuBufferInformation& information = GetInformation();
	if(information.Type != GpuBufferType::SimpleStorage)
		format = BF_UNKNOWN;
	else if(format == BF_UNKNOWN)
		format = information.SimpleStorage.Format;

	return GetOrCreateView(format, ViewType::SRV);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetUAVHandle(GpuBufferFormat format) const
{
	const GpuBufferInformation& information = GetInformation();
	if(information.Type != GpuBufferType::SimpleStorage)
		format = BF_UNKNOWN;
	else if(format == BF_UNKNOWN)
		format = information.SimpleStorage.Format;

	return GetOrCreateView(format, ViewType::UAV);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetOrCreateView(GpuBufferFormat format, ViewType type) const
{
	const GpuBufferInformation& information = GetInformation();
	if(mBuffer == nullptr)
		return {};

	if(type == ViewType::CBV && information.Type != GpuBufferType::Uniform)
		return {};

	const bool supportsShaderViews = information.Type == GpuBufferType::SimpleStorage || information.Type == GpuBufferType::StructuredStorage || information.Type == GpuBufferType::Vertex;
	if(type != ViewType::CBV && !supportsShaderViews)
		return {};

	if(type == ViewType::UAV && !information.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		return {};

	const bool isStructured = information.Type != GpuBufferType::SimpleStorage;
	const DXGI_FORMAT viewFormat = isStructured ? DXGI_FORMAT_UNKNOWN : GetBufferViewFormat(format);
	if(information.Type == GpuBufferType::SimpleStorage && viewFormat == DXGI_FORMAT_UNKNOWN)
		return {};

	Lock lock(mViewMutex);

	BufferViews* matchingViews = nullptr;
	for(BufferViews& cachedViews : mViews)
	{
		if(cachedViews.Format == format)
		{
			matchingViews = &cachedViews;
			break;
		}
	}

	if(matchingViews == nullptr)
	{
		mViews.Add(BufferViews(format));
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
		cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
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

	if(type == ViewType::UAV)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Format = viewFormat;
		uavDesc.Buffer.FirstElement = mBuffer->GetOffset() / elementStride;
		uavDesc.Buffer.NumElements = elementCount;
		uavDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		device.GetD3D12Device()->CreateUnorderedAccessView(mBuffer->GetD3D12Resource(), nullptr, &uavDesc, *viewHandle);
	}
	else
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = viewFormat;
		srvDesc.Buffer.FirstElement = mBuffer->GetOffset() / elementStride;
		srvDesc.Buffer.NumElements = elementCount;
		srvDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

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
