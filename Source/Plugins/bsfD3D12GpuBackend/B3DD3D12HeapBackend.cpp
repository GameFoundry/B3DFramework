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

IGpuHeap* D3D12HeapBackend::CreateHeap(u64 sizeInBytes, const D3D12HeapCreateInformation& createInformation)
{
	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = sizeInBytes;
	heapDesc.Properties.Type = createInformation.Properties.Type;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.CreationNodeMask = 0;
	heapDesc.Properties.VisibleNodeMask = 0;
	heapDesc.Alignment = createInformation.Alignment;
	heapDesc.Flags = createInformation.Flags | D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;

	if(heapDesc.Properties.Type == D3D12_HEAP_TYPE_CUSTOM)
	{
		heapDesc.Properties.CPUPageProperty = createInformation.Properties.CpuPageProperty;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	}

	// Data the CPU writes is in GPU upload heaps only while it leaves a tenth of the budget to resources that must be in GPU memory. Otherwise
	// the heap falls back as its create information asks (see D3D12HeapFallback). The system memory fallback is a custom heap rather than an
	// upload heap, since GPU copies can write it (GPU memory defragmentation moves buffers into it).
	const bool fallsBackToUnmappedGpuMemory = createInformation.Properties.Fallback == D3D12HeapFallback::UnmappedGpuMemory;
	const char* fallbackDescription = fallsBackToUnmappedGpuMemory ? "unmapped GPU memory, written through staging buffers" : "system memory";
	const auto fnFallBack = [&heapDesc, fallsBackToUnmappedGpuMemory]()
	{
		if(fallsBackToUnmappedGpuMemory)
		{
			heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		}
		else
		{
			heapDesc.Properties.Type = D3D12_HEAP_TYPE_CUSTOM;
			heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
			heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		}
	};

	if(heapDesc.Properties.Type == D3D12_HEAP_TYPE_GPU_UPLOAD)
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInformation = {};
		const bool hasMemoryInformation = SUCCEEDED(mAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInformation));
		if(!hasMemoryInformation || memoryInformation.CurrentUsage + sizeInBytes > memoryInformation.Budget - memoryInformation.Budget / 10)
		{
			constexpr u64 kMb = 1024 * 1024;
			B3D_LOG(Warning, LogRenderBackend, "D3D12: GPU memory is close to its budget ({0} of {1} MiB used), so a {2} MiB heap of CPU-written buffers is in "
				"{3} instead.", memoryInformation.CurrentUsage / kMb, memoryInformation.Budget / kMb, sizeInBytes / kMb, fallbackDescription);
			fnFallBack();
		}
	}

	ComPtr<ID3D12Heap> nativeHeap;
	HRESULT hr = mDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&nativeHeap));
	if(FAILED(hr) && heapDesc.Properties.Type == D3D12_HEAP_TYPE_GPU_UPLOAD)
	{
		B3D_LOG(Warning, LogRenderBackend, "D3D12: Failed to create a {0}-byte GPU upload heap (hr={1}), so its CPU-written buffers are in {2} instead.",
			sizeInBytes, (u32)hr, fallbackDescription);
		fnFallBack();
		hr = mDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&nativeHeap));
	}

	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create {0}-byte GPU heap (hr={1}, type={2}, flags={3}, alignment={4}).",
			sizeInBytes, (u32)hr, (u32)createInformation.Properties.Type, (u32)heapDesc.Flags, createInformation.Alignment);
		return nullptr;
	}

	Lock lock(mHeapPoolMutex);
	D3D12GpuHeap* heap = mHeapPool.Allocate();
	heap->Heap = std::move(nativeHeap);
	heap->Size = sizeInBytes;
	heap->Type = heapDesc.Properties.Type;
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
