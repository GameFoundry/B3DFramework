//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuShaderBindingTable.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12HeapBackend.h"
#include "B3DD3D12ResourceManager.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuShaderBindingTable::D3D12GpuShaderBindingTable(D3D12GpuDevice& device, const GpuShaderBindingTableCreateInformation& createInformation)
	: GpuShaderBindingTable(createInformation, device.GetCapabilities().RayTracing)
{
	const D3D12GpuRayTracingPipelineState& pipeline = static_cast<const D3D12GpuRayTracingPipelineState&>(*mInformation.Pipeline);

	// The CPU writes the table once, through a mapping, into the GPU's local memory if it can. Tables start at a multiple of 64.
	const D3D12_RESOURCE_DESC resourceDescription = D3D12GpuBuffer::GetResourceDescription(GetSize(), D3D12_RESOURCE_FLAG_NONE, device.IsTightAlignmentSupported());
	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device.GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescription);
	if(!B3D_ENSURE_LOG(allocationInfo.SizeInBytes != UINT64_MAX && allocationInfo.Alignment <= UINT32_MAX, "D3D12: Unsupported shader binding table of {0} bytes.", GetSize()))
		return;

	const u32 alignment = (u32)std::max(allocationInfo.Alignment, (u64)device.GetCapabilities().RayTracing.ShaderTableAlignment);
	const D3D12HeapClass heapClass = device.GetBufferHeapClass(GpuBufferType::StructuredStorage, GpuBufferMemory::Mapped, GpuBufferFlag::None);

	IGpuAllocator& allocator = device.GetPersistentAllocator((u32)heapClass);
	GpuAllocation allocation;
	if(!allocator.TryAllocate(allocationInfo.SizeInBytes, alignment, allocation))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to allocate {0} bytes for a shader binding table.", allocationInfo.SizeInBytes);
		return;
	}

	ComPtr<ID3D12Resource> resource;
	GpuAllocation placedAllocation;
	HRESULT result = device.CreateResource(resourceDescription, allocation, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, resource, placedAllocation);

	// The CPU only writes the table
	void* mappedData = nullptr;
	if(SUCCEEDED(result))
	{
		const D3D12_RANGE readRange = {};
		result = resource->Map(0, &readRange, &mappedData);
	}

	if(FAILED(result))
	{
		resource.Reset();
		allocator.Free(allocation);

		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create a shader binding table of {0} bytes (hr={1}).", GetSize(), (u32)result);
		return;
	}

	mBuffer = device.GetResourceManager().Create<D3D12Buffer>(allocation, std::move(resource), mappedData, "Shader binding table");

	// The writes are visible to the GPU once a command buffer that uses the table is submitted
	WriteRecords((u8*)mappedData, [&pipeline](u32 groupIndex) { return pipeline.GetShaderGroupHandle(groupIndex); });
}

D3D12GpuShaderBindingTable::~D3D12GpuShaderBindingTable()
{
	// Freed once no command buffer that dispatched with the table still runs
	if(mBuffer != nullptr)
		mBuffer->Destroy();
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE D3D12GpuShaderBindingTable::GetRayGenerationRecordRange(u32 recordIndex) const
{
	const GpuShaderBindingTableRegion& records = GetRayGenerationRegion();

	D3D12_GPU_VIRTUAL_ADDRESS_RANGE range;
	range.StartAddress = mBuffer->GetGPUVirtualAddress() + records.Offset + (u64)recordIndex * records.Stride;
	range.SizeInBytes = records.Stride;

	return range;
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE D3D12GpuShaderBindingTable::GetAddressRange(const GpuShaderBindingTableRegion& region) const
{
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE range = {};
	if(region.Size == 0)
		return range;

	range.StartAddress = mBuffer->GetGPUVirtualAddress() + region.Offset;
	range.SizeInBytes = region.Size;
	range.StrideInBytes = region.Stride;

	return range;
}
