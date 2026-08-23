//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Queries.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuQueryPool::D3D12GpuQueryPool(D3D12GpuDevice& device, const GpuQueryPoolCreateInformation& createInformation) : GpuQueryPool(createInformation), mDevice(device), mPipelineStatisticsBits(createInformation.PipelineStatisticsQueryBits)
{
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
	if (mResolveConnection)
		mResolveConnection.Disconnect();

	mQueryHeap.Reset();
	mReadbackBuffer.Reset();
	mDevice.FreeMemory(mReadbackAllocation);
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

	const u64 bufferSize = mQueryType == GpuQueryType::PipelineStatistics ? sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) * mPoolSize : sizeof(u64) * mPoolSize;

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

	hr = mDevice.CreateResource(resourceDesc, D3D12_HEAP_TYPE_READBACK, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, mReadbackBuffer, mReadbackAllocation);
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

void D3D12GpuQueryPool::SelectOcclusionQueryType(GpuQueryFlags flags)
{
	if (mQueryType != GpuQueryType::Occlusion)
		return;

	const D3D12_QUERY_TYPE requestedType = flags.IsSet(GpuQueryFlag::PreciseOcclusion) ? D3D12_QUERY_TYPE_OCCLUSION : D3D12_QUERY_TYPE_BINARY_OCCLUSION;

	if (mIsOcclusionQueryTypeSelected)
	{
		if (mD3D12QueryType != requestedType)
			B3D_LOG(Error, LogRenderBackend, "Occlusion queries from a single pool must all agree on GpuQueryFlag::PreciseOcclusion, as D3D12 resolves the entire pool with one query type. Keeping the type the first query selected.");

		return;
	}

	mD3D12QueryType = requestedType;
	mIsOcclusionQueryTypeSelected = true;
}

void D3D12GpuQueryPool::Reset()
{
	if (mResolveConnection)
		mResolveConnection.Disconnect();

	mNextQueryId = 0;
	mIsOcclusionQueryTypeSelected = false;
	mResolved.store(false, std::memory_order_relaxed);
}

void D3D12GpuQueryPool::NotifyResolveScheduled(GpuCommandBuffer& commandBuffer)
{
	if (mResolveConnection)
		mResolveConnection.Disconnect();

	mResolveConnection = commandBuffer.OnDidComplete.Connect([this]()
	{
		mResolved.store(true, std::memory_order_release);
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

	// TODO - Probably not ideal mapping/unampping for every query & element, but we'll fix if it comes up in the profiler
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

	mCompleteConnection = commandBuffer.OnDidComplete.Connect([this]()
	{
		mReady.store(true, std::memory_order_release);
	});
}

bool D3D12EventQuery::IsReady() const
{
	return mReady.load(std::memory_order_acquire);
}
