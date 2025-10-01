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

		Vector<GpuProfilerSample> ChildSamples;
	};

	/** Profiler report containing information about GPU sampling data from a single frame. */
	struct GPUProfilerReport // TODO - Rename
	{
		Vector<GPUProfileViewSample> ViewSamples; /**< Profiler samples belonging to a particular view. */
		Vector<GpuProfilerSample> UncategorizedSamples; /**< Profiler samples not grouped under a particular view. */
	};

	// TODO - Doc
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
		~GpuCommandBufferProfiler() = default;

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

	private:
		friend class ProfilerGPU;

		/** Resets the object so it may be re-used. */
		void Reset();

		SPtr<render::GpuQueryPool> mTimestampQueryPool;

		TArray<Sample*> mRootSamples;
		TArray<Sample*> mActiveSampleChain;
		PoolAlloc<sizeof(Sample), 256> mSamplePool; // TODO - Grab this pool from the GPUProfiler, instead of allocating it for each COmmandBufferProfiler (in a thread safe way)
		u64 mCommandBufferId = 0;
	};

	/**
	 * Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Render thread only except where noted otherwise.
	 */
	class B3D_EXPORT ProfilerGPU : public Module<ProfilerGPU> // TODO - Rename
	{
	public:
		ProfilerGPU();
		~ProfilerGPU();

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
		Optional<TArray<GpuProfilerSample>> GetResults(const ProfilerString& name);

		/**
		 * Returns number of profiling reports that are ready but haven't been retrieved yet.
		 *
		 * @note
		 * There is an internal limit of maximum number of available reports, where oldest ones will get deleted so make
		 * sure to call this often if you don't want to miss some.
		 * @note
		 * Thread safe.
		 */
		u32 GetAvailableReportCount();

		/**
		 * Gets the oldest report available and removes it from the internal list. Throws an exception if no reports are
		 * available.
		 *
		 * @note	Thread safe.
		 */
		GPUProfilerReport GetNextReport();

	public:
		// ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * To be called once per frame from the render thread.
		 */
		void UpdateInternal();

		/** @} */

	private:
		friend class GpuCommandBufferProfiler;

		/**	Attempts to find an existing free pool, or creates a new one if free one cannot be found. */
		SPtr<render::GpuQueryPool> FindOrCreateQueryPool() const;

		/** Notifies the system that the query pool is no longer used and can be re-used. */
		void ReleaseQueryPool(const SPtr<render::GpuQueryPool>& queryPool);

		/** Frees the memory used by all the child samples. */
		void FreeSample(ProfiledSample& sample);

		/** Frees the memory used by all the samples in the frame. */
		void FreeFrame(ProfiledFrame& frame);

		/** Resolves an active sample and converts it to report sample. */
		void ResolveSample(const ProfiledSample& sample, GpuProfilerSample& reportSample);

	private:
		bool mIsFrameActive = false;
		bool mIsViewActive = false;
		Stack<ProfiledSample*> mActiveSamples;
		ProfiledFrame mActiveFrame;

		Queue<ProfiledFrame> mUnresolvedFrames;
		GPUProfilerReport* mReadyReports = nullptr;

		static const u32 kMaxQueueElements;
		u32 mReportHeadPos = 0;
		u32 mReportCount = 0;

		PoolAlloc<sizeof(ProfiledScope), 16> mViewSamplePool;
		PoolAlloc<sizeof(ProfiledSample), 256> mSamplePool;

		mutable TArray<SPtr<render::GpuQueryPool>> mFreeTimestampQueryPools;
		mutable Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	B3D_EXPORT ProfilerGPU& GetProfilerGPU();

	/**
	 * Helper class that performs GPU profiling in the current block. Profiling sample is started when the class is
	 * constructed and ended upon destruction.
	 */
	struct ProfileGPUBlock
	{
#if B3D_PROFILING_ENABLED
		ProfileGPUBlock(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
			:mCommandBuffer(commandBuffer)
		{
			const SPtr<GpuCommandBufferProfiler>& commandBufferProfiler = commandBuffer.GetProfiler();
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
