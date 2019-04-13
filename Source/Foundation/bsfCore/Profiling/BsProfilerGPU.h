//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Profiling/BsRenderStats.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs
{
	/** @addtogroup Profiling
	 *  @{
	 */

	/** Contains various profiler statistics about a single GPU profiling sample. */
	struct GPUProfileSample
	{
		String name; /**< Name of the sample for easier identification. */
		float timeMs; /**< Time in milliseconds it took to execute the sampled block. */

		UINT32 numDrawCalls; /**< Number of draw calls that happened. */
		UINT32 numRenderTargetChanges; /**< How many times was render target changed. */
		UINT32 numPresents; /**< How many times did a buffer swap happen on a double buffered render target. */
		UINT32 numClears; /**< How many times was render target cleared. */

		UINT32 numVertices; /**< Total number of vertices sent to the GPU. */
		UINT32 numPrimitives; /**< Total number of primitives sent to the GPU. */
		UINT32 numDrawnSamples; /**< Number of samples drawn by the GPU. */

		UINT32 numPipelineStateChanges; /**< How many times did the pipeline state change. */

		UINT32 numGpuParamBinds; /**< How many times were GPU parameters bound. */
		UINT32 numVertexBufferBinds; /**< How many times was a vertex buffer bound. */
		UINT32 numIndexBufferBinds; /**< How many times was an index buffer bound. */

		UINT32 numResourceWrites; /**< How many times were GPU resources written to. */
		UINT32 numResourceReads; /**< How many times were GPU resources read from. */

		UINT32 numObjectsCreated; /**< How many GPU objects were created. */
		UINT32 numObjectsDestroyed; /**< How many GPU objects were destroyed. */

		Vector<GPUProfileSample> children;
	};

	/** Profiler report containing information about GPU sampling data from a single frame. */
	struct GPUProfilerReport
	{
		GPUProfileSample frameSample; /**< Sample containing data for entire frame. */
	};

	/**
	 * Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Core thread only except where noted otherwise.
	 */
	class BS_CORE_EXPORT ProfilerGPU : public Module<ProfilerGPU>
	{
	private:
		struct ProfiledSample
		{
			ProfilerString name;
			RenderStatsData startStats;
			RenderStatsData endStats;
			SPtr<ct::TimerQuery> activeTimeQuery;
			SPtr<ct::OcclusionQuery> activeOcclusionQuery;

			Vector<ProfiledSample*> children;
		};

	public:
		ProfilerGPU();
		~ProfilerGPU();

		/**
		 * Signals a start of a new frame. Every frame will generate a separate profiling report. This call must be followed
		 * by endFrame(), and any sampling operations must happen between beginFrame() and endFrame().
		 */
		void beginFrame();

		/**
		 * Signals an end of the currently sampled frame. Results of the sampling will be available once
		 * getNumAvailableReports increments. This may take a while as the sampling is scheduled on the core thread and
		 * on the GPU.
		 */
		void endFrame();

		/**
		 * Begins sample measurement. Must be followed by endSample().
		 *
		 * @param[in]	name	Unique name for the sample you can later use to find the sampling data.
		 *
		 * @note	Must be called between beginFrame()/endFrame() calls.
		 */
		void beginSample(ProfilerString name);

		/**
		 * Ends sample measurement.
		 *
		 * @param[in]	name	Unique name for the sample.
		 *
		 * @note
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name in
		 * beginSample() would be enough. Must be called between beginFrame()/endFrame() calls.
		 */
		void endSample(const ProfilerString& name);

		/**
		 * Returns number of profiling reports that are ready but haven't been retrieved yet.
		 *
		 * @note
		 * There is an internal limit of maximum number of available reports, where oldest ones will get deleted so make
		 * sure to call this often if you don't want to miss some.
		 * @note
		 * Thread safe.
		 */
		UINT32 getNumAvailableReports();

		/**
		 * Gets the oldest report available and removes it from the internal list. Throws an exception if no reports are
		 * available.
		 *
		 * @note	Thread safe.
		 */
		GPUProfilerReport getNextReport();

	public: 
		// ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * To be called once per frame from the Core thread.
		 */
		void _update();

		/** @} */

	private:
		/** Assigns start values for the provided sample. */
		void beginSampleInternal(ProfiledSample& sample, bool issueOcclusion);

		/**	Assigns end values for the provided sample. */
		void endSampleInternal(ProfiledSample& sample);

		/**	Creates a new timer query or returns an existing free query. */
		SPtr<ct::TimerQuery> getTimerQuery() const;

		/**	Creates a new occlusion query or returns an existing free query. */
		SPtr<ct::OcclusionQuery> getOcclusionQuery() const;

		/** Frees the memory used by all the child samples. */
		void freeSample(ProfiledSample& sample);

		/** Resolves an active sample and converts it to report sample. */
		void resolveSample(const ProfiledSample& sample, GPUProfileSample& reportSample);

	private:
		ProfiledSample mFrameSample;
		bool mIsFrameActive = false;
		Stack<ProfiledSample*> mActiveSamples;

		Queue<ProfiledSample> mUnresolvedFrames;
		GPUProfilerReport* mReadyReports = nullptr;

		static const UINT32 MAX_QUEUE_ELEMENTS;
		UINT32 mReportHeadPos = 0;
		UINT32 mReportCount = 0;

		PoolAlloc<sizeof(ProfiledSample), 256> mSamplePool;

		mutable Stack<SPtr<ct::TimerQuery>> mFreeTimerQueries;
		mutable Stack<SPtr<ct::OcclusionQuery>> mFreeOcclusionQueries;

		Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	BS_CORE_EXPORT ProfilerGPU& gProfilerGPU();

	/** Profiling macros that allow profiling functionality to be disabled at compile time. */
#if BS_PROFILING_ENABLED
	#define BS_GPU_PROFILE_BEGIN(name) gProfilerGPU().beginSample(name);
	#define BS_GPU_PROFILE_END(name) gProfilerGPU().endSample(name);
#else
	#define BS_GPU_PROFILE_BEGIN(name)
	#define BS_GPU_PROFILE_END(name)
#endif

	/** 
	 * Helper class that performs GPU profiling in the current block. Profiling sample is started when the class is 
	 * constructed and ended upon destruction. 
	 */
	struct ProfileGPUBlock
	{
#if BS_PROFILING_ENABLED
		ProfileGPUBlock(ProfilerString name)
		{
			mSampleName = std::move(name);
			gProfilerGPU().beginSample(mSampleName);
		}
#else
		ProfileGPUBlock(const ProfilerString& name)
		{ }
#endif

#if BS_PROFILING_ENABLED
		~ProfileGPUBlock()
		{
			gProfilerGPU().endSample(mSampleName);
		}
#endif

	private:
#if BS_PROFILING_ENABLED
		ProfilerString mSampleName;
#endif
	};

	/** @} */
}