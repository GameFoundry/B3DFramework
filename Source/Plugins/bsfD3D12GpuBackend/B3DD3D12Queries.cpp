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
	, mPipelineStatisticsBits(createInformation.PipelineStatisticsQueryBits)
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

		// D3D12 always returns the full D3D12_QUERY_DATA_PIPELINE_STATISTICS structure, but only the requested
		// statistics are exposed as query elements (see GetQueryResult).
		mElementsPerQuery = 0;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::PrimitiveCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::FragmentShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ComputeShaderInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingInvocationCount))
			mElementsPerQuery++;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingGeneratedPrimitiveCount))
			mElementsPerQuery++;

		break;

	default:
		B3D_LOG(Error, LogRenderBackend, "Unsupported query type");
		return;
	}

	CreateQueryResources();

	B3D_LOG(Verbose, LogRenderBackend, "Created D3D12 query pool: type={0}, size={1}", (u32)createInformation.Type, mPoolSize);
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

void D3D12GpuQueryPool::CreateQueryResources()
{
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

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

	// Pipeline statistics resolve into a full statistics structure per query, every other type into a single u64.
	const u64 bufferSize = mQueryType == GpuQueryType::PipelineStatistics
		? sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) * mPoolSize
		: sizeof(u64) * mPoolSize;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

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

	hr = mDevice.GetAllocator()->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &mReadbackAllocation, IID_PPV_ARGS(&mReadbackBuffer));
	if (FAILED(hr))
		B3D_LOG(Error, LogRenderBackend, "Failed to create query readback buffer");
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
		const D3D12_QUERY_DATA_PIPELINE_STATISTICS* statistics = static_cast<const D3D12_QUERY_DATA_PIPELINE_STATISTICS*>(mappedData) + queryId.Id;

		// Elements are numbered over the enabled bits only, in the same order they were counted in the constructor,
		// so walk the enabled bits until the requested element is reached.
		u32 currentElement = 0;
		if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexCount) && currentElement++ == elementIndex)
			result = statistics->IAVertices;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::PrimitiveCount) && currentElement++ == elementIndex)
			result = statistics->IAPrimitives;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::VertexShaderInvocationCount) && currentElement++ == elementIndex)
			result = statistics->VSInvocations;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::FragmentShaderInvocationCount) && currentElement++ == elementIndex)
			result = statistics->PSInvocations;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ComputeShaderInvocationCount) && currentElement++ == elementIndex)
			result = statistics->CSInvocations;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingInvocationCount) && currentElement++ == elementIndex)
			result = statistics->CInvocations;
		else if (mPipelineStatisticsBits.IsSet(GpuPipelineStatisticsQueryBit::ClippingGeneratedPrimitiveCount) && currentElement++ == elementIndex)
			result = statistics->CPrimitives;
	}
	else
	{
		result = static_cast<const u64*>(mappedData)[queryId.Id];
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
