//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Profiling/B3DRenderStats.h"
#include "Allocators/B3DPoolAlloc.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuQueries.h"

namespace b3d
{
	/** @addtogroup Profiling
	 *  @{
	 */

	/** Contains various profiler statistics about a single GPU profiling sample. */
	struct GpuProfilerSample // TODO - Rename
	{
		String Name; /**< Name of the sample for easier identification. */
		float TimeMs; /**< Time in milliseconds it took to execute the sampled block. */

		u32 DrawCallCount; /**< Number of draw calls that happened. */
		u32 RenderTargetChangesCount; /**< How many times was render target changed. */
		u32 PresentCount; /**< How many times did a buffer swap happen on a double buffered render target. */
		u32 ClearCount; /**< How many times was render target cleared. */

		u32 VerticesDrawn; /**< Total number of vertices sent to the GPU. */
		u32 PrimitivesDrawn; /**< Total number of primitives sent to the GPU. */
		u32 SamplesDrawn; /**< Number of samples drawn by the GPU. */

		u32 PipelineStateChangeCount; /**< How many times did the pipeline state change. */

		u32 GpuParameterBindCount; /**< How many times were GPU parameters bound. */
		u32 VertexBufferBindCount; /**< How many times was a vertex buffer bound. */
		u32 IndexBufferBindCount; /**< How many times was an index buffer bound. */

		u32 ResourceWriteCount; /**< How many times were GPU resources written to. */
		u32 ResourceReadCount; /**< How many times were GPU resources read from. */

		u32 ObjectsCreatedCount; /**< How many GPU objects were created. */
		u32 ObjectsDestroyedCount; /**< How many GPU objects were destroyed. */

		TArray<GpuProfilerSample> ChildSamples;
	};

	/** Contains resolved samples from a GPU profiling operation. */
	struct GpuProfilerResults
	{
		TArray<GpuProfilerSample> Samples;
	};

	/** Allows you to record timing and statistics for GPU command execution on a GPU command buffer. */
	class B3D_EXPORT GpuCommandBufferProfiler
	{
	private:
		/** Timestamp query written by a sample, and the index of the profiler's query pool it was allocated from. */
		struct Timestamp
		{
			u32 PoolIndex = 0;
			render::GpuQueryId QueryId;
		};

		/** Information about a single profiling sample. Each sample can have multiple child samples. */
		struct Sample
		{
			ProfilerString Name;

			RenderStatsData BeginRenderStatistics;
			RenderStatsData EndRenderStatistics;

			Timestamp BeginTimestamp;
			Timestamp EndTimestamp;

			TArray<Sample*> Children;
		};

	public:
		/**
		 * Constructs a new command buffer profiler and allocates its first query pool. Query pool reset is issued on the provided command buffer. Command
		 * buffer must not be in a render pass.
		 *
		 * Once a query pool runs low, the profiler continues in a new one. Query pools can only be reset outside of a render pass, so the profiler
		 * switches while outside of one, early enough to leave kRenderPassQueryCount queries for the next render pass. Samples that start
		 * within a render pass after the pool ran out are skipped.
		 */
		GpuCommandBufferProfiler(render::GpuCommandBuffer& commandBuffer);
		~GpuCommandBufferProfiler();

		/**
		 * Begins sample measurement. Must be followed by EndSample(). If command buffer is currently within a render pass, EndSample()
		 * must also be issued within a render pass. If command buffer is currently outside of a render pass, EndSample() must be issued
		 * outside of a render pass.
		 *
		 * @param	commandBuffer	Command buffer to record the sample on, must be the same as the profiler was created for.
		 * @param	name			Unique name for the sample you can later use to find the sampling data.
		 */
		void BeginSample(render::GpuCommandBuffer& commandBuffer, ProfilerString name);

		/**
		 * Ends sample measurement that started in BeginSample().
		 *
		 * @param	commandBuffer	Command buffer to record the sample on, must be the same as the profiler was created for.
		 */
		void EndSample(render::GpuCommandBuffer& commandBuffer);

		/** Returns true if the profiler doesn't have any samples. */
		bool IsEmpty() const { return mRootSamples.Empty(); }
	private:
		friend class GpuProfiler;

		/** Clears all the internal data. */
		void Clear();

		/** Resets the object so it may be re-used. */
		void Reset(render::GpuCommandBuffer& commandBuffer);

		/** Converts a command buffer profiler sample and converts it to a result sample. Caller must ensure that query pool has resolved the queries before calling. */
		void ConvertToResultSample(const Sample& sample, GpuProfilerSample& reportSample);

		/** Converts all command buffer profiler samples into report samples. Caller must ensure that query pool has resolved the queries before calling. */
		GpuProfilerResults GetResults();

		/**
		 * Writes a timestamp on @p commandBuffer. Outside of a render pass it continues in a new query pool if fewer than kRenderPassQueryCount
		 * queries are left in the current one. Returns false if no query could be allocated.
		 */
		bool WriteTimestamp(render::GpuCommandBuffer& commandBuffer, Timestamp& outTimestamp);

		/** Attempts to resolve the queries of every query pool. Returns true if all are resolved, or there are none. */
		bool TryResolve();

		/** Number of queries left free in a query pool when a render pass starts, for the samples within it. */
		static constexpr u32 kRenderPassQueryCount = 32;

		/** Query pools in the order they were used. Empty if the device does not support timer queries. */
		TInlineArray<TShared<render::GpuQueryPool>, 1> mTimestampQueryPools;
		u32 mAllocatedQueryCount = 0; /**< Number of queries allocated from the last of mTimestampQueryPools. */

		TArray<Sample*> mRootSamples;
		TArray<Sample*> mActiveSampleChain;
		PoolAlloc<sizeof(Sample), 256> mSamplePool;
		u32 mSkippedSampleDepth = 0;
		u64 mCommandBufferId = 0;
	};

	/**
	 * Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuProfiler : public Module<GpuProfiler>
	{
	public:
		~GpuProfiler();

		/**
		 * Creates a profiler that can be used for profiling commands on the provided command buffer. Query pool reset
		 * command will be issued on the provided command buffer. Command buffer cannot be in render pass.
		 */
		TShared<GpuCommandBufferProfiler> CreateCommandBufferProfiler(render::GpuCommandBuffer& commandBuffer);

		/**
		 * Notifies the GPU profiler that we're done recording samples into the provided command buffer profiler. The systems
		 * will then internally monitor command buffer completion resolve the profiler results when they are ready.
		 *
		 * @param	name		Name you can use to retrieve the results when ready.
		 * @param	profiler	Profiler holding the samples to resolve.
		 */
		void ResolveProfileWhenReady(const ProfilerString& name, const TShared<GpuCommandBufferProfiler>& profiler);

		/**
		 * Returns latest profiling results, if available. Profiling results are consumed once retrieved and
		 * cannot be retrieved again.
		 *
		 * @param	name		Name given to the samples in call to ResolveProfileWhenReady.
		 * @return				Set of resolved root samples, or null if no results are available.
		 */
		TOptional<GpuProfilerResults> GetResults(const ProfilerString& name);

	public:
		// ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * To be called once per frame from the render thread.
		 */
		void Update();

		/**
		 * Releases all GPU resources held by the profiler. Must be called on the render thread before shutdown.
		 */
		void Clear();

		/** @} */

	private:
		friend class GpuCommandBufferProfiler;

		/**
		 * Attempts to find an existing free pool, recycling discarded profilers if there is none, or creates a new one if a free one
		 * cannot be found. mMutex must be locked.
		 */
		TShared<render::GpuQueryPool> FindOrCreateQueryPool();

		/** Notifies the system that the query pool is no longer used and can be re-used. */
		void ReleaseQueryPool(const TShared<render::GpuQueryPool>& queryPool);

		/** Makes the discarded profilers whose queries the GPU finished writing available for reuse. */
		void RecycleDiscardedProfilers();

	private:
		static constexpr u32 kMaxQueuedEntries = 3;

		/** Number of queries in each query pool. A command buffer profiler continues in another pool once its current one runs out. */
		static constexpr u32 kQueryPoolSize = 128;

		/** Information about all unresolved command buffer profilers with the same identifier. */
		struct ResolvedCommandBufferProfilerData
		{
			TShared<GpuCommandBufferProfiler> LastResolved;
		};

		/** Information about all unresolved command buffer profilers with the same identifier. */
		struct UnresolvedCommandBufferProfilerData
		{
			TInlineArray<TShared<GpuCommandBufferProfiler>, kMaxQueuedEntries> Queued;
		};

		/**
		 * Resolves the queued profilers of @p name whose queries the GPU finished writing. The latest resolved profiler becomes the
		 * name's results, and the profilers queued before it are discarded. mMutex must be locked.
		 */
		void ResolveQueuedProfilers(const ProfilerString& name, UnresolvedCommandBufferProfilerData& profilerData);

		UnorderedMap<ProfilerString, UnresolvedCommandBufferProfilerData> mUnresolvedProfilerData;
		UnorderedMap<ProfilerString, ResolvedCommandBufferProfilerData> mResolvedProfilerData;

		/** Profilers whose results were superseded before they resolved. Their query pools are reused once the GPU finished writing them. */
		TArray<TShared<GpuCommandBufferProfiler>> mDiscardedProfilers;

		mutable TArray<TShared<GpuCommandBufferProfiler>> mFreeCommandBufferProfilers;
		mutable TArray<TShared<render::GpuQueryPool>> mFreeTimestampQueryPools;
		mutable Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	B3D_EXPORT GpuProfiler& GetGpuProfiler();

	/**
	 * Measures a GPU profiler sample on a command buffer from its construction to its destruction, if the command buffer is being
	 * profiled (see GpuCommandBuffer::BeginProfiling()).
	 */
	struct GpuProfilingScope
	{
#if B3D_PROFILING_ENABLED
		GpuProfilingScope(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
			: mCommandBuffer(commandBuffer)
		{
			const TShared<GpuCommandBufferProfiler>& commandBufferProfiler = commandBuffer.GetProfiler();

			if(commandBufferProfiler != nullptr)
				commandBufferProfiler->BeginSample(commandBuffer, name);
		}
#else
		GpuProfilingScope(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
		{}
#endif

#if B3D_PROFILING_ENABLED
		~GpuProfilingScope()
		{
			const TShared<GpuCommandBufferProfiler>& commandBufferProfiler = mCommandBuffer.GetProfiler();

			if(commandBufferProfiler != nullptr)
				commandBufferProfiler->EndSample(mCommandBuffer);
		}
#endif

	private:
#if B3D_PROFILING_ENABLED
		render::GpuCommandBuffer& mCommandBuffer;
#endif
	};

	/** @} */
} // namespace b3d
