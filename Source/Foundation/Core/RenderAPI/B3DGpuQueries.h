//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/** Types of supported GPU queries. */
		enum class GpuQueryType
		{
			Timestamp,
			Occlusion,
			PipelineStatistics
		};

		/** Flags that may be used to control individual queries. */
		enum class GpuQueryFlag
		{
			None,
			PreciseOcclusion		= 1 << 0, /**< If set, occlusion queries will report exact drawn sample count, rather than just a binary value if anything was drawn. */
		};

		using GpuQueryFlags = Flags<GpuQueryFlag>;
		B3D_FLAGS_OPERATORS(GpuQueryFlag)

		/** Types of data that can be captured by the pipeline statistics query. */
		enum class GpuPipelineStatisticsQueryBit
		{
			None									= 0,
			VertexCount								= 1 << 0, /**< Number of vertices processed by input assembly stage. */
			PrimitiveCount							= 1 << 1, /**< Number of primitives processed by input assembly stage. */
			VertexShaderInvocationCount				= 1 << 2, /**< Number of vertex shader invocations. */
			FragmentShaderInvocationCount			= 1 << 3, /**< Number of fragment shader invocations. */
			ComputeShaderInvocationCount			= 1 << 4, /**< Number of compute shader invocations. */
			ClippingInvocationCount					= 1 << 5, /**< Number of primitives that were processed by the clipping stage. */
			ClippingGeneratedPrimitiveCount			= 1 << 6, /**< Number of primitives output by the clipping stage. */
		};

		using GpuPipelineStatisticsQueryBits = Flags<GpuPipelineStatisticsQueryBit>;
		B3D_FLAGS_OPERATORS(GpuPipelineStatisticsQueryBit)

		/** Uniquely identifies a GPU query allocated from a particular pool. */
		struct GpuQueryId
		{
			GpuQueryId() = default;
			GpuQueryId(u32 id)
				: Id(id)
			{ }

			bool IsValid() const { return Id != ~0u; }

			u32 Id = ~0u;
		};

		/** Descriptor structure used for initialization of a GpuQueryPool. */
		struct GpuQueryPoolCreateInformation
		{
			/** Type of queries that the pool will allocate. */
			GpuQueryType Type = GpuQueryType::Timestamp;

			/** Number of queries stored by the pool. This should be the maximum number of queries of this type you plan to allocate. */
			u32 PoolSize = 2048;

			/** Determine which elements of the pipeline statistics query to enable. At least one bit must be set if this is a pipeline statistics query bit. */
			GpuPipelineStatisticsQueryBits PipelineStatisticsQueryBits = GpuPipelineStatisticsQueryBit::None; 
		};

		/** Pool used for allocating GPU queries of a particular type. Note a pool must be reset on a command buffer before first use. */
		class B3D_EXPORT GpuQueryPool
		{
		public:
			GpuQueryPool(const GpuQueryPoolCreateInformation& createInformation);
			virtual ~GpuQueryPool() = default;

			/** Retrieves the type of query managed by the pool. */
			GpuQueryType GetQueryType() const { return mQueryType; }

			/** Returns the maximum number of queries to allocate by this pool. */
			u32 GetPoolSize() const { return mPoolSize; }

			/** Attempts to allocate a new query. Returns an invalid ID if all queries in the pool have been exhausted. */
			virtual GpuQueryId AllocateQuery() = 0;

			/**
			 * Attempts to retrieve the query information from the GPU.
			 *
			 * @param	wait	If true, the caller will block until the information is available on the GPU.
			 * @return			Return true if the query is ready and results can be retrieved, or false if results are not ready yet.
			 */
			virtual bool TryResolve(bool wait = false) = 0;

			/**
			 * Returns the result of the query with the provided index. Caller must ensure that TryEnsure() has been called previously,
			 * and it has returned true.
			 *
			 * @param	queryId			ID of the query to get the result for.
			 * @param	elementIndex	If query has multiple elements, index of the element. Timer and occlusion queries have only one
			 *							element, while pipeline statistic queries have an element for each enabled statistic bit.
			 */
			virtual u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) = 0;

		protected:
			GpuQueryType mQueryType;
			u32 mPoolSize = 0;
			u32 mElementsPerQuery = 1;
		};
		/** @} */
	} // namespace render
} // namespace b3d
