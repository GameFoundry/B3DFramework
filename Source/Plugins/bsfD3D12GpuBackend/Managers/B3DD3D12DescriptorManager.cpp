//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12DescriptorManager.h"
#include "B3DD3D12GpuDevice.h"

using namespace b3d;
using namespace b3d::render;

D3D12DescriptorManager::D3D12DescriptorManager(D3D12GpuDevice& device)
	: mDevice(device)
{
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	mDescriptorSizes[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV] = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mDescriptorSizes[(u32)D3D12DescriptorHeapType::Sampler] = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	mDescriptorSizes[(u32)D3D12DescriptorHeapType::RTV] = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDescriptorSizes[(u32)D3D12DescriptorHeapType::DSV] = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CreateHeaps();

	// Create the default sampler descriptor unset sampler bindings fall back to (see GetDefaultSamplerCPUHandle())
	mDefaultSamplerHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::Sampler);
	if (mDefaultSamplerHandle.ptr != 0)
	{
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		d3d12Device->CreateSampler(&samplerDesc, mDefaultSamplerHandle);
	}

	CreateNullDescriptors();
}

void D3D12DescriptorManager::CreateNullDescriptors()
{
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

	// The formats are arbitrary (null descriptors read as zero regardless) but must be valid for the view dimension
	mNullCBVHandle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
	if (mNullCBVHandle.ptr != 0)
		d3d12Device->CreateConstantBufferView(nullptr, mNullCBVHandle);

	const D3D12_SRV_DIMENSION srvDimensions[] = {
		D3D12_SRV_DIMENSION_BUFFER, D3D12_SRV_DIMENSION_TEXTURE1D, D3D12_SRV_DIMENSION_TEXTURE1DARRAY,
		D3D12_SRV_DIMENSION_TEXTURE2D, D3D12_SRV_DIMENSION_TEXTURE2DARRAY, D3D12_SRV_DIMENSION_TEXTURE2DMS,
		D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY, D3D12_SRV_DIMENSION_TEXTURE3D, D3D12_SRV_DIMENSION_TEXTURECUBE,
		D3D12_SRV_DIMENSION_TEXTURECUBEARRAY
	};

	for (D3D12_SRV_DIMENSION dimension : srvDimensions)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if (handle.ptr == 0)
			continue;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dimension == D3D12_SRV_DIMENSION_BUFFER ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = dimension;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		d3d12Device->CreateShaderResourceView(nullptr, &srvDesc, handle);
		mNullSRVHandles[dimension] = handle;
	}

	const D3D12_UAV_DIMENSION uavDimensions[] = {
		D3D12_UAV_DIMENSION_BUFFER, D3D12_UAV_DIMENSION_TEXTURE1D, D3D12_UAV_DIMENSION_TEXTURE1DARRAY,
		D3D12_UAV_DIMENSION_TEXTURE2D, D3D12_UAV_DIMENSION_TEXTURE2DARRAY, D3D12_UAV_DIMENSION_TEXTURE3D
	};

	for (D3D12_UAV_DIMENSION dimension : uavDimensions)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
		if (handle.ptr == 0)
			continue;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = dimension == D3D12_UAV_DIMENSION_BUFFER ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = dimension;

		d3d12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, handle);
		mNullUAVHandles[dimension] = handle;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::GetNullSRVHandle(D3D12_SRV_DIMENSION dimension) const
{
	if ((u32)dimension >= (u32)std::size(mNullSRVHandles))
		return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

	return mNullSRVHandles[dimension];
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::GetNullUAVHandle(D3D12_UAV_DIMENSION dimension) const
{
	if ((u32)dimension >= (u32)std::size(mNullUAVHandles))
		return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

	return mNullUAVHandles[dimension];
}

void D3D12DescriptorManager::CreateHeaps()
{
	ID3D12Device* device = mDevice.GetD3D12Device();

	auto fnCreateHeap = [device](DescriptorHeap& outHeap, D3D12_DESCRIPTOR_HEAP_TYPE nativeType, u32 descriptorCount, bool shaderVisible, const char* debugName)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = nativeType;
		heapDesc.NumDescriptors = descriptorCount;
		heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		const HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&outHeap.Heap));
		B3D_ASSERT(SUCCEEDED(hr) && debugName);

		outHeap.CPUStart = outHeap.Heap->GetCPUDescriptorHandleForHeapStart();
		if (shaderVisible)
			outHeap.GPUStart = outHeap.Heap->GetGPUDescriptorHandleForHeapStart();

		outHeap.DescriptorCount = descriptorCount;
	};

	// The CBV/SRV/UAV heap is ring-allocated (see AllocateGPUDescriptorRange()), so it is sized generously enough that
	// wrap-around only ever reaches ranges many frames old.
	fnCreateHeap(mHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256 * 1024, true, "Failed to create CBV/SRV/UAV descriptor heap");
	fnCreateHeap(mHeaps[(u32)D3D12DescriptorHeapType::Sampler], D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048, true, "Failed to create Sampler descriptor heap");
	fnCreateHeap(mHeaps[(u32)D3D12DescriptorHeapType::RTV], D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256, false, "Failed to create RTV descriptor heap");
	fnCreateHeap(mHeaps[(u32)D3D12DescriptorHeapType::DSV], D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256, false, "Failed to create DSV descriptor heap");

	// Resource views are created into CPU-only staging heaps, which must be separate from the shader-visible heaps
	// above: parameter sets copy staged descriptors into their shader-visible ranges, and shader-visible heaps cannot
	// be a descriptor copy source. Every live resource view descriptor lives in these.
	fnCreateHeap(mStagingHeaps[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192, false, "Failed to create CBV/SRV/UAV staging descriptor heap");
	fnCreateHeap(mStagingHeaps[(u32)D3D12DescriptorHeapType::Sampler], D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048, false, "Failed to create Sampler staging descriptor heap");
}

D3D12DescriptorManager::DescriptorHeap& D3D12DescriptorManager::GetStagingHeap(D3D12DescriptorHeapType type)
{
	if (type == D3D12DescriptorHeapType::CBV_SRV_UAV || type == D3D12DescriptorHeapType::Sampler)
		return mStagingHeaps[(u32)type];

	// RTV/DSV heaps are CPU-only already
	return mHeaps[(u32)type];
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorManager::AllocateCPUDescriptor(D3D12DescriptorHeapType type)
{
	DescriptorHeap& heap = GetStagingHeap(type);

	if (!heap.FreeList.empty())
	{
		const u32 index = heap.FreeList.back();
		heap.FreeList.pop_back();

		D3D12_CPU_DESCRIPTOR_HANDLE handle = heap.CPUStart;
		handle.ptr += index * mDescriptorSizes[(u32)type];
		return handle;
	}

	if (heap.NextFreeIndex >= heap.DescriptorCount)
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
	DescriptorHeap& heap = GetStagingHeap(type);

	const SIZE_T offset = handle.ptr - heap.CPUStart.ptr;
	heap.FreeList.push_back((u32)(offset / mDescriptorSizes[(u32)type]));
}

void D3D12DescriptorManager::AllocateGPUDescriptorRange(D3D12DescriptorHeapType type, u32 count, D3D12_CPU_DESCRIPTOR_HANDLE& outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGPUHandle)
{
	// Only CBV_SRV_UAV and Sampler heaps are GPU-visible
	B3D_ASSERT(type == D3D12DescriptorHeapType::CBV_SRV_UAV || type == D3D12DescriptorHeapType::Sampler);

	DescriptorHeap& heap = mHeaps[(u32)type];

	if (!B3D_ENSURE(count <= heap.DescriptorCount))
	{
		outCPUHandle.ptr = 0;
		outGPUHandle.ptr = 0;
		return;
	}

	// Ring allocation: ranges are written once and consumed by the GPU at execution time, so allocation wraps to
	// the heap start when full. The heap is sized so a wrap only reaches ranges many frames old.
	// TODO(d3d12-port): fence-guard the wrap so it provably cannot reach a range the GPU may still consume.
	if (heap.NextFreeIndex + count > heap.DescriptorCount)
	{
		B3D_LOG(Verbose, LogRenderBackend, "GPU descriptor heap for type {0} wrapped after {1} descriptors.", (u32)type, heap.NextFreeIndex);
		heap.NextFreeIndex = 0;
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
