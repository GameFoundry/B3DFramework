//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Profiling/B3DProfilerCPU.h"

namespace b3d
{
	/** @addtogroup Profiling
	 *  @{
	 */

	/**	Contains data about a profiling session. */
	struct ProfilerReport
	{
		CPUProfilerReport CpuReport;
	};

	/**	Type of thread used by the profiler. */
	enum class ProfiledThread
	{
		Main,
		Render
	};

	/**
	 * Tracks CPU profiling information with each frame for main and render threads.
	 *
	 * @note	Main thread only unless specified otherwise.
	 */
	class B3D_EXPORT ProfilingManager : public Module<ProfilingManager>
	{
	public:
		ProfilingManager();
		~ProfilingManager();

		/** Called every frame. */
		void Update();

		/**
		 * Called every frame from the render thread.
		 *
		 * @note	Render thread only.
		 */
		void UpdateRenderThread();

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
		const ProfilerReport& GetReport(ProfiledThread thread, u32 idx = 0) const;

	private:
		static const u32 kNumSavedFrames;
		ProfilerReport* mSavedMainThreadReports = nullptr;
		u32 mNextSimulationReportIndex = 0;

		ProfilerReport* mSavedRenderThreadReports = nullptr;
		u32 mNextRenderThreadReportIndex = 0;

		mutable Mutex mSync;
	};

	/** Easy way to access ProfilingManager. */
	B3D_EXPORT ProfilingManager& GetProfiler();

	/** @} */
} // namespace b3d
