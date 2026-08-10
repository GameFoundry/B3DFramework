//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12HeapBackend.h"
#include "B3DD3D12GpuDevice.h"

using namespace b3d;
using namespace b3d::render;

D3D12HeapBackend::D3D12HeapBackend(D3D12GpuDevice& device)
	: mDevice(device.GetD3D12Device())
{}

D3D12HeapBackend::~D3D12HeapBackend() = default;

IGpuHeap* D3D12HeapBackend::CreateHeap(u64 sizeInBytes, const D3D12HeapCreateInformation& createInformation)
{
	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = sizeInBytes;
	heapDesc.Properties.Type = createInformation.Type;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.CreationNodeMask = 0;
	heapDesc.Properties.VisibleNodeMask = 0;
	heapDesc.Alignment = createInformation.Alignment;
	heapDesc.Flags = createInformation.Flags | D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;

	ComPtr<ID3D12Heap> nativeHeap;
	const HRESULT hr = mDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&nativeHeap));
	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create {0}-byte GPU heap (hr={1}, type={2}, flags={3}, alignment={4}).",
			sizeInBytes, (u32)hr, (u32)createInformation.Type, (u32)heapDesc.Flags, createInformation.Alignment);
		return nullptr;
	}

	Lock lock(mHeapPoolMutex);
	D3D12GpuHeap* heap = mHeapPool.Allocate();
	heap->Heap = std::move(nativeHeap);
	heap->Size = sizeInBytes;
	heap->Type = createInformation.Type;
	heap->Flags = heapDesc.Flags;

	return heap;
}

void D3D12HeapBackend::DestroyHeap(IGpuHeap* handle)
{
	if(handle == nullptr)
		return;

	D3D12GpuHeap* heap = static_cast<D3D12GpuHeap*>(handle);
	heap->Heap.Reset();
	heap->Size = 0;
	heap->Type = D3D12_HEAP_TYPE_DEFAULT;
	heap->Flags = D3D12_HEAP_FLAG_NONE;

	Lock lock(mHeapPoolMutex);
	mHeapPool.Release(heap);
}

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(D3D12HeapBackend);
