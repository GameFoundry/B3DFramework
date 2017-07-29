//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRenderStats.h"

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
	};

	/** Profiler report containing information about GPU sampling data from a single frame. */
	struct GPUProfilerReport
	{
		GPUProfileSample frameSample; /**< Sample containing data for entire frame. */
		Vector<GPUProfileSample> samples;
	};

	/**
	 * Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Core thread only except where noted otherwise.
	 */
	class BS_CORE_EXPORT ProfilerGPU : public Module<ProfilerGPU>
	{
	private:
		struct ActiveSample
		{
			ProfilerString sampleName;
			RenderStatsData startStats;
			RenderStatsData endStats;
			SPtr<ct::TimerQuery> activeTimeQuery;
			SPtr<ct::OcclusionQuery> activeOcclusionQuery;
		};

		struct ActiveFrame
		{
			ActiveSample frameSample;
			Vector<ActiveSample> samples;
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
		void beginSample(const ProfilerString& name);

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
		void beginSampleInternal(ActiveSample& sample);

		/**	Assigns end values for the provided sample. */
		void endSampleInternal(ActiveSample& sample);

		/**	Creates a new timer query or returns an existing free query. */
		SPtr<ct::TimerQuery> getTimerQuery() const;

		/**	Creates a new occlusion query or returns an existing free query. */
		SPtr<ct::OcclusionQuery> getOcclusionQuery() const;

		/**
		 * Interprets the active frame results and generates a profiler report for the frame. Provided frame queries must
		 * have finished before calling this.
		 */
		GPUProfilerReport resolveFrame(ActiveFrame& frame);

		/** Resolves an active sample and converts it to report sample. */
		void resolveSample(const ActiveSample& sample, GPUProfileSample& reportSample);

	private:
		ActiveFrame mActiveFrame;
		bool mIsFrameActive;
		Stack<UINT32> mActiveSampleIndexes;

		Queue<ActiveFrame> mUnresolvedFrames;
		GPUProfilerReport* mReadyReports;

		static const UINT32 MAX_QUEUE_ELEMENTS;
		UINT32 mReportHeadPos;
		UINT32 mReportCount;

		mutable Stack<SPtr<ct::TimerQuery>> mFreeTimerQueries;
		mutable Stack<SPtr<ct::OcclusionQuery>> mFreeOcclusionQueries;

		Mutex mMutex;
	};

	/** Provides global access to ProfilerGPU instance. */
	BS_CORE_EXPORT ProfilerGPU& gProfilerGPU();

	/** @} */
}