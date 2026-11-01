//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BindlessHeaps.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12SamplerState.h"
#include "B3DD3D12Texture.h"

using namespace b3d;
using namespace b3d::render;

D3D12BindlessHeaps::D3D12BindlessHeaps(D3D12GpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation)
	: GpuBindlessHeaps(device, completionTracker, createInformation), mDevice(device), mDescriptorManager(device.GetDescriptorManager())
{ }

D3D12BindlessHeaps::~D3D12BindlessHeaps()
{
	mDescriptorManager.SetResourceHeapPool(nullptr);
	Destroy();
}

bool D3D12BindlessHeaps::Initialize()
{
	if(!GpuBindlessHeaps::Initialize())
		return false;

	mDescriptorManager.SetResourceHeapPool(&GetResourceHeapPool());
	return true;
}

D3D12BindlessHeap* D3D12BindlessHeaps::GetCurrentResourceHeap() const
{
	return static_cast<D3D12BindlessHeap*>(GetResourceHeapPool().GetCurrent());
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12BindlessHeaps::GetViewHandle(const GpuBindlessResourceView& view) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE sourceHandle;
	if(view.Texture != nullptr)
		sourceHandle = static_cast<D3D12Texture*>(view.Texture)->GetSRVHandle(view.Surface);
	else
	{
		auto* buffer = static_cast<D3D12GpuBuffer*>(view.Buffer);
		sourceHandle = buffer->GetSRVHandle(view.ViewType, view.Format);
	}

	if(!B3D_ENSURE_LOG(sourceHandle.ptr != 0, "Failed to create the view of '{0}' written into a bindless resource slot.",
		view.Texture != nullptr ? view.Texture->GetName() : view.Buffer->GetName()))
	{
		sourceHandle = mDescriptorManager.GetNullSRVHandle(GPOT_TEXTURE2D);
	}

	return sourceHandle;
}

void D3D12BindlessHeaps::WriteIntoCurrentHeap(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source)
{
	ID3D12Device* device = mDevice.GetD3D12Device();

	const D3D12_CPU_DESCRIPTOR_HANDLE mirrorHandle = mMirror.GetCPUHandle(slotIndex);
	device->CopyDescriptorsSimple(1, mirrorHandle, source, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Work using other native heaps was recorded before the write, so it never reads the slot
	device->CopyDescriptorsSimple(1, GetCurrentResourceHeap()->GetDescriptorHeap().GetCPUHandle(slotIndex), mirrorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GetResourceHeapPool().MarkSlotWritten(slotIndex);
}

void D3D12BindlessHeaps::WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view)
{
	WriteIntoCurrentHeap(slotIndex, GetViewHandle(view));
}

void D3D12BindlessHeaps::ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType)
{
	WriteIntoCurrentHeap(slotIndex, mDescriptorManager.GetNullSRVHandle(viewType));
}

void D3D12BindlessHeaps::WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler)
{
	mDescriptorManager.WriteBindlessSamplerSlot(slotIndex, static_cast<const D3D12SamplerState&>(sampler).GetDescriptorHandle());
}

bool D3D12BindlessHeaps::SwitchResourceHeap(u32 capacity, TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots)
{
	// Held across the switch, so no table is written between taking a native heap and copying the tables into it
	Lock lock(mDescriptorManager.GetResourceTableMutex());
	if(!GpuBindlessHeaps::SwitchResourceHeap(capacity, updates, liveSlots))
		return false;

	mDescriptorManager.CopyDirtyTables(*GetCurrentResourceHeap());
	return true;
}

GpuBindlessHeap* D3D12BindlessHeaps::CreateResourceHeap(u32 capacity)
{
	ID3D12Device* device = mDevice.GetD3D12Device();

	// The mirror is the source new native heaps are filled from, so it grows first
	if(capacity > mMirror.GetCapacity())
	{
		D3D12DescriptorHeap mirror;
		if(!mirror.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity, false))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create the CPU-only copy of the bindless resource heap, of {0} descriptors.", capacity);
			return nullptr;
		}

		if(mMirror.GetHeap() != nullptr)
			device->CopyDescriptorsSimple(mMirror.GetCapacity(), mirror.GetCPUHandle(0), mMirror.GetCPUHandle(0), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		mMirror = std::move(mirror);
	}

	return mDescriptorManager.CreateResourceHeap(capacity);
}

void D3D12BindlessHeaps::CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges)
{
	const D3D12DescriptorHeap& destinationHeap = static_cast<D3D12BindlessHeap&>(destination).GetDescriptorHeap();

	FrameAllocatorScope frameScope;
	FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> destinationStarts(ranges.Size());
	FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> sourceStarts(ranges.Size());
	FrameVector<UINT> rangeSizes(ranges.Size());

	for(u32 rangeIndex = 0; rangeIndex < (u32)ranges.Size(); rangeIndex++)
	{
		const TIndexRange<u32>& range = ranges[rangeIndex];
		destinationStarts[rangeIndex] = destinationHeap.GetCPUHandle(range.First);
		sourceStarts[rangeIndex] = mMirror.GetCPUHandle(range.First);
		rangeSizes[rangeIndex] = range.Count;
	}

	// The mirror holds the latest descriptor of every slot. Source and destination ranges have the same sizes.
	mDevice.GetD3D12Device()->CopyDescriptors((UINT)ranges.Size(), destinationStarts.data(), rangeSizes.data(), (UINT)ranges.Size(), sourceStarts.data(),
		rangeSizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12BindlessHeaps::WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates)
{
	ID3D12Device* device = mDevice.GetD3D12Device();
	const D3D12DescriptorHeap& destinationHeap = static_cast<D3D12BindlessHeap&>(destination).GetDescriptorHeap();

	for(const GpuBindlessSlotUpdate& update : updates)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE mirrorHandle = mMirror.GetCPUHandle(update.SlotIndex);
		device->CopyDescriptorsSimple(1, mirrorHandle, GetViewHandle(update.View), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CopyDescriptorsSimple(1, destinationHeap.GetCPUHandle(update.SlotIndex), mirrorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}
