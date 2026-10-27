//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuParameterSetPool.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12SamplerState.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

namespace
{
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

	/** Returns true if @p lhs and @p rhs place the same parameter at the same table offset. */
	bool IsSameBinding(const D3D12DescriptorBindingLayout& lhs, const D3D12DescriptorBindingLayout& rhs)
	{
		return lhs.Slot == rhs.Slot && lhs.TableOffset == rhs.TableOffset && lhs.DescriptorCount == rhs.DescriptorCount && lhs.Type == rhs.Type &&
			lhs.ObjectType == rhs.ObjectType;
	}
}

D3D12GpuParameters::D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex,
	D3D12GpuParameterSetPool& pool)
	: GpuParameterSet(parameterSetLayout, setIndex), mDevice(device)
{
	mOwnerPool = &pool;
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

			// Dynamic uniform buffers occupy a root CBV descriptor instead of a descriptor table (this must
			// mirror the root signature's promotion choice, see D3D12GpuPipelineParameterLayout::CreateRootSignature).
			if (type == GpuParameterType::UniformBuffer && uniformInformation->DynamicOffsetIndex != ~0u)
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
			binding.Type = type;
			binding.FirstDataIndex = uniformInformation->SequentialResourceIndex;
			binding.IsReadWrite = type == GpuParameterType::StorageBuffer && GpuObjectParameterTypeInformation::IsReadWriteBuffer(uniformInformation->ObjectType);

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
				binding.NullDescriptorHandle = GpuObjectParameterTypeInformation::IsReadWriteBuffer(uniformInformation->ObjectType) ? descriptorManager.GetNullUAVHandle(D3D12_UAV_DIMENSION_BUFFER) : descriptorManager.GetNullSRVHandle(D3D12_SRV_DIMENSION_BUFFER);
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

	// Dynamic uniform buffers are bound as root CBVs: the buffer and its suballocation offset are read from
	// the base class's bound-buffer data at bind time, no descriptor is involved.
	if (mRootConstantBuffers.find(slot) != mRootConstantBuffers.end())
		return true;

	if (DescriptorBinding* binding = FindDescriptorBinding(slot))
		StoreDescriptor(*binding, arrayIndex, ResolveCBV(static_cast<const D3D12GpuBuffer*>(uniformBuffer.get()), offset));

	return true;
}

bool D3D12GpuParameters::SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetSampledTexture(slot, texture, surface, arrayIndex))
		return false;

	if (DescriptorBinding* binding = FindDescriptorBinding(slot))
		StoreDescriptor(*binding, arrayIndex, ResolveSRV(static_cast<D3D12Texture*>(texture.get()), surface));

	return true;
}

bool D3D12GpuParameters::SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetStorageTexture(slot, texture, surface, arrayIndex))
		return false;

	if (DescriptorBinding* binding = FindDescriptorBinding(slot))
		StoreDescriptor(*binding, arrayIndex, ResolveUAV(static_cast<D3D12Texture*>(texture.get()), surface));

	return true;
}

bool D3D12GpuParameters::SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex, GpuBufferViewInformation view)
{
	if (!GpuParameterSet::SetStorageBuffer(slot, buffer, arrayIndex, view))
		return false;

	// TODO(d3d12-port): view.Offset/Range are not applied (no engine callers currently pass them for storage buffers).
	if (DescriptorBinding* binding = FindDescriptorBinding(slot))
		StoreDescriptor(*binding, arrayIndex, ResolveStorageBuffer(static_cast<const D3D12GpuBuffer*>(buffer.get()), view.Format, binding->IsReadWrite, slot));

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

bool D3D12GpuParameters::TrackResources(const GpuPipelineParameterSetLayout& pipelineSetLayout, GpuResourceTracker& resourceTracker)
{
	const TShared<GpuPipelineParameterSetLayout>& setLayout = GetLayout();
	if (setLayout == nullptr)
		return true;

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
						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(), GpuAccessFlag::Read, mUniformBufferData[dataIndex].Offset);

					break;
				}
				case GpuParameterType::SampledTexture:
				{
					if (Texture* const texture = mSampledTextureData[dataIndex].Texture.get())
					{
						D3D12Image* const image = static_cast<D3D12Texture*>(texture)->GetD3D12Image();
						if (image != nullptr)
						{
							GpuTextureSubresourceRange subresourceRange = image->GetRange(mSampledTextureData[dataIndex].Surface);
							// GetRange() reports resource aspects, while sampled depth-stencil views select only the depth plane.
							if(subresourceRange.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
								subresourceRange.AspectMask = GpuTextureAspectFlag::Depth;

							if(!resourceTracker.TrackShaderImageUsage(image, subresourceRange, GpuAccessFlag::Read))
								return false;
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

							if(!resourceTracker.TrackShaderImageUsage(image, subresourceRange, GpuAccessFlag::Read | GpuAccessFlag::Write))
								return false;
						}
					}
					break;
				}
				case GpuParameterType::StorageBuffer:
				{
					if (GpuBuffer* const buffer = mStorageBufferData[dataIndex].Buffer.get())
					{
						GpuAccessFlags accessFlags = GpuAccessFlag::Read;
						if (GpuObjectParameterTypeInformation::IsReadWriteBuffer(uniformInformation->ObjectType))
							accessFlags |= GpuAccessFlag::Write;

						resourceTracker.TrackBufferUsage(static_cast<D3D12GpuBuffer*>(buffer)->GetD3D12Buffer(), accessFlags);
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

	return true;
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

D3D12GpuParameters::DescriptorBinding* D3D12GpuParameters::FindDescriptorBinding(u32 slot)
{
	const auto bindingIterator = mDescriptorBindings.find(slot);
	return bindingIterator != mDescriptorBindings.end() ? &bindingIterator->second : nullptr;
}

void D3D12GpuParameters::StoreDescriptor(DescriptorBinding& binding, u32 arrayIndex, const ResolvedDescriptor& descriptor)
{
	if(arrayIndex >= binding.Descriptors.size())
		return;

	// A cleared handle makes the GPU-visible copy substitute a null descriptor (see UpdateGPUDescriptors). Resources without a
	// native resource resolve to one, until RefreshDescriptors() resolves them again once they have one.
	BoundDescriptor& boundDescriptor = binding.Descriptors[arrayIndex];
	boundDescriptor.CPUHandle = descriptor.Handle;
	boundDescriptor.ResourceGeneration = descriptor.ResourceGeneration;
	mResourceTable.IsDirty = true;
}

D3D12GpuParameters::ResolvedDescriptor D3D12GpuParameters::ResolveCBV(const D3D12GpuBuffer* buffer, u32 offset)
{
	if(buffer == nullptr)
		return ResolvedDescriptor();

	return ResolvedDescriptor{ buffer->GetCBVHandle(offset), buffer->GetResourceGeneration() };
}

D3D12GpuParameters::ResolvedDescriptor D3D12GpuParameters::ResolveSRV(D3D12Texture* texture, const TextureSurface& surface)
{
	if(texture == nullptr)
		return ResolvedDescriptor();

	return ResolvedDescriptor{ texture->GetSRVHandle(surface), texture->GetResourceGeneration() };
}

D3D12GpuParameters::ResolvedDescriptor D3D12GpuParameters::ResolveUAV(D3D12Texture* texture, const TextureSurface& surface)
{
	if(texture == nullptr)
		return ResolvedDescriptor();

	return ResolvedDescriptor{ texture->GetUAVHandle(surface), texture->GetResourceGeneration() };
}

D3D12GpuParameters::ResolvedDescriptor D3D12GpuParameters::ResolveStorageBuffer(const D3D12GpuBuffer* buffer, GpuBufferFormat format, bool isReadWrite, u32 slot) const
{
	if(buffer == nullptr)
		return ResolvedDescriptor();

	ResolvedDescriptor output;
	output.ResourceGeneration = buffer->GetResourceGeneration();
	if(buffer->GetD3D12Buffer() == nullptr)
		return output;

	output.Handle = isReadWrite ? buffer->GetUAVHandle(format) : buffer->GetSRVHandle(format);
	if (output.Handle.ptr == 0)
	{
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Storage buffer bound to set {0} slot {1} has no {2} view; binding its {3} view instead.", GetSet(), slot, isReadWrite ? "UAV" : "SRV", isReadWrite ? "SRV" : "UAV");
		output.Handle = isReadWrite ? buffer->GetSRVHandle(format) : buffer->GetUAVHandle(format);

		if (output.Handle.ptr == 0)
			B3D_LOG(Error, LogRenderBackend, "D3D12: Storage buffer '{0}' (type {1}) bound to set {2} slot {3} has no shader views at all; the slot is left unbound and reads will return zero.", buffer->GetName(), (u32)buffer->GetInformation().Type, GetSet(), slot);
	}

	return output;
}

bool D3D12GpuParameters::DescriptorTable::IsCurrent(const D3D12DescriptorTableLayout& layout) const
{
	return !IsDirty && DescriptorCount == layout.DescriptorCount && std::equal(Bindings.begin(), Bindings.end(), layout.Bindings.begin(), layout.Bindings.end(),
		IsSameBinding);
}

bool D3D12GpuParameters::RefreshDescriptors(bool canWrite)
{
	bool isStale = false;
	for(auto& entry : mDescriptorBindings)
	{
		DescriptorBinding& binding = entry.second;
		if(binding.FirstDataIndex == ~0u)
			continue;

		// Bindings without a resource stay at generation zero, which no resource has once it created its native resource
		for(u32 arrayIndex = 0; arrayIndex < (u32)binding.Descriptors.size(); arrayIndex++)
		{
			const u32 dataIndex = binding.FirstDataIndex + arrayIndex;
			const u32 resolvedGeneration = binding.Descriptors[arrayIndex].ResourceGeneration;
			switch(binding.Type)
			{
			case GpuParameterType::UniformBuffer:
			{
				const auto& data = mUniformBufferData[dataIndex];
				const auto* buffer = static_cast<const D3D12GpuBuffer*>(data.Buffer.get());
				if(buffer != nullptr && buffer->GetResourceGeneration() != resolvedGeneration)
				{
					if(!canWrite)
						return true;

					StoreDescriptor(binding, arrayIndex, ResolveCBV(buffer, data.Offset));
					isStale = true;
				}

				break;
			}
			case GpuParameterType::SampledTexture:
			{
				const auto& data = mSampledTextureData[dataIndex];
				auto* texture = static_cast<D3D12Texture*>(data.Texture.get());
				if(texture != nullptr && texture->GetResourceGeneration() != resolvedGeneration)
				{
					if(!canWrite)
						return true;

					StoreDescriptor(binding, arrayIndex, ResolveSRV(texture, data.Surface));
					isStale = true;
				}

				break;
			}
			case GpuParameterType::StorageTexture:
			{
				const auto& data = mStorageTextureData[dataIndex];
				auto* texture = static_cast<D3D12Texture*>(data.Texture.get());
				if(texture != nullptr && texture->GetResourceGeneration() != resolvedGeneration)
				{
					if(!canWrite)
						return true;

					StoreDescriptor(binding, arrayIndex, ResolveUAV(texture, data.Surface));
					isStale = true;
				}

				break;
			}
			case GpuParameterType::StorageBuffer:
			{
				const auto& data = mStorageBufferData[dataIndex];
				const auto* buffer = static_cast<const D3D12GpuBuffer*>(data.Buffer.get());
				if(buffer != nullptr && buffer->GetResourceGeneration() != resolvedGeneration)
				{
					if(!canWrite)
						return true;

					StoreDescriptor(binding, arrayIndex, ResolveStorageBuffer(buffer, data.View.Format, binding.IsReadWrite, entry.first));
					isStale = true;
				}

				break;
			}
			default:
				break;
			}
		}
	}

	return isStale;
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
			return GpuObjectParameterTypeInformation::IsReadWriteBuffer(bindingLayout.ObjectType) ? descriptorManager.GetNullUAVHandle(D3D12_UAV_DIMENSION_BUFFER) : descriptorManager.GetNullSRVHandle(D3D12_SRV_DIMENSION_BUFFER);
		case GpuParameterType::Sampler:
			return descriptorManager.GetDefaultSamplerCPUHandle();
		default:
			return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
		}
	};

	auto fnUpdateTable = [d3d12Device, &descriptorManager, &fnGetNullDescriptor](DescriptorTable& table, const D3D12DescriptorTableLayout& tableLayout,
		const UnorderedMap<u32, DescriptorBinding>& bindings, D3D12DescriptorHeapType heapType, D3D12_DESCRIPTOR_HEAP_TYPE nativeHeapType)
	{
		if(table.IsCurrent(tableLayout))
			return;

		table.IsDirty = true;

		if(tableLayout.DescriptorCount == 0)
		{
			if(table.Resource != nullptr)
			{
				table.Resource->Destroy();
				table.Resource = nullptr;
			}

			table.DescriptorCount = 0;
			table.Bindings.assign(tableLayout.Bindings.begin(), tableLayout.Bindings.end());
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
		table.Bindings.assign(tableLayout.Bindings.begin(), tableLayout.Bindings.end());
		table.IsDirty = false;
	};

	fnUpdateTable(mResourceTable, descriptorSetLayout.ResourceTable, mDescriptorBindings, D3D12DescriptorHeapType::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	fnUpdateTable(mSamplerTable, descriptorSetLayout.SamplerTable, mSamplerBindings, D3D12DescriptorHeapType::Sampler, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12GpuParameters::Flush(TArrayView<const GpuRenderPassAttachmentUsage> attachments)
{
	B3D_ASSERT(IsOwnedByThisThread() && "Parameter sets can only be flushed on the thread that owns their pool.");

	RefreshDescriptors(true);
	UpdateGPUDescriptors(mDevice, static_cast<const D3D12GpuPipelineParameterSetLayout&>(*mParameterSetLayout).GetDescriptorSetLayout());
}

void D3D12GpuParameters::BindDescriptors(D3D12GpuDevice& device, GpuResourceTracker& resourceTracker, ID3D12GraphicsCommandList* commandList, bool isGraphics,
	const D3D12DescriptorSetLayout& descriptorSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets)
{
	// A set holds one table per type, packed as its own layout packs it. Flushing writes that packing, and threads that do not own the set
	// cannot write another one, so only pipelines that pack the set the same way can bind it.
	const auto fnIsSamePacking = [](const D3D12DescriptorTableLayout& lhs, const D3D12DescriptorTableLayout& rhs)
	{
		return lhs.DescriptorCount == rhs.DescriptorCount && std::equal(lhs.Bindings.begin(), lhs.Bindings.end(), rhs.Bindings.begin(), rhs.Bindings.end(),
			IsSameBinding);
	};

	const D3D12DescriptorSetLayout& ownDescriptorSetLayout = static_cast<const D3D12GpuPipelineParameterSetLayout&>(*mParameterSetLayout).GetDescriptorSetLayout();
	if(!B3D_ENSURE_LOG(fnIsSamePacking(ownDescriptorSetLayout.ResourceTable, descriptorSetLayout.ResourceTable) &&
		fnIsSamePacking(ownDescriptorSetLayout.SamplerTable, descriptorSetLayout.SamplerTable),
		"Parameter set {0} is bound with a pipeline that packs it differently than its own layout. A set can only be bound with pipelines whose "
		"layout of the set has the same bindings as the layout it was created with.", GetSet()))
	{
		return;
	}

	// Only the owner thread writes the set. Other threads, such as those recording a frame graph in parallel, bind it as it was flushed.
	if(IsOwnedByThisThread())
	{
		RefreshDescriptors(true);
		UpdateGPUDescriptors(device, descriptorSetLayout);
	}
#if B3D_BUILD_TYPE_DEVELOPMENT
	else if(RefreshDescriptors(false) || !mResourceTable.IsCurrent(descriptorSetLayout.ResourceTable) || !mSamplerTable.IsCurrent(descriptorSetLayout.SamplerTable))
	{
		B3D_LOG(Error, LogRenderBackend, "Parameter set {0} is bound on a thread that does not own it, but was not flushed for the bind. Flush it with "
			"GpuParameterSet::Flush() on the owner thread first.", GetSet());
	}
#endif

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
