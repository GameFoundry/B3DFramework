//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuQueue.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuBuffer::D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device)
	: GpuBuffer(createInformation, device)
{
}

D3D12GpuBuffer::~D3D12GpuBuffer()
{
	// Make sure buffer is unmapped before destruction
	if (mMappedData)
	{
		Unmap();
	}

	// Release D3D12 resources
	if (mAllocation)
	{
		mAllocation->Release();
		mAllocation = nullptr;
	}

	mBuffer.Reset();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::Initialize()
{
	GpuBuffer::Initialize();

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	const GpuBufferProperties& props = GetProperties();

	// Determine heap type based on usage
	D3D12_HEAP_TYPE heapType;
	D3D12_RESOURCE_STATES initialState;

	if (props.Usage == GBU_STATIC)
	{
		// Static buffers use default heap (GPU-only memory)
		heapType = D3D12_HEAP_TYPE_DEFAULT;
		initialState = D3D12_RESOURCE_STATE_COMMON;
	}
	else if (props.Usage == GBU_DYNAMIC)
	{
		// Dynamic buffers use upload heap (CPU-writable, GPU-readable)
		heapType = D3D12_HEAP_TYPE_UPLOAD;
		initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else // GBU_READBACK
	{
		// Readback buffers use readback heap (CPU-readable, GPU-writable)
		heapType = D3D12_HEAP_TYPE_READBACK;
		initialState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	// Create heap properties
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = heapType;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	// Create resource description
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = props.ElementSize * props.ElementCount;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// Set resource flags based on buffer type
	if (props.Type == GBT_STORAGE)
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	// Create the buffer resource using D3D12MA allocator
	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = heapType;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = device.GetAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		initialState,
		nullptr, // No clear value for buffers
		&mAllocation,
		IID_PPV_ARGS(&mBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, RenderBackend, "Failed to create D3D12 buffer resource");
		return;
	}

	// Create vertex buffer view if this is a vertex buffer
	if (props.Type == GBT_VERTEX)
	{
		mVertexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = (UINT)(props.ElementSize * props.ElementCount);
		mVertexBufferView.StrideInBytes = (UINT)props.ElementSize;
	}

	// Create index buffer view if this is an index buffer
	if (props.Type == GBT_INDEX)
	{
		mIndexBufferView.BufferLocation = mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = (UINT)(props.ElementSize * props.ElementCount);

		// Determine index format (16-bit or 32-bit)
		if (props.ElementSize == 2)
			mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
		else if (props.ElementSize == 4)
			mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		else
		{
			B3D_LOG(Error, RenderBackend, "Invalid index buffer element size: {0}", props.ElementSize);
			mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		}
	}

	B3D_LOG(Info, RenderBackend, "Created D3D12 buffer: size={0} bytes, type={1}, usage={2}",
		props.ElementSize * props.ElementCount, (u32)props.Type, (u32)props.Usage);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_VertexBuffer);
}

D3D12_GPU_VIRTUAL_ADDRESS D3D12GpuBuffer::GetGPUVirtualAddress() const
{
	if (mBuffer)
		return mBuffer->GetGPUVirtualAddress();

	return 0;
}

void* D3D12GpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options)
{
	if (!mBuffer)
		return nullptr;

	const GpuBufferProperties& props = GetProperties();

	// Only upload and readback heaps can be mapped
	if (props.Usage == GBU_STATIC)
	{
		B3D_LOG(Error, RenderBackend, "Cannot map static buffer - use dynamic or readback buffer instead");
		return nullptr;
	}

	// D3D12 requires mapping the entire resource
	D3D12_RANGE readRange = {};
	if (props.Usage == GBU_READBACK)
	{
		// For readback, specify the range we want to read
		readRange.Begin = offset;
		readRange.End = offset + length;
	}
	else
	{
		// For upload (dynamic), we don't read so the range is empty
		readRange.Begin = 0;
		readRange.End = 0;
	}

	HRESULT hr = mBuffer->Map(0, &readRange, &mMappedData);
	if (FAILED(hr))
	{
		B3D_LOG(Error, RenderBackend, "Failed to map D3D12 buffer");
		return nullptr;
	}

	// Return pointer offset by the requested offset
	return static_cast<u8*>(mMappedData) + offset;
}

void D3D12GpuBuffer::Unmap()
{
	if (!mBuffer || !mMappedData)
		return;

	const GpuBufferProperties& props = GetProperties();

	// For upload buffers, we need to specify what range was written
	D3D12_RANGE writtenRange = {};
	if (props.Usage == GBU_DYNAMIC)
	{
		// Indicate the entire buffer was potentially written
		writtenRange.Begin = 0;
		writtenRange.End = props.ElementSize * props.ElementCount;
	}
	else
	{
		// For readback, no data was written
		writtenRange.Begin = 0;
		writtenRange.End = 0;
	}

	mBuffer->Unmap(0, &writtenRange);
	mMappedData = nullptr;
}

void D3D12GpuBuffer::WriteData(u32 offset, u32 length, const void* source, GpuBufferWriteFlags flags)
{
	if (!mBuffer || !source)
		return;

	const GpuBufferProperties& props = GetProperties();

	if (props.Usage == GBU_DYNAMIC)
	{
		// For dynamic buffers, we can directly map and write
		void* dest = Map(offset, length, GBL_WRITE_ONLY_NO_OVERWRITE);
		if (dest)
		{
			memcpy(dest, source, length);
			Unmap();
		}
	}
	else if (props.Usage == GBU_STATIC)
	{
		// For static buffers, we need to use an intermediate upload buffer
		D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
		ID3D12Device* d3d12Device = device.GetD3D12Device();

		// Create temporary upload buffer
		D3D12_HEAP_PROPERTIES uploadHeapProps = {};
		uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		uploadHeapProps.CreationNodeMask = 0;
		uploadHeapProps.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC uploadDesc = {};
		uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		uploadDesc.Alignment = 0;
		uploadDesc.Width = length;
		uploadDesc.Height = 1;
		uploadDesc.DepthOrArraySize = 1;
		uploadDesc.MipLevels = 1;
		uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
		uploadDesc.SampleDesc.Count = 1;
		uploadDesc.SampleDesc.Quality = 0;
		uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPtr<ID3D12Resource> uploadBuffer;
		D3D12MA::Allocation* uploadAllocation = nullptr;

		D3D12MA::ALLOCATION_DESC uploadAllocDesc = {};
		uploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		uploadAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

		HRESULT hr = device.GetAllocator()->CreateResource(
			&uploadAllocDesc,
			&uploadDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&uploadAllocation,
			IID_PPV_ARGS(&uploadBuffer)
		);

		if (FAILED(hr))
		{
			B3D_LOG(Error, RenderBackend, "Failed to create upload buffer for WriteData");
			return;
		}

		// Map and copy data to upload buffer
		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We won't read from this buffer
		hr = uploadBuffer->Map(0, &readRange, &mappedData);

		if (FAILED(hr))
		{
			B3D_LOG(Error, RenderBackend, "Failed to map upload buffer");
			return;
		}

		memcpy(mappedData, source, length);

		D3D12_RANGE writtenRange = { 0, length };
		uploadBuffer->Unmap(0, &writtenRange);

		// Get or create transfer command buffer for the current thread
		GpuQueue& queue = *device.GetQueue(GQT_GRAPHICS, 0);
		SPtr<render::GpuCommandBuffer> transferCommandBuffer = device.GetTransferBufferHelper().GetOrCreateTransferCommandBuffer(queue);
		D3D12GpuCommandBuffer* d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer*>(transferCommandBuffer.get());

		// Record copy commands on the transfer command buffer
		d3d12CommandBuffer->TransitionResource(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		d3d12CommandBuffer->CopyBufferToBuffer(uploadBuffer.Get(), mBuffer.Get(), 0, offset, length);
		d3d12CommandBuffer->TransitionResource(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

		// Upload buffer and allocation can be destroyed immediately - they're tracked by the transfer command buffer
		// The transfer buffer will be automatically submitted before next normal command buffer submission
		if (uploadAllocation)
			uploadAllocation->Release();
	}
	else
	{
		B3D_LOG(Error, RenderBackend, "Cannot write to readback buffer");
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::ReadData(u32 offset, u32 length, void* dest)
{
	if (!mBuffer || !dest)
		return;

	const GpuBufferProperties& props = GetProperties();

	if (props.Usage == GBU_READBACK)
	{
		// For readback buffers, we can directly map and read
		void* src = Map(offset, length, GBL_READ_ONLY);
		if (src)
		{
			memcpy(dest, src, length);
			Unmap();
		}
	}
	else
	{
		// For other buffer types, we need to copy to a readback buffer first
		D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
		ID3D12Device* d3d12Device = device.GetD3D12Device();

		// Create temporary readback buffer
		D3D12_HEAP_PROPERTIES readbackHeapProps = {};
		readbackHeapProps.Type = D3D12_HEAP_TYPE_READBACK;
		readbackHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		readbackHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		readbackHeapProps.CreationNodeMask = 0;
		readbackHeapProps.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC readbackDesc = {};
		readbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		readbackDesc.Alignment = 0;
		readbackDesc.Width = length;
		readbackDesc.Height = 1;
		readbackDesc.DepthOrArraySize = 1;
		readbackDesc.MipLevels = 1;
		readbackDesc.Format = DXGI_FORMAT_UNKNOWN;
		readbackDesc.SampleDesc.Count = 1;
		readbackDesc.SampleDesc.Quality = 0;
		readbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPtr<ID3D12Resource> readbackBuffer;
		D3D12MA::Allocation* readbackAllocation = nullptr;

		D3D12MA::ALLOCATION_DESC readbackAllocDesc = {};
		readbackAllocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		readbackAllocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

		HRESULT hr = device.GetAllocator()->CreateResource(
			&readbackAllocDesc,
			&readbackDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			&readbackAllocation,
			IID_PPV_ARGS(&readbackBuffer)
		);

		if (FAILED(hr))
		{
			B3D_LOG(Error, RenderBackend, "Failed to create readback buffer for ReadData");
			return;
		}

		// Get or create transfer command buffer for the current thread
		GpuTransferBufferHelper& transferHelper = device.GetTransferBufferHelper();
		GpuQueue& queue = *device.GetQueue(GQT_GRAPHICS, 0);
		SPtr<render::GpuCommandBuffer> transferCommandBuffer = transferHelper.GetOrCreateTransferCommandBuffer(queue);
		D3D12GpuCommandBuffer* d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer*>(transferCommandBuffer.get());

		// Record copy commands on the transfer command buffer
		d3d12CommandBuffer->TransitionResource(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
		d3d12CommandBuffer->CopyBufferToBuffer(mBuffer.Get(), readbackBuffer.Get(), offset, 0, length);
		d3d12CommandBuffer->TransitionResource(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

		// For ReadData, we must wait for completion since the caller needs the data
		transferHelper.SubmitTransferCommandBuffer(queue, true);

		// Map readback buffer and copy to destination
		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, length };
		hr = readbackBuffer->Map(0, &readRange, &mappedData);

		if (FAILED(hr))
		{
			B3D_LOG(Error, RenderBackend, "Failed to map readback buffer");
			return;
		}

		memcpy(dest, mappedData, length);

		D3D12_RANGE writtenRange = { 0, 0 }; // We didn't write anything
		readbackBuffer->Unmap(0, &writtenRange);

		// Release readback allocation
		if (readbackAllocation)
			readbackAllocation->Release();
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_VertexBuffer);
}

void D3D12GpuBuffer::CopyData(GpuBuffer& destination, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer)
{
	// TODO: Implement buffer-to-buffer copy
	// This requires:
	// 1. Get D3D12 resources for both source and destination
	// 2. Create a command list for the copy operation
	// 3. Use CopyBufferRegion to copy between buffers
	// 4. Execute the command list

	B3D_LOG(Warning, RenderBackend, "D3D12GpuBuffer::CopyData not yet implemented");
}

void D3D12GpuBuffer::Flush(u32 offset, u32 size)
{
	// D3D12 uses coherent memory by default, no explicit flush needed
}

void D3D12GpuBuffer::Invalidate(u32 offset, u32 size)
{
	// D3D12 uses coherent memory by default, no explicit invalidate needed
}
