//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DEventQuery.h"
#include "GpuBackend/B3DGpuQueries.h"

#include <atomic>

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of a GPU query pool. */
		class D3D12GpuQueryPool : public GpuQueryPool
		{
		public:
			D3D12GpuQueryPool(D3D12GpuDevice& device, const GpuQueryPoolCreateInformation& createInformation);
			~D3D12GpuQueryPool() override;

			GpuQueryId AllocateQuery() override;
			bool TryResolve(bool wait = false) override;
			u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) override;

			/** Returns the D3D12 query heap. */
			ID3D12QueryHeap* GetD3D12QueryHeap() const { return mQueryHeap.Get(); }

			/** Returns the D3D12 query type. */
			D3D12_QUERY_TYPE GetD3D12QueryType() const { return mD3D12QueryType; }

			/** Returns the readback buffer used for query results. */
			ID3D12Resource* GetReadbackBuffer() const { return mReadbackBuffer.Get(); }

			/** Returns the number of elements per query (1 for timer/occlusion, multiple for pipeline statistics). */
			u32 GetElementsPerQuery() const { return mElementsPerQuery; }

			/** Returns the number of allocated queries. */
			u32 GetAllocatedQueryCount() const { return mNextQueryId; }

		private:
			/** Creates the query heap and readback buffer. */
			void CreateQueryHeap();

			D3D12GpuDevice& mDevice;
			ComPtr<ID3D12QueryHeap> mQueryHeap;
			ComPtr<ID3D12Resource> mReadbackBuffer;
			D3D12MA::Allocation* mReadbackAllocation = nullptr;
			D3D12_QUERY_TYPE mD3D12QueryType;
			D3D12_QUERY_HEAP_TYPE mD3D12QueryHeapType;
			GpuPipelineStatisticsQueryBits mPipelineStatsBits;

			u32 mNextQueryId = 0;
			bool mResolved = false;
		};

		/**
		 * DirectX 12 implementation of an event query. Readiness is derived from the command buffer's OnDidComplete
		 * event: once the command buffer the query was scheduled on finishes executing on the GPU, an atomic ready
		 * flag is flipped and IsReady() begins returning true.
		 *
		 * @note	The event fires on the command buffer's owning thread. IsReady() may be polled from another thread,
		 *			hence the atomic ready flag.
		 */
		class D3D12EventQuery : public EventQuery
		{
		public:
			D3D12EventQuery(D3D12GpuDevice& device);
			~D3D12EventQuery() override;

			/** @copydoc EventQuery::Begin */
			void Begin(GpuCommandBuffer& commandBuffer) override;

			/** @copydoc EventQuery::IsReady */
			bool IsReady() const override;

		private:
			D3D12GpuDevice& mDevice;

			/** Set to true once the command buffer this query was scheduled on completes on the GPU. */
			std::atomic<bool> mReady{ false };

			/** Connection to the command buffer's OnDidComplete event. Disconnected on re-Begin and on destruction. */
			HEvent mCompleteConnection;
		};

		/** @} */
	} // namespace render
} // namespace b3d
