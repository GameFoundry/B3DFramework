//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsProfilerCPU.h"

namespace bs
{
	/** @addtogroup Profiling-Internal
	 *  @{
	 */

	/**	Contains data about a profiling session. */
	struct ProfilerReport
	{
		CPUProfilerReport cpuReport;
	};

	/**	Type of thread used by the profiler. */
	enum class ProfiledThread
	{
		Sim,
		Core
	};

	/**
	 * Tracks CPU profiling information with each frame for sim and core threads.
	 *
	 * @note	Sim thread only unless specified otherwise.
	 */
	class BS_CORE_EXPORT ProfilingManager : public Module<ProfilingManager>
	{
	public:
		ProfilingManager();
		~ProfilingManager();

		/** Called every frame. */
		void _update();

		/**
		 * Called every frame from the core thread.
		 * 			
		 * @note	Core thread only.
		 */
		void _updateCore();

		/**
		 * Returns a profiler report for the specified frame, for the specified thread.
		 *
		 * @param[in]	thread	Thread for which to retrieve the profiler report.
		 * @param[in]	idx		Profiler report index, ranging [0, NUM_SAVED_FRAMES]. 0 always returns the latest  report. 
		 *						Increasing indexes return reports for older and older frames. Out of range  indexes will be
		 *						clamped.
		 *
		 * @note	
		 * Profiler reports get updated every frame. Oldest reports that no longer fit in the saved reports buffer are 
		 * discarded.
		 */
		const ProfilerReport& getReport(ProfiledThread thread, UINT32 idx = 0) const;

	private:
		static const UINT32 NUM_SAVED_FRAMES;
		ProfilerReport* mSavedSimReports;
		UINT32 mNextSimReportIdx;

		ProfilerReport* mSavedCoreReports;
		UINT32 mNextCoreReportIdx;

		mutable Mutex mSync;
	};

	/** Easy way to access ProfilingManager. */
	BS_CORE_EXPORT ProfilingManager& gProfiler();

	/** @} */
}