//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Profiling/BsProfilerCPU.h"

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
		ProfilerReport* mSavedSimReports = nullptr;
		UINT32 mNextSimReportIdx = 0;

		ProfilerReport* mSavedCoreReports = nullptr;
		UINT32 mNextCoreReportIdx = 0;

		mutable Mutex mSync;
	};

	/** Easy way to access ProfilingManager. */
	BS_CORE_EXPORT ProfilingManager& gProfiler();

	/** @} */
}