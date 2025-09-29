//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Profiling/B3DRenderStats.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	/** @addtogroup Profiling
	 *  @{
	 */

	/** Contains various profiler statistics about a single GPU profiling sample. */
	struct GPUProfileSample
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

		Vector<GPUProfileSample> ChildSamples;
	};

	/** Contains various profiler statistics for a particular view. */
	struct GPUProfileViewSample : GPUProfileSample
	{
		u64 ViewId;
	};

	/** Profiler report containing information about GPU sampling data from a single frame. */
	struct GPUProfilerReport
	{
		Vector<GPUProfileViewSample> ViewSamples; /**< Profiler samples belonging to a particular view. */
		Vector<GPUProfileSample> UncategorizedSamples; /**< Profiler samples not grouped under a particular view. */
	};

	/**
	 * Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Render thread only except where noted otherwise.
	 */
	class B3D_EXPORT ProfilerGPU : public Module<ProfilerGPU>
	{
	private:
		struct ProfiledSample
		{
			ProfilerString Name;
			RenderStatsData StartStats;
			RenderStatsData EndStats;
			SPtr<render::TimerQuery> ActiveTimeQuery;
			SPtr<render::OcclusionQuery> ActiveOcclusionQuery;

			Vector<ProfiledSample*> Children;
		};

		struct ProfiledViewSample : ProfiledSample
		{
			u64 ViewId;
		};

		struct ProfiledFrame
		{
			Vector<ProfiledViewSample*> ViewSamples;
			Vector<ProfiledSample*> UncategorizedSamples;
		};

	public:
		ProfilerGPU();
		~ProfilerGPU();

		/**
		 * Signals a start of a new frame. Every frame will generate a separate profiling report. This call must be followed
		 * by endFrame(), and any sampling operations must happen between beginFrame() and endFrame().
		 */
		void BeginFrame();

		/**
		 * Signals an end of the currently sampled frame. Results of the sampling will be available once
		 * getNumAvailableReports increments. This may take a while as the sampling is scheduled on the render thread and
		 * on the GPU.
		 *
		 * @param[in]	 discard		If true, the results of the frame will not be resolved and it will be discarded.
		 */
		void EndFrame(bool discard = false);

		/**
		 * Signals that all following sample calls are used for rendering a particular view represented with the provided
		 * id. A top-level timing and occlusion query is issued for the entire view and all following samples will
		 * be grouped under the view in the output report. Must be followed by endView() when done sampling.
		 *
		 * @param	commandBuffer	Command buffer to record the view sample on.
		 * @param	id				Identifier that can be used to uniquely identify the view.
		 * @param	title			Title describing the view.
		 */
		void BeginView(render::GpuCommandBuffer& commandBuffer, u64 id, ProfilerString title);

		/**
		 * Signals the end of rendering for a particular view. Must match the corresponding beginView() call.
		 *
		 * @param	commandBuffer	Command buffer to record the view sample on. Must match the buffer provided in BeginView().
		 */
		void EndView(render::GpuCommandBuffer& commandBuffer);

		/**
		 * Begins sample measurement. Must be followed by endSample().
		 *
		 * @param	commandBuffer	Command buffer to record the sample on.
		 * @param	name			Unique name for the sample you can later use to find the sampling data.
		 *
		 * @note	Must be called between beginFrame()/endFrame() calls.
		 */
		void BeginSample(render::GpuCommandBuffer& commandBuffer, ProfilerString name);

		/**
		 * Ends sample measurement.
		 *
		 * @param	commandBuffer	Command buffer to record the sample on. Must match the buffer provided to BeginSample()
		 * @param	name			Unique name for the sample.
		 *
		 * @note
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name in
		 * BeginSample() would be enough. Must be called between BeginFrame()/EndFrame() calls.
		 */
		void EndSample(render::GpuCommandBuffer& commandBuffer, const ProfilerString& name);

		/**
		 * Returns number of profiling reports that are ready but haven't been retrieved yet.
		 *
		 * @note
		 * There is an internal limit of maximum number of available reports, where oldest ones will get deleted so make
		 * sure to call this often if you don't want to miss some.
		 * @note
		 * Thread safe.
		 */
		u32 GetNumAvailableReports();

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
		/** Assigns start values for the provided sample. */
		void BeginSampleInternal(ProfiledSample& sample, render::GpuCommandBuffer& commandBuffer, bool issueOcclusion);

		/**	Assigns end values for the provided sample. */
		void EndSampleInternal(ProfiledSample& sample, render::GpuCommandBuffer& commandBuffer);

		/**	Creates a new timer query or returns an existing free query. */
		SPtr<render::TimerQuery> GetTimerQuery() const;

		/**	Creates a new occlusion query or returns an existing free query. */
		SPtr<render::OcclusionQuery> GetOcclusionQuery() const;

		/** Frees the memory used by all the child samples. */
		void FreeSample(ProfiledSample& sample);

		/** Frees the memory used by all the samples in the frame. */
		void FreeFrame(ProfiledFrame& frame);

		/** Resolves an active sample and converts it to report sample. */
		void ResolveSample(const ProfiledSample& sample, GPUProfileSample& reportSample);

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

		PoolAlloc<sizeof(ProfiledViewSample), 16> mViewSamplePool;
		PoolAlloc<sizeof(ProfiledSample), 256> mSamplePool;

		mutable Stack<SPtr<render::TimerQuery>> mFreeTimerQueries;
		mutable Stack<SPtr<render::OcclusionQuery>> mFreeOcclusionQueries;

		Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	B3D_EXPORT ProfilerGPU& GetProfilerGPU();

	/** Profiling macros that allow profiling functionality to be disabled at compile time. */
#if B3D_PROFILING_ENABLED
#	define BS_GPU_PROFILE_BEGIN(CommandBuffer, SampleName) GetProfilerGPU().BeginSample(CommandBuffer, SampleName);
#	define BS_GPU_PROFILE_END(CommandBuffer, SampleName) GetProfilerGPU().EndSample(CommandBuffer, SampleName);
#else
#	define BS_GPU_PROFILE_BEGIN(CommandBuffer, SampleName)
#	define BS_GPU_PROFILE_END(CommandBuffer, SampleName)
#endif

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
			mSampleName = std::move(name);
			GetProfilerGPU().BeginSample(commandBuffer, mSampleName);
		}
#else
		ProfileGPUBlock(const ProfilerString& name)
		{}
#endif

#if B3D_PROFILING_ENABLED
		~ProfileGPUBlock()
		{
			GetProfilerGPU().EndSample(mCommandBuffer, mSampleName);
		}
#endif

	private:
#if B3D_PROFILING_ENABLED
		ProfilerString mSampleName;
		render::GpuCommandBuffer& mCommandBuffer;
#endif
	};

	/** @} */
} // namespace b3d
