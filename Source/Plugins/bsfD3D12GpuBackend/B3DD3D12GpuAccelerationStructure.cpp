//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuAccelerationStructure.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "Managers/B3DD3D12DescriptorManager.h"

using namespace b3d;
using namespace b3d::render;

D3D12AccelerationStructure::D3D12AccelerationStructure(D3D12ResourceManager* owner, D3D12Buffer& buffer, const StringView& name)
	: D3D12Resource(owner, name), mBuffer(&buffer), mGpuVirtualAddress(buffer.GetGPUVirtualAddress())
{
	D3D12GpuDevice& device = GetDevice();

	// The view locates the structure by its address, so it names no resource
	mSrv = device.GetDescriptorManager().AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
	if(mSrv.ptr != 0)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = mGpuVirtualAddress;

		device.GetD3D12Device()->CreateShaderResourceView(nullptr, &srvDesc, mSrv);
	}

	if(!name.empty())
	{
		buffer.SetDebugName(name);

		const WString wideName = ToWideString(String(name));
		buffer.GetD3D12Resource()->SetName(wideName.c_str());
	}
}

D3D12AccelerationStructure::~D3D12AccelerationStructure()
{
	if(mSrv.ptr != 0)
		GetDevice().GetDescriptorManager().FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, mSrv);

	// Nothing else tracks or pins the buffer, so it is freed now
	mBuffer->Destroy();
}

void D3D12AccelerationStructure::GetBuildInputs(const GpuAccelerationStructureInputs& inputs, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& outInputs,
	FrameVector<D3D12_RAYTRACING_GEOMETRY_DESC>& outGeometries)
{
	const auto fnGetAddress = [](const TShared<GpuBuffer>& buffer, u32 offset)
	{
		const D3D12_GPU_VIRTUAL_ADDRESS address = buffer != nullptr ? static_cast<const D3D12GpuBuffer&>(*buffer).GetGPUVirtualAddress() : 0;
		return address != 0 ? address + offset : 0;
	};

	outInputs = {};
	outInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::PreferFastTrace))
		outInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::PreferFastBuild))
		outInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::MinimizeMemory))
		outInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::AllowRefit))
		outInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	if(inputs.Type == GpuAccelerationStructureType::TopLevel)
	{
		outInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		outInputs.NumDescs = inputs.InstanceCount;
		outInputs.InstanceDescs = inputs.InstanceCount > 0 ? fnGetAddress(inputs.InstanceBuffer, inputs.InstanceOffset) : 0;
		return;
	}

	outGeometries.resize(inputs.Geometries.size());
	for(u32 geometryIndex = 0; geometryIndex < (u32)inputs.Geometries.size(); geometryIndex++)
	{
		const GpuRayTracingGeometry& geometry = inputs.Geometries[geometryIndex];

		D3D12_RAYTRACING_GEOMETRY_DESC& nativeGeometry = outGeometries[geometryIndex];
		nativeGeometry = {};
		nativeGeometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

		if(geometry.Flags.IsSet(GpuRayTracingGeometryFlag::Opaque))
			nativeGeometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		if(geometry.Flags.IsSet(GpuRayTracingGeometryFlag::NoDuplicateAnyHitInvocation))
			nativeGeometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

		// Offsets are part of the addresses
		D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& triangles = nativeGeometry.Triangles;
		triangles.Transform3x4 = 0;
		triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		triangles.VertexCount = geometry.VertexCount;
		triangles.VertexBuffer.StartAddress = fnGetAddress(geometry.VertexBuffer, geometry.VertexOffset);
		triangles.VertexBuffer.StrideInBytes = geometry.VertexStride;

		if(geometry.IndexBuffer != nullptr)
		{
			triangles.IndexFormat = geometry.IndexType == IT_32BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			triangles.IndexCount = geometry.IndexCount;
			triangles.IndexBuffer = fnGetAddress(geometry.IndexBuffer, geometry.IndexOffset);
		}
		else
			triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
	}

	outInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	outInputs.NumDescs = (u32)outGeometries.size();
	outInputs.pGeometryDescs = outGeometries.data();
}

D3D12GpuAccelerationStructure::D3D12GpuAccelerationStructure(D3D12GpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation,
	D3D12AccelerationStructure& accelerationStructure)
	: GpuAccelerationStructure(device, createInformation, accelerationStructure), mAccelerationStructure(&accelerationStructure)
{ }
