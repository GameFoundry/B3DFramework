//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/BsProfilingManager.h"
#include "Math/BsMath.h"

using namespace bs;

const u32 ProfilingManager::NUM_SAVED_FRAMES = 200;

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

void ProfilingManager::UpdateInternal()
{
#if BS_PROFILING_ENABLED
	mSavedSimReports[mNextSimReportIdx].CpuReport = gProfilerCPU().GenerateReport();

	gProfilerCPU().Reset();

	mNextSimReportIdx = (mNextSimReportIdx + 1) % NUM_SAVED_FRAMES;
#endif
}

void ProfilingManager::UpdateCoreInternal()
{
#if BS_PROFILING_ENABLED
	Lock lock(mSync);
	mSavedCoreReports[mNextCoreReportIdx].CpuReport = gProfilerCPU().GenerateReport();

	gProfilerCPU().Reset();

	mNextCoreReportIdx = (mNextCoreReportIdx + 1) % NUM_SAVED_FRAMES;
#endif
}

const ProfilerReport& ProfilingManager::GetReport(ProfiledThread thread, u32 idx) const
{
	idx = Math::Clamp(idx, 0U, (u32)(NUM_SAVED_FRAMES - 1));

	if(thread == ProfiledThread::Core)
	{
		Lock lock(mSync);

		u32 reportIdx = mNextCoreReportIdx + (u32)((i32)NUM_SAVED_FRAMES - ((i32)idx + 1));
		reportIdx = (reportIdx) % NUM_SAVED_FRAMES;

		return mSavedCoreReports[reportIdx];
	}
	else
	{
		u32 reportIdx = mNextSimReportIdx + (u32)((i32)NUM_SAVED_FRAMES - ((i32)idx + 1));
		reportIdx = (reportIdx) % NUM_SAVED_FRAMES;

		return mSavedSimReports[reportIdx];
	}
}

namespace bs
{
ProfilingManager& gProfiler()
{
	return ProfilingManager::Instance();
}
} // namespace bs
