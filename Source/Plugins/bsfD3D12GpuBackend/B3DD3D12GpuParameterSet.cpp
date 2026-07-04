//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12ResourceTracker.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12SamplerState.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Returns true when the storage-buffer object type is writeable from the shader (UAV-class binding). */
	bool IsReadWriteStorageBuffer(GpuParameterObjectType type)
	{
		switch(type)
		{
		case GPOT_RWTYPED_BUFFER:
		case GPOT_RWBYTE_BUFFER:
		case GPOT_RWSTRUCTURED_BUFFER:
		case GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER:
		case GPOT_RWAPPEND_BUFFER:
		case GPOT_RWCONSUME_BUFFER:
			return true;
		default:
			return false;
		}
	}
}

D3D12GpuParameters::D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex)
	: GpuParameterSet(parameterSetLayout, setIndex)
	, mDevice(device)
{
}

D3D12GpuParameters::~D3D12GpuParameters()
{
	// GPU-visible descriptor ranges are owned by the descriptor manager's linear allocator and reclaimed in bulk on
	// pool reset; CPU descriptors referenced by the tables belong to the individual resources, not this set.
}

void D3D12GpuParameters::Initialize()
{
	const TShared<GpuPipelineParameterSetLayout> setLayout = GetLayout();
	if (!setLayout)
		return;

	// The root signature (B3DD3D12GpuPipelineParameterLayout::CreateRootSignature) emits one root parameter per used
	// binding, iterating sets in order, then parameter types in enum order, then bindings within each type. We mirror
	// that iteration to compute each binding's root parameter index.
	//
	// TODO(d3d12-port): The root parameter numbering is flat across ALL sets, but a GpuParameterSet only sees its own
	//					 set layout and cannot count bindings in preceding sets. We therefore assume this set's bindings
	//					 start at root parameter 0, which is only correct for single-set pipelines (mSet == 0). Multi-set
	//					 pipelines need the per-set root-parameter base offset threaded through from the pipeline layout.
	u32 rootParameterIndex = 0;

	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = setLayout->GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* uniformInfo = setLayout->TryGetUniformInformation(type, sequentialIndex);
			if (uniformInfo == nullptr)
				continue;

			const bool isSampler = type == GpuParameterType::Sampler;
			UnorderedMap<u32, DescriptorTable>& tables = isSampler ? mSamplerTables : mDescriptorTables;

			DescriptorTable& table = tables[uniformInfo->Slot];
			table.Slot = uniformInfo->Slot;
			table.RootParameterIndex = rootParameterIndex;
			table.DescriptorCount = uniformInfo->ArraySize;
			table.Descriptors.resize(uniformInfo->ArraySize);
			table.IsDirty = true;

			rootParameterIndex++;
		}
	}
}

bool D3D12GpuParameters::SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
{
	if (!GpuParameterSet::SetUniformBuffer(slot, uniformBuffer, arrayIndex, offset))
		return false;

	// TODO(d3d12-port): Dynamic offset (suballocation) is stored by the base class but not yet applied to the CBV; the
	//					 CBV created on the buffer always covers suballocation 0.
	if (uniformBuffer == nullptr)
	{
		// TODO(d3d12-port): builtin dummy resources. Without a dummy CBV the slot is left with a null descriptor and the
		//					 copy into the GPU-visible heap is skipped (see UpdateGPUDescriptors).
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Null uniform buffer bound to set {0} slot {1}; no dummy resource available.", GetSet(), slot);
		SetDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	auto* d3d12Buffer = static_cast<D3D12GpuBuffer*>(uniformBuffer.get());
	SetDescriptor(slot, arrayIndex, d3d12Buffer->GetCBVHandle());
	return true;
}

bool D3D12GpuParameters::SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetSampledTexture(slot, texture, surface, arrayIndex))
		return false;

	if (texture == nullptr)
	{
		// TODO(d3d12-port): builtin dummy resources.
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Null sampled texture bound to set {0} slot {1}; no dummy resource available.", GetSet(), slot);
		SetDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	auto* d3d12Texture = static_cast<D3D12Texture*>(texture.get());
	SetDescriptor(slot, arrayIndex, d3d12Texture->GetSRVHandle(surface));
	return true;
}

bool D3D12GpuParameters::SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetStorageTexture(slot, texture, surface, arrayIndex))
		return false;

	if (texture == nullptr)
	{
		// TODO(d3d12-port): builtin dummy resources.
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Null storage texture bound to set {0} slot {1}; no dummy resource available.", GetSet(), slot);
		SetDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	auto* d3d12Texture = static_cast<D3D12Texture*>(texture.get());
	SetDescriptor(slot, arrayIndex, d3d12Texture->GetUAVHandle(surface));
	return true;
}

bool D3D12GpuParameters::SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex, GpuBufferViewInformation view)
{
	if (!GpuParameterSet::SetStorageBuffer(slot, buffer, arrayIndex, view))
		return false;

	if (buffer == nullptr)
	{
		// TODO(d3d12-port): builtin dummy resources.
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Null storage buffer bound to set {0} slot {1}; no dummy resource available.", GetSet(), slot);
		SetDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	// The root signature range type (SRV vs UAV) is derived from the shader-declared object type. Read-only storage
	// buffers (Byte/StructuredBuffer) use the buffer's SRV; read-write ones (RWByte/RWStructuredBuffer) use its UAV. We
	// select based on which views the buffer actually created: a UAV exists only for AllowUnorderedAccessOnTheGPU
	// buffers, so prefer it when present, otherwise fall back to the SRV.
	//
	// TODO(d3d12-port): Thread the shader object type through so a read-write shader binding never silently receives an
	//					 SRV (and vice versa). Also apply view.Offset/Range/Format instead of the default full-buffer view.
	auto* d3d12Buffer = static_cast<D3D12GpuBuffer*>(buffer.get());

	D3D12_CPU_DESCRIPTOR_HANDLE handle = d3d12Buffer->GetUAVHandle();
	if (handle.ptr == 0)
		handle = d3d12Buffer->GetSRVHandle();

	SetDescriptor(slot, arrayIndex, handle);
	return true;
}

bool D3D12GpuParameters::SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex)
{
	if (!GpuParameterSet::SetSamplerState(slot, sampler, arrayIndex))
		return false;

	if (sampler == nullptr)
	{
		// TODO(d3d12-port): builtin dummy resources.
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Null sampler bound to set {0} slot {1}; no dummy resource available.", GetSet(), slot);
		SetSamplerDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	auto* d3d12Sampler = static_cast<D3D12SamplerState*>(sampler.get());
	SetSamplerDescriptor(slot, arrayIndex, d3d12Sampler->GetDescriptorHandle());
	return true;
}

void D3D12GpuParameters::TrackBoundResources(D3D12ResourceTracker& resourceTracker, D3D12BarrierHelper& barrierHelper)
{
	const TShared<GpuPipelineParameterSetLayout> setLayout = GetLayout();
	if (setLayout == nullptr)
		return;

	// No per-stage information is threaded through yet, so shader accesses are tracked conservatively as visible to
	// all stages. TODO(d3d12-port): derive the stage flags from the pipeline's shader stages.
	const GpuResourceUseFlags stageUseFlags = GpuResourceUseFlag::AnyStage;

	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = setLayout->GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* uniformInfo = setLayout->TryGetUniformInformation(type, sequentialIndex);
			if (uniformInfo == nullptr)
				continue;

			for (u32 arrayIndex = 0; arrayIndex < uniformInfo->ArraySize; arrayIndex++)
			{
				const u32 dataIndex = setLayout->GetSequentialResourceIndex(uniformInfo->Slot, arrayIndex);
				if (dataIndex == ~0u)
					continue;

				switch (type)
				{
				case GpuParameterType::UniformBuffer:
				{
					if (GpuBuffer* const buffer = mUniformBufferData[dataIndex].Buffer.get())
					{
						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(),
							stageUseFlags | GpuResourceUseFlag::UniformBuffer, GpuAccessFlag::Read, barrierHelper,
							mUniformBufferData[dataIndex].Offset);
					}
					break;
				}
				case GpuParameterType::SampledTexture:
				{
					if (Texture* const texture = mSampledTextureData[dataIndex].Texture.get())
					{
						D3D12Image* const image = static_cast<D3D12Texture*>(texture)->GetD3D12Image();
						if (image != nullptr)
						{
							const GpuTextureSubresourceRange range = image->GetRange(mSampledTextureData[dataIndex].Surface);
							resourceTracker.TrackImageUsage(image, range, GpuImageLayout::ShaderReadOnly,
								GpuImageLayout::ShaderReadOnly, stageUseFlags | GpuResourceUseFlag::ShaderAccess,
								GpuAccessFlag::Read, barrierHelper);
						}
					}
					break;
				}
				case GpuParameterType::StorageTexture:
				{
					if (Texture* const texture = mStorageTextureData[dataIndex].Texture.get())
					{
						D3D12Image* const image = static_cast<D3D12Texture*>(texture)->GetD3D12Image();
						if (image != nullptr)
						{
							// Conservative read-write: UAV image bindings do not declare their access.
							const GpuTextureSubresourceRange range = image->GetRange(mStorageTextureData[dataIndex].Surface);
							resourceTracker.TrackImageUsage(image, range, GpuImageLayout::General,
								GpuImageLayout::General, stageUseFlags | GpuResourceUseFlag::ShaderAccess,
								GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper);
						}
					}
					break;
				}
				case GpuParameterType::StorageBuffer:
				{
					if (GpuBuffer* const buffer = mStorageBufferData[dataIndex].Buffer.get())
					{
						GpuAccessFlags accessFlags = GpuAccessFlag::Read;
						if (IsReadWriteStorageBuffer(uniformInfo->ObjectType))
							accessFlags |= GpuAccessFlag::Write;

						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(),
							stageUseFlags | GpuResourceUseFlag::ShaderAccess, accessFlags, barrierHelper);
					}
					break;
				}
				case GpuParameterType::Sampler:
					// Sampler states carry no GPU resource to track.
					break;
				default:
					break;
				}
			}
		}
	}
}

void D3D12GpuParameters::SetDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	auto it = mDescriptorTables.find(slot);
	if (it == mDescriptorTables.end())
		return;

	DescriptorTable& table = it->second;
	if (arrayIndex >= table.Descriptors.size())
		return;

	table.Descriptors[arrayIndex].CPUHandle = handle;
	table.Descriptors[arrayIndex].IsDirty = true;
	table.IsDirty = true;
}

void D3D12GpuParameters::SetSamplerDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	auto it = mSamplerTables.find(slot);
	if (it == mSamplerTables.end())
		return;

	DescriptorTable& table = it->second;
	if (arrayIndex >= table.Descriptors.size())
		return;

	table.Descriptors[arrayIndex].CPUHandle = handle;
	table.Descriptors[arrayIndex].IsDirty = true;
	table.IsDirty = true;
}

void D3D12GpuParameters::AllocateGPUDescriptorRanges(D3D12GpuDevice& device)
{
	if (mDescriptorsAllocated)
		return;

	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

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

	const u32 resourceDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::CBV_SRV_UAV);
	for (auto& entry : mDescriptorTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty)
			continue;

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

	const u32 samplerDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::Sampler);
	for (auto& entry : mSamplerTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty)
			continue;

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
	if (!mDescriptorsAllocated)
		AllocateGPUDescriptorRanges(device);

	UpdateGPUDescriptors(device);

	for (const auto& entry : mDescriptorTables)
	{
		const DescriptorTable& table = entry.second;
		if (table.DescriptorCount == 0 || table.GPUVisibleGPUStart.ptr == 0)
			continue;

		if (isGraphics)
			commandList->SetGraphicsRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
		else
			commandList->SetComputeRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
	}

	for (const auto& entry : mSamplerTables)
	{
		const DescriptorTable& table = entry.second;
		if (table.DescriptorCount == 0 || table.GPUVisibleGPUStart.ptr == 0)
			continue;

		if (isGraphics)
			commandList->SetGraphicsRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
		else
			commandList->SetComputeRootDescriptorTable(table.RootParameterIndex, table.GPUVisibleGPUStart);
	}
}
