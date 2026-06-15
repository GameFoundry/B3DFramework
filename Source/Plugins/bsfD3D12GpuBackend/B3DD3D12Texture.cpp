//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12Utility.h"
#include "Profiling/B3DRenderStats.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include <algorithm>

using namespace b3d;
using namespace b3d::render;

D3D12Texture::D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device)
	: Texture(createInformation, device)
	, mDXGIFormat(DXGI_FORMAT_UNKNOWN)
{
}

D3D12Texture::~D3D12Texture()
{
	// Release staging buffer if mapped
	if (mMappedData)
	{
		Unmap(0, 0);
	}

	// Release D3D12 resources
	if (mAllocation)
	{
		mAllocation->Release();
		mAllocation = nullptr;
	}

	mTexture.Reset();
	mStagingBuffer.Reset();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}


void D3D12Texture::Initialize()
{
	Texture::Initialize();

	CreateTexture();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
}

void D3D12Texture::CreateTexture()
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	const TextureProperties& props = GetProperties();

	// Convert pixel format to DXGI format
	mDXGIFormat = D3D12Utility::GetDXGIFormat(props.Format);
	if (mDXGIFormat == DXGI_FORMAT_UNKNOWN)
	{
		B3D_LOG(Error, LogRenderBackend, "Unsupported texture format");
		return;
	}

	// Determine resource dimension
	D3D12_RESOURCE_DIMENSION dimension;
	u32 arraySize = 1;

	switch (props.TextureType)
	{
	case TEX_TYPE_1D:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		break;
	case TEX_TYPE_2D:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case TEX_TYPE_3D:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	case TEX_TYPE_CUBE_MAP:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		arraySize = 6;
		break;
	case TEX_TYPE_1D_ARRAY:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		arraySize = props.NumFaces;
		break;
	case TEX_TYPE_2D_ARRAY:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		arraySize = props.NumFaces;
		break;
	case TEX_TYPE_CUBE_MAP_ARRAY:
		dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		arraySize = props.NumFaces * 6;
		break;
	default:
		B3D_LOG(Error, LogRenderBackend, "Unsupported texture type");
		return;
	}

	// Create resource description
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = dimension;
	resourceDesc.Alignment = 0; // Let D3D12 choose appropriate alignment
	resourceDesc.Width = props.Width;
	resourceDesc.Height = props.Height;
	resourceDesc.DepthOrArraySize = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? props.Depth : arraySize;
	resourceDesc.MipLevels = props.NumMipmaps;
	resourceDesc.Format = mDXGIFormat;
	resourceDesc.SampleDesc.Count = props.MultisampleCount > 0 ? props.MultisampleCount : 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// Set resource flags based on usage
	if (props.Usage.IsSet(TextureUsageFlag::RenderTarget))
	{
		if (PixelUtility::IsDepth(props.Format))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		else
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	if (props.Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	// Determine initial state
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
	if (props.Usage.IsSet(TextureUsageFlag::RenderTarget))
	{
		if (PixelUtility::IsDepth(props.Format))
			initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		else
			initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	// Create heap properties for GPU-only memory
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	// Determine clear value for render targets
	D3D12_CLEAR_VALUE clearValue = {};
	D3D12_CLEAR_VALUE* pClearValue = nullptr;

	if (props.Usage.IsSet(TextureUsageFlag::RenderTarget))
	{
		clearValue.Format = mDXGIFormat;
		if (PixelUtility::IsDepth(props.Format))
		{
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;
		}
		else
		{
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 0.0f;
		}
		pClearValue = &clearValue;
	}

	// Create the texture resource using D3D12MA allocator
	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		initialState,
		pClearValue,
		&mAllocation,
		IID_PPV_ARGS(&mTexture)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 texture resource");
		return;
	}

	// Set debug name if available
	if (!props.Name.empty())
	{
		WString wideName = ToWideString(props.Name);
		mTexture->SetName(wideName.c_str());
	}

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 texture: {0}x{1}, format={2}, mips={3}",
		props.Width, props.Height, (u32)mDXGIFormat, props.NumMipmaps);
}

u32 D3D12Texture::CalculateSubresourceIndex(u32 face, u32 mipLevel) const
{
	const TextureProperties& props = GetProperties();

	// D3D12 subresource index = MipLevel + (ArraySlice * MipLevels)
	return mipLevel + (face * props.NumMipmaps);
}

void D3D12Texture::CreateStagingBuffer(u32 subresourceIndex, u32 width, u32 height)
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Get the required buffer size for this subresource
	D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();

	u64 totalBytes = 0;
	u64 rowSizeInBytes = 0;
	u32 numRows = 0;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

	// Create staging buffer in upload heap for CPU-to-GPU, or readback heap for GPU-to-CPU
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_READBACK; // We'll use readback for both read and write
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = totalBytes;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::Allocation* stagingAllocation = nullptr;

	D3D12MA::ALLOCATION_DESC stagingAllocDesc = {};
	stagingAllocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
	stagingAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&stagingAllocDesc,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		&stagingAllocation,
		IID_PPV_ARGS(&mStagingBuffer)
	);

	// Release old staging allocation if exists (we don't track it separately)
	if (stagingAllocation)
		stagingAllocation->Release();

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create staging buffer for texture mapping");
		mStagingBuffer.Reset();
	}
}

PixelData D3D12Texture::Map(GpuResourceUsage usage, u32 face, u32 mipLevel)
{
	const TextureProperties& props = GetProperties();
	u32 subresourceIndex = CalculateSubresourceIndex(face, mipLevel);

	// Calculate mip dimensions
	u32 mipWidth = std::max(1u, props.Width >> mipLevel);
	u32 mipHeight = std::max(1u, props.Height >> mipLevel);

	// Create staging buffer if needed
	if (!mStagingBuffer || mMappedSubresource != subresourceIndex)
	{
		if (mStagingBuffer)
			mStagingBuffer.Reset();

		CreateStagingBuffer(subresourceIndex, mipWidth, mipHeight);
		if (!mStagingBuffer)
			return PixelData();
	}

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// If reading, we need to copy from the GPU texture to the staging buffer
	if (usage == GRU_READ || usage == GRU_READ_WRITE)
	{
		// TODO: This requires command buffer execution
		// For now, we'll create a temporary command list
		// In a production implementation, this should use a copy queue

		ComPtr<ID3D12CommandAllocator> tempAllocator;
		ComPtr<ID3D12GraphicsCommandList> tempCommandList;

		HRESULT hr = d3d12Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&tempAllocator)
		);

		if (FAILED(hr))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create temporary command allocator for texture mapping");
			return PixelData();
		}

		hr = d3d12Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			tempAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&tempCommandList)
		);

		if (FAILED(hr))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create temporary command list for texture mapping");
			return PixelData();
		}

		// Transition texture to copy source state
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = mTexture.Get();
		barrier.Transition.Subresource = subresourceIndex;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON; // TODO: Track actual state
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		tempCommandList->ResourceBarrier(1, &barrier);

		// Copy texture to staging buffer
		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = mTexture.Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = subresourceIndex;

		D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		u64 rowSizeInBytes, totalBytes;
		u32 numRows;
		d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = mStagingBuffer.Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLocation.PlacedFootprint = layout;

		tempCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

		// Transition back to common state
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		tempCommandList->ResourceBarrier(1, &barrier);

		tempCommandList->Close();

		// Execute the command list
		TShared<GpuQueue> queue = device.GetQueue(GQT_GRAPHICS, 0);
		D3D12GpuQueue* d3d12Queue = static_cast<D3D12GpuQueue*>(queue.get());
		ID3D12CommandQueue* commandQueue = d3d12Queue->GetD3D12Handle();

		ID3D12CommandList* commandLists[] = { tempCommandList.Get() };
		commandQueue->ExecuteCommandLists(1, commandLists);

		// Wait for copy to complete
		// TODO: Use fence for proper synchronization
		ComPtr<ID3D12Fence> fence;
		hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		if (SUCCEEDED(hr))
		{
			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			commandQueue->Signal(fence.Get(), 1);
			fence->SetEventOnCompletion(1, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}
	}

	// Map the staging buffer
	D3D12_RANGE readRange = { 0, 0 };
	if (usage == GRU_READ || usage == GRU_READ_WRITE)
	{
		// Read the entire buffer
		readRange.End = (SIZE_T)mStagingBuffer->GetDesc().Width;
	}

	HRESULT hr = mStagingBuffer->Map(0, &readRange, &mMappedData);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to map staging buffer");
		return PixelData();
	}

	mMappedSubresource = subresourceIndex;

	// Get layout information
	D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	u64 rowSizeInBytes, totalBytes;
	u32 numRows;
	d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

	// Create PixelData
	PixelVolume volume(0, 0, 0, mipWidth, mipHeight, 1);
	PixelData pixelData(volume, props.Format);
	pixelData.SetExternalBuffer((u8*)mMappedData);
	pixelData.SetRowPitch((u32)layout.Footprint.RowPitch);
	pixelData.SetSlicePitch((u32)(layout.Footprint.RowPitch * numRows));

	return pixelData;
}

void D3D12Texture::Unmap(u32 face, u32 mipLevel)
{
	if (!mStagingBuffer || !mMappedData)
		return;

	u32 subresourceIndex = CalculateSubresourceIndex(face, mipLevel);

	if (mMappedSubresource != subresourceIndex)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unmapping subresource that wasn't mapped");
		return;
	}

	// Unmap the staging buffer
	D3D12_RANGE writtenRange = { 0, (SIZE_T)mStagingBuffer->GetDesc().Width };
	mStagingBuffer->Unmap(0, &writtenRange);
	mMappedData = nullptr;

	// Copy from staging buffer back to GPU texture
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Create temporary command list for upload
	ComPtr<ID3D12CommandAllocator> tempAllocator;
	ComPtr<ID3D12GraphicsCommandList> tempCommandList;

	HRESULT hr = d3d12Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&tempAllocator)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create temporary command allocator for texture unmapping");
		return;
	}

	hr = d3d12Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		tempAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&tempCommandList)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create temporary command list for texture unmapping");
		return;
	}

	// We need to change staging buffer to upload heap for writing back
	// But since we created it as readback, we need to create a new upload buffer
	D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	u64 rowSizeInBytes, totalBytes;
	u32 numRows;
	d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

	// Create upload buffer
	ComPtr<ID3D12Resource> uploadBuffer;
	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC uploadBufferDesc = {};
	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadBufferDesc.Width = totalBytes;
	uploadBufferDesc.Height = 1;
	uploadBufferDesc.DepthOrArraySize = 1;
	uploadBufferDesc.MipLevels = 1;
	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadBufferDesc.SampleDesc.Count = 1;
	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::Allocation* unmapUploadAllocation = nullptr;

	D3D12MA::ALLOCATION_DESC unmapUploadAllocDesc = {};
	unmapUploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
	unmapUploadAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	hr = device.GetAllocator()->CreateResource(
		&unmapUploadAllocDesc,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		&unmapUploadAllocation,
		IID_PPV_ARGS(&uploadBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create upload buffer for texture unmapping");
		tempCommandList->Close();
		return;
	}

	// Copy staging buffer data to upload buffer
	void* uploadData = nullptr;
	hr = uploadBuffer->Map(0, nullptr, &uploadData);
	if (SUCCEEDED(hr))
	{
		void* stagingData = nullptr;
		D3D12_RANGE readRange = { 0, (SIZE_T)totalBytes };
		if (SUCCEEDED(mStagingBuffer->Map(0, &readRange, &stagingData)))
		{
			memcpy(uploadData, stagingData, (size_t)totalBytes);
			mStagingBuffer->Unmap(0, nullptr);
		}
		uploadBuffer->Unmap(0, nullptr);
	}

	// Transition texture to copy destination
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mTexture.Get();
	barrier.Transition.Subresource = subresourceIndex;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON; // TODO: Track actual state
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	tempCommandList->ResourceBarrier(1, &barrier);

	// Copy from upload buffer to texture
	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = uploadBuffer.Get();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint = layout;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = mTexture.Get();
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = subresourceIndex;

	tempCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	// Transition back to common state
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	tempCommandList->ResourceBarrier(1, &barrier);

	tempCommandList->Close();

	// Execute the command list
	TShared<GpuQueue> queue = device.GetQueue(GQT_GRAPHICS, 0);
	D3D12GpuQueue* d3d12Queue = static_cast<D3D12GpuQueue*>(queue.get());
	ID3D12CommandQueue* commandQueue = d3d12Queue->GetD3D12Handle();

	ID3D12CommandList* commandLists[] = { tempCommandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);

	// Wait for copy to complete
	ComPtr<ID3D12Fence> fence;
	hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (SUCCEEDED(hr))
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		commandQueue->Signal(fence.Get(), 1);
		fence->SetEventOnCompletion(1, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}

	// Release upload allocation
	if (unmapUploadAllocation)
		unmapUploadAllocation->Release();

	mMappedSubresource = (u32)-1;
}

void D3D12Texture::WriteData(const PixelData& data, u32 face, u32 mipLevel, bool discardEntireBuffer)
{
	if (!mTexture)
		return;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	const TextureProperties& props = GetProperties();
	u32 subresourceIndex = CalculateSubresourceIndex(face, mipLevel);

	// Get the texture layout information
	D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	u64 rowSizeInBytes, totalBytes;
	u32 numRows;
	d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

	// Create upload buffer
	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProps.CreationNodeMask = 0;
	uploadHeapProps.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC uploadBufferDesc = {};
	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadBufferDesc.Alignment = 0;
	uploadBufferDesc.Width = totalBytes;
	uploadBufferDesc.Height = 1;
	uploadBufferDesc.DepthOrArraySize = 1;
	uploadBufferDesc.MipLevels = 1;
	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadBufferDesc.SampleDesc.Count = 1;
	uploadBufferDesc.SampleDesc.Quality = 0;
	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> uploadBuffer;
	D3D12MA::Allocation* writeUploadAllocation = nullptr;

	D3D12MA::ALLOCATION_DESC writeUploadAllocDesc = {};
	writeUploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
	writeUploadAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&writeUploadAllocDesc,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		&writeUploadAllocation,
		IID_PPV_ARGS(&uploadBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create upload buffer for texture WriteData");
		return;
	}

	// Map the upload buffer and copy pixel data
	void* mappedData = nullptr;
	hr = uploadBuffer->Map(0, nullptr, &mappedData);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to map upload buffer for texture WriteData");
		return;
	}

	// Copy pixel data to upload buffer, accounting for D3D12's row pitch alignment
	const u8* srcData = data.GetData();
	u8* dstData = static_cast<u8*>(mappedData);

	u32 srcRowPitch = data.GetRowPitch();
	u32 dstRowPitch = (u32)layout.Footprint.RowPitch;

	// If row pitches match, we can do a single memcpy
	if (srcRowPitch == dstRowPitch && numRows > 0)
	{
		memcpy(dstData, srcData, srcRowPitch * numRows);
	}
	else
	{
		// Row pitches differ, copy row by row
		u32 rowDataSize = std::min(srcRowPitch, dstRowPitch);
		for (u32 row = 0; row < numRows; row++)
		{
			memcpy(dstData + row * dstRowPitch, srcData + row * srcRowPitch, rowDataSize);
		}
	}

	uploadBuffer->Unmap(0, nullptr);

	// Get or create transfer command buffer for the current thread
	TShared<render::GpuCommandBuffer> transferCommandBuffer = device.GetInternalWorkContext().GetTransferCommandBuffer();
	D3D12GpuCommandBuffer* d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer*>(transferCommandBuffer.get());

	// Record copy commands on the transfer command buffer
	d3d12CommandBuffer->TransitionResource(mTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST, subresourceIndex);
	d3d12CommandBuffer->CopyBufferToTexture(uploadBuffer.Get(), mTexture.Get(), layout, subresourceIndex);
	d3d12CommandBuffer->TransitionResource(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON, subresourceIndex);

	// Upload buffer and allocation can be destroyed immediately - they're tracked by the transfer command buffer
	// The transfer buffer will be automatically submitted before next normal command buffer submission
	if (writeUploadAllocation)
		writeUploadAllocation->Release();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
}

void D3D12Texture::ReadData(PixelData& data, u32 face, u32 mipLevel)
{
	if (!mTexture)
		return;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	const TextureProperties& props = GetProperties();
	u32 subresourceIndex = CalculateSubresourceIndex(face, mipLevel);

	// Get the texture layout information
	D3D12_RESOURCE_DESC textureDesc = mTexture->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	u64 rowSizeInBytes, totalBytes;
	u32 numRows;
	d3d12Device->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

	// Create readback buffer
	D3D12_HEAP_PROPERTIES readbackHeapProps = {};
	readbackHeapProps.Type = D3D12_HEAP_TYPE_READBACK;
	readbackHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	readbackHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	readbackHeapProps.CreationNodeMask = 0;
	readbackHeapProps.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC readbackBufferDesc = {};
	readbackBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	readbackBufferDesc.Alignment = 0;
	readbackBufferDesc.Width = totalBytes;
	readbackBufferDesc.Height = 1;
	readbackBufferDesc.DepthOrArraySize = 1;
	readbackBufferDesc.MipLevels = 1;
	readbackBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	readbackBufferDesc.SampleDesc.Count = 1;
	readbackBufferDesc.SampleDesc.Quality = 0;
	readbackBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	readbackBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> readbackBuffer;
	D3D12MA::Allocation* readbackAllocation = nullptr;

	D3D12MA::ALLOCATION_DESC readbackAllocDesc = {};
	readbackAllocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
	readbackAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&readbackAllocDesc,
		&readbackBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		&readbackAllocation,
		IID_PPV_ARGS(&readbackBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create readback buffer for texture ReadData");
		return;
	}

	// Get or create transfer command buffer for the current thread
	TShared<render::GpuCommandBuffer> transferCommandBuffer = device.GetInternalWorkContext().GetTransferCommandBuffer();
	D3D12GpuCommandBuffer* d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer*>(transferCommandBuffer.get());

	// Record copy commands on the transfer command buffer
	d3d12CommandBuffer->TransitionResource(mTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE, subresourceIndex);
	d3d12CommandBuffer->CopyTextureToBuffer(mTexture.Get(), readbackBuffer.Get(), layout, subresourceIndex);
	d3d12CommandBuffer->TransitionResource(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON, subresourceIndex);

	// For ReadData, we must wait for completion since the caller needs the data
	device.GetInternalWorkContext().SubmitTransferCommandBuffers(true);

	// Map the readback buffer and copy to PixelData
	void* mappedData = nullptr;
	D3D12_RANGE readRange = { 0, (SIZE_T)totalBytes };
	hr = readbackBuffer->Map(0, &readRange, &mappedData);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to map readback buffer for texture ReadData");
		return;
	}

	// Ensure the PixelData has allocated buffer
	u32 mipWidth = std::max(1u, props.Width >> mipLevel);
	u32 mipHeight = std::max(1u, props.Height >> mipLevel);

	if (data.GetData() == nullptr)
	{
		// Allocate internal buffer if not already allocated
		PixelVolume volume(0, 0, 0, mipWidth, mipHeight, 1);
		data = PixelData(volume, props.Format);
	}

	// Copy from readback buffer to PixelData, accounting for row pitch differences
	const u8* srcData = static_cast<const u8*>(mappedData);
	u8* dstData = data.GetData();

	u32 srcRowPitch = (u32)layout.Footprint.RowPitch;
	u32 dstRowPitch = data.GetRowPitch();

	// If row pitches match, we can do a single memcpy
	if (srcRowPitch == dstRowPitch && numRows > 0)
	{
		memcpy(dstData, srcData, srcRowPitch * numRows);
	}
	else
	{
		// Row pitches differ, copy row by row
		u32 rowDataSize = std::min(srcRowPitch, dstRowPitch);
		for (u32 row = 0; row < numRows; row++)
		{
			memcpy(dstData + row * dstRowPitch, srcData + row * srcRowPitch, rowDataSize);
		}
	}

	// Unmap the readback buffer
	readbackBuffer->Unmap(0, nullptr);

	// Release readback allocation
	if (readbackAllocation)
		readbackAllocation->Release();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_Texture);
}

void D3D12Texture::CopyData(Texture& destination, const PixelData& srcData, const PixelData& dstData)
{
	// TODO: Implement texture-to-texture copy
	// This requires:
	// 1. Get D3D12 resources for both source and destination
	// 2. Use CopyTextureRegion to copy between textures
	// 3. Handle resource state transitions

	B3D_LOG(Warning, LogRenderBackend, "D3D12Texture::CopyData not yet implemented");
}
