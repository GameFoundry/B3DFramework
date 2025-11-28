//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12SamplerState.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuParameters::D3D12GpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout, GpuDevice& device)
	: GpuParameters(parameterLayout, device)
	, mDevice(static_cast<D3D12GpuDevice&>(device))
{
}

D3D12GpuParameters::~D3D12GpuParameters()
{
	// Descriptors will be automatically freed by descriptor manager
	// GPU-visible descriptor ranges are managed by the descriptor manager
}

void D3D12GpuParameters::Initialize()
{
	GpuParameters::Initialize();

	// Get the pipeline parameter layout
	D3D12GpuPipelineParameterLayout* d3d12Layout = static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
	if (!d3d12Layout)
		return;

	const GpuProgramParameterLayout& layout = d3d12Layout->GetLayout();

	// Initialize descriptor tables for each set
	for (const auto& setEntry : layout.Sets)
	{
		u32 setIndex = setEntry.first;
		const GpuProgramParameterSet& paramSet = setEntry.second;

		// Count descriptors needed for this set (separate for CBV/SRV/UAV and Samplers)
		u32 resourceDescriptorCount = 0;
		u32 samplerDescriptorCount = 0;

		for (const auto& slotEntry : paramSet.Uniforms)
		{
			const GpuProgramUniformInformation& uniformInfo = slotEntry.second;

			if (uniformInfo.Type == GPT_SAMPLER)
			{
				samplerDescriptorCount += uniformInfo.ArraySize;
			}
			else
			{
				// CBV, SRV, UAV
				resourceDescriptorCount += uniformInfo.ArraySize;
			}
		}

		// Create resource descriptor table if needed
		if (resourceDescriptorCount > 0)
		{
			DescriptorTable& table = mDescriptorTables[setIndex];
			table.SetIndex = setIndex;
			table.RootParameterIndex = setIndex; // TODO: Map correctly from layout
			table.DescriptorCount = resourceDescriptorCount;
			table.Descriptors.resize(resourceDescriptorCount);
			table.IsDirty = true;
		}

		// Create sampler descriptor table if needed
		if (samplerDescriptorCount > 0)
		{
			DescriptorTable& table = mSamplerTables[setIndex];
			table.SetIndex = setIndex;
			table.RootParameterIndex = setIndex + 100; // TODO: Map correctly from layout
			table.DescriptorCount = samplerDescriptorCount;
			table.Descriptors.resize(samplerDescriptorCount);
			table.IsDirty = true;
		}
	}
}

void D3D12GpuParameters::SetDescriptor(u32 set, u32 slot, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	// Find the descriptor table for this set
	auto it = mDescriptorTables.find(set);
	if (it != mDescriptorTables.end())
	{
		DescriptorTable& table = it->second;
		if (slot < table.Descriptors.size())
		{
			table.Descriptors[slot].CPUHandle = handle;
			table.Descriptors[slot].IsDirty = true;
			table.IsDirty = true;
		}
	}
}

void D3D12GpuParameters::WriteParameters()
{
	// This is called when parameters are modified
	// We mark descriptor tables as dirty here
	// Actual descriptor copying happens in BindDescriptors()

	// Mark all tables as dirty
	for (auto& entry : mDescriptorTables)
	{
		entry.second.IsDirty = true;
		for (auto& desc : entry.second.Descriptors)
		{
			desc.IsDirty = true;
		}
	}

	for (auto& entry : mSamplerTables)
	{
		entry.second.IsDirty = true;
		for (auto& desc : entry.second.Descriptors)
		{
			desc.IsDirty = true;
		}
	}
}

void D3D12GpuParameters::AllocateGPUDescriptorRanges(D3D12GpuDevice& device)
{
	if (mDescriptorsAllocated)
		return;

	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	// Allocate GPU-visible descriptor ranges for resource tables
	for (auto& entry : mDescriptorTables)
	{
		DescriptorTable& table = entry.second;
		if (table.DescriptorCount > 0)
		{
			descriptorManager.AllocateGPUDescriptorRange(
				D3D12DescriptorHeapType::CBV_SRV_UAV,
				table.DescriptorCount,
				table.GPUVisibleCPUStart,
				table.GPUVisibleGPUStart
			);
		}
	}

	// Allocate GPU-visible descriptor ranges for sampler tables
	for (auto& entry : mSamplerTables)
	{
		DescriptorTable& table = entry.second;
		if (table.DescriptorCount > 0)
		{
			descriptorManager.AllocateGPUDescriptorRange(
				D3D12DescriptorHeapType::Sampler,
				table.DescriptorCount,
				table.GPUVisibleCPUStart,
				table.GPUVisibleGPUStart
			);
		}
	}

	mDescriptorsAllocated = true;
}

void D3D12GpuParameters::UpdateGPUDescriptors(D3D12GpuDevice& device)
{
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	// Update resource descriptor tables
	u32 resourceDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::CBV_SRV_UAV);
	for (auto& entry : mDescriptorTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty)
			continue;

		// Copy dirty descriptors to GPU-visible heap
		for (u32 i = 0; i < table.Descriptors.size(); i++)
		{
			BoundDescriptor& desc = table.Descriptors[i];
			if (desc.IsDirty && desc.CPUHandle.ptr != 0)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = table.GPUVisibleCPUStart;
				dstHandle.ptr += i * resourceDescriptorSize;

				d3d12Device->CopyDescriptorsSimple(1, dstHandle, desc.CPUHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				desc.IsDirty = false;
			}
		}

		table.IsDirty = false;
	}

	// Update sampler descriptor tables
	u32 samplerDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::Sampler);
	for (auto& entry : mSamplerTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty)
			continue;

		// Copy dirty descriptors to GPU-visible heap
		for (u32 i = 0; i < table.Descriptors.size(); i++)
		{
			BoundDescriptor& desc = table.Descriptors[i];
			if (desc.IsDirty && desc.CPUHandle.ptr != 0)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = table.GPUVisibleCPUStart;
				dstHandle.ptr += i * samplerDescriptorSize;

				d3d12Device->CopyDescriptorsSimple(1, dstHandle, desc.CPUHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				desc.IsDirty = false;
			}
		}

		table.IsDirty = false;
	}
}

void D3D12GpuParameters::BindDescriptors(D3D12GpuDevice& device, ID3D12GraphicsCommandList* commandList, bool isGraphics)
{
	// Allocate GPU descriptor ranges on first use
	if (!mDescriptorsAllocated)
	{
		AllocateGPUDescriptorRanges(device);
	}

	// Update GPU-visible descriptors with any dirty CPU descriptors
	UpdateGPUDescriptors(device);

	// Bind descriptor tables to the pipeline
	for (const auto& entry : mDescriptorTables)
	{
		const DescriptorTable& table = entry.second;
		if (table.DescriptorCount > 0 && table.GPUVisibleGPUStart.ptr != 0)
		{
			if (isGraphics)
			{
				commandList->SetGraphicsRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
			}
			else
			{
				commandList->SetComputeRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
			}
		}
	}

	// Bind sampler descriptor tables
	for (const auto& entry : mSamplerTables)
	{
		const DescriptorTable& table = entry.second;
		if (table.DescriptorCount > 0 && table.GPUVisibleGPUStart.ptr != 0)
		{
			if (isGraphics)
			{
				commandList->SetGraphicsRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
			}
			else
			{
				commandList->SetComputeRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
			}
		}
	}
}
