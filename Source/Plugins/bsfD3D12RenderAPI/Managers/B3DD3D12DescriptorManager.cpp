//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12DescriptorManager.h"
#include "B3DD3D12GpuDevice.h"

using namespace b3d;
using namespace b3d::render;

D3D12DescriptorManager::D3D12DescriptorManager(D3D12GpuDevice& device)
	: mDevice(device)
{
	// Get descriptor sizes for each heap type
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	mDescriptorSizes[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV] =
		d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mDescriptorSizes[(u32)D3D12DescriptorHeapType::Sampler] =
		d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	mDescriptorSizes[(u32)D3D12DescriptorHeapType::RTV] =
		d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	mDescriptorSizes[(u32)D3D12DescriptorHeapType::DSV] =
		d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CreateHeaps();
}

D3D12DescriptorManager::~D3D12DescriptorManager()
{
	// Heaps will be released automatically via ComPtr
}

void D3D12DescriptorManager::CreateHeaps()
{
	ID3D12Device* device = mDevice.GetD3D12Device();

	// Create CPU-only descriptor heaps for RTVs and DSVs
	// These are used for render target and depth-stencil views

	// RTV heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.NumDescriptors = 256; // Allocate space for 256 render target views
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // CPU-only
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeaps[(u32)D3D12DescriptorHeapType::RTV].Heap));
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create RTV descriptor heap");

		mHeaps[(u32)D3D12DescriptorHeapType::RTV].CPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::RTV].Heap->GetCPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::RTV].NumDescriptors = heapDesc.NumDescriptors;
		mHeaps[(u32)D3D12DescriptorHeapType::RTV].NextFreeIndex = 0;
	}

	// DSV heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.NumDescriptors = 256; // Allocate space for 256 depth-stencil views
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // CPU-only
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeaps[(u32)D3D12DescriptorHeapType::DSV].Heap));
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create DSV descriptor heap");

		mHeaps[(u32)D3D12DescriptorHeapType::DSV].CPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::DSV].Heap->GetCPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::DSV].NumDescriptors = heapDesc.NumDescriptors;
		mHeaps[(u32)D3D12DescriptorHeapType::DSV].NextFreeIndex = 0;
	}

	// Create GPU-visible descriptor heaps for CBV/SRV/UAV and Samplers
	// These are used for shader resource binding

	// CBV/SRV/UAV heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = 4096; // Large heap for shader resources
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // GPU-visible
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].Heap));
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create CBV/SRV/UAV descriptor heap");

		mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].CPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].Heap->GetCPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].GPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].Heap->GetGPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].NumDescriptors = heapDesc.NumDescriptors;
		mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].NextFreeIndex = 0;
	}

	// Sampler heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.NumDescriptors = 2048; // Heap for samplers
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // GPU-visible
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeaps[(u32)D3D12DescriptorHeapType::Sampler].Heap));
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create Sampler descriptor heap");

		mHeaps[(u32)D3D12DescriptorHeapType::Sampler].CPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::Sampler].Heap->GetCPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::Sampler].GPUStart =
			mHeaps[(u32)D3D12DescriptorHeapType::Sampler].Heap->GetGPUDescriptorHandleForHeapStart();
		mHeaps[(u32)D3D12DescriptorHeapType::Sampler].NumDescriptors = heapDesc.NumDescriptors;
		mHeaps[(u32)D3D12DescriptorHeapType::Sampler].NextFreeIndex = 0;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::AllocateCPUDescriptor(D3D12DescriptorHeapType type)
{
	DescriptorHeap& heap = mHeaps[(u32)type];

	// Check free list first
	if (!heap.FreeList.empty())
	{
		u32 index = heap.FreeList.back();
		heap.FreeList.pop_back();

		D3D12_CPU_DESCRIPTOR_HANDLE handle = heap.CPUStart;
		handle.ptr += index * mDescriptorSizes[(u32)type];
		return handle;
	}

	// Allocate new descriptor
	if (heap.NextFreeIndex >= heap.NumDescriptors)
	{
		B3D_LOG(Error, LogRenderBackend, "Descriptor heap exhausted for type {0}", (u32)type);
		return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap.CPUStart;
	handle.ptr += heap.NextFreeIndex * mDescriptorSizes[(u32)type];
	heap.NextFreeIndex++;

	return handle;
}

void D3D12DescriptorManager::FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	DescriptorHeap& heap = mHeaps[(u32)type];

	// Calculate index from handle
	SIZE_T offset = handle.ptr - heap.CPUStart.ptr;
	u32 index = (u32)(offset / mDescriptorSizes[(u32)type]);

	// Add to free list
	heap.FreeList.push_back(index);
}

void D3D12DescriptorManager::AllocateGPUDescriptorRange(D3D12DescriptorHeapType type, u32 count,
	D3D12_CPU_DESCRIPTOR_HANDLE& outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGPUHandle)
{
	DescriptorHeap& heap = mHeaps[(u32)type];

	// For now, only support CBV_SRV_UAV and Sampler heaps (GPU-visible)
	B3D_ASSERT(type == D3D12DescriptorHeapType::CBV_SRV_UAV || type == D3D12DescriptorHeapType::Sampler);

	// Simple linear allocation (no free list for GPU descriptors yet)
	if (heap.NextFreeIndex + count > heap.NumDescriptors)
	{
		B3D_LOG(Error, LogRenderBackend, "GPU descriptor heap exhausted for type {0}", (u32)type);
		outCPUHandle.ptr = 0;
		outGPUHandle.ptr = 0;
		return;
	}

	outCPUHandle = heap.CPUStart;
	outCPUHandle.ptr += heap.NextFreeIndex * mDescriptorSizes[(u32)type];

	outGPUHandle = heap.GPUStart;
	outGPUHandle.ptr += heap.NextFreeIndex * mDescriptorSizes[(u32)type];

	heap.NextFreeIndex += count;
}

ID3D12DescriptorHeap* D3D12DescriptorManager::GetDescriptorHeap(D3D12DescriptorHeapType type) const
{
	return mHeaps[(u32)type].Heap.Get();
}
