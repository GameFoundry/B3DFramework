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

D3D12Buffer::D3D12Buffer(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, GpuResourceLocation allocation, D3D12_HEAP_TYPE heapType, const StringView& name)
	: TD3D12Resource<IGpuBufferResource>(owner, name), mResource(std::move(resource)), mAllocation(allocation), mHeapType(heapType)
{}

D3D12Buffer::~D3D12Buffer()
{
	// IGpuResource destruction is gated on all command-buffer uses completing. Destroy the placed resource first,/ then return its range to TLSF so a later resource may safely reuse it.
	mResource.Reset();
	GetDevice().FreeMemory(mAllocation);
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
	ReleaseShaderDescriptors();

	if(mBuffer == nullptr)
		return;

	if(mMappedMemory != nullptr)
	{
		mBuffer->GetD3D12Resource()->Unmap(0, nullptr);
		mMappedMemory = nullptr;
	}

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

	D3D12_RESOURCE_STATES initialState;
	switch(heapType)
	{
	case D3D12_HEAP_TYPE_UPLOAD:
		initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
		break;
	case D3D12_HEAP_TYPE_READBACK:
		initialState = D3D12_RESOURCE_STATE_COPY_DEST;
		break;
	default: // D3D12_HEAP_TYPE_DEFAULT
		initialState = D3D12_RESOURCE_STATE_COMMON;
		break;
	}

	// Not allowed to have size 0 buffer
	u32 bufferSize = Math::Max(mTotalSize, 64u);

	// Constant buffer views must be sized to a 256-byte multiple, and may not extend past the end of the
	// resource, so uniform buffers get their backing resource padded accordingly.
	if(information.Type == GpuBufferType::Uniform)
		bufferSize = Math::CeilToMultiple(bufferSize, kConstantBufferViewSizeAlignment);

	// Create resource description.
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = bufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12Utility::GetBufferResourceFlags(information.Flags);

	ComPtr<ID3D12Resource> resource;
	GpuResourceLocation allocation;
	HRESULT hr = device.CreateResource(resourceDesc, heapType, initialState, nullptr, resource, allocation);

	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create buffer resource (hr={0}, size={1}, type={2}, heapType={3}, resourceFlags={4})", (u32)hr, mTotalSize, (u32)information.Type, (u32)heapType, (u32)resourceDesc.Flags);
		return;
	}

	// Persistently map CPU-accessible buffers
	void* mappedData = nullptr;
	if(heapType == D3D12_HEAP_TYPE_UPLOAD || heapType == D3D12_HEAP_TYPE_READBACK)
	{
		D3D12_RANGE readRange = { 0, 0 }; // Zero range: the CPU won't read on map (only relevant for readback flushing).
		hr = resource->Map(0, &readRange, &mappedData);

		if(FAILED(hr))
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to persistently map buffer");

			resource.Reset();
			device.FreeMemory(allocation);
			return;
		}
	}

	mBuffer = device.GetResourceManager().Create<D3D12Buffer>(std::move(resource), allocation, heapType, mName);
	mMappedMemory = mappedData;

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

	// Create the shader-binding descriptors (CBV/SRV/UAV) applicable to this buffer's type/flags.
	CreateShaderDescriptors();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::CreateShaderDescriptors()
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	const GpuBufferInformation& information = GetInformation();

	// Uniform buffer -> CBV. Views cover the first suballocation only; the dynamic-offset path used for further
	// suballocations is applied at bind time via a root CBV/dynamic offset rather than baked into the view.
	// TODO(d3d12-port): Per-suballocation CBVs for SuballocationCount > 1 (bind currently only supports suballocation 0).
	if(information.Type == GpuBufferType::Uniform)
	{
		mCBVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(mCBVHandle.ptr != 0)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = Math::CeilToMultiple(mSuballocationSize, kConstantBufferViewSizeAlignment);

			d3d12Device->CreateConstantBufferView(&cbvDesc, mCBVHandle);
		}
	}

	// Simple/structured storage -> read-only SRV, plus a UAV if the buffer allows GPU writes. Vertex buffers
	// also get structured views: the engine binds them as storage buffers for GPU vertex pulling
	if(information.Type == GpuBufferType::SimpleStorage || information.Type == GpuBufferType::StructuredStorage || information.Type == GpuBufferType::Vertex)
	{
		// Only structured views carry an element stride; typed (simple storage) views are described by their format.
		const bool isStructured = information.Type != GpuBufferType::SimpleStorage;

		u32 elementCount;
		u32 elementStride = 0;
		DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN;

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
			elementCount = information.SimpleStorage.Count;
			viewFormat = GetBufferViewFormat(information.SimpleStorage.Format);
		}

		// SRV (read).
		mSRVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(mSRVHandle.ptr != 0)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = isStructured ? DXGI_FORMAT_UNKNOWN : viewFormat;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = elementCount;
			srvDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			d3d12Device->CreateShaderResourceView(mBuffer->GetD3D12Resource(), &srvDesc, mSRVHandle);
		}

		// UAV (write) — only when the buffer explicitly allows unordered access.
		if(information.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		{
			mUAVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
			if(mUAVHandle.ptr != 0)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				uavDesc.Format = isStructured ? DXGI_FORMAT_UNKNOWN : viewFormat;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = elementCount;
				uavDesc.Buffer.CounterOffsetInBytes = 0;
				uavDesc.Buffer.StructureByteStride = isStructured ? elementStride : 0;
				uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

				d3d12Device->CreateUnorderedAccessView(mBuffer->GetD3D12Resource(), nullptr, &uavDesc, mUAVHandle);
			}
		}
	}
}

void D3D12GpuBuffer::ReleaseShaderDescriptors()
{
	D3D12DescriptorManager& descriptorManager = static_cast<D3D12GpuDevice&>(mDevice).GetDescriptorManager();

	auto fnFreeDescriptor = [&descriptorManager](D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		if(handle.ptr == 0)
			return;

		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, handle);
		handle.ptr = 0;
	};

	fnFreeDescriptor(mCBVHandle);
	fnFreeDescriptor(mSRVHandle);
	fnFreeDescriptor(mUAVHandle);

	// TODO - Perhaps just generalize all view types into a single list of views, created on-demand?
	Lock lock(mViewMutex);
	for(FormatOverrideViews& views : mFormatViews)
	{
		fnFreeDescriptor(views.Srv);
		fnFreeDescriptor(views.Uav);
	}

	mFormatViews.clear();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetCBVHandle(u32 suballocationIndex) const
{
	// TODO(d3d12-port): Only suballocation 0 currently has a dedicated CBV. See CreateShaderDescriptors.
	(void)suballocationIndex;
	return mCBVHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetSRVHandle(GpuBufferFormat format) const
{
	const GpuBufferInformation& information = GetInformation();
	if(format == BF_UNKNOWN || information.Type != GpuBufferType::SimpleStorage || format == information.SimpleStorage.Format)
		return mSRVHandle;

	return GetFormatOverrideView(format, false);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetUAVHandle(GpuBufferFormat format) const
{
	const GpuBufferInformation& information = GetInformation();
	if(format == BF_UNKNOWN || information.Type != GpuBufferType::SimpleStorage || format == information.SimpleStorage.Format)
		return mUAVHandle;

	return GetFormatOverrideView(format, true);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetFormatOverrideView(GpuBufferFormat format, bool readWrite) const
{
	const GpuBufferInformation& information = GetInformation();

	const DXGI_FORMAT viewFormat = GetBufferViewFormat(format);
	if(viewFormat == DXGI_FORMAT_UNKNOWN || mBuffer == nullptr)
		return {};

	if(readWrite && !information.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		return {};

	Lock lock(mViewMutex);

	FormatOverrideViews* entry = nullptr;
	for(FormatOverrideViews& views : mFormatViews)
	{
		if(views.Format == format)
		{
			entry = &views;
			break;
		}
	}

	if(entry == nullptr)
	{
		mFormatViews.push_back(FormatOverrideViews());
		entry = &mFormatViews.back();
		entry->Format = format;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& handle = readWrite ? entry->Uav : entry->Srv;
	if(handle.ptr != 0)
		return handle;

	// The view covers the whole buffer, reinterpreting its contents through the override format
	const u32 byteSize = information.SimpleStorage.Count * b3d::GpuBuffer::GetFormatSize(information.SimpleStorage.Format);
	const u32 elementCount = byteSize / b3d::GpuBuffer::GetFormatSize(format);

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);

	handle = device.GetDescriptorManager().AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
	if(handle.ptr == 0)
		return handle;

	if(readWrite)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Format = viewFormat;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = elementCount;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		device.GetD3D12Device()->CreateUnorderedAccessView(mBuffer->GetD3D12Resource(), nullptr, &uavDesc, handle);
	}
	else
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = viewFormat;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = elementCount;
		srvDesc.Buffer.StructureByteStride = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		device.GetD3D12Device()->CreateShaderResourceView(mBuffer->GetD3D12Resource(), &srvDesc, handle);
	}

	return handle;
}

void D3D12GpuBuffer::SetName(const StringView& name)
{
	GpuBuffer::SetName(name);

	if(mBuffer != nullptr && mBuffer->GetD3D12Resource() != nullptr)
	{
		const WString wideName = ToWideString(mName);
		mBuffer->GetD3D12Resource()->SetName(wideName.c_str());
	}
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
