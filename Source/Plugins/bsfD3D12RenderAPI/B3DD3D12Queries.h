//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DEventQuery.h"
#include "RenderAPI/B3DTimerQuery.h"
#include "RenderAPI/B3DOcclusionQuery.h"
#include "RenderAPI/B3DGpuQueries.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
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

		/** DirectX 12 implementation of an event query. */
		class D3D12EventQuery : public EventQuery
		{
		public:
			D3D12EventQuery(GpuDevice& device);
			~D3D12EventQuery() override;

			/** @copydoc EventQuery::Begin */
			void Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer) override;

			/** @copydoc EventQuery::IsReady */
			bool IsReady() const override;

		private:
			ComPtr<ID3D12Fence> mFence;
			u64 mFenceValue = 0;
		};

		/** DirectX 12 implementation of a timer query. */
		class D3D12TimerQuery : public TimerQuery
		{
		public:
			D3D12TimerQuery(GpuDevice& device);
			~D3D12TimerQuery() override;

			/** @copydoc TimerQuery::Begin */
			void Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer) override;

			/** @copydoc TimerQuery::End */
			void End(const SPtr<render::GpuCommandBuffer>& commandBuffer) override;

			/** @copydoc TimerQuery::IsReady */
			bool IsReady() const override;

			/** @copydoc TimerQuery::GetTimeInMilliseconds */
			float GetTimeInMilliseconds() override;

		private:
			// TODO: Implement using query pools
			bool mIsReady = false;
		};

		/** DirectX 12 implementation of an occlusion query. */
		class D3D12OcclusionQuery : public OcclusionQuery
		{
		public:
			D3D12OcclusionQuery(bool isBinary, GpuDevice& device);
			~D3D12OcclusionQuery() override;

			/** @copydoc OcclusionQuery::Begin */
			void Begin(const SPtr<render::GpuCommandBuffer>& commandBuffer) override;

			/** @copydoc OcclusionQuery::End */
			void End(const SPtr<render::GpuCommandBuffer>& commandBuffer) override;

			/** @copydoc OcclusionQuery::IsReady */
			bool IsReady() const override;

			/** @copydoc OcclusionQuery::GetNumSamples */
			u32 GetNumSamples() override;

		private:
			// TODO: Implement using query pools
			bool mIsBinary;
			bool mIsReady = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
