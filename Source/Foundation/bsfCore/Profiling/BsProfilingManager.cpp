//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/BsProfilingManager.h"
#include "Math/BsMath.h"

namespace bs
{
	const UINT32 ProfilingManager::NUM_SAVED_FRAMES = 200;

	ProfilingManager::ProfilingManager()
	{
		mSavedSimReports = bs_newN<ProfilerReport, ProfilerAlloc>(NUM_SAVED_FRAMES);
		mSavedCoreReports = bs_newN<ProfilerReport, ProfilerAlloc>(NUM_SAVED_FRAMES);
	}

	ProfilingManager::~ProfilingManager()
	{
		if(mSavedSimReports != nullptr)
			bs_deleteN<ProfilerReport, ProfilerAlloc>(mSavedSimReports, NUM_SAVED_FRAMES);

		if(mSavedCoreReports != nullptr)
			bs_deleteN<ProfilerReport, ProfilerAlloc>(mSavedCoreReports, NUM_SAVED_FRAMES);
	}

	void ProfilingManager::_update()
	{
#if BS_PROFILING_ENABLED
		mSavedSimReports[mNextSimReportIdx].cpuReport = gProfilerCPU().generateReport();

		gProfilerCPU().reset();

		mNextSimReportIdx = (mNextSimReportIdx + 1) % NUM_SAVED_FRAMES;
#endif
	}

	void ProfilingManager::_updateCore()
	{
#if BS_PROFILING_ENABLED
		Lock lock(mSync);
		mSavedCoreReports[mNextCoreReportIdx].cpuReport = gProfilerCPU().generateReport();

		gProfilerCPU().reset();

		mNextCoreReportIdx = (mNextCoreReportIdx + 1) % NUM_SAVED_FRAMES;
#endif
	}

	const ProfilerReport& ProfilingManager::getReport(ProfiledThread thread, UINT32 idx) const
	{
		idx = Math::clamp(idx, 0U, (UINT32)(NUM_SAVED_FRAMES - 1));

		if(thread == ProfiledThread::Core)
		{
			Lock lock(mSync);

			UINT32 reportIdx = mNextCoreReportIdx + (UINT32)((INT32)NUM_SAVED_FRAMES - ((INT32)idx + 1));
			reportIdx = (reportIdx) % NUM_SAVED_FRAMES;

			return mSavedCoreReports[reportIdx];
		}
		else
		{
			UINT32 reportIdx = mNextSimReportIdx + (UINT32)((INT32)NUM_SAVED_FRAMES - ((INT32)idx + 1));
			reportIdx = (reportIdx) % NUM_SAVED_FRAMES;

			return mSavedSimReports[reportIdx];
		}
	}

	ProfilingManager& gProfiler()
	{
		return ProfilingManager::instance();
	}
}