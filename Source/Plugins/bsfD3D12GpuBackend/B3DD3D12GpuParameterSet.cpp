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

	// Create the per-binding resource storage from this set's own layout, keyed by slot. Root parameter indices are
	// NOT recorded here - they depend on the ACTIVE pipeline's layout and are derived at bind time (BindDescriptors
	// walks the pipeline's set layout and looks these containers up by slot).
	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = setLayout->GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* uniformInfo = setLayout->TryGetUniformInformation(type, sequentialIndex);
			if (uniformInfo == nullptr)
				continue;

			// Single-element uniform buffers occupy a root CBV descriptor instead of a descriptor table (this must
			// mirror the root signature's promotion choice, see D3D12GpuPipelineParameterLayout::CreateRootSignature).
			if (type == GpuParameterType::UniformBuffer && uniformInfo->ArraySize == 1)
			{
				RootConstantBuffer& rootBuffer = mRootConstantBuffers[uniformInfo->Slot];
				rootBuffer.DataIndex = uniformInfo->SequentialResourceIndex;
				rootBuffer.DynamicOffsetIndex = uniformInfo->DynamicOffsetIndex;
				continue;
			}

			const bool isSampler = type == GpuParameterType::Sampler;
			UnorderedMap<u32, DescriptorTable>& tables = isSampler ? mSamplerTables : mDescriptorTables;

			DescriptorTable& table = tables[uniformInfo->Slot];
			table.Slot = uniformInfo->Slot;
			table.DescriptorCount = uniformInfo->ArraySize;
			table.Descriptors.resize(uniformInfo->ArraySize);
			table.IsDirty = true;
		}
	}
}

bool D3D12GpuParameters::SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
{
	if (!GpuParameterSet::SetUniformBuffer(slot, uniformBuffer, arrayIndex, offset))
		return false;

	// Single-element uniform buffers are bound as root CBVs: the buffer and its suballocation offset are read from
	// the base class's bound-buffer data at bind time, no descriptor is involved.
	if (mRootConstantBuffers.find(slot) != mRootConstantBuffers.end())
		return true;

	// TODO(d3d12-port): Suballocation offset is stored by the base class but not applied to arrayed CBVs; the CBV
	//					 created on the buffer always covers suballocation 0.
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

	// The root signature range type (SRV vs UAV) is derived from the shader-declared object type, so the descriptor
	// must match it: read-only storage buffers (Byte/StructuredBuffer) take the buffer's SRV, read-write ones
	// (RWByte/RWStructuredBuffer/...) its UAV. Selecting by the buffer's own capabilities instead would hand a UAV
	// descriptor to an SRV register whenever a GPU-writable buffer is bound read-only (generate-then-consume passes).
	//
	// The view format override matters for correctness, not just interpretation: a typed view whose component count
	// doesn't match the shader's declared element type is undefined behavior in D3D12 and hardware drops the
	// loads/stores (e.g. GpuSort moving 16X2U index payloads through RWBuffer<uint> needs an R32_UINT view).
	//
	// TODO(d3d12-port): view.Offset/Range are not applied (no engine callers currently pass them for storage buffers).
	const UniformInformation* uniformInfo = GetLayout()->TryGetUniformInformation(slot);
	const bool isReadWrite = uniformInfo != nullptr && IsReadWriteStorageBuffer(uniformInfo->ObjectType);

	auto* d3d12Buffer = static_cast<D3D12GpuBuffer*>(buffer.get());

	D3D12_CPU_DESCRIPTOR_HANDLE handle = isReadWrite ? d3d12Buffer->GetUAVHandle(view.Format) : d3d12Buffer->GetSRVHandle(view.Format);
	if (handle.ptr == 0)
	{
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Storage buffer bound to set {0} slot {1} has no {2} view; binding its {3} view instead.",
			GetSet(), slot, isReadWrite ? "UAV" : "SRV", isReadWrite ? "SRV" : "UAV");
		handle = isReadWrite ? d3d12Buffer->GetSRVHandle(view.Format) : d3d12Buffer->GetUAVHandle(view.Format);

		if (handle.ptr == 0)
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12: Storage buffer '{0}' (type {1}) bound to set {2} slot {3} has no shader views at all; the slot is left unbound and reads will return zero.",
				buffer->GetName(), (u32)buffer->GetInformation().Type, GetSet(), slot);
		}
	}

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

void D3D12GpuParameters::UpdateGPUDescriptors(D3D12GpuDevice& device)
{
	// A dirty table gets a FRESH shader-visible range and a full recopy of its descriptors. The previously issued
	// range must not be overwritten: the command list consumes descriptors at EXECUTION time, so earlier
	// draws/dispatches recorded against the old range would otherwise all read the newest descriptors (the engine
	// re-binds the same parameter set with different resources many times per command buffer, e.g. buffer clears
	// or GUI batches). This mirrors Vulkan's per-update descriptor set allocation; old ranges are reclaimed by the
	// heap's ring allocator (see D3D12DescriptorManager::AllocateGPUDescriptorRange).
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	const u32 resourceDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::CBV_SRV_UAV);
	for (auto& entry : mDescriptorTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty || table.DescriptorCount == 0)
			continue;

		descriptorManager.AllocateGPUDescriptorRange(D3D12DescriptorHeapType::CBV_SRV_UAV, table.DescriptorCount,
			table.GPUVisibleCPUStart, table.GPUVisibleGPUStart);

		if (table.GPUVisibleCPUStart.ptr != 0)
		{
			for (u32 i = 0; i < table.Descriptors.size(); i++)
			{
				BoundDescriptor& desc = table.Descriptors[i];
				if (desc.CPUHandle.ptr != 0)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = table.GPUVisibleCPUStart;
					dstHandle.ptr += i * resourceDescriptorSize;

					d3d12Device->CopyDescriptorsSimple(1, dstHandle, desc.CPUHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}

				desc.IsDirty = false;
			}
		}

		table.IsDirty = false;
	}

	const u32 samplerDescriptorSize = descriptorManager.GetDescriptorSize(D3D12DescriptorHeapType::Sampler);
	for (auto& entry : mSamplerTables)
	{
		DescriptorTable& table = entry.second;
		if (!table.IsDirty || table.DescriptorCount == 0)
			continue;

		descriptorManager.AllocateGPUDescriptorRange(D3D12DescriptorHeapType::Sampler, table.DescriptorCount,
			table.GPUVisibleCPUStart, table.GPUVisibleGPUStart);

		if (table.GPUVisibleCPUStart.ptr != 0)
		{
			for (u32 i = 0; i < table.Descriptors.size(); i++)
			{
				BoundDescriptor& desc = table.Descriptors[i];

				// Sampler bindings never (or null-) set by the caller fall back to the default sampler: unlike
				// resource views there is no null descriptor for samplers, and a heap slot left unwritten is
				// undefined to sample through.
				const D3D12_CPU_DESCRIPTOR_HANDLE sourceHandle =
					desc.CPUHandle.ptr != 0 ? desc.CPUHandle : descriptorManager.GetDefaultSamplerCPUHandle();

				if (sourceHandle.ptr != 0)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = table.GPUVisibleCPUStart;
					dstHandle.ptr += i * samplerDescriptorSize;

					d3d12Device->CopyDescriptorsSimple(1, dstHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}

				desc.IsDirty = false;
			}
		}

		table.IsDirty = false;
	}
}

void D3D12GpuParameters::BindDescriptors(D3D12GpuDevice& device, ID3D12GraphicsCommandList* commandList, bool isGraphics, u32 rootParameterOffset,
	const GpuPipelineParameterSetLayout& pipelineSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets)
{
	UpdateGPUDescriptors(device);

	// Walk the PIPELINE's set layout in root-signature order (sets iterate parameter types in enum order, then
	// bindings sequentially - see D3D12GpuPipelineParameterLayout::CreateRootSignature) and bind this set's matching
	// resource, found by slot, to each root parameter. Bindings the set does not cover are left unset, same as null
	// resources; the caller re-sets matching parameters before any draw/dispatch that actually reads them.
	u32 rootParameterIndex = rootParameterOffset;

	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = pipelineSetLayout.GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* uniformInfo = pipelineSetLayout.TryGetUniformInformation(type, sequentialIndex);
			if (uniformInfo == nullptr)
				continue;

			const u32 currentRootParameterIndex = rootParameterIndex++;

			// Root CBV: resolve the bound uniform buffer's current GPU address at bind time (the backing resource
			// may have been renamed since it was set) and apply the suballocation offset, or its dynamic override.
			if (type == GpuParameterType::UniformBuffer && uniformInfo->ArraySize == 1)
			{
				const auto rootBufferIt = mRootConstantBuffers.find(uniformInfo->Slot);
				if (rootBufferIt == mRootConstantBuffers.end())
					continue;

				const RootConstantBuffer& rootBuffer = rootBufferIt->second;

				GpuBuffer* const buffer = mUniformBufferData[rootBuffer.DataIndex].Buffer.get();
				if (buffer == nullptr)
					continue; // Warned about at set time; leaving the root CBV unset matches the null-descriptor table path

				u32 offset = mUniformBufferData[rootBuffer.DataIndex].Offset;
				if (dynamicOffsets != nullptr && rootBuffer.DynamicOffsetIndex != ~0u)
				{
					if (const auto overrideIt = dynamicOffsets->find(rootBuffer.DynamicOffsetIndex); overrideIt != dynamicOffsets->end())
						offset = overrideIt->second;
				}

				const D3D12_GPU_VIRTUAL_ADDRESS address = static_cast<D3D12GpuBuffer*>(buffer)->GetGPUVirtualAddress() + offset;

				if (isGraphics)
					commandList->SetGraphicsRootConstantBufferView(currentRootParameterIndex, address);
				else
					commandList->SetComputeRootConstantBufferView(currentRootParameterIndex, address);

				continue;
			}

			const UnorderedMap<u32, DescriptorTable>& tables = type == GpuParameterType::Sampler ? mSamplerTables : mDescriptorTables;

			const auto tableIt = tables.find(uniformInfo->Slot);
			if (tableIt == tables.end())
				continue;

			const DescriptorTable& table = tableIt->second;
			if (table.DescriptorCount == 0 || table.GPUVisibleGPUStart.ptr == 0)
				continue;

			if (isGraphics)
				commandList->SetGraphicsRootDescriptorTable(currentRootParameterIndex, table.GPUVisibleGPUStart);
			else
				commandList->SetComputeRootDescriptorTable(currentRootParameterIndex, table.GPUVisibleGPUStart);
		}
	}
}
