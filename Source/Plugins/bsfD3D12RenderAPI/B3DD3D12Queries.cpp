//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Queries.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuQueryPool::D3D12GpuQueryPool(D3D12GpuDevice& device, const GpuQueryPoolCreateInformation& createInformation)
	: GpuQueryPool(createInformation)
	, mDevice(device)
	, mPipelineStatsBits(createInformation.PipelineStatisticsQueryBits)
{
	// Determine D3D12 query type and heap type
	switch (createInformation.Type)
	{
	case GpuQueryType::Timestamp:
		mD3D12QueryType = D3D12_QUERY_TYPE_TIMESTAMP;
		mD3D12QueryHeapType = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		mElementsPerQuery = 1;
		break;

	case GpuQueryType::Occlusion:
		mD3D12QueryType = D3D12_QUERY_TYPE_OCCLUSION;
		mD3D12QueryHeapType = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		mElementsPerQuery = 1;
		break;

	case GpuQueryType::PipelineStatistics:
		mD3D12QueryType = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
		mD3D12QueryHeapType = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;

		// Count the number of enabled statistics bits
		mElementsPerQuery = 0;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::PrimitiveCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::FragmentShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ComputeShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingGeneratedPrimitiveCount))
			mElementsPerQuery++;

		// D3D12 pipeline statistics queries return all 11 statistics as D3D12_QUERY_DATA_PIPELINE_STATISTICS
		// We'll need to extract only the ones requested
		break;

	default:
		B3D_LOG(Error, LogRenderBackend, "Unsupported query type");
		return;
	}

	CreateQueryHeap();

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 query pool: type={0}, size={1}", (u32)createInformation.Type, mPoolSize);
}

D3D12GpuQueryPool::~D3D12GpuQueryPool()
{
	if (mReadbackAllocation)
	{
		mReadbackAllocation->Release();
		mReadbackAllocation = nullptr;
	}

	mQueryHeap.Reset();
	mReadbackBuffer.Reset();
}

void D3D12GpuQueryPool::CreateQueryHeap()
{
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

	// Create query heap
	D3D12_QUERY_HEAP_DESC heapDesc = {};
	heapDesc.Type = mD3D12QueryHeapType;
	heapDesc.Count = mPoolSize;
	heapDesc.NodeMask = 0;

	HRESULT hr = d3d12Device->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&mQueryHeap));
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 query heap");
		return;
	}

	// Create readback buffer for query results
	// Each query can have multiple elements (e.g., pipeline statistics)
	u64 bufferSize = 0;

	if (mQueryType == GpuQueryType::PipelineStatistics)
	{
		// Pipeline statistics queries return D3D12_QUERY_DATA_PIPELINE_STATISTICS structure
		bufferSize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) * mPoolSize;
	}
	else
	{
		// Timestamp and occlusion queries return u64
		bufferSize = sizeof(u64) * mPoolSize;
	}

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_READBACK;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = bufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	hr = mDevice.GetAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		&mReadbackAllocation,
		IID_PPV_ARGS(&mReadbackBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create query readback buffer");
		return;
	}
}

GpuQueryId D3D12GpuQueryPool::AllocateQuery()
{
	if (mNextQueryId >= mPoolSize)
	{
		B3D_LOG(Warning, LogRenderBackend, "Query pool exhausted, returning invalid query ID");
		return GpuQueryId();
	}

	return GpuQueryId(mNextQueryId++);
}

bool D3D12GpuQueryPool::TryResolve(bool wait)
{
	// In D3D12, query resolution happens on the command buffer via ResolveQueryData
	// The readback buffer must be mapped to read the results
	// For now, we assume the resolve has been called from the command buffer
	// and we just need to check if the data is available

	if (wait)
	{
		// Wait for the GPU to finish (this is a simple implementation)
		// In production, you would want to wait on a fence
		mDevice.WaitUntilIdle();
		mResolved = true;
	}

	return mResolved;
}

u64 D3D12GpuQueryPool::GetQueryResult(GpuQueryId queryId, u32 elementIndex)
{
	if (!queryId.IsValid() || queryId.Id >= mNextQueryId)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query ID: {0}", queryId.Id);
		return 0;
	}

	if (!mResolved)
	{
		B3D_LOG(Warning, LogRenderBackend, "Attempting to read query results before resolve");
		return 0;
	}

	// Map the readback buffer
	void* mappedData = nullptr;
	D3D12_RANGE readRange = { 0, 0 };

	if (mQueryType == GpuQueryType::PipelineStatistics)
	{
		readRange.Begin = queryId.Id * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
		readRange.End = (queryId.Id + 1) * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
	}
	else
	{
		readRange.Begin = queryId.Id * sizeof(u64);
		readRange.End = (queryId.Id + 1) * sizeof(u64);
	}

	HRESULT hr = mReadbackBuffer->Map(0, &readRange, &mappedData);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to map query readback buffer");
		return 0;
	}

	u64 result = 0;

	if (mQueryType == GpuQueryType::PipelineStatistics)
	{
		// Extract the requested statistic from the pipeline statistics structure
		D3D12_QUERY_DATA_PIPELINE_STATISTICS* stats = static_cast<D3D12_QUERY_DATA_PIPELINE_STATISTICS*>(mappedData) + queryId.Id;

		// Map element index to the specific statistic based on enabled bits
		u32 currentElement = 0;
		if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexCount) && currentElement++ == elementIndex)
			result = stats->IAVertices;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::PrimitiveCount) && currentElement++ == elementIndex)
			result = stats->IAPrimitives;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexShaderInvocationCount) && currentElement++ == elementIndex)
			result = stats->VSInvocations;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::FragmentShaderInvocationCount) && currentElement++ == elementIndex)
			result = stats->PSInvocations;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ComputeShaderInvocationCount) && currentElement++ == elementIndex)
			result = stats->CSInvocations;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingInvocationCount) && currentElement++ == elementIndex)
			result = stats->CInvocations;
		else if (mPipelineStatsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingGeneratedPrimitiveCount) && currentElement++ == elementIndex)
			result = stats->CPrimitives;
	}
	else
	{
		// For timestamp and occlusion queries, just read the u64 value
		u64* results = static_cast<u64*>(mappedData);
		result = results[queryId.Id];
	}

	D3D12_RANGE writtenRange = { 0, 0 };
	mReadbackBuffer->Unmap(0, &writtenRange);

	return result;
}

D3D12EventQuery::D3D12EventQuery(GpuDevice& device)
	: EventQuery(device)
{
	D3D12GpuDevice& d3d12Device = static_cast<D3D12GpuDevice&>(device);
	ID3D12Device* d3d12DevicePtr = d3d12Device.GetD3D12Device();

	// Create a D3D12 fence for the event query
	HRESULT hr = d3d12DevicePtr->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 fence for event query");
	}
}

D3D12EventQuery::~D3D12EventQuery()
{
	mFence.Reset();
}

void D3D12EventQuery::Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer)
{
	// Increment fence value and signal from the command buffer
	mFenceValue++;

	// TODO: Signal fence from command buffer when it's executed
	// For now, this is a placeholder implementation
}

bool D3D12EventQuery::IsReady() const
{
	if (!mFence)
		return false;

	// Check if the fence has been signaled
	return mFence->GetCompletedValue() >= mFenceValue;
}

D3D12TimerQuery::D3D12TimerQuery(GpuDevice& device)
	: TimerQuery(device)
{
	// TODO: Implement using query pools
}

D3D12TimerQuery::~D3D12TimerQuery()
{
}

void D3D12TimerQuery::Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer)
{
	// TODO: Implement using query pools
	mIsReady = false;
}

void D3D12TimerQuery::End(const SPtr<render::GpuCommandBuffer>& commandBuffer)
{
	// TODO: Implement using query pools
}

bool D3D12TimerQuery::IsReady() const
{
	return mIsReady;
}

float D3D12TimerQuery::GetTimeInMilliseconds()
{
	// TODO: Implement using query pools
	return 0.0f;
}

D3D12OcclusionQuery::D3D12OcclusionQuery(bool isBinary, GpuDevice& device)
	: OcclusionQuery(isBinary, device)
	, mIsBinary(isBinary)
{
	// TODO: Implement using query pools
}

D3D12OcclusionQuery::~D3D12OcclusionQuery()
{
}

void D3D12OcclusionQuery::Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer)
{
	// TODO: Implement using query pools
	mIsReady = false;
}

void D3D12OcclusionQuery::End(const SPtr<render::GpuCommandBuffer>& commandBuffer)
{
	// TODO: Implement using query pools
}

bool D3D12OcclusionQuery::IsReady() const
{
	return mIsReady;
}

u32 D3D12OcclusionQuery::GetNumSamples()
{
	// TODO: Implement using query pools
	return 0;
}
