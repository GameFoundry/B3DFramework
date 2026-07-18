//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Queries.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

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
	// Disconnect from any command buffer still tracking this pool, so its OnDidComplete callback doesn't touch a
	// destroyed pool.
	if (mResolveConnection)
		mResolveConnection.Disconnect();

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

void D3D12GpuQueryPool::NotifyPoolReset()
{
	if (mResolveConnection)
		mResolveConnection.Disconnect();

	mNextQueryId = 0;
	mResolved.store(false, std::memory_order_relaxed);
}

void D3D12GpuQueryPool::NotifyResolveScheduled(GpuCommandBuffer& commandBuffer)
{
	// Drop any prior subscription - the pool may be re-resolved after a reset.
	if (mResolveConnection)
		mResolveConnection.Disconnect();

	// Capture a raw pointer to the atomic flag; the connection is disconnected in the destructor and on reset, so
	// the flag outlives every invocation of this callback.
	std::atomic<bool>* resolvedFlag = &mResolved;
	mResolveConnection = commandBuffer.OnDidComplete.Connect([resolvedFlag]()
	{
		resolvedFlag->store(true, std::memory_order_release);
	});
}

bool D3D12GpuQueryPool::TryResolve(bool wait)
{
	// Query results are copied into the readback buffer by the ResolveQueryData() the command buffer records when
	// it ends, and are readable once that command buffer completes on the GPU (which flips mResolved).
	if (mNextQueryId == 0)
		return true;

	if (mResolved.load(std::memory_order_acquire))
		return true;

	if (!wait)
		return false;

	mDevice.WaitUntilIdle();
	mResolved.store(true, std::memory_order_relaxed);
	return true;
}

u64 D3D12GpuQueryPool::GetQueryResult(GpuQueryId queryId, u32 elementIndex)
{
	if (!queryId.IsValid() || queryId.Id >= mNextQueryId)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query ID: {0}", queryId.Id);
		return 0;
	}

	if (!mResolved.load(std::memory_order_acquire))
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

D3D12EventQuery::D3D12EventQuery(D3D12GpuDevice& device)
	: EventQuery()
	, mDevice(device)
{
}

D3D12EventQuery::~D3D12EventQuery()
{
	// Disconnect from any command buffer still tracking this query, so its OnDidComplete callback doesn't touch a
	// destroyed query.
	if (mCompleteConnection)
		mCompleteConnection.Disconnect();
}

void D3D12EventQuery::Begin(GpuCommandBuffer& commandBuffer)
{
	// Drop any prior subscription and reset readiness - Begin may be called to re-use a query.
	if (mCompleteConnection)
		mCompleteConnection.Disconnect();

	mReady.store(false, std::memory_order_relaxed);

	// OnDidComplete fires (on the command buffer's owning thread) once the command buffer finishes executing on the
	// GPU. That's at or after the point the query was scheduled, which satisfies the EventQuery contract. Capture a
	// raw pointer to the atomic flag; the connection is disconnected in the destructor and on re-Begin, so the flag
	// outlives every invocation of this callback.
	std::atomic<bool>* readyFlag = &mReady;
	mCompleteConnection = commandBuffer.OnDidComplete.Connect([readyFlag]()
	{
		readyFlag->store(true, std::memory_order_release);
	});
}

bool D3D12EventQuery::IsReady() const
{
	return mReady.load(std::memory_order_acquire);
}
