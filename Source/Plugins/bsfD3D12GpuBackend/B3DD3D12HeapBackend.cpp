//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12HeapBackend.h"
#include "B3DD3D12GpuDevice.h"

using namespace b3d;
using namespace b3d::render;

D3D12HeapBackend::D3D12HeapBackend(D3D12GpuDevice& device)
	: mDevice(device.GetD3D12Device()), mAdapter(device.GetDXGIAdapter())
{}

D3D12HeapBackend::~D3D12HeapBackend() = default;

namespace
{
	/** Returns a description of @p memory for log messages. */
	const char* GetHeapMemoryDescription(const D3D12HeapMemory& memory)
	{
		if(memory.CpuPageProperty == D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE)
			return "unmapped GPU memory, written through staging buffers";

		return memory.MemoryPool == D3D12_MEMORY_POOL_L0 ? "system memory" : "mapped GPU memory";
	}
}

IGpuHeap* D3D12HeapBackend::CreateHeap(u64 sizeInBytes, const D3D12HeapCreateInformation& createInformation)
{
	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = sizeInBytes;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapDesc.Properties.CPUPageProperty = createInformation.Memory.CpuPageProperty;
	heapDesc.Properties.MemoryPoolPreference = createInformation.Memory.MemoryPool;
	heapDesc.Properties.CreationNodeMask = 0;
	heapDesc.Properties.VisibleNodeMask = 0;
	heapDesc.Alignment = createInformation.Alignment;
	heapDesc.Flags = createInformation.Flags | D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;

	D3D12HeapMemory memory = createInformation.Memory;
	const auto fnFallBack = [&heapDesc, &memory, &createInformation]()
	{
		memory = *createInformation.FallbackMemory;
		heapDesc.Properties.CPUPageProperty = memory.CpuPageProperty;
		heapDesc.Properties.MemoryPoolPreference = memory.MemoryPool;
	};

	// A heap in GPU memory that has a fallback is only there while it leaves a tenth of the budget to heaps that must be in GPU memory
	if(createInformation.FallbackMemory.has_value() && memory.MemoryPool == D3D12_MEMORY_POOL_L1)
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInformation = {};
		const bool hasMemoryInformation = SUCCEEDED(mAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInformation));
		if(!hasMemoryInformation || memoryInformation.CurrentUsage + sizeInBytes > memoryInformation.Budget - memoryInformation.Budget / 10)
		{
			constexpr u64 kMb = 1024 * 1024;
			B3D_LOG(Warning, LogRenderBackend, "D3D12: GPU memory is close to its budget ({0} of {1} MiB used), so a {2} MiB heap of {3} is in {4} instead.",
				memoryInformation.CurrentUsage / kMb, memoryInformation.Budget / kMb, sizeInBytes / kMb, GetHeapMemoryDescription(memory),
				GetHeapMemoryDescription(*createInformation.FallbackMemory));
			fnFallBack();
		}
	}

	ComPtr<ID3D12Heap> nativeHeap;
	HRESULT hr = mDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&nativeHeap));
	if(FAILED(hr) && createInformation.FallbackMemory.has_value() && memory != *createInformation.FallbackMemory)
	{
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Failed to create a {0}-byte heap of {1} (hr={2}), so it is in {3} instead.", sizeInBytes,
			GetHeapMemoryDescription(memory), (u32)hr, GetHeapMemoryDescription(*createInformation.FallbackMemory));
		fnFallBack();
		hr = mDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&nativeHeap));
	}

	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create {0}-byte GPU heap (hr={1}, CPU page property={2}, memory pool={3}, flags={4}, alignment={5}).",
			sizeInBytes, (u32)hr, (u32)memory.CpuPageProperty, (u32)memory.MemoryPool, (u32)heapDesc.Flags, createInformation.Alignment);
		return nullptr;
	}

	Lock lock(mHeapPoolMutex);
	D3D12GpuHeap* heap = mHeapPool.Allocate();
	heap->Heap = std::move(nativeHeap);
	heap->Size = sizeInBytes;
	heap->Memory = memory;
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
	heap->Memory = D3D12HeapMemory();
	heap->Flags = D3D12_HEAP_FLAG_NONE;

	Lock lock(mHeapPoolMutex);
	mHeapPool.Release(heap);
}

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(D3D12HeapBackend);
