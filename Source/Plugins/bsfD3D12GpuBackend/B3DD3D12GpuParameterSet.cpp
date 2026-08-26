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
#include "Allocators/B3DFrameAllocator.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

#include <algorithm>

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

	/** Converts reflected shader-stage visibility into the shader-stage flags used by resource tracking. */
	GpuResourceUseFlags GetShaderResourceUseFlags(const GpuProgramStageBits& stages)
	{
		GpuResourceUseFlags output = GpuResourceUseFlag::Undefined;
		if (stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain | GpuProgramStageBit::Geometry))
			output |= GpuResourceUseFlag::StageVertexShader;

		if (stages.IsSet(GpuProgramStageBit::Fragment))
			output |= GpuResourceUseFlag::StageFragmentShader;

		if (stages.IsSet(GpuProgramStageBit::Compute))
			output |= GpuResourceUseFlag::StageComputeShader;

		// Stage metadata should always be present on a pipeline layout, but retain a safe fallback for manually built or legacy layouts.
		return output != GpuResourceUseFlag::Undefined ? output : GpuResourceUseFlag::AnyStage;
	}

	/** Returns the SRV dimension matching a sampled-texture object type. */
	D3D12_SRV_DIMENSION GetSRVDimension(GpuParameterObjectType type)
	{
		switch(type)
		{
		case GPOT_TEXTURE1D:			return D3D12_SRV_DIMENSION_TEXTURE1D;
		case GPOT_TEXTURE1DARRAY:		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		case GPOT_TEXTURE2D:			return D3D12_SRV_DIMENSION_TEXTURE2D;
		case GPOT_TEXTURE2DARRAY:		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		case GPOT_TEXTURE2DMS:			return D3D12_SRV_DIMENSION_TEXTURE2DMS;
		case GPOT_TEXTURE2DMSARRAY:		return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
		case GPOT_TEXTURE3D:			return D3D12_SRV_DIMENSION_TEXTURE3D;
		case GPOT_TEXTURECUBE:			return D3D12_SRV_DIMENSION_TEXTURECUBE;
		case GPOT_TEXTURECUBEARRAY:		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		default:						return D3D12_SRV_DIMENSION_TEXTURE2D;
		}
	}

	/** Returns the UAV dimension matching a storage-texture object type. */
	D3D12_UAV_DIMENSION GetUAVDimension(GpuParameterObjectType type)
	{
		switch(type)
		{
		case GPOT_RWTEXTURE1D:			return D3D12_UAV_DIMENSION_TEXTURE1D;
		case GPOT_RWTEXTURE1DARRAY:		return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		case GPOT_RWTEXTURE2D:
		case GPOT_RWTEXTURE2DMS:		return D3D12_UAV_DIMENSION_TEXTURE2D;
		case GPOT_RWTEXTURE2DARRAY:
		case GPOT_RWTEXTURE2DMSARRAY:	return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		case GPOT_RWTEXTURE3D:			return D3D12_UAV_DIMENSION_TEXTURE3D;
		default:						return D3D12_UAV_DIMENSION_TEXTURE2D;
		}
	}
}

D3D12GpuParameters::D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex) : GpuParameterSet(parameterSetLayout, setIndex), mDevice(device)
{
}

D3D12GpuParameters::~D3D12GpuParameters()
{
	if(mResourceTable.Resource != nullptr)
		mResourceTable.Resource->Destroy();

	if(mSamplerTable.Resource != nullptr)
		mSamplerTable.Resource->Destroy();
}

void D3D12GpuParameters::Initialize()
{
	const TShared<GpuPipelineParameterSetLayout>& setLayout = GetLayout();
	if (setLayout == nullptr)
		return;

	// Create the per-binding resource storage from this set's own layout, keyed by slot. The active pipeline supplies
	// the packed table offsets at bind time, allowing compatible parameter sets to be matched by slot.
	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = setLayout->GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* uniformInformation = setLayout->TryGetUniformInformation(type, sequentialIndex);
			if (uniformInformation == nullptr)
				continue;

			// Single-element uniform buffers occupy a root CBV descriptor instead of a descriptor table (this must
			// mirror the root signature's promotion choice, see D3D12GpuPipelineParameterLayout::CreateRootSignature).
			if (type == GpuParameterType::UniformBuffer && uniformInformation->ArraySize == 1)
			{
				RootConstantBuffer& rootBuffer = mRootConstantBuffers[uniformInformation->Slot];
				rootBuffer.DataIndex = uniformInformation->SequentialResourceIndex;
				rootBuffer.DynamicOffsetIndex = uniformInformation->DynamicOffsetIndex;

				continue;
			}

			const bool isSampler = type == GpuParameterType::Sampler;
			UnorderedMap<u32, DescriptorBinding>& bindings = isSampler ? mSamplerBindings : mDescriptorBindings;

			DescriptorBinding& binding = bindings[uniformInformation->Slot];
			binding.Descriptors.resize(uniformInformation->ArraySize);

			// Fallback descriptor for elements the caller leaves unbound, matching the binding's declared type (see the field's documentation)
			D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();
			switch (type)
			{
			case GpuParameterType::UniformBuffer:
				binding.NullDescriptorHandle = descriptorManager.GetNullCBVHandle();
				break;
			case GpuParameterType::SampledTexture:
				binding.NullDescriptorHandle = descriptorManager.GetNullSRVHandle(GetSRVDimension(uniformInformation->ObjectType));
				break;
			case GpuParameterType::StorageTexture:
				binding.NullDescriptorHandle = descriptorManager.GetNullUAVHandle(GetUAVDimension(uniformInformation->ObjectType));
				break;
			case GpuParameterType::StorageBuffer:
				binding.NullDescriptorHandle = IsReadWriteStorageBuffer(uniformInformation->ObjectType) ? descriptorManager.GetNullUAVHandle(D3D12_UAV_DIMENSION_BUFFER) : descriptorManager.GetNullSRVHandle(D3D12_SRV_DIMENSION_BUFFER);
				break;
			case GpuParameterType::Sampler:
				binding.NullDescriptorHandle = descriptorManager.GetDefaultSamplerCPUHandle();
				break;
			default:
				break;
			}
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

	// TODO(d3d12-port): Suballocation offset is stored by the base class but not applied to arrayed CBVs; the CBV created on the buffer always covers suballocation 0. 
	// Technically we don't want to support this at all, so this should be rejected at the core level (arrays of dynamic offset buffers)
	if (uniformBuffer == nullptr)
	{
		// A cleared handle makes the GPU-visible copy substitute a null CBV (see UpdateGPUDescriptors)
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
		// A cleared handle makes the GPU-visible copy substitute a null SRV (see UpdateGPUDescriptors)
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
		// A cleared handle makes the GPU-visible copy substitute a null UAV (see UpdateGPUDescriptors)
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
		// A cleared handle makes the GPU-visible copy substitute a null SRV/UAV (see UpdateGPUDescriptors)
		SetDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	// TODO(d3d12-port): view.Offset/Range are not applied (no engine callers currently pass them for storage buffers).
	const UniformInformation* uniformInformation = GetLayout()->TryGetUniformInformation(slot);
	const bool isReadWrite = uniformInformation != nullptr && IsReadWriteStorageBuffer(uniformInformation->ObjectType);

	auto* d3d12Buffer = static_cast<D3D12GpuBuffer*>(buffer.get());

	D3D12_CPU_DESCRIPTOR_HANDLE handle = isReadWrite ? d3d12Buffer->GetUAVHandle(view.Format) : d3d12Buffer->GetSRVHandle(view.Format);
	if (handle.ptr == 0)
	{
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Storage buffer bound to set {0} slot {1} has no {2} view; binding its {3} view instead.", GetSet(), slot, isReadWrite ? "UAV" : "SRV", isReadWrite ? "SRV" : "UAV");
		handle = isReadWrite ? d3d12Buffer->GetSRVHandle(view.Format) : d3d12Buffer->GetUAVHandle(view.Format);

		if (handle.ptr == 0)
			B3D_LOG(Error, LogRenderBackend, "D3D12: Storage buffer '{0}' (type {1}) bound to set {2} slot {3} has no shader views at all; the slot is left unbound and reads will return zero.", buffer->GetName(), (u32)buffer->GetInformation().Type, GetSet(), slot);
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
		// A cleared handle makes the GPU-visible copy substitute the default sampler (see UpdateGPUDescriptors)
		SetSamplerDescriptor(slot, arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
		return true;
	}

	auto* d3d12Sampler = static_cast<D3D12SamplerState*>(sampler.get());
	SetSamplerDescriptor(slot, arrayIndex, d3d12Sampler->GetDescriptorHandle());
	return true;
}

void D3D12GpuParameters::TrackBoundResources(D3D12ResourceTracker& resourceTracker, D3D12BarrierHelper& barrierHelper, const GpuPipelineParameterSetLayout& pipelineSetLayout, D3D12RenderPassResourceUsage* renderPassUsage)
{
	const TShared<GpuPipelineParameterSetLayout>& setLayout = GetLayout();
	if (setLayout == nullptr)
		return;

	for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		const u32 bindingCount = pipelineSetLayout.GetBindingCount(type);

		for (u32 sequentialIndex = 0; sequentialIndex < bindingCount; sequentialIndex++)
		{
			const UniformInformation* pipelineUniformInformation = pipelineSetLayout.TryGetUniformInformation(type, sequentialIndex);
			if (pipelineUniformInformation == nullptr)
				continue;

			// The parameter set may have been created for a different compatible pipeline. Mirror descriptor binding by
			// walking the active layout and resolving this set's resource storage by slot.
			const UniformInformation* uniformInformation = setLayout->TryGetUniformInformation(pipelineUniformInformation->Slot);
			if (uniformInformation == nullptr || uniformInformation->Type != type)
				continue;

			const GpuResourceUseFlags stageUseFlags = GetShaderResourceUseFlags(pipelineUniformInformation->Usage);
			const u32 arraySize = std::min(uniformInformation->ArraySize, pipelineUniformInformation->ArraySize);

			for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
			{
				const u32 dataIndex = setLayout->GetSequentialResourceIndex(uniformInformation->Slot, arrayIndex);
				if (dataIndex == ~0u)
					continue;

				switch (type)
				{
				case GpuParameterType::UniformBuffer:
				{
					if (GpuBuffer* const buffer = mUniformBufferData[dataIndex].Buffer.get())
						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(), stageUseFlags | GpuResourceUseFlag::UniformBuffer, GpuAccessFlag::Read, barrierHelper, mUniformBufferData[dataIndex].Offset);

					break;
				}
				case GpuParameterType::SampledTexture:
				{
					if (Texture* const texture = mSampledTextureData[dataIndex].Texture.get())
					{
						D3D12Image* const image = static_cast<D3D12Texture*>(texture)->GetD3D12Image();
						if (image != nullptr)
						{
							const GpuTextureSubresourceRange subresourceRange = image->GetRange(mSampledTextureData[dataIndex].Surface);

							resourceTracker.TrackSampledImageUsage(image, subresourceRange, stageUseFlags | GpuResourceUseFlag::ShaderAccess, barrierHelper, renderPassUsage);
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
							const GpuTextureSubresourceRange subresourceRange = image->GetRange(mStorageTextureData[dataIndex].Surface);

							resourceTracker.TrackImageUsage(image, subresourceRange, GpuImageLayout::General, GpuImageLayout::General, stageUseFlags | GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper);
						}
					}
					break;
				}
				case GpuParameterType::StorageBuffer:
				{
					if (GpuBuffer* const buffer = mStorageBufferData[dataIndex].Buffer.get())
					{
						GpuAccessFlags accessFlags = GpuAccessFlag::Read;
						if (IsReadWriteStorageBuffer(uniformInformation->ObjectType))
							accessFlags |= GpuAccessFlag::Write;

						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(), stageUseFlags | GpuResourceUseFlag::ShaderAccess, accessFlags, barrierHelper);
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

bool D3D12GpuParameters::SetBindingDescriptor(UnorderedMap<u32, DescriptorBinding>& bindings, u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	const auto bindingIterator = bindings.find(slot);
	if(bindingIterator == bindings.end())
		return false;

	DescriptorBinding& binding = bindingIterator->second;
	if(arrayIndex >= binding.Descriptors.size())
		return false;

	binding.Descriptors[arrayIndex].CPUHandle = handle;
	return true;
}

void D3D12GpuParameters::UpdateGPUDescriptors(D3D12GpuDevice& device, const D3D12DescriptorSetLayout& descriptorSetLayout)
{
	// Dirty tables are updated in place only after all command buffers referencing their current range have completed.
	// Otherwise a new range preserves the descriptor snapshot seen by already recorded work.
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	auto fnGetNullDescriptor = [&descriptorManager](const D3D12DescriptorBindingLayout& bindingLayout)
	{
		switch(bindingLayout.Type)
		{
		case GpuParameterType::UniformBuffer:
			return descriptorManager.GetNullCBVHandle();
		case GpuParameterType::SampledTexture:
			return descriptorManager.GetNullSRVHandle(GetSRVDimension(bindingLayout.ObjectType));
		case GpuParameterType::StorageTexture:
			return descriptorManager.GetNullUAVHandle(GetUAVDimension(bindingLayout.ObjectType));
		case GpuParameterType::StorageBuffer:
			return IsReadWriteStorageBuffer(bindingLayout.ObjectType) ? descriptorManager.GetNullUAVHandle(D3D12_UAV_DIMENSION_BUFFER) : descriptorManager.GetNullSRVHandle(D3D12_SRV_DIMENSION_BUFFER);
		case GpuParameterType::Sampler:
			return descriptorManager.GetDefaultSamplerCPUHandle();
		default:
			return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
		}
	};

	auto fnUpdateTable = [d3d12Device, &descriptorManager, &fnGetNullDescriptor](DescriptorTable& table, const D3D12DescriptorTableLayout& tableLayout,
		const UnorderedMap<u32, DescriptorBinding>& bindings, D3D12DescriptorHeapType heapType, D3D12_DESCRIPTOR_HEAP_TYPE nativeHeapType)
	{
		if(table.Layout != &tableLayout || table.DescriptorCount != tableLayout.DescriptorCount)
			table.IsDirty = true;

		if(!table.IsDirty)
			return;

		if(tableLayout.DescriptorCount == 0)
		{
			if(table.Resource != nullptr)
			{
				table.Resource->Destroy();
				table.Resource = nullptr;
			}

			table.DescriptorCount = 0;
			table.Layout = &tableLayout;
			table.IsDirty = false;

			return;
		}

		const bool canReuseTable = table.Resource != nullptr && table.DescriptorCount == tableLayout.DescriptorCount && !table.Resource->IsBound();
		D3D12DescriptorTable* descriptorTable = canReuseTable ? table.Resource : descriptorManager.AllocateGPUDescriptorTable(heapType, tableLayout.DescriptorCount);
		if(descriptorTable == nullptr)
			return;

		FrameAllocatorScope frameScope;
		FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> sourceHandles(tableLayout.DescriptorCount, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });

		const D3D12_CPU_DESCRIPTOR_HANDLE destinationStart = descriptorTable->GetCPUHandle();
		for(const D3D12DescriptorBindingLayout& bindingLayout : tableLayout.Bindings)
		{
			const auto bindingIterator = bindings.find(bindingLayout.Slot);
			const DescriptorBinding* binding = bindingIterator != bindings.end() ? &bindingIterator->second : nullptr;
			const D3D12_CPU_DESCRIPTOR_HANDLE nullDescriptor = binding != nullptr ? binding->NullDescriptorHandle : fnGetNullDescriptor(bindingLayout);

			for(u32 arrayIndex = 0; arrayIndex < bindingLayout.DescriptorCount; arrayIndex++)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE sourceHandle = nullDescriptor;
				if(binding != nullptr && arrayIndex < binding->Descriptors.size() && binding->Descriptors[arrayIndex].CPUHandle.ptr != 0)
					sourceHandle = binding->Descriptors[arrayIndex].CPUHandle;

				B3D_ASSERT(sourceHandle.ptr != 0);
				sourceHandles[bindingLayout.TableOffset + arrayIndex] = sourceHandle;
			}
		}

		for(const D3D12_CPU_DESCRIPTOR_HANDLE sourceHandle : sourceHandles)
			B3D_ASSERT(sourceHandle.ptr != 0);

		const UINT destinationRangeSize = tableLayout.DescriptorCount;
		d3d12Device->CopyDescriptors(1, &destinationStart, &destinationRangeSize, tableLayout.DescriptorCount, sourceHandles.data(), nullptr, nativeHeapType);

		if(!canReuseTable && table.Resource != nullptr)
			table.Resource->Destroy();

		table.Resource = descriptorTable;
		table.DescriptorCount = tableLayout.DescriptorCount;
		table.Layout = &tableLayout;
		table.IsDirty = false;
	};

	fnUpdateTable(mResourceTable, descriptorSetLayout.ResourceTable, mDescriptorBindings, D3D12DescriptorHeapType::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	fnUpdateTable(mSamplerTable, descriptorSetLayout.SamplerTable, mSamplerBindings, D3D12DescriptorHeapType::Sampler, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12GpuParameters::BindDescriptors(D3D12GpuDevice& device, D3D12ResourceTracker& resourceTracker, ID3D12GraphicsCommandList* commandList, bool isGraphics,
	const D3D12DescriptorSetLayout& descriptorSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets)
{
	UpdateGPUDescriptors(device, descriptorSetLayout);

	for(const D3D12RootConstantBufferLayout& rootBufferLayout : descriptorSetLayout.RootConstantBuffers)
	{
		const auto rootBufferIterator = mRootConstantBuffers.find(rootBufferLayout.Slot);
		if(rootBufferIterator == mRootConstantBuffers.end())
			continue;

		const RootConstantBuffer& rootBuffer = rootBufferIterator->second;
		GpuBuffer* const buffer = mUniformBufferData[rootBuffer.DataIndex].Buffer.get();
		if(buffer == nullptr)
			continue;

		u32 offset = mUniformBufferData[rootBuffer.DataIndex].Offset;
		if(dynamicOffsets != nullptr && rootBuffer.DynamicOffsetIndex != ~0u)
		{
			if(const auto overrideIterator = dynamicOffsets->find(rootBuffer.DynamicOffsetIndex); overrideIterator != dynamicOffsets->end())
				offset = overrideIterator->second;
		}

		const D3D12_GPU_VIRTUAL_ADDRESS address = static_cast<D3D12GpuBuffer*>(buffer)->GetGPUVirtualAddress() + offset;
		if(isGraphics)
			commandList->SetGraphicsRootConstantBufferView(rootBufferLayout.RootParameterIndex, address);
		else
			commandList->SetComputeRootConstantBufferView(rootBufferLayout.RootParameterIndex, address);
	}

	auto fnBindTable = [commandList, isGraphics, &resourceTracker](const DescriptorTable& table, const D3D12DescriptorTableLayout& tableLayout)
	{
		// If allocation failed, IsDirty remains set and the previous table must not be rebound with stale descriptors.
		if(table.DescriptorCount == 0 || table.Resource == nullptr || table.IsDirty)
			return;

		resourceTracker.TrackResourceUsage(table.Resource, GpuAccessFlag::Read);

		const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = table.Resource->GetGPUHandle();
		if(isGraphics)
			commandList->SetGraphicsRootDescriptorTable(tableLayout.RootParameterIndex, gpuHandle);
		else
			commandList->SetComputeRootDescriptorTable(tableLayout.RootParameterIndex, gpuHandle);
	};

	fnBindTable(mResourceTable, descriptorSetLayout.ResourceTable);
	fnBindTable(mSamplerTable, descriptorSetLayout.SamplerTable);
}
