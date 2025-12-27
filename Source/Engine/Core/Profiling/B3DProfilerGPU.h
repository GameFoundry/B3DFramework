//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Profiling/B3DRenderStats.h"
#include "Allocators/B3DPoolAlloc.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuQueries.h"

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
		/** Information about a single profiling sample. Each sample can have multiple child samples. */
		struct Sample
		{
			ProfilerString Name;

			RenderStatsData BeginRenderStatistics;
			RenderStatsData EndRenderStatistics;

			render::GpuQueryId TimestampBeginQueryId;
			render::GpuQueryId TimestampEndQueryId;
			SPtr<render::GpuQueryPool> TimestampQueryPool;

			TArray<Sample*> Children;
		};

	public:
		/**
		 * Constructs a new command buffer profiler and allocates query pool. Query pool reset is issued on the provided command buffer. Command buffer must not
		 * be in a render pass.
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

		SPtr<render::GpuQueryPool> mTimestampQueryPool;

		TArray<Sample*> mRootSamples;
		TArray<Sample*> mActiveSampleChain;
		PoolAlloc<sizeof(Sample), 256> mSamplePool;
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
		SPtr<GpuCommandBufferProfiler> CreateCommandBufferProfiler(render::GpuCommandBuffer& commandBuffer);

		/**
		 * Notifies the GPU profiler that we're done recording samples into the provided command buffer profiler. The systems
		 * will then internally monitor command buffer completion resolve the profiler results when they are ready.
		 *
		 * @param	name		Name you can use to retrieve the results when ready.
		 * @param	profiler	Profiler holding the samples to resolve.
		 */
		void ResolveProfileWhenReady(const ProfilerString& name, const SPtr<GpuCommandBufferProfiler>& profiler);

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

		/** @} */

	private:
		friend class GpuCommandBufferProfiler;

		/**	Attempts to find an existing free pool, or creates a new one if free one cannot be found. */
		SPtr<render::GpuQueryPool> FindOrCreateQueryPool() const;

		/** Notifies the system that the query pool is no longer used and can be re-used. */
		void ReleaseQueryPool(const SPtr<render::GpuQueryPool>& queryPool);

	private:
		static constexpr u32 kMaxQueuedEntries = 3;

		/** Information about all unresolved command buffer profilers with the same identifier. */
		struct ResolvedCommandBufferProfilerData
		{
			SPtr<GpuCommandBufferProfiler> LastResolved;
		};

		/** Information about all unresolved command buffer profilers with the same identifier. */
		struct UnresolvedCommandBufferProfilerData
		{
			TInlineArray<SPtr<GpuCommandBufferProfiler>, kMaxQueuedEntries> Queued;
		};

		UnorderedMap<ProfilerString, UnresolvedCommandBufferProfilerData> mUnresolvedProfilerData;
		UnorderedMap<ProfilerString, ResolvedCommandBufferProfilerData> mResolvedProfilerData;

		mutable TArray<SPtr<GpuCommandBufferProfiler>> mFreeCommandBufferProfilers;
		mutable TArray<SPtr<render::GpuQueryPool>> mFreeTimestampQueryPools;
		mutable Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	B3D_EXPORT GpuProfiler& GetGpuProfiler();

	/**
	 * Helper class that performs GPU profiling in the current block. Profiling sample is started when the class is
	 * constructed and ended upon destruction.
	 */
	struct ProfileGPUBlock
	{
#if B3D_PROFILING_ENABLED
		ProfileGPUBlock(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
			: mCommandBuffer(commandBuffer)
		{
			const SPtr<GpuCommandBufferProfiler>& commandBufferProfiler = commandBuffer.GetProfiler();

			if(commandBufferProfiler != nullptr)
				commandBufferProfiler->BeginSample(commandBuffer, name);
		}
#else
		ProfileGPUBlock(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
		{}
#endif

#if B3D_PROFILING_ENABLED
		~ProfileGPUBlock()
		{
			const SPtr<GpuCommandBufferProfiler>& commandBufferProfiler = mCommandBuffer.GetProfiler();

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
