//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Rounds @p value up to the next multiple of @p alignment. */
	u32 AlignUp(u32 value, u32 alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	/**
	 * Converts an engine buffer element format into the DXGI format used to type a typed (simple storage) buffer
	 * view. Returns DXGI_FORMAT_UNKNOWN for formats without a direct 1:1 mapping.
	 */
	DXGI_FORMAT GetBufferViewFormat(GpuBufferFormat format)
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

// Constructed unmanaged (protected D3D12Resource ctor): the engine owns the buffer's lifetime through its
// shared pointer, and the native resource release is deferred via D3D12GpuDevice::DeferNativeRelease.
D3D12GpuBuffer::D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device)
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device))
	, D3D12Resource()
{
}

D3D12GpuBuffer::~D3D12GpuBuffer()
{
	ReleaseBuffer();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::ReleaseBuffer()
{
	// Shader-binding descriptors reference the (about to be freed) native resource, so drop them first.
	ReleaseShaderDescriptors();

	// Persistently mapped memory must be released before the resource is destroyed.
	if(mMappedMemory != nullptr)
	{
		if(mBuffer)
			mBuffer->Unmap(0, nullptr);

		mMappedMemory = nullptr;
	}

	if (mBuffer == nullptr && mAllocation == nullptr)
		return;

	// The GPU may still be referencing the resource through in-flight command buffers, so the native
	// release is deferred until the device can guarantee it is no longer used.
	static_cast<D3D12GpuDevice&>(mDevice).DeferNativeRelease(mBuffer, mAllocation);

	mAllocation = nullptr;
	mBuffer.Reset();
}

void D3D12GpuBuffer::Initialize()
{
	RecreateInternalBuffer();
}

void D3D12GpuBuffer::RecreateInternalBuffer()
{
	// Release any previously created buffer (RecreateInternalBuffer may be called to grow/reset the buffer).
	ReleaseBuffer();

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mDevice);

	const GpuBufferInformation& info = GetInformation();

	// Resolve heap type and initial state from the buffer's type and flags.
	const D3D12_HEAP_TYPE heapType = D3D12Utility::GetHeapType(info.Type, info.Flags);

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

	SetCurrentState(initialState);

	// Not allowed to have size 0 buffer
	u64 bufferSize = Math::Max(mTotalSize, 64u);

	// Constant buffer views must be sized to a 256-byte multiple, and may not extend past the end of the
	// resource, so uniform buffers get their backing resource padded accordingly.
	if(info.Type == GpuBufferType::Uniform)
		bufferSize = AlignUp((u32)bufferSize, 256);

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
	resourceDesc.Flags = D3D12Utility::GetBufferResourceFlags(info.Flags);

	// Create the buffer resource using the D3D12MA allocator.
	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = heapType;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		initialState,
		nullptr, // No clear value for buffers
		&mAllocation,
		IID_PPV_ARGS(&mBuffer)
	);

	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create buffer resource (hr={0}, size={1}, type={2}, heapType={3}, resourceFlags={4})",
			(u32)hr, mTotalSize, (u32)info.Type, (u32)heapType, (u32)resourceDesc.Flags);
		return;
	}

	if(!mName.empty())
		SetName(mName);

	// Persistently map CPU-accessible buffers (upload/readback heaps) so the base GpuBuffer Read/Write paths
	// can access the memory directly via GetMappedMemory(). GPU-only (default heap) buffers stay unmapped.
	if(heapType == D3D12_HEAP_TYPE_UPLOAD || heapType == D3D12_HEAP_TYPE_READBACK)
	{
		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // Zero range: the CPU won't read on map (only relevant for readback flushing).
		hr = mBuffer->Map(0, &readRange, &mappedData);

		if(FAILED(hr))
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to persistently map buffer");
			ReleaseBuffer();
			return;
		}

		mMappedMemory = mappedData;
	}

	// Create vertex buffer view if this is a vertex buffer.
	if(info.Type == GpuBufferType::Vertex)
	{
		mVertexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = (UINT)mTotalSize;
		mVertexBufferView.StrideInBytes = (UINT)info.Vertex.ElementSize;
	}

	// Create index buffer view if this is an index buffer.
	if(info.Type == GpuBufferType::Index)
	{
		mIndexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = (UINT)mTotalSize;
		mIndexBufferView.Format = info.Index.Type == IT_32BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
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

	const GpuBufferInformation& info = GetInformation();

	// Uniform buffer -> CBV. Views cover the first suballocation only; the dynamic-offset path used for further
	// suballocations is applied at bind time via a root CBV/dynamic offset rather than baked into the view.
	// TODO(d3d12-port): Per-suballocation CBVs for SuballocationCount > 1 (bind currently only supports suballocation 0).
	if(info.Type == GpuBufferType::Uniform)
	{
		mCBVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(mCBVHandle.ptr != 0)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
			// Constant buffer views must be sized to a 256-byte multiple.
			cbvDesc.SizeInBytes = AlignUp(mSuballocationSize, 256);

			d3d12Device->CreateConstantBufferView(&cbvDesc, mCBVHandle);
		}
	}

	// Simple/structured storage -> read-only SRV, plus a UAV if the buffer allows GPU writes.
	if(info.Type == GpuBufferType::SimpleStorage || info.Type == GpuBufferType::StructuredStorage)
	{
		const bool isStructured = info.Type == GpuBufferType::StructuredStorage;

		u32 elementCount;
		u32 elementStride;
		DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN;

		if(isStructured)
		{
			elementCount = info.StructuredStorage.Count;
			elementStride = info.StructuredStorage.ElementSize;
		}
		else
		{
			elementCount = info.SimpleStorage.Count;
			viewFormat = GetBufferViewFormat(info.SimpleStorage.Format);
			elementStride = b3d::GpuBuffer::GetFormatSize(info.SimpleStorage.Format);
		}

		// SRV (read).
		mSRVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if(mSRVHandle.ptr != 0)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = elementCount;

			if(isStructured)
			{
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.Buffer.StructureByteStride = elementStride;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			}
			else
			{
				srvDesc.Format = viewFormat;
				srvDesc.Buffer.StructureByteStride = 0;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			}

			d3d12Device->CreateShaderResourceView(mBuffer.Get(), &srvDesc, mSRVHandle);
		}

		// UAV (write) — only when the buffer explicitly allows unordered access.
		if(info.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		{
			mUAVHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
			if(mUAVHandle.ptr != 0)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = elementCount;
				uavDesc.Buffer.CounterOffsetInBytes = 0;

				if(isStructured)
				{
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					uavDesc.Buffer.StructureByteStride = elementStride;
					uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				}
				else
				{
					uavDesc.Format = viewFormat;
					uavDesc.Buffer.StructureByteStride = 0;
					uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				}

				d3d12Device->CreateUnorderedAccessView(mBuffer.Get(), nullptr, &uavDesc, mUAVHandle);
			}
		}
	}
}

void D3D12GpuBuffer::ReleaseShaderDescriptors()
{
	D3D12DescriptorManager& descriptorManager = static_cast<D3D12GpuDevice&>(mDevice).GetDescriptorManager();

	if(mCBVHandle.ptr != 0)
	{
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, mCBVHandle);
		mCBVHandle.ptr = 0;
	}

	if(mSRVHandle.ptr != 0)
	{
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, mSRVHandle);
		mSRVHandle.ptr = 0;
	}

	if(mUAVHandle.ptr != 0)
	{
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, mUAVHandle);
		mUAVHandle.ptr = 0;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetCBVHandle(u32 suballocationIndex) const
{
	// TODO(d3d12-port): Only suballocation 0 currently has a dedicated CBV. See CreateShaderDescriptors.
	(void)suballocationIndex;
	return mCBVHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetSRVHandle() const
{
	return mSRVHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12GpuBuffer::GetUAVHandle() const
{
	return mUAVHandle;
}

void D3D12GpuBuffer::SetName(const StringView& name)
{
	GpuBuffer::SetName(name);

	if(mBuffer)
	{
		const WString wideName = ToWideString(mName);
		mBuffer->SetName(wideName.c_str());
	}
}

GpuQueueMask D3D12GpuBuffer::GetUseMask(GpuAccessFlags accessFlags)
{
	// TODO(d3d12-port): Per-queue usage tracking is not yet implemented for the D3D12 backend.
	return GpuQueueMask::kNone;
}

D3D12_GPU_VIRTUAL_ADDRESS D3D12GpuBuffer::GetGPUVirtualAddress() const
{
	if(mBuffer)
		return mBuffer->GetGPUVirtualAddress();

	return 0;
}

void D3D12GpuBuffer::Flush(u32 offset, u32 size)
{
	// D3D12 upload/readback heaps use coherent memory by default, so no explicit flush is required.
}

void D3D12GpuBuffer::Invalidate(u32 offset, u32 size)
{
	// D3D12 upload/readback heaps use coherent memory by default, so no explicit invalidate is required.
}
