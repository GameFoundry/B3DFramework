//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12DescriptorHeap.h"

using namespace b3d;
using namespace b3d::render;

bool D3D12DescriptorHeap::Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, bool isShaderVisible, u32 tableRegionStart)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = type;
	heapDesc.NumDescriptors = capacity;
	heapDesc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ComPtr<ID3D12DescriptorHeap> heap;
	if(FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap))))
		return false;

	mHeap = std::move(heap);
	mType = type;
	mCPUStart = mHeap->GetCPUDescriptorHandleForHeapStart();
	mGPUStart = isShaderVisible ? mHeap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};
	mCapacity = capacity;
	mDescriptorSize = device->GetDescriptorHandleIncrementSize(type);
	mTableRegionStart = tableRegionStart;

	return true;
}
